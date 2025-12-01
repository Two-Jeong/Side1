using System.Collections.Concurrent;
using System.Net.Sockets;

namespace CSharp_NetworkClient;

public class MultiSender
{
    const int MAX_SEND_COUNT = 500;
    
    private Session m_session;
    private SocketAsyncEventArgs m_send_args;
    private ConcurrentQueue<Packet> m_send_register_queue;
    
    private int m_is_sending;
    
    void Init(Session session)
    {
        m_session = session;
        
        m_send_args = new SocketAsyncEventArgs();
        m_send_args.Completed += CompleteSend;
    }

    void RegisterSend(Packet packet)
    {
        int result = Interlocked.CompareExchange(ref m_is_sending, 1, 0);

        if (0 == result)
            ProcessSend();    
        else
            m_send_register_queue.Enqueue(packet);
        
    }

    void ProcessSend()
    {
        List<ArraySegment<byte>> register_array = new List<ArraySegment<byte>>();
        int send_packet_counter = 0;
        while (false == m_send_register_queue.IsEmpty)
        {
            if (MAX_SEND_COUNT <= send_packet_counter)
                break;
            
            Packet packet;
            if (false == m_send_register_queue.TryDequeue(out packet))
                continue;
            
            register_array.Add(packet.Data.ToArray());
            send_packet_counter += 1;
        }

        m_send_args.BufferList = register_array;
        bool is_pending = m_session.Socket.SendAsync(m_send_args);

        if (false == is_pending)
            CompleteSend(null, m_send_args);
        
    }

    void CompleteSend(object sender, SocketAsyncEventArgs args)
    {
        args.BufferList = null;
        m_session.OnSend();
        
        if(false == m_send_register_queue.IsEmpty)
            ProcessSend();
        else
        Interlocked.Exchange(ref m_is_sending, 0);
    }
}
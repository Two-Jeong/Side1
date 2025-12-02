using System.Net.Sockets;
using System.Reflection.Metadata.Ecma335;

namespace CSharp_NetworkClient;

public class Receiver
{
    private Session? m_session;
    private SocketAsyncEventArgs m_recv_args;
    
    private StreamBuffer m_recv_buffer;
    
    public Receiver()
    {
        m_recv_args = new SocketAsyncEventArgs();
        m_recv_buffer = new StreamBuffer();
    }

    public void Init(Session session)
    {
        m_session = session;
    }

    public void DoRecv(int recv_count = 0)
    {
        for(int i = 0; i < recv_count; i++)
            RegisterRecv();
    }
    private void RegisterRecv()
    {
        m_recv_buffer.Clean();
        ArraySegment<byte> recv_segment = m_recv_buffer.GetWriteSegment();
        m_recv_args.SetBuffer(recv_segment.Array, recv_segment.Offset, recv_segment.Count);

        bool is_pending = m_session.Socket.ReceiveAsync(m_recv_args);
        if (false == is_pending)
            CompleteRecv(null, m_recv_args);
    }

    private void CompleteRecv(object? receiver, SocketAsyncEventArgs args)
    {
        if (0 == args.BytesTransferred /*연결 끊김*/ && args.SocketError == SocketError.Success)
        {
            if (false == m_recv_buffer.OnWrite((args.BytesTransferred)))
            {
                //TODO: LOG AND DIsconnect
                return;
            }
            
            int process_data_len = m_session.OnRecv(m_recv_buffer.GetReadSegment());

            if (false == m_recv_buffer.OnRead(process_data_len))
            {
                //TODO: LOG AND DIsconnect
                return;
            }
            
            RegisterRecv();
        }
    }
}
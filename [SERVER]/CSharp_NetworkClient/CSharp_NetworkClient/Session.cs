using System.Net;
using System.Net.Sockets;
using Google.Protobuf;

namespace CSharp_NetworkClient;

public abstract class Session
{
    public Socket Socket
    {
        get => m_socket;
        set => m_socket = value;
    }

    private Socket? m_socket;
    private EndPoint? m_remote_end_point;
    private int m_is_disconnected = 0;

    private MultiSender m_sender;
    private Receiver m_receiver;

    protected Dictionary<ushort, Func<Packet, bool>> m_packet_handlers;

    public Session()
    {
        m_sender = new MultiSender();
        m_receiver = new Receiver();
        m_packet_handlers = new Dictionary<ushort, Func<Packet, bool>>();
    }
    public void Init(Socket socket, EndPoint end_point)
    {
        m_socket = socket;
        m_remote_end_point = end_point;
        
        m_sender.Init(this);

        m_receiver.Init(this);
        m_receiver.DoRecv();
        
        InitPacketHandlers();
    }
    

    public void DoSend(Packet packet)
    {
        m_sender.RegisterSend(packet);
    }

    public void DoSend(IMessage message)
    {
        ushort packet_id = PacketNumberMapper.GetProtocolNumber(message.GetType().Name);
        
        Packet p = new Packet();
        p.InitializePacket(packet_id);
        p.PushData(message);
        p.FinalizePacket();
        
        DoSend(p);
    }

    public int OnRecv(ArraySegment<byte> recv_datas)
    {
        int process_len = 0;
        while (true)
        {
            if (Convert.ToInt32(PacketDefine.PACKET_HEADER_VALUE_SIZEOF) > recv_datas.Count)
                break;
            
            ushort packet_size = BitConverter.ToUInt16(recv_datas.Array, recv_datas.Offset);

            if (packet_size > recv_datas.Count)
                break;

            Packet packet = new Packet(new ArraySegment<byte>(recv_datas.Array, recv_datas.Offset, packet_size));
            
            OnPacketAssambled(packet);
            process_len += packet_size;

            recv_datas = new ArraySegment<byte>(recv_datas.Array, recv_datas.Offset + packet_size, recv_datas.Count - packet_size);
        }

        return process_len;
    }

    private void OnPacketAssambled(Packet packet)
    {
        if (false == m_packet_handlers.ContainsKey(packet.Protocol))
        {
            //TODO: LOG
            return;
        }

        if (false == m_packet_handlers[packet.Protocol]?.Invoke(packet))
        {
            //TODO: LOG AND Disconnect
            return;
        }
    }

    protected abstract void InitPacketHandlers();
    public abstract void OnConnected();
    public abstract void OnDisconnected();
    public abstract void OnSend();
}
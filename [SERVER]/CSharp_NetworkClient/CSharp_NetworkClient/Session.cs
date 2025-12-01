using System.Net;
using System.Net.Sockets;

namespace CSharp_NetworkClient;

public abstract class Session
{
    public Socket Socket
    {
        get => m_socket;
        set => m_socket = value;
    }

    private Socket m_socket;
    private EndPoint m_remote_end_point;
    private int m_is_disconnected = 0;

    private MultiSender m_sender;

    public void Init(Socket socket, EndPoint end_point)
    {
        m_socket = socket;
        m_remote_end_point = end_point;
    }
    

    public void DoSend(Packet packet)
    {
        
    }

    public void OnRecv()
    {
        
    }
    
    public abstract void OnConnected();
    public abstract void OnDisconnected();
    public abstract void OnPacketAssambled(Packet packet);
    public abstract void OnSend();
}
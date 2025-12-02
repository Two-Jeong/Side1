using System.Net;
using System.Net.Sockets;

namespace CSharp_NetworkClient;

public class Connector
{
    private Func<Session> m_session_factory;
    public void Connect(IPEndPoint end_point, int count, Func<Session> factory)
    {
        m_session_factory = factory;
        
        for(int i = 0; i < count; ++i)
        {
            Socket socket = new Socket(end_point.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

            SocketAsyncEventArgs args = new SocketAsyncEventArgs();
            args.Completed += CompleteConnect;
            args.RemoteEndPoint = end_point;
            args.UserToken = socket;

            if (false == StartConnect(args))
            {
                //TODO: LOG
                return;
            }
        }
    }


    private bool StartConnect(SocketAsyncEventArgs args)
    {
        Socket socket = args.UserToken as Socket;
        
        if (null == socket)
            return false;
        
        bool is_pending = socket.ConnectAsync(args);
        if(false == is_pending)
            CompleteConnect(null, args);

        return true;
    }

    private void CompleteConnect(object sender, SocketAsyncEventArgs args)
    {
        if (args.SocketError == SocketError.Success)
        {
            Session session = m_session_factory.Invoke();
            session.Init(args.ConnectSocket, args.RemoteEndPoint);
            
            session.OnConnected();
        }
    }
}
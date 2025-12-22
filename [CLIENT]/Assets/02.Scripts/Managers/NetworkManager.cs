using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Net;
using CSharp_NetworkClient;
using Google.Protobuf;
using UnityEngine;

public class NetworkManager : Singleton<NetworkManager> 
{
    private ServerSession session;
    private Connector connector;
    
    ConcurrentQueue<Packet> packet_queue =  new ConcurrentQueue<Packet>();
    void Start()
    {
        session = new ServerSession();
        connector = new Connector();
        
        IPAddress ip = IPAddress.Parse("222.110.17.100");
        IPEndPoint endpoint = new IPEndPoint(ip, 25000);
        connector.Connect(endpoint, 1, () => { return session; });
    }

    void Update()
    {
        Packet packet;
        if (false == packet_queue.TryDequeue(out packet))
            return;

        if (session == null)
        {
            Debug.LogError("Session is not initialized. Cannot execute packet handlers.");
            return;
        }
        
        if (false == session.ExecuteHandlers(packet))
        {
            Debug.Log($"execute handler fail => protocol: {packet.Protocol}");
            return;
        }
    }

    public void PushPacket(Packet packet)
    {
        packet_queue.Enqueue(packet);
    }

    public void DoSend(IMessage message)
    {
        if (session == null)
        {
            Debug.LogError("Session is not initialized. Cannot send message.");
            return;
        }
        
        session.DoSend(message);
    }
}

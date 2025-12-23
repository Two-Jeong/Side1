using System;
using System.Reflection;
using CSharp_NetworkClient;
using UnityEngine;

public class ServerSession : Session
{
    private PacketHandlers handlers =  new PacketHandlers();
    
    protected override void OnPacketAssambled(Packet packet)
    {
        NetworkManager.Instance.PushPacket(packet);
    }

    protected override void InitPacketHandlers()
    {
        var methods = handlers.GetType().GetMethods(BindingFlags.Public | BindingFlags.Instance);

        foreach (var method in methods)
        {
            var attribute = method.GetCustomAttribute<PacketHandlerAttribute>();
            if (attribute != null)
            {
                var func = (Func<Packet,bool>)Delegate.CreateDelegate(typeof(Func<Packet,bool>), handlers, method);
                m_packet_handlers[(ushort)attribute.PacketId] = func;

                Debug.Log($"Registered Packet Handler: {attribute.PacketId} -> {method.Name}");
            }
        }
    }

    public override void OnConnected()
    {
        Debug.Log($"server on connected");
    }

    public override void OnDisconnected()
    {
        Debug.Log($"server on disconnected");
    }

    public override void OnSend()
    {
        Debug.Log($"server on send");
    }

    public bool ExecuteHandlers(Packet packet)
    {
        if (false == m_packet_handlers.ContainsKey(packet.Protocol))
        {
            Debug.Log($"packet handler not found => protocol: {packet.Protocol}");
            return false;
        }

        if (false == m_packet_handlers[packet.Protocol].Invoke(packet))
        {
            Debug.Log($"packet handler invoke fail => protocol: {packet.Protocol}");
            return false;
        }

        return true;
    } 
}

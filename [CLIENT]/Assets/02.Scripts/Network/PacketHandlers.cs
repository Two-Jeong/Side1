using System;
using CSharp_NetworkClient;
using UnityEngine;

public class PacketHandlerAttribute : Attribute
{
    public packet_number PacketId { get; set; }
    public PacketHandlerAttribute(packet_number id) => PacketId = id;
}

public class PacketHandlers
{
    [PacketHandler(packet_number.TestEcho)]
    public bool TestHandler(Packet packet)
    {
        S2C_TestEcho RecvTestEchoPacketFromServer = new  S2C_TestEcho();
        packet.PopData(RecvTestEchoPacketFromServer);
        Debug.Log($"session id: {RecvTestEchoPacketFromServer.SessionId}, random number: {RecvTestEchoPacketFromServer.RandNumber}");
        
        return true;
    }

    [PacketHandler(packet_number.AccountLogin)]
    public bool AccountLoginHandler(Packet packet)
    {
        S2C_AccountLogin RecvAccountLoginPacketFromServer = new  S2C_AccountLogin();
        packet.PopData(RecvAccountLoginPacketFromServer);
        Debug.Log($"account login result: {RecvAccountLoginPacketFromServer.ResultCode}");

        return true;
    }
}

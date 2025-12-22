using System;
using CSharp_NetworkClient;

public class PacketHandlerAttribute : Attribute
{
    public packet_number PacketId { get; set; }
    public PacketHandlerAttribute(packet_number id) => PacketId = id;
}

public class PacketHandlers
{
    [PacketHandler(packet_number.TestEcho)]
    public void TestHandler(Packet packet)
    {
               
    }
}

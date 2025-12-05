namespace CSharp_NetworkClient;

public class TestSession : Session
{
    protected override void InitPacketHandlers()
    {
        m_packet_handlers.Add(Convert.ToUInt16(packet_number.TestEcho), OnRecvTestEcho);
    }

    public override void OnConnected()
    {
        Thread thread = new Thread(test_session_loop_func);
        thread.Start();
    }

    public override void OnDisconnected()
    {
    }

    public override void OnSend()
    {
    }

    private void test_session_loop_func()
    {
        Random random = new Random();
        while (true)
        {
            C2S_TestEcho send_packet = new C2S_TestEcho();
            send_packet.RandNumber = random.Next(100);

            DoSend(send_packet);
            
            Thread.Sleep(1000);
        }
    }
    private bool OnRecvTestEcho(Packet packet)
    {
        S2C_TestEcho recv_pacekt_from_server = new S2C_TestEcho();
        packet.PopData(recv_pacekt_from_server);

        Console.WriteLine($"echo packet recived: session id: {recv_pacekt_from_server.SessionId}, random number: {recv_pacekt_from_server.RandNumber}");
        return true;
    }
}
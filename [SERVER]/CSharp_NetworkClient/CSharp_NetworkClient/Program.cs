using System;
using System.Net;
using CSharp_NetworkClient;

class Program
{
    public static void Main(string[] args)
    {
        TestSession session = new TestSession();

        Connector connector = new Connector();

        IPAddress ip = IPAddress.Parse("127.0.0.1");
        IPEndPoint endpoint = new IPEndPoint(ip, 7777);
        connector.Connect(endpoint, 1, () => { return session; });

        while (true)
        {
        }
    }
}
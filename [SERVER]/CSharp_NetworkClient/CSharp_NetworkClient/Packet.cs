namespace CSharp_NetworkClient;

public class Packet
{
    public ArraySegment<byte> Data
    {
        get => m_data;
        set => m_data = value;
    }
    
    private ArraySegment<byte> m_data;

}
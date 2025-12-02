using System.Collections.Concurrent;

namespace CSharp_NetworkClient;

public static class PacketBufferManager
{
    public static StreamBuffer GetBuffer()
    {
        if (true == m_send_buffers.IsEmpty)
        {
            return new StreamBuffer();
        }
        else
        {
            while (true)
            {
                if (false == m_send_buffers.TryDequeue(out StreamBuffer? buffer))
                    continue;
                
                return buffer;
            }
        }
    }

    public static void ReturnBuffer(StreamBuffer buffer)
    {
        buffer.Reset();
        m_send_buffers.Enqueue(buffer);
    }
    
    private static ConcurrentQueue<StreamBuffer> m_send_buffers;
}
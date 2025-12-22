using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Concurrent;
namespace CSharp_NetworkClient
{

    public static class PacketBufferManager
    {
        private static ConcurrentQueue<StreamBuffer> m_send_buffers = new ConcurrentQueue<StreamBuffer>();
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
    }
}


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharp_NetworkClient
{
    public class StreamBuffer
    {
        private readonly byte[] m_buffer;
        private int m_read_pos;
        private int m_write_pos;

        public int MaxBufferSize { get; }
        public int ReadPos => m_read_pos;
        public int WritePos => m_write_pos;
        public int RemainingSize => MaxBufferSize - m_write_pos;
        public int DataSize => m_write_pos - m_read_pos;

        public StreamBuffer() : this(UInt16.MaxValue * 3)
        {
        }

        public StreamBuffer(int bufferSize)
        {
            if (bufferSize <= 0)
                throw new ArgumentException("Buffer size must be positive", nameof(bufferSize));

            m_buffer = new byte[bufferSize];
            MaxBufferSize = bufferSize;
            m_read_pos = 0;
            m_write_pos = 0;
        }

        public Span<byte> GetReadSpan()
        {
            return m_buffer.AsSpan(m_read_pos, DataSize);
        }

        public Span<byte> GetWriteSpan()
        {
            return m_buffer.AsSpan(m_write_pos, RemainingSize);
        }

        public ArraySegment<byte> GetReadSegment()
        {
            return new ArraySegment<byte>(m_buffer, m_read_pos, DataSize);
        }

        public ArraySegment<byte> GetWriteSegment()
        {
            return new ArraySegment<byte>(m_buffer, m_write_pos, RemainingSize);
        }

        public bool OnRead(int dataSize)
        {
            if (DataSize < dataSize)
                return false;

            m_read_pos += dataSize;
            Clean();
            return true;
        }

        public bool OnWrite(int dataSize)
        {
            if (RemainingSize < dataSize)
                return false;

            m_write_pos += dataSize;
            return true;
        }

        public void Clean()
        {
            int dataSize = DataSize;

            if (dataSize == 0)
            {
                m_read_pos = 0;
                m_write_pos = 0;
            }
            else if (dataSize < Convert.ToInt32(PacketDefine.PACKET_SIZE_VALUE_SIZEOF) && m_read_pos > 0)
            {
                Buffer.BlockCopy(m_buffer, m_read_pos, m_buffer, 0, dataSize);
                m_read_pos = 0;
                m_write_pos = dataSize;
            }
        }

        public void Reset()
        {
            m_read_pos = 0;
            m_write_pos = 0;
        }
    }
}

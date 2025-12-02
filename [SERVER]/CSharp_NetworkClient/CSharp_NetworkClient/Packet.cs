using Google.Protobuf;

namespace CSharp_NetworkClient
{
    public struct PacketHeader
    {
        public ushort packet_size;
        public ushort packet_protocol;
    }

    enum PacketDefine
    {
        PACKET_SIZE_VALUE_SIZEOF = sizeof(ushort),
        PACKET_PROTOCOL_VALUE_SIZEOF = sizeof(ushort),
        PACKET_HEADER_VALUE_SIZEOF = PACKET_SIZE_VALUE_SIZEOF + PACKET_PROTOCOL_VALUE_SIZEOF,
    }

    public class Packet
    {
        private StreamBuffer? m_data;
        private int m_pos;

        public ArraySegment<byte> Data => new ArraySegment<byte>(m_data.GetWriteSpan().ToArray(), 0, m_pos);
        public ushort Size => BitConverter.ToUInt16(m_data.GetWriteSpan().Slice(0, Convert.ToInt32(PacketDefine.PACKET_SIZE_VALUE_SIZEOF)));
        public ushort Protocol => BitConverter.ToUInt16(m_data.GetWriteSpan().Slice(Convert.ToInt32(PacketDefine.PACKET_SIZE_VALUE_SIZEOF), Convert.ToInt32(PacketDefine.PACKET_PROTOCOL_VALUE_SIZEOF)));

        public Packet()
        {
            m_data = PacketBufferManager.GetBuffer();
        }
        
        public Packet(ArraySegment<byte> packet_buffer)
        {
            m_data = PacketBufferManager.GetBuffer();
            m_data.Reset();
            
            packet_buffer.CopyTo(m_data.GetWriteSegment());
        }
        
        ~Packet()
        {
            DisposePacket();
        }

        public void DisposePacket()
        {
            if (null != m_data)
            {
                m_data.OnWrite(m_pos);
                PacketBufferManager.ReturnBuffer(m_data);
            }
            m_data = null;

            m_pos = 0;
        }

        public bool InitializePacket(ushort protocol_no)
        {
            if (false == BitConverter.TryWriteBytes(m_data.GetWriteSpan().Slice(Convert.ToInt32(PacketDefine.PACKET_SIZE_VALUE_SIZEOF), Convert.ToInt32(PacketDefine.PACKET_PROTOCOL_VALUE_SIZEOF)), protocol_no))
            {
                //TODO: LOG
                return false;
            }

            m_pos += Convert.ToInt32(PacketDefine.PACKET_HEADER_VALUE_SIZEOF);

            return true;
        }

        public bool PushData(IMessage message)
        {
            if (null == m_data)
            {
                //TODO: LOG
                return false;
            }
            message.WriteTo(m_data.GetWriteSpan().Slice(m_pos));

            m_pos += message.CalculateSize();
            return true;
        }

        public bool FinalizePacket()
        {
            if(null == m_data)
            {
                //TODO: LOG
                return false;
            }

            if (false == BitConverter.TryWriteBytes(m_data.GetWriteSpan().Slice(0, Convert.ToInt32(PacketDefine.PACKET_SIZE_VALUE_SIZEOF)), m_pos))
            {
                //TODO: LOG
                return false;
            }

            return true;
        }

        public void PopHeader(ref PacketHeader header)
        {
            header.packet_size = BitConverter.ToUInt16(m_data.GetReadSpan().Slice(0, Convert.ToInt32(PacketDefine.PACKET_SIZE_VALUE_SIZEOF)));
            header.packet_protocol = BitConverter.ToUInt16(m_data.GetReadSpan().Slice(Convert.ToInt32(PacketDefine.PACKET_SIZE_VALUE_SIZEOF), Convert.ToInt32(PacketDefine.PACKET_PROTOCOL_VALUE_SIZEOF)));

            m_pos += Convert.ToInt32(PacketDefine.PACKET_HEADER_VALUE_SIZEOF);
        }
        public bool PopData(ref IMessage message)
        {
            if(null == m_data)
            {
                //TODO: LOG
                return false;
            }    

            message.MergeFrom(m_data.GetReadSpan().Slice(m_pos, message.CalculateSize()));
            return true;
        }
    }
}
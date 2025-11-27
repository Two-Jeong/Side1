#include "pch.h"
#include "Packet.h"

Packet::Packet()
{
}

Packet::Packet(Packet* packet)
{
    m_owner = packet->m_owner;
    m_buffer = packet->m_buffer;
    m_current_idx = packet->m_current_idx;
}

Packet::~Packet()
{
}

void Packet::reserve_packet_buffer(int size)
{
    m_buffer.reserve(size);
}

void Packet::set_packet(char* data, int size)
{
    m_buffer.assign(data, data + size);
}

void Packet::set_owner(Session* session)
{
    m_owner = session;
}

Session* Packet::get_owner()
{
    return m_owner;
}

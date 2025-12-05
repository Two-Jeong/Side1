#include "pch.h"
#include "ClientSession.h"

void ClientSession::init_handlers()
{
}

NetworkCore* ClientSession::get_network_core()
{
    if (m_section == nullptr)
        return nullptr;
    
    return m_section->get_network_core();
}

void ClientSession::on_connected()
{
}

int ClientSession::on_recieve()
{
    return Session::on_recieve();
}

void ClientSession::on_send(int data_size)
{
}

void ClientSession::on_disconnected()
{
    Session::on_disconnected();
    m_section->exit_section(get_id());
    m_section = nullptr;
}

void ClientSession::execute_packet(Packet* packet)
{
    Session::execute_packet(packet);

    xdelete packet;
}

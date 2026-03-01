#include "pch.h"
#include "ClientSession.h"

void ClientSession::init_handlers()
{
}

NetworkCore* ClientSession::get_network_core()
{
    auto section = m_section.lock();
    if (nullptr == section)
        return nullptr;
    
    return section->get_network_core();
}

ServerBase* ClientSession::get_server_base()
{
    auto section = m_section.lock();
    if (nullptr == section)
        return nullptr;
    
    return static_cast<ServerBase*>(section->get_network_core());
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
    auto section = m_section.lock();
    if (nullptr != section)
        section->exit_section(get_id());
    m_section.reset();
}

void ClientSession::execute_packet(Packet* packet)
{
    Session::execute_packet(packet);

    xdelete packet;
}

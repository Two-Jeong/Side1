#include "pch.h"
#include "LoginClientSession.h"

void LoginClientSession::init()
{
    ClientSession::init();
}

void LoginClientSession::init_handlers()
{
    m_handlers.emplace(packet_number::TestEcho, [this](auto* p){this->test_echo_handler(p); });
}

void LoginClientSession::finalize()
{
    ClientSession::finalize();
}

void LoginClientSession::on_connected()
{
    ClientSession::on_connected();
}

void LoginClientSession::on_send(int data_size)
{
    ClientSession::on_send(data_size);
}

void LoginClientSession::on_disconnected()
{
    ClientSession::on_disconnected();
}

void LoginClientSession::test_echo_handler(Packet* packet)
{
    C2S_TestEcho recv_message_from_client;
    packet->pop_message(recv_message_from_client);

    std::cout << "Recv Echo: " << recv_message_from_client.rand_number() << std::endl;

    S2C_TestEcho send_message_to_client;
    send_message_to_client.set_session_id(get_id());
    send_message_to_client.set_rand_number(recv_message_from_client.rand_number());

    if (false == do_send(send_message_to_client))
    {
        //TODO: LOG AND DISCONNECT
        return;
    }
}
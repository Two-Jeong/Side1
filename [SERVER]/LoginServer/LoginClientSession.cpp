#include "pch.h"
#include "LoginClientSession.h"

void LoginClientSession::init()
{
    ClientSession::init();
}

void LoginClientSession::init_handlers()
{
    m_handlers.emplace(packet_number::TestEcho, [this](auto* p){this->test_echo_handler(p); });
    m_handlers.emplace(packet_number::AccountRegister, [this](auto* p){this->account_register_handler(p); });
    m_handlers.emplace(packet_number::AccountLogin, [this](auto* p){this->account_login_handler(p); });
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

void LoginClientSession::account_register_handler(Packet* packet)
{
    C2S_AccountRegister recv_message_from_client;
    packet->pop_message(recv_message_from_client);

    AccountRegisterResult::Code result = AccountRegisterResult::SUCCESS;
    if (m_accounts.count(recv_message_from_client.id()))
        result = AccountRegisterResult::ID_ALREADY_EXIST;
    else
        m_accounts[recv_message_from_client.id()] = recv_message_from_client.password();

    std::cout << "regiter account => id: " << recv_message_from_client.id()  <<"," <<  "password:" << recv_message_from_client.password() << "result: %d" << result << std::endl;

    
    S2C_AccountRegister send_message_to_client;
    send_message_to_client.set_result_code(result);

    if (false == do_send(send_message_to_client))
    {
        //TODO: LOG AND DISCONNECT
        return;
    }
}

void LoginClientSession::account_login_handler(Packet* packet)
{
    C2S_AccountLogin recv_message_from_client;
    packet->pop_message(recv_message_from_client);

    AccountLoginResult::Code result = AccountLoginResult::SUCCESS;
    if (0 != m_accounts.count(recv_message_from_client.id()))
        result = (m_accounts[recv_message_from_client.id()] == recv_message_from_client.password()) ? result = AccountLoginResult::SUCCESS : AccountLoginResult::ID_OR_PASSWORD_WRONG;
    else
        result = AccountLoginResult::ID_OR_PASSWORD_WRONG;
    
    std::cout << "login account => id: " << recv_message_from_client.id()  <<"," <<  "password:" << recv_message_from_client.password() << "result: %d" << result << std::endl;
    
    S2C_AccountLogin send_message_to_client;
    send_message_to_client.set_result_code(result);

    if (false == do_send(send_message_to_client))
    {
        //TODO: LOG AND DISCONNECT
        return;
    }
}

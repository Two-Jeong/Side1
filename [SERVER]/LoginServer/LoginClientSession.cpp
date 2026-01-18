#include "pch.h"
#include "LoginClientSession.h"
#include "DatabaseManager.h"
#include "AsyncDBContext.h"

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


    auto db_context = DB::create_async_context(
        [session = this](DB::QueryResult& result)
        {
            if (session->is_connected())
            {
                auto row = result.fetch_one();

                auto register_result = row->get<bool>("result");

                AccountRegisterResult::Code result_code = AccountRegisterResult::SUCCESS;
                if (true == register_result)
                    result_code = AccountRegisterResult::SUCCESS;
                else
                    result_code = AccountRegisterResult::ID_ALREADY_EXIST;
            
                std::cout << "regiter account success result: " << result_code << std::endl;

                S2C_AccountRegister send_message_to_client;
                send_message_to_client.set_result_code(result_code);

                if (false == session->do_send(send_message_to_client))
                {
                    //TODO: LOG AND DISCONNECT
                    return;
                }
            }
        }
        ,
        [](const std::string& cause)
        {
            std::cout << cause << std::endl;
        }
    );

    auto task = xnew iTask;
    task->func = [db_context, user_id = recv_message_from_client.id(), password = recv_message_from_client.password()]() {
        try {
            auto result = DB_INSTANCE().execute_query("CALL register_account('" + user_id + "','" + password + "');");
            
            db_context->deliver_success(result);
        } catch (const std::exception& e) {
            db_context->deliver_error(e.what());
        }
    };
    
    get_server_base()->push_hard_task(task);
}

void LoginClientSession::account_login_handler(Packet* packet)
{
    C2S_AccountLogin recv_message_from_client;
    packet->pop_message(recv_message_from_client);

    auto db_context = DB::create_async_context(
        [session = this](const DB::QueryResult& result)
        {
            if (session->is_connected()) {

                auto result_code = AccountLoginResult::SUCCESS;
                if (0 == result.row_count())
                    result_code = AccountLoginResult::ID_OR_PASSWORD_WRONG;

                S2C_AccountLogin send_packet_to_client;
                send_packet_to_client.set_result_code(result_code);
                session->do_send(send_packet_to_client);
                std::cout << "Login DB Execute successful session: " << session->get_id() << std::endl;
            }
        },
        [session = this](const std::string& error)
        {
            if (session->is_connected()) {
                S2C_AccountLogin response;
                response.set_result_code(AccountLoginResult::ID_OR_PASSWORD_WRONG);
                session->do_send(response);
                std::cout << "Login DB Execute failed: " << error << std::endl;
            }
        }
    );
    
    auto task = xnew iTask;
    task->func = [db_context, user_id = recv_message_from_client.id(), password = recv_message_from_client.password()]() {
        try {
            auto result = DB_INSTANCE().execute_query(
                "SELECT id FROM account WHERE id = '"+ user_id + "' AND password = '" + password + "'");
            
            db_context->deliver_success(result);
        } catch (const std::exception& e) {
            db_context->deliver_error(e.what());
        }
    };
    
    get_server_base()->push_hard_task(task);
}

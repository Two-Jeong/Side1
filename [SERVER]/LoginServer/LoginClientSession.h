#pragma once

class LoginClientSession : public ClientSession
{
public:
    LoginClientSession() = default;
    ~LoginClientSession() override = default;
public:
    void init() override;
    void init_handlers() override;
    void finalize() override;

public:
    void on_connected() override;
    void on_send(int data_size) override;
    void on_disconnected() override;

private:
    void test_echo_handler(Packet* packet);
    void account_register_handler(Packet* packet);
    void account_login_handler(Packet* packet);

private:
    std::map<std::string, std::string> m_accounts; // DB 연결 전 임시 
};
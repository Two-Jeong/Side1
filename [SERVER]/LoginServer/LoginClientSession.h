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
};
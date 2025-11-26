#pragma once

class ServerSession : public Session
{
public:
    ServerSession() = default;
    ~ServerSession() override = default;
public:
    NetworkCore* get_network_core() override { return m_owner; }
    void set_network_core(class ClientBase* owner) { m_owner = owner; }

public:
    void on_connected() override;
    int on_recieve() final;
    void on_send(int data_size) override;
    void on_disconnected() override;
    
private:
    class ClientBase* m_owner;
};

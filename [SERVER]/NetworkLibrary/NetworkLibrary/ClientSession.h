#pragma once

class ClientSession : public Session
{
public:
    ClientSession() = default;
    ~ClientSession() override = default;

public:
    void init_handlers() override;
public:
    std::shared_ptr<NetworkSection> get_section() override { return m_section.lock(); }
    void set_section(std::shared_ptr<NetworkSection> section) { m_section = section; }
    
    NetworkCore* get_network_core() override;
    virtual ServerBase* get_server_base();
public:
    void on_connected() override;
    int on_recieve() final;
    void on_send(int data_size) override;
    void on_disconnected() override;
    void execute_packet(Packet* packet) override;

protected:
    std::weak_ptr<NetworkSection> m_section;
};

#pragma once

class ClientSession : public Session
{
public:
    ClientSession() = default;
    ~ClientSession() override = default;
    
public:
    unsigned int get_section_id();
    class NetworkSection* get_section()  { return m_section; }
    void set_section(class NetworkSection* section) { m_section = section; }
    
    NetworkCore* get_network_core() override;

public:
    void on_connected() override;
    int on_recieve() final;
    void on_send(int data_size) override;
    void on_disconnected() override;
    void execute_packet(Packet* packet) override;

protected:
    class NetworkSection* m_section;
};

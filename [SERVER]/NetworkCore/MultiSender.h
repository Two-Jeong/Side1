#pragma once
#include <queue>

class MultiSender
{
public:
    MultiSender(Session* session);
    ~MultiSender();

public:
    bool is_register_queue_empty() { return m_register_packet.empty(); }
public:
    bool register_packet(std::shared_ptr<Packet> packet);
    bool on_send();
    void clear();
private:
    bool send();

private:
    Concurrency::concurrent_queue<std::shared_ptr<Packet>> m_register_packet;
    std::queue<std::shared_ptr<Packet>> m_sending_packet; // send중인 패킷

    std::atomic<bool> m_sending_flag;

    Session* m_owner;
    SendIO m_send_io;
};

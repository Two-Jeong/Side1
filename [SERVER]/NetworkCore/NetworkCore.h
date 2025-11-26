#pragma once
class NetworkIO;

class NetworkCore
{
public:
    NetworkCore();
    virtual ~NetworkCore();

public:
    HANDLE get_iocp_handle() { return m_iocp_handle; }
    
public:
    virtual void init(int iocp_thread_count);
public:
    bool is_running() { return m_is_running; }

public:
    void push_packet(Packet* packet) { m_packet_queue.push(packet); }
    
protected:
    void iocp_thread_work();
    virtual void on_iocp_io(NetworkIO* io, int bytes_transferred) abstract;
    
protected:
    std::atomic<bool> m_is_running;
    
    HANDLE m_iocp_handle;
    std::vector<std::thread> m_iocp_threads;
    
    concurrency::concurrent_queue<Packet*> m_packet_queue;
};


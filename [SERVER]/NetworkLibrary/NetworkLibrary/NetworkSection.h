#pragma once

struct task_cmp
{
    bool operator()(const iTask* a, const iTask* b) const
    {
        return a->execute_time > b->execute_time;
    }
};

class NetworkSection
{
public:
    NetworkSection() = default;
    virtual ~NetworkSection() = default;
public:
    void init(ServerBase* owner, int section_id);
    
public:
    static unsigned int generate_section_id();

    unsigned int get_id() const { return m_section_id; }
    HANDLE get_iocp_handle();
    NetworkCore* get_network_core() { return m_owner; }

public:
    ClientSession* find_session(unsigned int session_id);
    
public:
    virtual void enter_section(class ClientSession* session);
    virtual void exit_section(int session_id);
    void push_task(iTask* task);

    void broadcast(std::shared_ptr<Packet> packet);
    void broadcast(std::shared_ptr<Packet> packet, Session* exception_session);
    
private:
    void section_thread_work();

public:
    double get_fps() const { return m_current_fps; }
    void update_fps_info();
    double get_recv_tps() const { return m_current_recv_tps; }
    void update_recv_tps_info();
    void increment_recv_count_for_tps();
    double get_send_tps() const { return m_current_send_tps; }
    void update_send_tps_info();
    void increment_send_count_for_tps();
    
private:
    unsigned int m_section_id;

    class ServerBase* m_owner; 
    std::thread m_section_thread;
    std::map<unsigned int, class ClientSession*> m_sessions;
    
    Concurrency::concurrent_priority_queue<iTask*, task_cmp> m_task_queue;
    
    // FPS 측정 관련
    std::chrono::high_resolution_clock::time_point m_last_frame_time;
    int m_frame_count;
    double m_current_fps;
    
    // TPS 측정 관련
    std::chrono::high_resolution_clock::time_point m_last_recv_tps_time;
    std::atomic<int> m_recv_count;
    double m_current_recv_tps;
    
    std::chrono::high_resolution_clock::time_point m_last_send_tps_time;
    std::atomic<int> m_send_count;
    double m_current_send_tps;
};
#pragma once

class ServerBase : public NetworkCore
{
public:
    ServerBase() = default;
    virtual ~ServerBase() = default;
    
public:
    virtual void init(int iocp_thread_count = 1, int hard_task_thread_count = 1, std::function<class NetworkSection*()> section_factory = {}, int section_count = 0);
    void open(std::string open_ip, int open_port, std::function<class ClientSession*()> session_factory, int accpet_back_log = 1);
    
    double get_fps_avg();
    double get_recv_tps_avg();
    double get_send_tps_avg();
    void print_fps_info();
    
    double get_accept_tps() const { return m_current_accept_tps; }
    void update_accept_tps_info();
    void increment_accept_count_for_tps();

public:
    void on_accept(int bytes_transferred, NetworkIO* io);
    
    void push_hard_task(iTask* task);
    void push_hard_task(std::shared_ptr<iTask> task);
    
private:
    void central_thread_work();
    void fps_monitor_thread_work();
    void hard_task_thread_work();

protected:
    void on_iocp_io(NetworkIO* io, int bytes_transferred) override;
    virtual NetworkSection* select_first_section() abstract;

protected:
    SOCKET m_listen_socket;
    
    std::thread m_central_thread;
    std::thread m_performance_monitor_thread;
    
    std::vector<std::thread> m_hard_task_threads;
    concurrency::concurrent_queue<iTask*> m_hard_task_queue;

    std::map<unsigned int, class NetworkSection*> m_sections;
    std::function<class NetworkSection*()> m_section_factory;
    std::function<class ClientSession*()> m_session_factory;
    
    // Accept TPS 측정 관련
    std::chrono::high_resolution_clock::time_point m_last_accept_tps_time;
    std::atomic<int> m_accept_count;
    double m_current_accept_tps;
};

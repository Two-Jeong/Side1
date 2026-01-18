#include "pch.h"
#include "ServerBase.h"

#include <memory>
#include <memory>

void ServerBase::init(int iocp_thread_count, int hard_task_thread_count, std::function<NetworkSection*()> section_factory, int section_count)
{
    NetworkCore::init(iocp_thread_count);

    if (performance_check_mode)
    {
        m_last_accept_tps_time = std::chrono::high_resolution_clock::now();
        m_accept_count = 0;
        m_current_accept_tps = 0;
        
        m_performance_monitor_thread = std::thread(&ServerBase::fps_monitor_thread_work, this);
    }
    
    m_central_thread = std::thread(&ServerBase::central_thread_work, this);

    for (int i = 0; i < hard_task_thread_count; ++i)
    {
        m_hard_task_threads.push_back(std::thread(&ServerBase::hard_task_thread_work, this));
    }
    
    m_listen_socket = NetworkUtil::create_socket();
    m_section_factory = section_factory;
    
    for(int i = 0; i < section_count; ++i)
    {
        NetworkSection* section = m_section_factory();
        int section_id = NetworkSection::generate_section_id();
        section->init(this, section_id);
        m_sections.emplace(section_id, section);
    }
}
void ServerBase::open(std::string open_ip, int open_port, std::function<ClientSession*()> session_factory, int accept_back_log)
{
    m_session_factory = session_factory;

    NetworkUtil::bind(m_listen_socket, open_ip.c_str(), open_port);
    NetworkUtil::listen(m_listen_socket, 1);
    NetworkUtil::register_socket(m_iocp_handle, m_listen_socket);

    for(int i = 0; i < accept_back_log; ++i)
    {
        AcceptIO* io = new AcceptIO;
        io->m_socket = NetworkUtil::create_socket();
        NetworkUtil::register_socket(m_iocp_handle, io->m_socket);
        
        if(false == NetworkUtil::accept(m_listen_socket, io))
        {
            // TODO: STOP SERVER 
            return; 
        }
    }
    

    std::cout << "Listening..." << std::endl;
}

void ServerBase::on_accept(int bytes_transferred, NetworkIO* io) {
    
    if (performance_check_mode)
        increment_accept_count_for_tps();
    
    AcceptIO* accept_io = reinterpret_cast<AcceptIO*>(io);

    
    ClientSession* session = m_session_factory();
    session->init();
    session->set_id(Session::generate_session_id());
    session->set_socket(accept_io->m_socket);

    sockaddr* remote_addr = NetworkUtil::get_remote_sockaddr(accept_io->m_accept_buffer);
    sockaddr_in* addr_in = reinterpret_cast<sockaddr_in*>(remote_addr);
    char output_ip[INET_ADDRSTRLEN] = {0, };
    inet_ntop(addr_in->sin_family, &addr_in->sin_addr, output_ip, INET_ADDRSTRLEN);
    
    session->set_remote_ip(output_ip);
    session->set_remote_port(ntohs(reinterpret_cast<sockaddr_in*>(remote_addr)->sin_port));

    NetworkUtil::register_socket(m_iocp_handle, session->get_socket());
    session->complete_connect();
    
    NetworkSection* first_section =  select_first_section();
    if (nullptr == first_section)
    {
        std::cout << "first section is nullptr" << std::endl;
        // TODO: LOG
        // TODO: stop server
        return;
    }
    
    first_section->enter_section(session); // TODO: 로드 밸런싱 로직


    std::cout << "Accept complete => ip: " << session->get_remote_ip() << ", port: " << session->get_remote_port() << std::endl;
    
    accept_io->Init();
    accept_io->m_socket = NetworkUtil::create_socket();
    if(false == NetworkUtil::accept(m_listen_socket, accept_io))
    {
        // 서버 중지
        return;
    }
}

void ServerBase::push_hard_task(iTask* task)
{
    if (task) {
        m_hard_task_queue.push(task);
    }
}

void ServerBase::push_hard_task(std::shared_ptr<iTask> task)
{
    if (task) {
        m_hard_task_queue.push(task.get());
    }
}

void ServerBase::central_thread_work()
{
    while(is_running() == true)
    {
        Packet* packet;

        if(false == m_packet_queue.try_pop(packet))
            continue;
        
        ClientSession* session = static_cast<ClientSession*>(packet->get_owner());
       
        iTask* task = xnew iTask;

        task->func = [session, packet]() { session->execute_packet(packet); };
        
        session->get_section()->push_task(task);
    }
}

double ServerBase::get_fps_avg()
{
    if (m_sections.empty()) 
        return 0;
    
    double total_fps = 0;
    int active_sections = 0;
    
    for (auto& section_pair : m_sections)
    {
        double section_fps = section_pair.second->get_fps();
        if (section_fps > 0)
        {
            total_fps += section_fps;
            active_sections++;
        }
    }
    
    return (active_sections > 0) ? total_fps / active_sections : 0;
}

double ServerBase::get_recv_tps_avg()
{
    if (m_sections.empty()) 
        return 0;
    
    double total_tps = 0;
    int active_sections = 0;
    
    for (auto& section_pair : m_sections)
    {
        double section_tps = section_pair.second->get_recv_tps();
        if (section_tps > 0)
        {
            total_tps += section_tps;
            active_sections++;
        }
    }
    
    return (active_sections > 0) ? total_tps / active_sections : 0;
}

double ServerBase::get_send_tps_avg()
{
    if (m_sections.empty()) 
        return 0;
    
    double total_tps = 0;
    int active_sections = 0;
    
    for (auto& section_pair : m_sections)
    {
        double section_tps = section_pair.second->get_send_tps();
        if (section_tps > 0)
        {
            total_tps += section_tps;
            active_sections++;
        }
    }
    return (active_sections > 0) ? total_tps / active_sections : 0;
}

void ServerBase::update_accept_tps_info()
{
    auto current_time = std::chrono::high_resolution_clock::now();
    auto delta_time = std::chrono::duration<double>(current_time - m_last_accept_tps_time).count();
        
    if (delta_time >= 1.0) // 1초마다 TPS 업데이트
    {
        int accept_count = m_accept_count.exchange(0);
        m_current_accept_tps = accept_count / delta_time;
        m_last_accept_tps_time = current_time;
    }
}

void ServerBase::increment_accept_count_for_tps()
{
    m_accept_count.fetch_add(1);
}

void ServerBase::print_fps_info()
{
    std::cout << "=== Server Performance Info ===" << std::endl;
    std::cout << "Average FPS: " << static_cast<int>(get_fps_avg()) << std::endl;
    std::cout << "Accept TPS: " << static_cast<int>(get_accept_tps()) << std::endl;
    std::cout << "Average RECV TPS: " << static_cast<int>(get_recv_tps_avg()) << std::endl;
    std::cout << "Average SEND TPS: " << static_cast<int>(get_send_tps_avg()) << std::endl;
    
    for (auto& section_pair : m_sections)
    {
        std::cout << "Section " << section_pair.first << " FPS: " << static_cast<int>(section_pair.second->get_fps()) 
                  << ", Recv TPS: " << section_pair.second->get_recv_tps()
                  << ", Send TPS: " << section_pair.second->get_send_tps() << std::endl;
    }
    std::cout << "===============================" << std::endl;
}

void ServerBase::fps_monitor_thread_work()
{
    while (is_running() == true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(server_fps_check_interval));
        
        update_accept_tps_info();
        print_fps_info();
    }
}

void ServerBase::hard_task_thread_work()
{
    while (true == is_running())
    {
        iTask* task = nullptr;

        if (false == m_hard_task_queue.try_pop(task))
            continue;
        
        if(std::chrono::steady_clock::now() < task->execute_time)
        {
            m_hard_task_queue.push(task);
            continue;
        }

        task->func();

        if (nullptr != task->post_processing_func)
            task->post_processing_func();

        if(task->is_repeat)
        {
            task->execute_time = std::chrono::steady_clock::now() + std::chrono::microseconds(task->delay_time);
            m_hard_task_queue.push(task);
        }
        else
            xdelete task;
    }
}

void ServerBase::on_iocp_io(NetworkIO* io, int bytes_transferred)
{
    Session* session = io->get_session();

    switch(io->get_type())
    {
    case IoType::DISCONNECT:
        session->complete_disconnect();
        break;
    case IoType::ACCEPT:
        on_accept(bytes_transferred, io);
        break;
    case IoType::RECV:
        session->complete_recieve(bytes_transferred);
        break;
    case IoType::SEND:
        session->complete_send(bytes_transferred);
        break;
    default:
        // TODO: error log
        break;
    }
}

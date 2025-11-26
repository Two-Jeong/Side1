#include "pch.h"
#include "NetworkSection.h"

void NetworkSection::init(ServerBase* owner, int section_id)
{
    m_owner = owner;
    m_section_id = section_id;

    if (performance_check_mode)
    {
        m_last_frame_time = std::chrono::high_resolution_clock::now();
        m_frame_count = 0;
        m_current_fps = 0;
    
        m_last_recv_tps_time = std::chrono::high_resolution_clock::now();
        m_recv_count = 0;
        m_current_recv_tps = 0;
    
        m_last_send_tps_time = std::chrono::high_resolution_clock::now();
        m_send_count = 0;
        m_current_send_tps = 0;
    }
    
    m_section_thread= std::thread([this](){ section_thread_work(); });
}

unsigned int NetworkSection::generate_section_id()
{
    static unsigned int id_generator = 0;
    return ++id_generator;
}

HANDLE NetworkSection::get_iocp_handle()
{
    return m_owner->get_iocp_handle();
}

ClientSession* NetworkSection::find_session(unsigned int session_id)
{
    return m_sessions.at(session_id);
}

void NetworkSection::enter_section(ClientSession* session)
{
    if(m_sessions.count(session->get_id()) != 0)
    {
        session->do_disconnect();
        return;
    }
    
    m_sessions.emplace(session->get_id(), session);
   session->set_section(this);
}

void NetworkSection::exit_section(int session_id)
{
    if(m_sessions.count(session_id) == 0)
        return;
    
    ClientSession* session = m_sessions.at(session_id);
    session->set_section(nullptr);
    m_sessions.erase(session_id);
}

void NetworkSection::push_task(iTask* task)
{
    task->execute_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(task->delay_time);
    m_task_queue.push(task);
}

void NetworkSection::broadcast(std::shared_ptr<Packet> packet)
{
    for (auto& session : m_sessions)
        session.second->do_send(packet);
}

void NetworkSection::broadcast(std::shared_ptr<Packet> packet, Session* exception_session)
{
    for (auto& session : m_sessions)
    {
        if (session.second->get_id() == exception_session->get_id()) continue;
        session.second->do_send(packet);
    }
}

void NetworkSection::section_thread_work()
{
    while(m_owner->is_running() == true)
    {
        if (performance_check_mode)
        {
            update_fps_info();
            update_recv_tps_info();
            update_send_tps_info();
        }
        
        if(m_task_queue.empty()) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        
        iTask* task = nullptr;
        if(false == m_task_queue.try_pop(task)) continue;
        
        if(std::chrono::steady_clock::now() < task->execute_time)
        {
            m_task_queue.push(task);
            continue;
        }

        task->func();

         if(task->is_repeat)
        {
            task->execute_time = std::chrono::steady_clock::now() + std::chrono::microseconds(task->delay_time);
            m_task_queue.push(task);
        }
        else
            xdelete task;
            
    }
}

void NetworkSection::update_fps_info()
{
    m_frame_count++;
    
    auto current_time = std::chrono::high_resolution_clock::now();
    auto delta_time = std::chrono::duration<double>(current_time - m_last_frame_time).count();
        
    if (delta_time >= 1.0) // 1초마다 FPS 업데이트
    {
        m_current_fps = m_frame_count / delta_time;
        m_frame_count = 0;
        m_last_frame_time = current_time;
    }
}

void NetworkSection::update_recv_tps_info()
{
    auto current_time = std::chrono::high_resolution_clock::now();
    auto delta_time = std::chrono::duration<double>(current_time - m_last_recv_tps_time).count();
        
    if (delta_time >= 1.0) // 1초마다 TPS 업데이트
    {
        int recv_count = m_recv_count.exchange(0);
        m_current_recv_tps = recv_count / delta_time;
        m_last_recv_tps_time = current_time;
    }
}

void NetworkSection::increment_recv_count_for_tps()
{
    m_recv_count.fetch_add(1);
}

void NetworkSection::update_send_tps_info()
{
    auto current_time = std::chrono::high_resolution_clock::now();
    auto delta_time = std::chrono::duration<double>(current_time - m_last_send_tps_time).count();
        
    if (delta_time >= 1.0) // 1초마다 TPS 업데이트
    {
        int send_count = m_send_count.exchange(0);
        m_current_send_tps = send_count / delta_time;
        m_last_send_tps_time = current_time;
    }
}

void NetworkSection::increment_send_count_for_tps()
{
    m_send_count.fetch_add(1);
}

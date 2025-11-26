#include "pch.h"
#include "ClientBase.h"

#include "ServerSession.h"

void ClientBase::init(int iocp_thread_count)
{
    NetworkCore::init(iocp_thread_count);
    m_job_thread = std::thread(&ClientBase::job_thread_work, this); 
}

void ClientBase::open(std::string connecting_ip, int connecting_port, std::function<ServerSession*()> session_factory, int session_count)
{
    m_session_factory = session_factory;

    for (int i = 0; i < session_count; ++i)
    {
        ServerSession* session = m_session_factory();
        session->init();
        
        
        session->set_id(Session::generate_session_id());
        session->set_socket(NetworkUtil::create_socket());
        session->set_remote_ip(connecting_ip.c_str());
        session->set_remote_port(connecting_port);

        session->set_network_core(this);

        SOCKADDR_IN my_addr;
        my_addr.sin_family = AF_INET;
        my_addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
        my_addr.sin_port = ::htons(0);
        if (false == NetworkUtil::bind(session->get_socket(), my_addr))
        {
            std::cout << "bind error" << std::endl;
            return;
        }
        
        std::cout << "Connecting..." << std::endl;
        session->do_connect();

        m_sessions.emplace(session->get_id(), session);
    }
}

void ClientBase::on_iocp_io(NetworkIO* io, int bytes_transferred)
{
    Session* session = io->get_session();

    switch(io->get_type())
    {
    case IoType::CONNECT:
        session->complete_connect();
        break;
    case IoType::DISCONNECT:
        session->complete_disconnect();
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
    }}

void ClientBase::job_thread_work()
{
    while(is_running() == true)
    {
        Packet* packet;

        if(false == m_packet_queue.try_pop(packet))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        
        Session* session = packet->get_owner();
        
        session->execute_packet(packet);

        xdelete packet;
    }
}

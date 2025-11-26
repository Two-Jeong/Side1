#include "pch.h"

NetworkCore::NetworkCore()
    :m_iocp_handle(nullptr)
{
}

NetworkCore::~NetworkCore()
{
    
}

void NetworkCore::init(int iocp_thread_count)
{
    m_iocp_handle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if(nullptr == m_iocp_handle)
    {
        std::cout << "CreateIoCompletionPort Error" << std::endl;
        // TODO: Crash
        return;
    }
    
    m_is_running = true;
    
    for(int i = 0; i < iocp_thread_count; ++i)
        m_iocp_threads.emplace_back([this](){ iocp_thread_work(); });

}

void NetworkCore::iocp_thread_work()
{
    while(m_is_running == true)
    {
        DWORD bytes_transferred = 0;
        ULONG_PTR key = 0;
        NetworkIO* io = nullptr;
        if(false == ::GetQueuedCompletionStatus(m_iocp_handle, &bytes_transferred, &key, reinterpret_cast<LPOVERLAPPED*>(&io), INFINITE))
        {
            const int err_no = ::WSAGetLastError();
            std::cout << "GQCS error: " << err_no << std::endl; 
            // TODO: error log
            continue;
        }

        on_iocp_io(io, bytes_transferred);
    }
}
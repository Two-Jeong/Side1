#include "pch.h"
#include "LoginServerService.h"

void LoginServerService::init(int iocp_thread_count, std::function<class NetworkSection*()> section_factory,
    int section_count)
{
    ServerBase::init(iocp_thread_count, section_factory, section_count);
}

NetworkSection* LoginServerService::select_first_section()
{
    return m_sections.begin()->second;
}

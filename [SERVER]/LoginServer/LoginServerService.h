#pragma once

class LoginServerService : public ServerBase
{
public:
    LoginServerService() = default;
    ~LoginServerService() override = default;
    
public:
    void init(int iocp_thread_count, int hard_task_thread_count, std::function<class NetworkSection*()> section_factory, int section_count) override;

protected:
    NetworkSection* select_first_section() override;
};
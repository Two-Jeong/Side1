#pragma once
#include "LoginServerConfig.h"

class LoginServerService : public ServerBase
{
public:
    LoginServerService() = default;
    ~LoginServerService() override = default;

public:
    LoginServerConfig& get_config() {return server_config;}
    
public:
    void init(int iocp_thread_count, int hard_task_thread_count, std::function<std::shared_ptr<NetworkSection>()> section_factory, int section_count) override;

protected:
    std::shared_ptr<NetworkSection> select_first_section() override;

private:
    LoginServerConfig server_config;
};

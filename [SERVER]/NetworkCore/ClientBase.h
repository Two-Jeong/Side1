#pragma once

class ClientBase : public NetworkCore
{
public:
    ClientBase() = default;
    virtual ~ClientBase() = default;

public:
    void init(int iocp_thread_count = 1) override;
public:
    void open(std::string connecting_ip, int connecting_port, std::function<class ServerSession*()> session_factory, int
              session_count = 1);
    
protected:
    void on_iocp_io(NetworkIO* io, int bytes_transferred) override;

protected:
    void job_thread_work();

private:
    std::thread m_job_thread;
    
    ConnectIO m_connect_io;
    std::function<class ServerSession*()> m_session_factory;

    std::map<unsigned int, class ServerSession*> m_sessions;
    
};

#pragma once

enum IoType
{
    CONNECT,
    DISCONNECT,
    ACCEPT,
    RECV,
    SEND,
};

class NetworkIO : public OVERLAPPED
{
public:
    NetworkIO(IoType type) : m_io_type(type){ Init(); }
    ~NetworkIO() = default;

    void Init()
    {
       ZeroMemory(this, sizeof(OVERLAPPED));
    }

public:
    Session* get_session() { return m_session; }
    void set_session(Session* session) { m_session = session; }
    IoType get_type() { return m_io_type; }
private:
    Session* m_session;
    IoType m_io_type;
};

class AcceptIO : public NetworkIO
{
public:
    AcceptIO() : NetworkIO(IoType::ACCEPT) { }

    SOCKET m_socket;
    char m_accept_buffer[1024];
};

class ConnectIO : public NetworkIO
{
public:
    ConnectIO() : NetworkIO(IoType::CONNECT) { }

    std::string m_ip;
    int m_port;
};

class RecvIO : public NetworkIO
{
public:
    RecvIO() : NetworkIO(IoType::RECV) { }
};

class SendIO : public NetworkIO
{
public:
    SendIO() : NetworkIO(IoType::SEND) { }

    void Clear()
    {
        Init();
        m_buffers.clear();
    }

    std::vector<WSABUF> m_buffers;
};

class DisconnectIO : public NetworkIO
{
public:
    DisconnectIO() : NetworkIO(IoType::DISCONNECT) { }
};
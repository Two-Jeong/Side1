#pragma once
#include "RecvBuffer.h"

class Session
{
public:
    Session() : m_multi_sender(this)
    {
        m_recv_io.set_session(this);
        m_disconnect_io.set_session(this);
    }
    virtual ~Session()
    {
        finalize();
    }

public:
    static int generate_session_id();
public:
    virtual void init();
    virtual void init_handlers() abstract;
    virtual void finalize();
public:
    int get_id() { return m_session_id; };
    void set_id(int id) { m_session_id = id; };
    bool is_connected() { return m_is_connected; }

    SOCKET get_socket() { return m_connecting_socket; }

    std::string& get_remote_ip(){ return m_remote_ip; }
    int get_remote_port(){ return m_remote_port; }
    void set_socket(SOCKET socket){ m_connecting_socket = socket; }
    void set_remote_ip(const char* ip)
    {
        m_connect_io.m_ip = std::string(ip);
        m_remote_ip = ip;
    }
    void set_remote_ip(std::string&& ip) { set_remote_ip(ip.c_str()); }
    void set_remote_port(int port)
    {
        m_connect_io.m_port = port;
        m_remote_port = port;
    }
    virtual NetworkCore* get_network_core() abstract;
    virtual NetworkSection* get_section() abstract;
public:
    bool do_connect();
    bool do_recieve();
    bool do_send(std::shared_ptr<Packet> packet);
    bool do_send(google::protobuf::Message& message);
    bool do_disconnect();

    void complete_connect();
    void complete_recieve(int bytes_transferred);
    void complete_send(int bytes_transferred);
    void complete_disconnect();
public:
    virtual void on_connected() abstract;
    virtual int on_recieve();
    virtual void on_send(int data_size) abstract;
    virtual void on_disconnected() abstract;
    virtual void execute_packet(Packet* packet);
    

    RecvBuffer& get_recv_buffer() { return m_recv_buffer; }
    RecvIO& get_recv_io(){ return m_recv_io; }
protected:
    int m_session_id;
    bool m_is_connected;
    
    SOCKET m_connecting_socket;
    std::string m_remote_ip;
    int m_remote_port;

    RecvBuffer m_recv_buffer;
    MultiSender m_multi_sender;
    ConnectIO m_connect_io;
    RecvIO m_recv_io;
    DisconnectIO m_disconnect_io;

    std::map<unsigned short, std::function<void(Packet*)>> m_handlers;
};

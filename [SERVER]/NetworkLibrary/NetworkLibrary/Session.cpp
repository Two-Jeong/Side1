#include "pch.h"
#include "Session.h"

int Session::generate_session_id()
{
    static int session_id = 0;

    return ++session_id;
}

void Session::init()
{
    init_handlers();
}

void Session::finalize()
{
    m_recv_buffer.Clean();
    m_connecting_socket = INVALID_SOCKET;
    m_remote_ip = "";
    m_remote_port = 0;
    m_multi_sender.clear();
}

bool Session::do_connect()
{
    NetworkCore* network_core = get_network_core();
    if (nullptr == network_core)
    {
        std::cout << "network_core is nullptr" << std::endl;
        // TODO: LOG
        return false;
    }

    if (false == NetworkUtil::register_socket(network_core->get_iocp_handle(), m_connecting_socket))
    {
        std::cout << "register socket fail" << std::endl;
        // TODO: LOG
        return false;    
    }

    m_connect_io.Init();
    bool is_not_pending = false;
    if (false == NetworkUtil::connect(m_connecting_socket, &m_connect_io, is_not_pending))
    {
        // TODO: LOG
        return false;
    }

    if (is_not_pending)
        complete_connect();
        

    return true;
}

bool Session::do_recieve()
{
    if(false == NetworkUtil::receive(get_socket(), &m_recv_io))
    {
        do_disconnect();
        // TODO:로그
        return false;
    }
    
    return true;
}

bool Session::do_send(std::shared_ptr<Packet> packet)
{
    if (performance_check_mode)
    {
        ClientSession* client_session = dynamic_cast<ClientSession*>(this);
        if (client_session && client_session->get_section())
            client_session->get_section()->increment_send_count_for_tps();
    }
    
    m_multi_sender.register_packet(packet);
    return true;
}

bool Session::do_send(google::protobuf::Message& message)
{
    unsigned short protocol_number = PacketNumberMapper::GetProtocolNumber(message.GetTypeName());

    std::shared_ptr<Packet> packet = std::make_shared<Packet>();
    packet->initialize(protocol_number);
    packet->push(message);
    packet->finalize();

    return do_send(packet);
}

bool Session::do_disconnect()
{
    if (false == NetworkUtil::disconnect(m_connecting_socket, &m_disconnect_io))
        return false;

    std::cout << "do disconnect session id " << get_id() << std::endl;
    return true;
}

void Session::complete_connect()
{
    m_is_connected = true;
    on_connected();

    if (false == do_recieve())
    {
        std::cout << "recieve fail" << std::endl;
        // TODO: LOG
        return;
    }
}

void Session::complete_recieve(int bytes_transferred)
{
    if (bytes_transferred == 0)
    {
        // TODO: LOG
        do_disconnect();
        return;
    }

    if (performance_check_mode)
    {
        ClientSession* client_session = dynamic_cast<ClientSession*>(this);
        if (client_session != nullptr && client_session->get_section() != nullptr)
            client_session->get_section()->increment_recv_count_for_tps();
    }

    if (false == m_recv_buffer.OnWrite(bytes_transferred))
    {
        // TODO: LOG
        do_disconnect();
        return;
    }

    int process_byte_size = on_recieve();

    m_recv_buffer.OnRead(process_byte_size);
    
    if(false == do_recieve())
    {
        // TODO: LOG
        return;
    }
}

void Session::complete_send(int bytes_transferred)
{
    on_send(bytes_transferred);
    m_multi_sender.on_send();
}

void Session::complete_disconnect()
{
    m_is_connected = false;
    closesocket(m_connecting_socket);
    on_disconnected();
}

int Session::on_recieve()
{
    int complete_byte_length = 0;
    
    while(true)
    {
        int remain_len = m_recv_buffer.GetDataSize() - complete_byte_length;
        if(remain_len < PACKET_HEADER_SIZEOF) break;

        PacketHeader header = *(reinterpret_cast<PacketHeader*>(m_recv_buffer.GetReadPos() + complete_byte_length));
        
        if(header.packet_size > remain_len) break;

        Packet* packet = xnew Packet;
        
        packet->set_packet(m_recv_buffer.GetReadPos() + complete_byte_length, header.packet_size);
        packet->set_owner(this);

        
        NetworkCore* network_core = get_network_core();
        if (nullptr == network_core)
        {
            //TODO: LOG
            do_disconnect();
            xdelete packet;
            
            return 0;
        }
    
        network_core->push_packet(packet);
        
        complete_byte_length += header.packet_size;
    }

    return complete_byte_length;
}

void Session::on_disconnected()
{
    
}

void Session::execute_packet(Packet* packet)
{
    if (0 == m_handlers.count(packet->get_protocol()))
    {
        // TODO: 로그
        return;
    }

    m_handlers[packet->get_protocol()](packet);
}

#include "pch.h"
#include "NetworkUtil.h"


extern NetworkUtil* g_network_util = xnew NetworkUtil;

NetworkUtil::NetworkUtil()
{
    WORD req_version = 0;
    req_version = MAKEWORD(2, 2);
    
    WSADATA wsa_data;
    int err_code = WSAStartup(req_version, &wsa_data);
    if (err_code != 0) {
        // TOODO: LOG
        std::cout << "wsa startup fail" << std::endl;
        // TODO: STOP SERVER
        return;
    }
    
    SOCKET temp_socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    DWORD recv_bytes = 0;
    GUID guid = WSAID_DISCONNECTEX;
    
    if (SOCKET_ERROR == ::WSAIoctl(temp_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &DisconnectEx, sizeof(&DisconnectEx), &recv_bytes, NULL, NULL))
    {
        // TODO: LOG 
        // TODO: QUIT PROGRAM 
    }
}

sockaddr* NetworkUtil::get_remote_sockaddr(char* lpOutputBuffer)
{
    sockaddr* local_addr = nullptr;
    sockaddr* remote_addr = nullptr;
    int remote_addr_len = 0;
    int local_addr_len = 0;
    GetAcceptExSockaddrs(lpOutputBuffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &local_addr, &local_addr_len, &remote_addr, &remote_addr_len);

    return remote_addr;
}

SOCKET NetworkUtil::create_socket()
{
    return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

bool NetworkUtil::register_socket(HANDLE iocp_handle, SOCKET socket)
{
    return ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), iocp_handle,0,0);
}

bool NetworkUtil::bind(SOCKET socket, const char* ip, int port)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(addr.sin_addr.s_addr));
    addr.sin_port = htons(port);
    
    if(SOCKET_ERROR == ::bind(socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)))
    {
        // TODO: ERROR LOG
        int err_code = ::WSAGetLastError();
        std::cout << "bind error: " << err_code << std::endl;
        return false;
    }

    return true;
}

bool NetworkUtil::bind(SOCKET socket, SOCKADDR_IN addr)
{
    if(SOCKET_ERROR == ::bind(socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)))
    {
        // TODO: ERROR LOG
        int err_code = ::WSAGetLastError();
        std::cout << "bind error: " << err_code << std::endl;
        return false;
    }

    return true;
}

bool NetworkUtil::listen(SOCKET socket, int backlog)
{
    if(SOCKET_ERROR == ::listen(socket, backlog))
    {
        int err_code = GetLastError();
        std::cout << "listen error: " << err_code << std::endl;
        return false;
    }
    
    return true;
}

bool NetworkUtil::accept(SOCKET listen_socket, AcceptIO* io)
{
    constexpr DWORD addr_length = sizeof(sockaddr_in) + 16;
    DWORD dwBytes = 0;
    

    if(false == ::AcceptEx(listen_socket, io->m_socket, reinterpret_cast<PVOID>(io->m_accept_buffer),0
        , addr_length, addr_length,
        &dwBytes, reinterpret_cast<LPOVERLAPPED>(io)))
    {
		const int err_no = ::WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
		{
		    std::cout << "accept error: " << err_no << std::endl;
		    // TODO: Error Log
			return false;
		}
	}
    
	return true;
}

bool NetworkUtil::connect(SOCKET socket, ConnectIO* io, bool& is_not_pending)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, io->m_ip.c_str(), &(addr.sin_addr.s_addr));
    addr.sin_port = htons(io->m_port);

    auto result = ::WSAConnect(socket,reinterpret_cast<sockaddr*>(&addr), sizeof(addr), nullptr, nullptr, nullptr, nullptr);

    if (0 == result)
        is_not_pending = true;
    else if (SOCKET_ERROR == result)
    {
        const int err_no = ::WSAGetLastError();
        if(WSA_IO_PENDING != err_no)
        {
		    std::cout << "connect error: " << err_no << std::endl;
            // TODO: Error LOG
            return false;
        }
    }


    return true;
}

bool NetworkUtil::send(SendIO* io, bool& is_not_pending, DWORD& send_byte_size)
{
    auto result = ::WSASend(io->get_session()->get_socket(), io->m_buffers.data(), static_cast<DWORD>(io->m_buffers.size()), &send_byte_size, 0, io,  nullptr);

    if(result == 0)
        is_not_pending = true;
    else if(SOCKET_ERROR == result)
    {
        auto err_code = ::WSAGetLastError();

        if(err_code != ERROR_IO_PENDING)
        {
		    std::cout << "send error: " << err_code << std::endl;
            // TODO: log 
            return false;
        }
    }
    
    return true;
}

bool NetworkUtil::receive(SOCKET socket, RecvIO* io)
{
    WSABUF buf;
    buf.buf = io->get_session()->get_recv_buffer().GetReadPos();
    buf.len = io->get_session()->get_recv_buffer().GetRemainingSize();

    DWORD recv_bytes = 0;
    DWORD flag = 0;

    if(0 != WSARecv(socket, &buf, 1, &recv_bytes, &flag, io, nullptr))
    {
        int err_code = ::GetLastError();

        if(err_code != WSA_IO_PENDING)
        {
		    std::cout << "recv error: " << err_code << std::endl;
            // TODO: 로그
            return false;
        }
    }
    return true;
}

bool NetworkUtil::disconnect(SOCKET socket, class DisconnectIO* io)
{
    if (false == g_network_util->DisconnectEx(socket, io, TF_REUSE_SOCKET, 0))
    {
        int err_code = ::GetLastError();
        if (err_code != ERROR_IO_PENDING)
        {
		    std::cout << "disconnect error: " << err_code << std::endl;
            // TODO: 로그
            return false;
        }
    }
    return true;
}
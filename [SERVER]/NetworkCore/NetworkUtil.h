#pragma once

class NetworkUtil
{
public:
   NetworkUtil();
   ~NetworkUtil();

public:
public:
   static bool socket_opt_setting(SOCKET socket);
   static sockaddr* get_remote_sockaddr(char* lpOutputBuffer);
   static SOCKET create_socket();
   static bool register_socket(HANDLE iocp_handle, SOCKET socket);
   static bool bind(SOCKET socket, const char* ip, int port);
   static bool bind(SOCKET socket, SOCKADDR_IN addr);
   static bool listen(SOCKET socket, int backlog = 1);
   static bool accept(SOCKET listen_socket, class AcceptIO* io);
   static bool connect(SOCKET socket, class ConnectIO* io, bool& is_not_pending);
   static bool send(class SendIO* io, bool& is_not_pending, DWORD& send_byte_size);
   static bool receive(SOCKET socket, class RecvIO* io);
   static bool disconnect(SOCKET socket, class DisconnectIO* io);

public:
   LPFN_DISCONNECTEX DisconnectEx;
};

extern NetworkUtil* g_network_util;
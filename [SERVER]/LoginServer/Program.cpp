#include "pch.h"

#include "LoginClientSession.h"
#include "LoginServerService.h"

int main()
{
    LoginServerService service;
    service.init(1, 1, [](){ return xnew NetworkSection; }, 1);
    service.open(service.get_config().login_server_ip, service.get_config().login_server_port, [](){return xnew LoginClientSession; });

    while (true) {}
}
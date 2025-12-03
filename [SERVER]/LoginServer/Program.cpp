#include "pch.h"

#include "LoginClientSession.h"
#include "LoginServerService.h"

int main()
{
    LoginServerService service;
    service.init(1, [](){ return xnew NetworkSection; }, 1);
    service.open("127.0.0.1", 9010, [](){return xnew LoginClientSession; });

    while (true) {}
}

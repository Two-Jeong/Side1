#include "pch.h"

#include "LoginClientSession.h"
#include "LoginServerService.h"

int main()
{
    LoginServerService service;
    service.init(1, 1, [](){ return xnew NetworkSection; }, 1);
    service.open("0.0.0.0", 25000, [](){return xnew LoginClientSession; });

    while (true) {}
}
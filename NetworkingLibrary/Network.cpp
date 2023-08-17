#include "Network.h"
#include <iostream>

namespace Networking {
    bool Network::initialize()  //initialize WSA Api
    {
        WSADATA wsadata;
        int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
        if (result != 0) {
            std::cerr << "Failed to start up the winsock API." << "\n";
            return false;
        }
        if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
            std::cerr << "Could not find a usable version of the winsock api dll." << "\n";
            return false;
        }

        return true;
    }

    void Network::shutdown()  //Shutdown WSA Api
    {
        WSACleanup();
    }
}

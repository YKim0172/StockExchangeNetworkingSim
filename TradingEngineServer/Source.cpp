#include <NetworkingLibrary\Networking.h>
#include <iostream>
#include "Server.h"

using namespace Networking;
int main() {
	Server server;
	if (server.InitializeServer()) {
		while (true) {
			server.frame();
		}

		server.CloseServer();
	}
	else {
		std::cout << "WSA Startup failed" << "\n";
	}
}


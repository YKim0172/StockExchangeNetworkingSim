#include "Client.h"
#include <iostream>
using namespace Networking;

int main() {
	Client client;
	if (client.connectToServer(IPEndpoint("127.0.0.1", 4790))) {
		while (true) {
			client.frame();
		}
	}
	Network::shutdown();
	system("pause");
}

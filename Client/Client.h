#pragma once
#include <NetworkingLibrary\Networking.h>
#include <string>
namespace Networking {
	class Client {
	public:
		bool connectToServer(IPEndpoint ip);
		bool isConnected();
		bool frame();
		bool processPacket(std::shared_ptr<Packet> packet);
		bool closeConnection(std::string reason);

	private:
		bool connected = false;
		Socket clientSocket;
		TCPConnection connection;
		WSAPOLLFD master_fd;
		WSAPOLLFD use_fd;
	};
}


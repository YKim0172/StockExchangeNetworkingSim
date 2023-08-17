#pragma once
#include <NetworkingLibrary\Networking.h>
#include "StockExchange.h"
namespace Networking {
	class Server {
	public:
		Server(): listeningSocket(INVALID_SOCKET) {}
		bool InitializeServer();
		void CloseServer();
		void frame();


	private:
		void closeConnection(int connectionIndex, std::string reason);
		bool processOrderPacket(std::shared_ptr<Packet> packet, uint32_t connection_id);
		Socket listeningSocket;
		std::vector<TCPConnection> connections;  //list of client connections
		std::vector<WSAPOLLFD> master_fd;  //every single socket, including the listening socket, must have a WSAPOLLFD so we can poll for data and do stuff
		std::vector<WSAPOLLFD> use_fd;  // will be used just as master_fd
		uint32_t connection_id = 0;
		StockExchange exchange;

	};
}

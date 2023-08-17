#pragma once
#include "Socket.h"
#include "PacketManager.h"
namespace Networking {
	class TCPConnection {
	public:
		TCPConnection();
		TCPConnection(Socket socket, IPEndpoint endpoint);
		void setConnectionID(uint32_t newConnectionID);
		uint32_t getConnectionID();
		void Close();
		std::string ToString();
		Socket socket;

		PacketManager pm_incoming;  //handling packets to accept
		PacketManager pm_outgoing;  //handling packets to send
		char buffer[8192];
	private:
		IPEndpoint endpoint;
		std::string stringRepresentation = "";
		uint32_t connection_id;
	};
}
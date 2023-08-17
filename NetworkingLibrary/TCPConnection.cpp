#include "TCPConnection.h"
#include <iostream>
namespace Networking {

	TCPConnection::TCPConnection() {

	}


	TCPConnection::TCPConnection(Socket socket, IPEndpoint endpoint)
		:socket(socket), endpoint(endpoint)
	{
		stringRepresentation = "[" + endpoint.getIPString();
		stringRepresentation += ":" + std::to_string(endpoint.getPort()) + "]";

	}

	void TCPConnection::setConnectionID(uint32_t newConnectionID)
	{
		connection_id = newConnectionID;
	}

	uint32_t TCPConnection::getConnectionID()
	{
		return connection_id;
	}

	void TCPConnection::Close()
	{
		socket.closeSocket();
	}

	std::string TCPConnection::ToString()
	{
		return stringRepresentation;
	}
}
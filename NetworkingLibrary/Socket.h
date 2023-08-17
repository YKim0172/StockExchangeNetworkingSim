#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include "Result.h"
#include "IPEndpoint.h"
#include "Packet.h"

namespace Networking {
	class Socket {
	public:
		//Constructors and getting socketObj
		Socket(): socketObj(INVALID_SOCKET) {}
		Socket(SOCKET socketIn) : socketObj(socketIn) {}
		SOCKET getSocketObj();

		//creating socket objects and socket operations
		Result createSocket(bool isBlocking);
		Result setBlocking(bool isBlocking);
		Result closeSocket();
		Result bindSocket(IPEndpoint endpoint);
		Result listenSocket(IPEndpoint endpoint, int backlog = 5);
		Result acceptSocket(Socket& newConnectionSocket, IPEndpoint* endpoint = nullptr);
		Result connectSocket(IPEndpoint endpoint);

		//sending data
		Result sendAll(const void* data, int numberOfBytes);
		Result recvAll(void* destination, int numberOfBytes);
		Result sendPacket(Packet& packet);
		Result recvPacket(Packet& packet);
				
	private:
		SOCKET socketObj;
	};
}
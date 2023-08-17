#pragma once
#include <string>
#include <vector>
#include <WS2tcpip.h>
namespace Networking {
	class IPEndpoint {
	public:
		IPEndpoint() {}
		IPEndpoint(const char* ip, unsigned short port);
		IPEndpoint(sockaddr* addr);
		std::vector<uint8_t> getIPBytes();
		std::string getHostname();
		std::string getIPString();
		unsigned short getPort();
		sockaddr_in getSockaddrIPv4();
		void print();
	private:
		//ipv4 is used
		std::string hostname = "";  //www.google.com
		std::string ip_string = "";  //ip address as string
		std::vector<uint8_t> ip_bytes;  //ip address but each digit is an element in the vector
		unsigned short port = 0;  //the port number
	};
}
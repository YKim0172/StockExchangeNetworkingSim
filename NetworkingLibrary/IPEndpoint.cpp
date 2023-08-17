#include "IPEndpoint.h"
#include <iostream>

Networking::IPEndpoint::IPEndpoint(const char* ip, unsigned short port)
{
	this->port = port;  //set port number
	in_addr addr; //in_addr stores IPv4 internet address
	int result = inet_pton(AF_INET, ip, &addr);  //presentation format -> network format

	if (result == 1) {
		if (addr.S_un.S_addr != INADDR_NONE) {
			ip_string = ip;
			hostname = ip;

			ip_bytes.resize(sizeof(ULONG));  //ULONG size is 4 bytes, since we using IPV4
			memcpy(&ip_bytes[0], &addr.S_un.S_addr, sizeof(ULONG));
			return;
		}
	}

	//attempt to resovle hostname to ipv4 address;
	//getaddrinfo translate ANSI host name to address
	addrinfo hints = {}; //hints will filter the results we get back for getaddrinfo
	hints.ai_family = AF_INET; //ipv4 address only
	addrinfo* hostinfo = nullptr;
	result = getaddrinfo(ip, NULL, &hints, &hostinfo);
	if (result == 0) {
		sockaddr_in* host_addr = reinterpret_cast<sockaddr_in*>(hostinfo->ai_addr);  //get the ip address

		//host_addr->sin_addr.S_un.S_addr network presentation
		ip_string.resize(16);
		inet_ntop(AF_INET, &host_addr->sin_addr, &ip_string[0], 16);  //set ip_string value
		hostname = ip;

		ULONG ip_long = host_addr->sin_addr.S_un.S_addr;
		ip_bytes.resize(sizeof(ULONG));
		memcpy(&ip_bytes[0], &ip_long, sizeof(ULONG));  //set ip_bytes vector

		freeaddrinfo(hostinfo);
		return;
	}
}

Networking::IPEndpoint::IPEndpoint(sockaddr* addr)
{
	//prints the sockaddr info of connected user's socket
	sockaddr_in* addrv4 = reinterpret_cast<sockaddr_in*>(addr);
	//ipversion is ipv4
	port = ntohs(addrv4->sin_port);  //network to host byte order conversation
	ip_bytes.resize(sizeof(ULONG));
	memcpy(&ip_bytes[0], &addrv4->sin_addr, sizeof(ULONG));
	ip_string.resize(16);
	inet_ntop(AF_INET, &addrv4->sin_addr, &ip_string[0], 16);  //set ip_string value
	hostname = ip_string;
}

std::vector<uint8_t> Networking::IPEndpoint::getIPBytes()
{
	return ip_bytes;
}

std::string Networking::IPEndpoint::getHostname()
{
	return hostname;
}

std::string Networking::IPEndpoint::getIPString()
{
	return ip_string;
}

unsigned short Networking::IPEndpoint::getPort()
{
	return port;
}

sockaddr_in Networking::IPEndpoint::getSockaddrIPv4()
{
	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	memcpy(&addr.sin_addr, &ip_bytes[0], sizeof(ULONG));
	addr.sin_port = htons(port);
	return addr;
}

void Networking::IPEndpoint::print()
{
	std::cout << "Hostname: " << hostname << "\n";
	std::cout << "IP:" << ip_string << "\n";
	std::cout << "Port: " << port << "\n";
}

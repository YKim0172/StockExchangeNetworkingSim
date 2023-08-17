#pragma once
#define WIN32_LEAN_AND_MEAN
#include <vector>
#include <string>
#include <winsock2.h>

namespace Networking {
	class Packet {
	public:
		void clearData();
		void appendData(const void* data, uint32_t size);

		Packet& operator << (uint32_t data);
		Packet& operator >> (uint32_t& data);

		Packet& operator << (const std::string& data);
		Packet& operator >> (std::string& data);

		uint32_t extractionOffset = 0;
		std::vector<char> buffer;

	};
}
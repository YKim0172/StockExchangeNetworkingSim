#include "Packet.h"

namespace Networking {
	void Packet::clearData()
	{
		buffer.clear();
		extractionOffset = 0;

	}
	void Packet::appendData(const void* data, uint32_t size)  //this is essentially going to add more data to the payload
	{
		buffer.insert(buffer.end(), (char*)data, (char*)data + size);
	}
	Packet& Packet::operator<<(uint32_t data)
	{
		data = htonl(data);
		appendData(&data, sizeof(uint32_t));
		return *this;
	}
	Packet& Packet::operator>>(uint32_t& data)
	{
		data = *reinterpret_cast<uint32_t*>(&buffer[extractionOffset]);
		data = ntohl(data);
		extractionOffset += sizeof(uint32_t);
		return *this;
	}
	Packet& Packet::operator<<(const std::string& data)
	{
		//we will put the size of the string in our buffer FIRST
		*this << (uint32_t)data.size();
		//then we will put the actual string
		appendData(data.data(), data.size());
		return *this;
	}
	Packet& Packet::operator>>(std::string& data)
	{
		data.clear();

		//read in string size
		uint32_t stringSize = 0;
		*this >> stringSize;

		//now resize string buffer and read in the string that's being sent
		data.resize(stringSize);
		data.assign(&buffer[extractionOffset], stringSize);
		extractionOffset += stringSize;
		return *this;
	}
}

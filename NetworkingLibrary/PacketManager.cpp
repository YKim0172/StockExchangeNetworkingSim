#include "PacketManager.h"

namespace Networking {
	void PacketManager::clear()
	{
		packets = std::queue<std::shared_ptr<Packet>>{}; //Clear out packet queue
	}

	bool PacketManager::hasPendingPackets()
	{
		return (!packets.empty());  //returns true if packets are pending
	}

	void PacketManager::append(std::shared_ptr<Packet> p)
	{
		packets.push(std::move(p));  //add packet to queue
	}

	std::shared_ptr<Packet> PacketManager::retrieve()
	{
		std::shared_ptr<Packet> p = packets.front();
		return p;
	}

	void PacketManager::pop()
	{
		packets.pop();
	}
}
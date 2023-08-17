#pragma once
#include <queue>
#include <memory>
#include "Packet.h"

namespace Networking {
	enum PacketManagerTask {
		ProcessPacketSize,
		ProcessPacketContents
	};

	class PacketManager {
	private:
		std::queue<std::shared_ptr<Packet>> packets;
	public:
		void clear();
		bool hasPendingPackets();
		void append(std::shared_ptr<Packet> p);
		std::shared_ptr<Packet> retrieve();
		void pop();

		uint16_t currentPacketSize = 0;
		int currentPacketExtractionOffset = 0;
		PacketManagerTask currentTask = PacketManagerTask::ProcessPacketSize;
	};

}
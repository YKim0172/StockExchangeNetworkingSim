#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>

namespace Networking {
	class Network {
	public:
		static bool initialize();  //start WSA Api
		static void shutdown();  //close WSA Api
	};
}
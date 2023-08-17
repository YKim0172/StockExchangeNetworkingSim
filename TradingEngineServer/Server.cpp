#include "Server.h"
#include <iostream>

namespace Networking {
    bool Server::InitializeServer()
    {
        //start WSA
        if (Networking::Network::initialize()) {  // if successful in starting WSA
            listeningSocket = Socket();

            //now create the listening socket
            if (listeningSocket.createSocket(false) == Result::SUCCESS) {
                if (listeningSocket.listenSocket(IPEndpoint("127.0.0.1", 4790)) == Result::SUCCESS) {
					WSAPOLLFD listeningSocketFD = {};
					listeningSocketFD.fd = listeningSocket.getSocketObj();
					listeningSocketFD.events = POLLRDNORM; // we are only going to be listening for info from out listneing socket
					listeningSocketFD.revents = 0;
					master_fd.push_back(listeningSocketFD);
					std::cout << "Server started successfully and is ready to receive clients." << "\n";
					return true; //server has been initialized and listening socket is in listening state

                }
                listeningSocket.closeSocket();
            }
            return true;
        }
        else {  //if WSA startup not successful
            return false;
        }
    }

    void Server::CloseServer()  //close the WSA API after done
    {
        Networking::Network::shutdown();
    }
    void Server::frame()
    {
		use_fd = master_fd;  //so the very first FD is for the listening socket, but after is for the individual clients
		if (WSAPoll(use_fd.data(), use_fd.size(), 1) > 0) {//how to poll for data to be 1
			//listening socket
			WSAPOLLFD& listeningSocketFD = use_fd[0];
			if (listeningSocketFD.revents & POLLRDNORM) {  //we can accept connection
				Socket newConnectionSocket;
				IPEndpoint newConnectionEndpoint;
				if (listeningSocket.acceptSocket(newConnectionSocket, &newConnectionEndpoint) == Result::SUCCESS) {

					connections.emplace_back(TCPConnection(newConnectionSocket, newConnectionEndpoint));
					TCPConnection& acceptedConnection = connections[connections.size() - 1];
					acceptedConnection.setConnectionID(connection_id++);
					std::cout << acceptedConnection.ToString() << " - NEW CONNECTION ACCEPTED ID: " << acceptedConnection.getConnectionID() << "\n";

					//for future reference when reading on this client's socket
					WSAPOLLFD newConnectionFD = {};
					newConnectionFD.fd = newConnectionSocket.getSocketObj();
					newConnectionFD.events = POLLRDNORM | POLLWRNORM;
					newConnectionFD.revents = 0;
					master_fd.push_back(newConnectionFD);
				}
				else {
					std::cerr << "Failed to accept new connection." << "\n";
				}
			}
			//done with listening socket, now work on the clients
			for (int i = use_fd.size() - 1; i >= 1; i--) { //very first file descriptor is for the listening socket
				int connectionIndex = i - 1;
				TCPConnection& connection = connections[connectionIndex];
				//check for any errors
				if (master_fd[i].revents & POLLERR) { // if error occured on this socket
					//remove this connection from our list of connections for clients
					closeConnection(connectionIndex, "POLLERR");
					continue;
				}

				if (use_fd[i].revents & POLLHUP) { // if poll hangup occured on this socket
					//remove this connection from our list of connections for clients
					closeConnection(connectionIndex, "POLLHUP");
					continue;
				}

				if (use_fd[i].revents & POLLNVAL) { // if invalid socket occured on this socket
					//remove this connection from our list of connections for clients
					closeConnection(connectionIndex, "POLLNVAL");
					continue;
				}

				//done error checking
				//now check if data can be read

				if (use_fd[i].revents & POLLRDNORM) {

					int bytesReceived = 0;
					//first check to see if we need to read in packet size or packet contents
					if (connection.pm_incoming.currentTask == PacketManagerTask::ProcessPacketSize) {
						bytesReceived = recv(use_fd[i].fd, (char*)&connection.pm_incoming.currentPacketSize + connection.pm_incoming.currentPacketExtractionOffset,
							                 sizeof(uint16_t) - connection.pm_incoming.currentPacketExtractionOffset, 0);
					}
					else {  //process the packet contents
						bytesReceived = recv(use_fd[i].fd, (char*)&connection.buffer + connection.pm_incoming.currentPacketExtractionOffset,
							                 connection.pm_incoming.currentPacketSize - connection.pm_incoming.currentPacketExtractionOffset, 0);
					}
					if (bytesReceived == 0) {  //if connection is lost
						closeConnection(connectionIndex, "Recv received 0 bytes");
						continue;
					}

					if (bytesReceived == SOCKET_ERROR) {  //if error occured on socket
						int error = WSAGetLastError();
						if (error != WSAEWOULDBLOCK) {  //if there is an actual error
							closeConnection(connectionIndex, "There'ssa socket error");
							continue;
						}
					}
					//we receive some kind of data
					if (bytesReceived > 0) {
						connection.pm_incoming.currentPacketExtractionOffset += bytesReceived;
						if (connection.pm_incoming.currentTask == PacketManagerTask::ProcessPacketSize) {  //process the packet size
							if (connection.pm_incoming.currentPacketExtractionOffset == sizeof(uint16_t)) {
								connection.pm_incoming.currentPacketSize = ntohs(connection.pm_incoming.currentPacketSize);
								if (connection.pm_incoming.currentPacketSize > 8192) {
									closeConnection(connectionIndex, "Packet size is too large.");
									continue;
								}
								connection.pm_incoming.currentPacketExtractionOffset = 0;
								connection.pm_incoming.currentTask = PacketManagerTask::ProcessPacketContents;
							}
						}
						else {  //process the packet contents
							if (connection.pm_incoming.currentPacketExtractionOffset == connection.pm_incoming.currentPacketSize) {
								std::shared_ptr<Packet> packet = std::make_shared<Packet>();
								packet->buffer.resize(connection.pm_incoming.currentPacketSize);
								memcpy(&packet->buffer[0], connection.buffer, connection.pm_incoming.currentPacketSize);

								connection.pm_incoming.append(packet);  //append to packet manager


								connection.pm_incoming.currentPacketSize = 0;
								connection.pm_incoming.currentPacketExtractionOffset = 0;
								connection.pm_incoming.currentTask = PacketManagerTask::ProcessPacketSize;
							}
						}
					}
				}

				if (use_fd[i].revents & POLLWRNORM) {
					PacketManager& pm = connection.pm_outgoing;
					while (pm.hasPendingPackets()) {  //check to see if there are any packets that need to be sent
						if (pm.currentTask == PacketManagerTask::ProcessPacketSize) {  //sending packet size
							pm.currentPacketSize = pm.retrieve()->buffer.size();
							uint16_t bigEndianPacketSize = htons(pm.currentPacketSize);
							int bytesSent = send(use_fd[i].fd, (char*)(&bigEndianPacketSize) + pm.currentPacketExtractionOffset, sizeof(uint16_t) - pm.currentPacketExtractionOffset, 0);
							if (bytesSent > 0) {
								pm.currentPacketExtractionOffset += bytesSent;
							}
							if (pm.currentPacketExtractionOffset == sizeof(uint16_t)) {  //if full packet size was sent
								pm.currentPacketExtractionOffset = 0;
								pm.currentTask = PacketManagerTask::ProcessPacketContents;
							}
							else {  //if full packet size was not sent, break out of the loop for sending outgoing packets
								break;
							}
						}
						else { //sending packet contetnts
							char* bufferPtr = &pm.retrieve()->buffer[0];
							int bytesSent = send(use_fd[i].fd, (char*)(bufferPtr)+pm.currentPacketExtractionOffset, pm.currentPacketSize - pm.currentPacketExtractionOffset, 0);
							if (bytesSent > 0) {
								pm.currentPacketExtractionOffset += bytesSent;
							}
							if (pm.currentPacketExtractionOffset == pm.currentPacketSize) {  //if full packet size was sent
								pm.currentPacketExtractionOffset = 0;
								pm.currentTask = PacketManagerTask::ProcessPacketSize;
								pm.pop();
							}
							else {
								break;
							}

						}
					}
				}
			}
			//check if there's pending packets and process them
			for (int i = connections.size() - 1; i >= 0; i--) {
				while (connections[i].pm_incoming.hasPendingPackets()) {
					std::shared_ptr<Packet> frontPacket = connections[i].pm_incoming.retrieve();
					if (!processOrderPacket(frontPacket, (connections[i]).getConnectionID() ) ) {
						closeConnection(i, "Failed to process incoming packet.");
						break;
					}
					connections[i].pm_incoming.pop();
				}
			}
		}
    }
    void Server::closeConnection(int connectionIndex, std::string reason)
    {
        TCPConnection& connection = connections[connectionIndex];
        std::cout << "[" << reason << "] Connection lost: " << connection.ToString() << "." << "\n";
        master_fd.erase(master_fd.begin() + (connectionIndex + 1));
        use_fd.erase(use_fd.begin() + (connectionIndex + 1));
        connection.Close();
        connections.erase(connections.begin() + connectionIndex);

    }

    bool Server::processOrderPacket(std::shared_ptr<Packet> packet, uint32_t connection_id) //process the packet and then make the stock exchange
    {
        std::string ticker = "";
		std::string buyOrSellStr = "";
		bool buyOrSell;
        std::string priceStr = "";
		double price;
		uint32_t quantity;
        *packet >> ticker >> buyOrSellStr >> priceStr >> quantity;
		buyOrSell = buyOrSellStr == "BUY" ? true : false;
		price = stod(priceStr);
		//create a new order in the exchange
		if (!exchange.createOrder(buyOrSell, price, quantity, connection_id, ticker)) {
			std::cerr << "Error creating order" << "\n";
		}
		exchange.printAllData();
        return true;
    }
}
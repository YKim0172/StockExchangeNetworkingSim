#include "Client.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <fstream>
namespace Networking {
    bool Client::connectToServer(IPEndpoint ip)
    {
        connected = false;
        if (Network::initialize()) {
            clientSocket = Socket();
            if (clientSocket.createSocket(true) == Result::SUCCESS) {
                //now connect
                if (clientSocket.connectSocket(ip) == Result::SUCCESS) {
                    if (clientSocket.setBlocking(false) == Result::SUCCESS) {
                        connection = TCPConnection(clientSocket, ip);
                        master_fd.fd = connection.socket.getSocketObj();
                        master_fd.events = POLLRDNORM | POLLWRNORM;
                        master_fd.revents = 0;
                        connected = true;
                        //send something to server
                        std::cout << "Successfully connected to server!" << "\n";
                        return true;
                    } else {
                    }
                }
                else {
                    std::cerr << "Failed to connect to server." << "\n";
                }

                clientSocket.closeSocket();
            }
            else {
				return false;
            }
        }
        return false;
    }


    bool Client::isConnected()
    {
        return connected;
    }

    bool Client::frame()
    {
		use_fd = master_fd;   //we only have one connection for client so just use one FD
		if (WSAPoll(&use_fd, 1, 1) > 0) {//how to poll for data to be 1
			//check for any errors
			if (use_fd.revents & POLLERR) { // if error occured on this socket
				//remove this connection from our list of connections for clients
				closeConnection("POLLERR");
				return false;
			}

			if (use_fd.revents & POLLHUP) { // if poll hangup occured on this socket
				//remove this connection from our list of connections for clients
				closeConnection("POLLHUP");
				return false;
			}

			if (use_fd.revents & POLLNVAL) { // if invalid socket occured on this socket
				//remove this connection from our list of connections for clients
				closeConnection("POLLNVAL");
				return false;
			}

			//done error checking
			//now check if data can be read

			if (use_fd.revents & POLLRDNORM) {

				int bytesReceived = 0;
				//first check to see if we need to read in packet size or packet contents
				if (connection.pm_incoming.currentTask == PacketManagerTask::ProcessPacketSize) {
					bytesReceived = recv(use_fd.fd, (char*)&connection.pm_incoming.currentPacketSize + connection.pm_incoming.currentPacketExtractionOffset, sizeof(uint16_t) - connection.pm_incoming.currentPacketExtractionOffset, 0);
					//std::cout << "reading packet size: " << bytesReceived << "\n";
				}
				else {  //process the packet contents
					bytesReceived = recv(use_fd.fd, (char*)&connection.buffer + connection.pm_incoming.currentPacketExtractionOffset, connection.pm_incoming.currentPacketSize - connection.pm_incoming.currentPacketExtractionOffset, 0);
					//std::cout << "reading packet contents: " << bytesReceived << "\n";
				}
				if (bytesReceived == 0) {  //if connection is lost
					closeConnection("Recv==0");
					return false;
				}

				if (bytesReceived == SOCKET_ERROR) {  //if error occured on socket
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK) {  //if there is an actual error
						closeConnection("Recv<0");
						return false;
					}
				}
				//we receive some kind of data
				if (bytesReceived > 0) {
					connection.pm_incoming.currentPacketExtractionOffset += bytesReceived;
					if (connection.pm_incoming.currentTask == PacketManagerTask::ProcessPacketSize) {  //process the packet size
						if (connection.pm_incoming.currentPacketExtractionOffset == sizeof(uint16_t)) {
							connection.pm_incoming.currentPacketSize = ntohs(connection.pm_incoming.currentPacketSize);
							if (connection.pm_incoming.currentPacketSize > 8192) {
								closeConnection("Packet size too large.");
								return false;
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

			//ask user to enter there order
			std::shared_ptr<Packet> packageOrder = std::make_shared<Packet>();
			std::string str = "";

			std::cout << "Enter Ticker: ";
			std::cin >> str;
			*packageOrder << str;
			

			std::cout << "Do you want to BUY or SELL: ";
			std::cin >> str;
			*packageOrder << str;

			std::cout << "Enter price: ";
			std::cin >> str;
			*packageOrder << str;

			uint32_t floatValue;
			std::cout << "Enter quantity of shares: ";
			std::cin >> floatValue;

			*packageOrder << floatValue;


			connection.pm_outgoing.append(packageOrder);


			if (use_fd.revents & POLLWRNORM) {
				PacketManager& pm = connection.pm_outgoing;
				while (pm.hasPendingPackets()) {  //check to see if there are any packets that need to be sent
					if (pm.currentTask == PacketManagerTask::ProcessPacketSize) {  //sending packet size
						pm.currentPacketSize = pm.retrieve()->buffer.size();
						uint16_t bigEndianPacketSize = htons(pm.currentPacketSize);
						int bytesSent = send(use_fd.fd, (char*)(&bigEndianPacketSize) + pm.currentPacketExtractionOffset, sizeof(uint16_t) - pm.currentPacketExtractionOffset, 0);
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
						int bytesSent = send(use_fd.fd, (char*)(bufferPtr)+pm.currentPacketExtractionOffset, pm.currentPacketSize - pm.currentPacketExtractionOffset, 0);
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
			//check if there's pending packets and process them
			while (connection.pm_incoming.hasPendingPackets()) {
				std::shared_ptr<Packet> frontPacket = connection.pm_incoming.retrieve();
				if (!processPacket(frontPacket)) {
					closeConnection("Failed to process incoming packet.");
					return false;
				}
				connection.pm_incoming.pop();
			}
		}
    }

    bool Client::processPacket(std::shared_ptr<Packet> packet)
    {
		std::string str = "";
		*packet >> str;
		std::cout << "Message from server: " << str << "\n";
		return true;
    }

    bool Client::closeConnection(std::string reason)
    {
        return false;
    }
}

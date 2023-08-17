#include "Socket.h"
#include <iostream>
namespace Networking {
    SOCKET Networking::Socket::getSocketObj()  //returns 
    {
        return socketObj;
    }
    Result Socket::createSocket(bool isBlocking)  //create the socket object
    {
        //first create socket object
        socketObj = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //creating the socket
        if (socketObj == INVALID_SOCKET) {
            int error = WSAGetLastError();
            std::cout << "Error creating socket." << "\n";
            return Result::FAIL;
        }
        //now set the blocking type
        Result setBlockingResult = setBlocking(isBlocking);
        if (setBlockingResult != Result::SUCCESS) {  //if there's error with setting blocking type
            std::cout << "Error setting socket while creating new socket." << "\n";
            return Result::FAIL;
        }
        //now disable nagle's algorithm
        BOOL value = TRUE;
        int result = setsockopt(socketObj, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
        if (result != 0) {
            int error = WSAGetLastError();
            std::cout << "Issue disabling Nagle's algorithm" << "\n";
            return Result::FAIL;
        }
        //everything is fine, and socket is successfully created and configured
        return Result::SUCCESS;
    }

    Result Socket::setBlocking(bool isBlocking)
    {
        unsigned long nonBlocking = 1;
        unsigned long blocking = 0;
        int result = ioctlsocket(socketObj, FIONBIO, isBlocking ? &blocking : &nonBlocking);
        if (result == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            return Result::FAIL;
        }
        return Result::SUCCESS;
    }

    Result Socket::closeSocket()
    {
        int result = closesocket(socketObj);
        if (result != 0) {
            int error = WSAGetLastError();
            return Result::FAIL;
        }
        socketObj = INVALID_SOCKET;
        return Result::SUCCESS;
    }

    Result Socket::bindSocket(IPEndpoint endpoint)
    {
        sockaddr_in addr = endpoint.getSockaddrIPv4();  //this sockaddr_in contains all the info to call bind
        int result = bind(socketObj, (sockaddr*)&addr, sizeof(sockaddr_in));  //bind the socket
        if (result != 0) {  //if an error occured 
            int error = WSAGetLastError();
            return Result::FAIL;
        }
        return Result::SUCCESS;
    }
    Result Socket::listenSocket(IPEndpoint endpoint, int backlog)
    {
        if (bindSocket(endpoint) != Result::SUCCESS) {  //if there's an issue binding to the listening socket
            return Result::FAIL;
        }

        int result = listen(socketObj, backlog);
        if (result != 0) {
            int error = WSAGetLastError();
            return Result::FAIL;
        }

        return Result::SUCCESS;
    }
    Result Socket::acceptSocket(Socket& newConnectionSocket, IPEndpoint* endpoint)
    {
        sockaddr_in addr = {};
        int length = sizeof(sockaddr_in);
        SOCKET acceptedConnectionSocket = accept(socketObj, (sockaddr*)(&addr), &length);
        if (acceptedConnectionSocket == INVALID_SOCKET) {
            int error = WSAGetLastError();
            return Result::FAIL;
        }

        if (endpoint != nullptr) {
            *endpoint = IPEndpoint((sockaddr*)(&addr));
        }

        IPEndpoint newConnectionEndpoint((sockaddr*)&addr);
        std::cout << "New connection accepted!" << "\n";
        newConnectionEndpoint.print();
        newConnectionSocket = Socket(acceptedConnectionSocket);
        return Result::SUCCESS;
    }
    Result Socket::connectSocket(IPEndpoint endpoint)
    {
        sockaddr_in addr = endpoint.getSockaddrIPv4();
        int result = connect(socketObj, (sockaddr*)(&addr), sizeof(sockaddr_in));
        if (result != 0) {
            int error = WSAGetLastError();
            std::cout << error << "\n";
            return Result::FAIL;
        }

        return Result::SUCCESS;
    }

    Result Socket::sendAll(const void* data, int numberOfBytes)
    {
        int totalBytesSent = 0;

        while (totalBytesSent < numberOfBytes) {
            int bytesRemaining = numberOfBytes - totalBytesSent;
            int bytesSent = 0;
            char* bufferOffset = (char*)data + totalBytesSent;
            bytesSent = send(socketObj, (const char*)bufferOffset, bytesRemaining, NULL);
            if (bytesSent == SOCKET_ERROR) {
                int error = WSAGetLastError();
                return Result::FAIL;
            }

            totalBytesSent += bytesSent;
        }
        return Result::SUCCESS;
    }

    Result Socket::recvAll(void* destination, int numberOfBytes)
    {
        int totalBytesReceived = 0;

        while (totalBytesReceived < numberOfBytes) {
            int bytesRemaining = numberOfBytes - totalBytesReceived;
            int bytesReceived = 0;
            char* bufferOffset = (char*)destination + totalBytesReceived;

            bytesReceived = recv(socketObj, bufferOffset, bytesRemaining, NULL);
            if (bytesReceived == 0) {  //when connection was gracefully closed
                return Result::FAIL;
            }
            if (bytesReceived == SOCKET_ERROR) {
                int error = WSAGetLastError();
                return Result::FAIL;
            }

            totalBytesReceived += bytesReceived;
        }
        return Result::SUCCESS;
    }
    Result Socket::sendPacket(Packet& packet)
    {
        //determine packet size first
        uint16_t packetSize = htons(packet.buffer.size()); // we need to make sure that the packet size is in network byte order since we are going to send it
        //send the packet size
        Result result = sendAll(&packetSize, sizeof(uint16_t));
        if (result != Result::SUCCESS) {
            std::cout << "Over here" << "\n";
            return Result::FAIL;
        }

        //now we send the packet data;
        result = sendAll(packet.buffer.data(), packet.buffer.size());
        if (result != Result::SUCCESS) {
            return Result::FAIL;
        }

        return Result::SUCCESS;
    }
    Result Socket::recvPacket(Packet& packet)
    {
        packet.clearData();
        //receive the encoded size
        uint16_t packetSize = 0;
        Result result = recvAll(&packetSize, sizeof(uint16_t));
        if (result != Result::SUCCESS) {
            return Result::FAIL;
        }

        //now receive the actual buffer containing the data

        uint16_t bufferSize = ntohs(packetSize);

        if (bufferSize > 8192) {
            return Result::FAIL;
        }

        packet.buffer.resize(bufferSize);
        result = recvAll(&packet.buffer[0], bufferSize);
        if (result != Result::SUCCESS) {
            return Result::FAIL;
        }

        return Result::SUCCESS;
    }
}

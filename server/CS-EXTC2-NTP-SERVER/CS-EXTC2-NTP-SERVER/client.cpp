#define NOMINMAX //win macros disable

#include <vector>
#include <random>
#include <cstdint>
#include <algorithm>
#include "client.hpp"
#include <iostream>

ClientSession::ClientSession(const std::vector<uint8_t>& clientId)
    : clientID(clientId)
{

    if (clientID.size() != 4) {
        throw std::runtime_error("Session ID must be 4 bytes");
    }

    //Next, init socket for client
    this->initSocket();
}


//std::vector<uint8_t> ClientSession::getOutboundDataBuffer() const {
//    return this->outboundDataBuffer;
//}
//
//int ClientSession::setOutboundDataBuffer(const std::vector<uint8_t>& outboundBuffer) {
//    this->fromClientBuffer = outboundBuffer;
//    return 0; // You can return something more meaningful later if needed
//}

//woudl need to convert to a vector
//std::vector<uint8_t> ClientSession::getforClientBuffer() const {
//    return this->forClientBuffer;
//}

/*

HEY - going to need a "get next chunk", so the rest of the code doesnt have to worry abotu tracking chunks. 
This getNextChunk will return the next chunk of data based on chunk size for client. 

*/

std::vector<uint8_t> ClientSession::getNextChunk(size_t chunkSize) {
    std::vector<uint8_t> chunk;
    size_t take = std::min(chunkSize, this->forClientBuffer.size());

    for (size_t i = 0; i < take; ++i) {
        chunk.push_back(this->forClientBuffer.front());
        this->forClientBuffer.pop_front(); // O(1) in deque
    }

    return chunk;
}



/*
sets this->forClientBuffer with a vector of bytes.

*/
void ClientSession::setForClientBuffer(const std::vector<uint8_t>& data) {
    forClientBuffer.clear();                       // remove any existing data
    forClientBuffer.insert(forClientBuffer.end(),  // append new data
        data.begin(),
        data.end());
}

void ClientSession::setFromClientBufferSize(uint32_t bufferSize) {
    this->fromClientBufferSize = bufferSize;
    std::cout << "[DEBUG] setFromClientBufferSize called, value set to " << bufferSize << std::endl;
}

uint32_t ClientSession::getFromClientBufferSize() const {
    std::cout << "[DEBUG] getFromClientBufferSize called, returning " << fromClientBufferSize << std::endl;
    return fromClientBufferSize;
}


std::vector<uint8_t> ClientSession::getForClientBuffer() const {
    return std::vector<uint8_t>(this->forClientBuffer.begin(), this->forClientBuffer.end());
}



SOCKET ClientSession :: getSocket() {
    return this->_extc2Socket;
}

int ClientSession::initSocket() {
    std::cout << "[+] Initializing socket for client" << std::endl;
    struct sockaddr_in 	sock;
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = inet_addr(TeamServer::address.c_str());
    sock.sin_port = htons(TeamServer::port);

    this->_extc2Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(this->_extc2Socket, (struct sockaddr*)&sock, sizeof(sock))) {
        printf("Could not connect to %s:%d\n", TeamServer::address.c_str(), TeamServer::port);
        exit(0);
        return 1;
    }
    return 0;
}


uint32_t generateClientID() {
    static std::random_device rd;  // Non-deterministic seed
    static std::mt19937 gen(rd()); // Mersenne Twister RNG
    static std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

    return dist(gen);
}

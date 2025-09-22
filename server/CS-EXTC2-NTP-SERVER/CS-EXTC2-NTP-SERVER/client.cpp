#include <vector>
#include <random>
#include <cstdint>

#include "client.hpp"

ClientSession::ClientSession(const std::vector<uint8_t>& sessionId)
    : sessionID(sessionId)
{

    if (sessionID.size() != 4) {
        throw std::runtime_error("Session ID must be 4 bytes");
    }
}


std::vector<uint8_t> ClientSession::getOutboundDataBuffer() const {
    return this->outboundDataBuffer;
}

int ClientSession::setOutboundDataBuffer(const std::vector<uint8_t>& outboundBuffer) {
    this->outboundDataBuffer = outboundBuffer;
    return 0; // You can return something more meaningful later if needed
}

std::vector<uint8_t> ClientSession::getInboundDataBuffer() const {
    return this->inboundDataBuffer;
}

/*

HEY - going to need a "get next chunk", so the rest of the code doesnt have to worry abotu tracking chunks. 
This getNextChunk will return the next chunk of data based on chunk size for client. 

*/


uint32_t generateClientID() {
    static std::random_device rd;  // Non-deterministic seed
    static std::mt19937 gen(rd()); // Mersenne Twister RNG
    static std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

    return dist(gen);
}

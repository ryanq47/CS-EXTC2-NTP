#pragma once
#include <vector>
#include <cstdint>
#include <deque>
#include "constants.hpp"
class ClientSession {
public:
    explicit ClientSession(const std::vector<uint8_t>& clientId);


    // --- Getters / Setters ---
    //std::vector<uint8_t> getOutboundDataBuffer() const;
    //int setOutboundDataBuffer(const std::vector<uint8_t>& outboundBuffer);
    std::vector<uint8_t> getForClientBuffer() const;
    void setForClientBuffer(const std::vector<uint8_t>& data); //set forClientbuffer with a vector
    std::vector<uint8_t> getNextChunk(size_t chunkSize);


private:
    std::vector<uint8_t> clientID;
    std::vector<uint8_t> fromClientBuffer;
    std::deque<uint8_t> forClientBuffer;
    int chunkSize = Chunk::maxChunkSize - 8; //setting max chunksize to whatever the constnat is, minus 8 for headers.AKA how much data we can fit per extension
};


uint32_t generateClientID();
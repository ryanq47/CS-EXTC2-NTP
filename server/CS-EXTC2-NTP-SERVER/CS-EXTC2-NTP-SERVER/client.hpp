#pragma once
#include <vector>
#include <cstdint>
#include <deque>
#include "constants.hpp"
#include <windows.h>
class ClientSession {
public:
    explicit ClientSession(const std::vector<uint8_t>& clientId);


    // --- Getters / Setters ---
    //std::vector<uint8_t> getOutboundDataBuffer() const;
    //int setOutboundDataBuffer(const std::vector<uint8_t>& outboundBuffer);
    std::vector<uint8_t> getForClientBuffer() const;
    void setForClientBuffer(const std::vector<uint8_t>& data); //set forClientbuffer with a vector
    std::vector<uint8_t> getNextChunk(size_t chunkSize);
    void setFromClientBufferSize(uint32_t bufferSize); //sets how big the incoming message is.
    uint32_t getFromClientBufferSize() const; //gets fromClientBufferSize

    //being lazy and putting this in public instad of private with a getter method
    std::vector<uint8_t> fromClientBuffer; //vector and not a queue cuz we send the whole vector to the TS, and onlyapped to this. Not pop

    SOCKET getSocket();
    int initSocket();

private:
    std::vector<uint8_t> clientID;
    uint32_t fromClientBufferSize;
    std::deque<uint8_t> forClientBuffer;
    int chunkSize = Chunk::maxChunkSize - 8; //setting max chunksize to whatever the constnat is, minus 8 for headers.AKA how much data we can fit per extension
    SOCKET _extc2Socket;
};


uint32_t generateClientID();
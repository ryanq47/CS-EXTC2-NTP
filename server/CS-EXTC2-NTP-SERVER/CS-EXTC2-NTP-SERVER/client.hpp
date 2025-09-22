#pragma once
#include <vector>
#include <cstdint>

class ClientSession {
public:
    explicit ClientSession(const std::vector<uint8_t>& sessionId);


    // --- Getters / Setters ---
    std::vector<uint8_t> getOutboundDataBuffer() const;
    int setOutboundDataBuffer(const std::vector<uint8_t>& outboundBuffer);

    std::vector<uint8_t> getInboundDataBuffer() const;

private:
    std::vector<uint8_t> sessionID;
    std::vector<uint8_t> inboundDataBuffer;
    std::vector<uint8_t> outboundDataBuffer;
};


uint32_t generateClientID();
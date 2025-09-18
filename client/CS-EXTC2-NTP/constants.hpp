#pragma once
#include <array>

//Extension Field headers for identifying what extension this is in the packet
namespace NtpExtensionField {
    constexpr std::array<uint8_t, 2> giveMePayload          = { 0x4d, 0x5a }; //or { 0x10, 0xad } for load, payload
    constexpr std::array<uint8_t, 2> getDataFromTeamserver  = { 0xAA, 0xBB };
    constexpr std::array<uint8_t, 2> sendDataToTeamserver   = { 0xBB, 0xAA };
    constexpr std::array<uint8_t, 2> sizePacket             = { 0x51, 0x2E }; //size packet, how big the incoming data is

}

namespace Chunk {
    constexpr int maxChunkSize = 28; //28 for data (plus 4 for headers/size), as the packets must be aligned to 32 bit boundaries. There is logic to handle if they are less, but 28 (for an even 32) seemed easiest. 

}


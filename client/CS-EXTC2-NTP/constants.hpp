#pragma once
#include <array>

//Extension Field headers for identifying what extension this is in the packet
namespace NtpExtensionField {
    constexpr std::array<uint8_t, 2> giveMePayload          = { 0x4d, 0x5a }; //or { 0x10, 0xad } for load, payload
    constexpr std::array<uint8_t, 2> getDataFromTeamserver  = { 0xAA, 0xBB };
    constexpr std::array<uint8_t, 2> sendDataToTeamserver   = { 0xBB, 0xAA };
    constexpr std::array<uint8_t, 2> sizePacket             = { 0x51, 0x2E }; //size packet, how big the incoming data is

}



#pragma once
#include <array>

//Extension Field headers for identifying what extension this is in the packet
namespace NtpExtensionField {
    constexpr std::array<uint8_t, 2> giveMePayload          = { 0x4d, 0x5a };
    constexpr std::array<uint8_t, 2> getDataFromTeamserver  = { 0xAA, 0xBB };
    constexpr std::array<uint8_t, 2> sendDataToTeamserver   = { 0xBB, 0xAA };
}



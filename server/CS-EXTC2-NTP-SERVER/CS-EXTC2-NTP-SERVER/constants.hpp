#pragma once
#include <array>
#include <string>

//do NOT print data if 1, otherwise do if 0
#define DONT_PRINT_DATA 1

//Extension Field headers for identifying what extension this is in the packet
//also why in the world did I do this as an array, but eveerythign esle as a vector.
namespace NtpExtensionField {

    /*
   Extension Field layout. This comes after the 48 byte NTP packet
   -----------------------------------------------
   | Bytes | Description                          |
   |-------|--------------------------------------|
   | 0-1   | Extension Field Type (2 bytes)       |
   | 2-3   | Extension Field Length (2 bytes)     |
   | 4-7   | Session ID (4 bytes), makes sure uniqueness when talking to server
   | 8-?   | Data of extension fields             |
   -----------------------------------------------
   */

   // Payload-related
    constexpr std::array<uint8_t, 2> giveMePayload = { 0x00, 0x01 };

    // Teamserver requests/responses
    constexpr std::array<uint8_t, 2> getDataFromTeamserver = { 0x00, 0x02 };
    constexpr std::array<uint8_t, 2> dataFromTeamserver = { 0x02, 0x04 };
    constexpr std::array<uint8_t, 2> getDataFromTeamserverSize = { 0x03, 0x04 };
    constexpr std::array<uint8_t, 2> dataForTeamserver = { 0x02, 0x05 };

    // Identification / ID
    constexpr std::array<uint8_t, 2> getIdPacket = { 0x12, 0x34 };
    constexpr std::array<uint8_t, 2> idPacket = { 0x1D, 0x1D };


    // Size-related
    constexpr std::array<uint8_t, 2> sizePacket = { 0x51, 0x2E };
    constexpr std::array<uint8_t, 2> sizePacketAcknowledge = { 0x50, 0x50 };


}

namespace Chunk {
    constexpr int maxChunkSize = 1016; //28 for data (plus 4 for headers/size, and 4 for clientid), as the packets must be aligned to 32 bit boundaries. There is logic to handle if they are less, but 28 (for an even 32) seemed easiest. 

}
//
//namespace Net {
//    constexpr uint16_t port = 123;
//    const std::string serverAddress = "127.0.0.1";
//}

namespace TeamServer {
    const std::string address = "10.0.1.24";
    constexpr int port = 8080;

}

namespace Controller {
    constexpr int ntpListenPort = 123;

}

namespace Client {
    const std::vector<uint8_t> emptyClientId = { 0xFF, 0xFF, 0xFF, 0xFF }; //const cuz this shuold never change
    //std::vector<uint8_t> clientId = {};       //not a const cuz this will change

}
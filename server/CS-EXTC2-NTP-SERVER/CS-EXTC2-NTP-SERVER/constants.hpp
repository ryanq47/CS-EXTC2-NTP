#pragma once
#include <array>
#include <string>
//Extension Field headers for identifying what extension this is in the packet
//also why in the world did I do this as an array, but eveerythign esle as a vector.
namespace NtpExtensionField {
    constexpr std::array<uint8_t, 2> giveMePayload = { 0x00, 0x01 }; //{ 0x4d, 0x5a }; //or { 0x10, 0xad } for load, payload
    constexpr std::array<uint8_t, 2> dataFromTeamserver = { 0x02, 0x04 }; //NTS Cookie, 43-64 bytes
    constexpr std::array<uint8_t, 2> dataForTeamserver = { 0x02, 0x05 }; //NTS cookie placeholder, sent by client

    /*
    giveMePayload, getDataFromTeamserver, sendDataToTeamserver Extension Field Layout
    -----------------------------------------------
    | Bytes | Description                          |
    |-------|--------------------------------------|
    | 0-1   | Extension Field Type (2 bytes)       |
    | 2-3   | Extension Field Length (2 bytes)     |
    | 4-7   | Session ID (4 bytes), makes sure uniqueness when talking to server
    | 8-32  | Payload: Data to tunnel              | //probably could make bigger, maybe up unto max NTP size, or max UDP packet size. Define in constants?
    -----------------------------------------------
    Notes:
    - Total extension field size is always exactly 8 bytes.
    - The 4-byte data payload represents the full message size.
    - Data is stored in **host byte order**, not network order.
        > This makes parsing easier. Worth a change after the rest of this program is figured out.

    */
    //left off defining these, and getting teh chunking working/making sure it makes sense. Getting close to actual server being needed now. 

    constexpr std::array<uint8_t, 2> idPacket = { 0x1D, 0x1D }; //idPacket, has ID in it for client to use
    constexpr std::array<uint8_t, 2> getIdPacket = { 0x12, 0x34 }; //Get ID packet
    constexpr std::array<uint8_t, 2> sizePacket = { 0x51, 0x2E }; //size packet, how big the incoming data is
    /*
    sizePacket Extension Field Layout (Total: 8 bytes)
    -----------------------------------------------
    | Bytes | Description                          |
    |-------|--------------------------------------|
    | 0-1   | Extension Field Type (2 bytes)       |
    | 2-3   | Extension Field Length (2 bytes)     |
    | 4-7   | Session ID (4 bytes), makes sure uniqueness when talking to server (for this, all 0xFF if no client id)
    | 8-11   | Payload: Data Size (4 bytes, host order) |
    -----------------------------------------------
    Notes:
    - Total extension field size is always exactly 8 bytes.
    - The 4-byte data payload represents the full message size.
    - Data is stored in **host byte order**, not network order.
        > This makes parsing easier. Worth a change after the rest of this program is figured out.

    This is the initial packet to start comms with the server
    respnose:
        a 2 byte client ID

    */

    //ID extension, what the client sees in a response 
    constexpr std::array<uint8_t, 2> sessionID = { 0x53, 0x55 }; //size packet, how big the incoming data is
    /*
        SessionID Extension Field Layout (Total: 8 bytes)
    -----------------------------------------------
    | Bytes | Description                          |
    |-------|--------------------------------------|
    | 0-1   | Extension Field Type (2 bytes)       |
    | 2-3   | Extension Field Length (2 bytes)     |
    | 4-7   | Session ID (4 bytes), makes sure uniqueness when talking to server
    -----------------------------------------------

    Ex: 0x53,0x55,0x00,0x02,0x00,0x01 //session id of 1

    */

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
    const std::string address = "10.0.1.22";
    constexpr int port = 8080;

}

namespace Client {
    const std::vector<uint8_t> emptySessionId = { 0xFF, 0xFF, 0xFF, 0xFF }; //const cuz this shuold never change
    //std::vector<uint8_t> sessionId = {};       //not a const cuz this will change

}
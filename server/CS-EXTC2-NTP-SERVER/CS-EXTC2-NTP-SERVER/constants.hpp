#pragma once
#include <array>
#include <string>
//Extension Field headers for identifying what extension this is in the packet
//also why in the world did I do this as an array, but eveerythign esle as a vector.
namespace NtpExtensionField {
    constexpr std::array<uint8_t, 2> giveMePayload = { 0x00, 0x01 }; //{ 0x4d, 0x5a }; //or { 0x10, 0xad } for load, payload
    constexpr std::array<uint8_t, 2> getDataFromTeamserver = { 0x02, 0x04 }; //NTS Cookie, 43-64 bytes
    constexpr std::array<uint8_t, 2> sendDataToTeamserver = { 0x02, 0x05 }; //NTS cookie placeholder, sent by client

    /*
    giveMePayload, getDataFromTeamserver, sendDataToTeamserver Extension Field Layout
    -----------------------------------------------
    | Bytes | Description                          |
    |-------|--------------------------------------|
    | 0-1   | Extension Field Type (2 bytes)       |
    | 2-3   | Extension Field Length (2 bytes)     |
    | 4-5   | Session ID (2 bytes), makes sure uniqueness when talking to server
    | 6-32  | Payload: Data to tunnel              | //probably could make bigger, maybe up unto max NTP size, or max UDP packet size. Define in constants?
    -----------------------------------------------
    Notes:
    - Total extension field size is always exactly 8 bytes.
    - The 4-byte data payload represents the full message size.
    - Data is stored in **host byte order**, not network order.
        > This makes parsing easier. Worth a change after the rest of this program is figured out.

    */
    //left off defining these, and getting teh chunking working/making sure it makes sense. Getting close to actual server being needed now. 

    constexpr std::array<uint8_t, 2> sizePacket = { 0x51, 0x2E }; //size packet, how big the incoming data is
    /*
    sizePacket Extension Field Layout (Total: 8 bytes)
    -----------------------------------------------
    | Bytes | Description                          |
    |-------|--------------------------------------|
    | 0-1   | Extension Field Type (2 bytes)       |
    | 2-3   | Extension Field Length (2 bytes)     |
    | 4-7   | Payload: Data Size (4 bytes, host order) |
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
    | 4-5   | SessionID: The session ID to include in each packet |
    -----------------------------------------------

    Ex: 0x53,0x55,0x00,0x02,0x00,0x01 //session id of 1

    */

}

//namespace Chunk {
//    constexpr int maxChunkSize = 28; //28 for data (plus 4 for headers/size), as the packets must be aligned to 32 bit boundaries. There is logic to handle if they are less, but 28 (for an even 32) seemed easiest. 
//
//}
//
//namespace Net {
//    constexpr uint16_t port = 123;
//    const std::string serverAddress = "127.0.0.1";
//}


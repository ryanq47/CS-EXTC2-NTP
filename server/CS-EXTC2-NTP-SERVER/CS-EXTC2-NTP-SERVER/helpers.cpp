#include <iostream>
#include <vector>
#include <iomanip> // for std::setw and std::setfill
#include <unordered_map>
#include <cstring> // for std::memcpy
#include <winsock2.h>
#include <ws2tcpip.h>
#include "constants.hpp"
#include "createntp.hpp"
#include "client.hpp"
#include "globals.hpp"

// Print a vector in hex
void printHexVector(const std::vector<uint8_t>& vec) {
#if DONT_PRINT_DATA
    return;
#endif
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex
            << static_cast<int>(vec[i]) << " ";
    }
    std::cout << std::dec << std::endl;
}

// Print a vector in hex with line breaks every 8 bytes
void printHexVectorPacket(const std::vector<uint8_t>& vec) {
#if DONT_PRINT_DATA
    return;
#endif
    std::cout << "Packet ------------------" << std::endl;
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex
            << static_cast<int>(vec[i]) << " ";
        if ((i + 1) % 8 == 0)
            std::cout << std::endl;
    }
    if (vec.size() % 8 != 0)
        std::cout << std::endl;
    std::cout << "-------------------------" << std::endl;
    std::cout << std::dec;
}

// Print raw packet data from char buffer
void print_packet_hex(const char* data, int len) {
#if DONT_PRINT_DATA
    return;
#endif
    std::cout << "Packet ------------------" << std::endl;
    for (int i = 0; i < len; ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex
            << static_cast<int>(static_cast<unsigned char>(data[i])) << " ";
        if ((i + 1) % 8 == 0)
            std::cout << std::endl;
    }
    if (len % 8 != 0)
        std::cout << std::endl;
    std::cout << "-------------------------" << std::endl;
    std::cout << std::dec;
}

// Convert uint32_t to network-order byte vector
std::vector<uint8_t> uint32ToBytes(uint32_t value) {
    std::vector<uint8_t> bytes(4);
    bytes[0] = static_cast<uint8_t>((value >> 24) & 0xFF);
    bytes[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
    bytes[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
    bytes[3] = static_cast<uint8_t>(value & 0xFF);
    return bytes;
}

// Send a default NTP packet
void sendNormalNtpPacket(sockaddr_in* client_addr, SOCKET sock) {
    NTPPacket defaultPacket;
    std::vector<uint8_t> defaultPacketData = defaultPacket.getPacket();

    //std::cout << "[?] Sending normal NTP packet back" << std::endl;
    printHexVectorPacket(defaultPacketData);

    sendto(sock,
        reinterpret_cast<const char*>(defaultPacketData.data()),
        static_cast<int>(defaultPacketData.size()),
        0,
        reinterpret_cast<sockaddr*>(client_addr),
        sizeof(*client_addr));

    //std::cout << "[?] Sent successfully" << std::endl;
}

// Send an arbitrary NTP packet
void sendNtpPacket(sockaddr_in* client_addr, SOCKET sock, std::vector<uint8_t> ntpPacket) {
    //std::cout << "[?] Sending NTP packet" << std::endl;
    printHexVectorPacket(ntpPacket);

    sendto(sock,
        reinterpret_cast<const char*>(ntpPacket.data()),
        static_cast<int>(ntpPacket.size()),
        0,
        reinterpret_cast<sockaddr*>(client_addr),
        sizeof(*client_addr));

    //std::cout << "[?] Sent successfully" << std::endl;
}

// Convert 4-byte vector to uint32_t in host byte order
uint32_t vectorToUint32(const std::vector<uint8_t>& vec) {
    if (vec.size() < 4)
        throw std::runtime_error("Vector too small to convert to uint32_t");

    uint32_t value;
    std::memcpy(&value, vec.data(), sizeof(value));
    return ntohl(value);
}

// Print all client IDs in the session map (pre-C++17)
void printClientIDs(const std::unordered_map<ClientID, ClientSession>& map) {
    for (std::unordered_map<ClientID, ClientSession>::const_iterator it = map.begin(); it != map.end(); ++it) {
        std::cout << it->first << std::endl;
    }
}

// Print a raw buffer as hex
void printHex(const uint8_t* buffer, size_t length) {
    for (size_t i = 0; i < length; ++i)
        printf("%02X ", buffer[i]);
    printf("\n");
}

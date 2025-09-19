#include <iostream>
#include <vector>
#include <iomanip> // for std::setw and std::setfill

void printHexVector(const std::vector<uint8_t>& vec) {
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)vec[i] << " ";
    }
    std::cout << std::dec << std::endl; // Reset back to decimal
}

//for specificalyl printing a packet with a newline after 8 hex chars
void printHexVectorPacket(const std::vector<uint8_t>& vec) {
    std::cout << "Packet ------------------" << std::endl;
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)vec[i] << " ";
        if ((i + 1) % 8 == 0)
            std::cout << std::endl;
    }
    if (vec.size() % 8 != 0)
        std::cout << std::endl; // Final newline if not already printed
    std::cout << std::dec; // Reset back to decimal
    std::cout << "-------------------------" << std::endl;

}

void print_packet_hex(const char* data, int len) {
    std::cout << "Packet ------------------\n";

    for (int i = 0; i < len; ++i) {
        // Print each byte as two hex digits
        std::cout << std::hex << std::setw(2) << std::setfill('0')
            << (static_cast<unsigned int>(static_cast<unsigned char>(data[i]))) << " ";

        // Print newline every 8 bytes
        if ((i + 1) % 8 == 0) {
            std::cout << "\n";
        }
    }

    // If length is not multiple of 8, add newline at the end
    if (len % 8 != 0) {
        std::cout << "\n";
    }

    std::cout << "-------------------------\n";
}

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include "createntp.hpp"

void sendNormalNtpPacket(sockaddr_in* client_addr, SOCKET sock) {
    //send back a default packet
    NTPPacket defaultPacket;

    std::vector<uint8_t> defaultPacketData = defaultPacket.getPacket();

    std::cout << "[?] Sending normal NTP packet back" << std::endl;
    printHexVectorPacket(defaultPacketData);

    sendto(sock,
        //convert the vector into waht it needs to be
        reinterpret_cast<const char*>(defaultPacketData.data()),
        static_cast<int>(defaultPacketData.size()),
        0,
        (sockaddr*)client_addr,
        sizeof(*client_addr)
    );

    std::cout << "[?] Sent successfully" << std::endl;
}

/*
For sending an NTP packet back.
*/
void sendNtpPacket(sockaddr_in* client_addr, SOCKET sock, std::vector<uint8_t> ntpPacket) {

    std::cout << "[?] Sending NTP packet" << std::endl;
    printHexVectorPacket(ntpPacket);

    sendto(sock,
        //convert the vector into waht it needs to be
        reinterpret_cast<const char*>(ntpPacket.data()),
        static_cast<int>(ntpPacket.size()),
        0,
        (sockaddr*)client_addr,
        sizeof(*client_addr)
    );

    std::cout << "[?] Sent successfully" << std::endl;
}
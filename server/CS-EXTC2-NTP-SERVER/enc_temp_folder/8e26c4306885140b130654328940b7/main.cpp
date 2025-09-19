#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "parsentp.hpp"
#include "helpers.hpp"
#include "createntp.hpp"
#pragma comment(lib, "ws2_32.lib")

constexpr int NTP_PORT = 123;
constexpr int BUFFER_SIZE = 128; // NTP packet size

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

//void printHexVector(const std::vector<uint8_t>& vec) {
//    for (size_t i = 0; i < vec.size(); ++i) {
//        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)vec[i] << " ";
//    }
//    std::cout << std::dec << std::endl; // Reset back to decimal
//}


void handle_ntp_packet(char* data, int len, sockaddr_in* client_addr, SOCKET sock) {
    std::vector<uint8_t> packet(data, data + 48);

    std::cout << "Received " << len << " bytes from "
        << inet_ntoa(client_addr->sin_addr) << ":"
        << ntohs(client_addr->sin_port) << std::endl;

    if (packet.size() <= 48) {
        std::cout << "[?] Normal NTP packet detected" << std::endl;
        //return a normal packet, make this a func
        NTPPacket defaultPacket;

        std::vector<uint8_t> defaultPacketData = defaultPacket.getPacket();
        //sendto(sock, data, len, 0, (sockaddr*)client_addr, sizeof(*client_addr));

        std::cout << "[?] Sending normal NTP packet back" << std::endl;
        sendto(sock,
            //convert the vector into waht it needs to be
            reinterpret_cast<const char*>(defaultPacketData.data()),
            static_cast<int>(defaultPacketData.size()),
            0,
            (sockaddr*)client_addr,
            sizeof(*client_addr)
        );

        std::cout << "[?] Sent successfully" << std::endl;

        return;
    }

    //each packet needs the 4 byte header, so check if there are bytes there
    if (packet.size() < 52) {
        std::cout << "Packet does not contain an extension field" << std::endl;
        return;
    }

    print_packet_hex(data, len);

    //Extract extension field
    NTPPacketParser ntpPacket(packet);
    std::vector<uint8_t> ntpPacketExtension = ntpPacket.getExtension();
    //printHexVector(packet);
    //vector subscriptiotn out of range^^ 


    //if ntpPacketExtension header == whatever of constnts... then this that

    // Echo back the received packet (for demonstration)
    //sendto(sock, data, len, 0, (sockaddr*)client_addr, sizeof(*client_addr));
    //std::cout << "Sent response back to client\n";
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in server_addr{}, client_addr{};
    int client_addr_len = sizeof(client_addr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(NTP_PORT);

    if (bind(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "NTP UDP server listening on port " << NTP_PORT << std::endl;

    char buffer[BUFFER_SIZE];
    while (true) {
        int recv_len = recvfrom(sock, buffer, BUFFER_SIZE, 0, (sockaddr*)&client_addr, &client_addr_len);
        if (recv_len == SOCKET_ERROR) {
            std::cerr << "recvfrom failed with error: " << WSAGetLastError() << std::endl;
            break;
        }

        handle_ntp_packet(buffer, recv_len, &client_addr, sock);
    }

    closesocket(sock);
    WSACleanup();

    return 0;
}

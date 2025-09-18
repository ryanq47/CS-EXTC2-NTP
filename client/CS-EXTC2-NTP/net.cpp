#include <vector>
#include <iostream>

std::vector<uint8_t> placeholderNtpPacket = {
    0x23, 0x00, 0x06, 0xEC, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x4D, 0x5A, 0x00, 0x08, 0xDE, 0xAD, 0xBE, 0xEF
};


//std::vector<uint8_t> sendChunk(std::vector <uint8_t> chunkData) {
//
//	std::cout << "Placholder to send a chunk" << std::endl;
//
//	////return FULL packet bytes here
//	//std::vector<uint8_t> placeholderVec = { 0 };
//	//return placeholderVec;
//    return placeholderNtpPacket;
//}

#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

//note - wireshark showing as malformed packet, so somethign might be up
std::vector<uint8_t> sendChunk(
    std::vector <uint8_t> packet
) {
    //placeholder address
    std::string serverAddress = "127.0.0.1";
    uint16_t port = 123;

    std::cout << "[NET] Size of chunk: " << packet.size() << std::endl;

    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    std::vector<uint8_t> response;



    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    try {
        // Create socket
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
            throw std::runtime_error("Failed to create socket");
        }

        // Set timeout (e.g., 5 seconds)
        DWORD timeout = 5000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

        // Setup destination address
        sockaddr_in destAddr = {};
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(port);
        if (inet_pton(AF_INET, serverAddress.c_str(), &destAddr.sin_addr) != 1) {
            throw std::runtime_error("Invalid IP address");
        }

        // Send packet
        int sent = sendto(sock, reinterpret_cast<const char*>(packet.data()), static_cast<int>(packet.size()), 0,
            reinterpret_cast<sockaddr*>(&destAddr), sizeof(destAddr));
        if (sent == SOCKET_ERROR) {
            throw std::runtime_error("Failed to send packet");
        }

        // Receive response
        uint8_t buffer[512];  // Typical NTP response is small, this is enough
        sockaddr_in fromAddr = {};
        int fromLen = sizeof(fromAddr);

        int recvLen = recvfrom(sock, reinterpret_cast<char*>(buffer), sizeof(buffer), 0,
            reinterpret_cast<sockaddr*>(&fromAddr), &fromLen);
        if (recvLen == SOCKET_ERROR) {
            throw std::runtime_error("Timeout or error receiving response");
        }

        response.assign(buffer, buffer + recvLen);
    }
    catch (...) {
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
        }
        WSACleanup();
        throw;
    }

    closesocket(sock);
    WSACleanup();
    return response;
}

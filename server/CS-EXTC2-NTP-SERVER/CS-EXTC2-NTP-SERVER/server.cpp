#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "parsentp.hpp"
#include "helpers.hpp"
#include "createntp.hpp"
#include "constants.hpp"
#include <array>
#include "ntphandler.hpp"
#pragma comment(lib, "ws2_32.lib")

constexpr int NTP_PORT = 123;
constexpr int BUFFER_SIZE = 128; // NTP packet size

int runServer() {
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

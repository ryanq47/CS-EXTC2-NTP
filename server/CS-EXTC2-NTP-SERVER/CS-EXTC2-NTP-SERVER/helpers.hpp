#pragma once

#include <vector>
#include <ws2tcpip.h>
#include "createntp.hpp"
#include <unordered_map>
#include "globals.hpp"
#include "client.hpp"
void printHexVector(const std::vector<uint8_t>& vec);

void printHexVectorPacket(const std::vector<uint8_t>& vec);

void print_packet_hex(const char* data, int len);

void sendNormalNtpPacket(sockaddr_in* client_addr, SOCKET sock);

std::vector<uint8_t> uint32ToBytes(uint32_t value);

void sendNtpPacket(sockaddr_in* client_addr, SOCKET sock, std::vector<uint8_t> ntpPacket);

uint32_t vectorToUint32(const std::vector<uint8_t>& vec);

void printClientIDs(const std::unordered_map<ClientID, ClientSession>& map);
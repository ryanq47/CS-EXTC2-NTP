#pragma once

#include <vector>

void printHexVector(const std::vector<uint8_t>& vec);

void printHexVectorPacket(const std::vector<uint8_t>& vec);

void print_packet_hex(char* data, int len);

void packetDebugger(std::vector<uint8_t> packetBytes);
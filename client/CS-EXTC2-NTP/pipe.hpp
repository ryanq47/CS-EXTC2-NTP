#pragma once
#include <windows.h>
#include <vector>
#include <cstdint>

DWORD read_frame(HANDLE my_handle, char* buffer, DWORD max);

void write_frame(HANDLE my_handle, char* buffer, DWORD length);

void pipeLoop(const std::vector<uint8_t> clientId);
#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

//pulled from https://github.com/Cobalt-Strike/External-C2/blob/main/extc2example.c#L84

/* receive a frame from a socket */
DWORD recv_frame(SOCKET my_socket, char* buffer, DWORD max);

/* send a frame via a socket */
void send_frame(SOCKET my_socket, char* buffer, int length);

std::vector<uint8_t> getx64Payload();
std::vector<uint8_t> getx86Payload();
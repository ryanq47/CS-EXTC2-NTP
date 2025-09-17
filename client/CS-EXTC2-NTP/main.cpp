/*

CS-EXTC2-NTP



*/

#include <iostream>
#include "ntp.cpp"

int main() {
	//Stuff here

	std::cout << "Started" << std::endl;

	auto packet = NTPPacket();
	packet.printPacket();
	//std::cout << packet << std::endl;
	

}
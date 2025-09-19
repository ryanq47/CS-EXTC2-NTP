#include <iostream>
#include <vector>
#include <iomanip> // for std::setw and std::setfill
#include "parsentp.hpp"
#include "helpers.hpp"
#include "createntp.hpp"
#include "constants.hpp"
#include <array>

void printHexVector(const std::vector<uint8_t>& vec) {
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)vec[i] << " ";
    }
    std::cout << std::dec << std::endl; // Reset back to decimal
}

//for specificalyl printing a packet with a newline after 8 hex chars
void printHexVectorPacket(const std::vector<uint8_t>& vec) {
    std::cout << "Packet ------------------" << std::endl;
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)vec[i] << " ";
        if ((i + 1) % 8 == 0)
            std::cout << std::endl;
    }
    if (vec.size() % 8 != 0)
        std::cout << std::endl; // Final newline if not already printed
    std::cout << std::dec; // Reset back to decimal
    std::cout << "-------------------------" << std::endl;

}

void print_packet_hex(char* data, int len) {
    std::cout << "Packet ------------------\n";

    for (int i = 0; i < len; ++i) {
        // Print each byte as two hex digits
        std::cout << std::hex << std::setw(2) << std::setfill('0')
            << (static_cast<unsigned int>(static_cast<unsigned char>(data[i]))) << " ";

        // Print newline every 8 bytes
        if ((i + 1) % 8 == 0) {
            std::cout << "\n";
        }
    }

    // If length is not multiple of 8, add newline at the end
    if (len % 8 != 0) {
        std::cout << "\n";
    }

    std::cout << "-------------------------\n";
}

/*
this is JUST a debug function to help with debugging the packets.

*/
void packetDebugger(std::vector<uint8_t> packetBytes) {
    //1. Run some checks to make sure an extension field exists, and extract it if so

    //3 seperate for different error messages. Could just be the < 52 one as well.
    if (packetBytes.size() < 48) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Malformed Packet" << std::endl;
        std::cout << "----------------------" << std::endl;        //do actoins
        return;
    }

    if (packetBytes.size() <= 48) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Normal NTP Packet " << std::endl;
        std::cout << "----------------------" << std::endl;        //do actoins
        return;
    }

    //each packet needs the 4 byte header, so check if there are bytes there
    if (packetBytes.size() < 52) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Normal NTP Packet " << std::endl;
        std::cout << "----------------------" << std::endl;
        std::cout << "Packet does not contain an extension field" << std::endl;

        return;
    }

    //Extract extension field
    NTPPacketParser ntpPacket(packetBytes);
    std::vector<uint8_t> ntpPacketExtension = ntpPacket.getExtension();
    std::array<uint8_t, 2> ntpPacketExtensionField = ntpPacket.getExtensionField();
    //standard here: print debug header of packet type, then printthe packet

    //2. Do different actions based on what type of packet/extension field is coming in

    if (ntpPacketExtensionField == NtpExtensionField::giveMePayload) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Give Me Payload " << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

    }


    else if (ntpPacketExtensionField == NtpExtensionField::dataForTeamserver) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: dataForTeamserver " << std::endl;
        std::cout << "----------------------" << std::endl;
        //this is data meant for teamserver. Need to fogure out chunkinghere too

        //identify session here too?
        //std::cout << "sendDataToTeamserver " << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

    }

    else {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Unknown " << std::endl;
        std::cout << "----------------------" << std::endl;
        std::cout << "[?] Packet extension did not match any known extensions" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());


    }

}

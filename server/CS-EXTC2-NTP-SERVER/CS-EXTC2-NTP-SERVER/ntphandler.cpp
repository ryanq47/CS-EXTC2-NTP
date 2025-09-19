#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "parsentp.hpp"
#include "helpers.hpp"
#include "createntp.hpp"
#include "constants.hpp"
#include <array>

void handle_ntp_packet(char* data, int len, sockaddr_in* client_addr, SOCKET sock) {
    //bug is here, as it was +48 instead of whtaever. Need to get size safely, as well.
    //safely moving into vector by doing data + len (data is a char* so it's the first item
    std::vector<uint8_t> packet(data, data + len);

    std::cout << "Received " << len << " bytes from "
        << inet_ntoa(client_addr->sin_addr) << ":"
        << ntohs(client_addr->sin_port) << std::endl;

    //1. Run some checks to make sure an extension field exists, and extract it if so

    //2 seperate for different error messages. Could just be the < 52 one as well.
    if (packet.size() <= 48) {
        std::cout << "[?] Normal NTP packet detected" << std::endl;

        //send back a default packet
        NTPPacket defaultPacket;

        std::vector<uint8_t> defaultPacketData = defaultPacket.getPacket();

        std::cout << "[?] Sending normal NTP packet back" << std::endl;
        printHexVectorPacket(defaultPacketData);

        sendto(sock,
            //convert the vector into waht it needs to be
            reinterpret_cast<const char*>(defaultPacketData.data()),
            static_cast<int>(defaultPacketData.size()),
            0,
            (sockaddr*)client_addr,
            sizeof(*client_addr)
        );

        std::cout << "[?] Sent successfully" << std::endl;

        return;
    }

    //each packet needs the 4 byte header, so check if there are bytes there
    if (packet.size() < 52) {
        std::cout << "Packet does not contain an extension field" << std::endl;

        //send back a default packet
        NTPPacket defaultPacket;
        std::vector<uint8_t> defaultPacketData = defaultPacket.getPacket();

        std::cout << "[?] Sending normal NTP packet back" << std::endl;
        printHexVectorPacket(defaultPacketData);

        sendto(sock,
            //convert the vector into waht it needs to be
            reinterpret_cast<const char*>(defaultPacketData.data()),
            static_cast<int>(defaultPacketData.size()),
            0,
            (sockaddr*)client_addr,
            sizeof(*client_addr)
        );

        std::cout << "[?] Sent successfully" << std::endl;

        return;
    }

    print_packet_hex(data, len);

    //Extract extension field
    NTPPacketParser ntpPacket(packet);
    std::vector<uint8_t> ntpPacketExtension = ntpPacket.getExtension();
    std::array<uint8_t, 2> ntpPacketExtensionField = ntpPacket.getExtensionField();

    //2. Do different actions based on what type of packet/extension field is coming in
    if (ntpPacketExtensionField == NtpExtensionField::giveMePayload) {
        std::cout << "Give Me Payload " << std::endl;
    }

    /*
    dataForTeamserver

    ...


    */
    else if (ntpPacketExtensionField == NtpExtensionField::dataForTeamserver) {
        //this is data meant for teamserver. Need to fogure out chunkinghere too

        //identify session here too?
        std::cout << "sendDataToTeamserver " << std::endl;

        //send to teamserver

        //get data back

        //send data back with header of NtpExtensionField::dataFromTeamserver
    }

    /* GiveMePayload
    
    
    When the client wants the payload, teh server will go grab it from the teamserver


    */
    else if (ntpPacketExtensionField == NtpExtensionField::giveMePayload) {
        std::cout << "Give Me Payload " << std::endl;
    }

    else {
        std::cout << "[?] Packet extension did not match any known extensions" << std::endl;

        //if none of the headers match,s end back regular NTP packet
                //send back a default packet
        NTPPacket defaultPacket;
        std::vector<uint8_t> defaultPacketData = defaultPacket.getPacket();

        std::cout << "[?] Sending normal NTP packet back" << std::endl;
        printHexVectorPacket(defaultPacketData);

        sendto(sock,
            //convert the vector into waht it needs to be
            reinterpret_cast<const char*>(defaultPacketData.data()),
            static_cast<int>(defaultPacketData.size()),
            0,
            (sockaddr*)client_addr,
            sizeof(*client_addr)
        );

        std::cout << "[?] Sent successfully" << std::endl;

    }

}

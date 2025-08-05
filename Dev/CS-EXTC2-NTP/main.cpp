/*

References:

 https://en.wikipedia.org/wiki/Network_Time_Protocol


*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctime>
#include <stdexcept>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#pragma pack(push, 1) //1 means align to 1 byte boundaries, aka no padding on a 1 byte field. 2 would add 1 byte of padding on a 1 byte field

//struct for c2 issues
struct NtpExtensionField {
    uint16_t type;    // Extension field type ID (2 bytes) - don't need
    uint16_t length;  // Length of this extension field in bytes (including header) ( 2 bytes)
    uint8_t data[8];   // Extension data payload array (8 bytes - can be however big as needed)
};

struct NtpPacket {
    // note, byte labels are 0 indexed
    // == LI (Leap Indicator), VN (Version Number), Mode (3 bits each): Byte 0 ==
    // LI: bits 7-6, VN: bits 5-3, Mode: bits 2-0
    uint8_t li_vn_mode = 0b11100011;  // LI=3, VN=4, Mode=3 (client)
    //11: clock unsync'd
    //100: version num 4
    //011: Client mode

    // == Stratum: 8 bits (Byte 1) ==
    // Distance from reference clock (0=invalid, 1=primary, etc.)
    uint8_t stratum = 0;

    // == Poll Interval: 8 bits (Byte 2) ==
    // Max interval between successive messages (log2 seconds)
    uint8_t poll = 0;

    // == Precision: 8 bits signed (Byte 3) ==
    // Signed log2 of system clock precision
    uint8_t precision = 0;

    // == Root Delay: 32 bits (Bytes 4-7) ==
    // Total round-trip delay to the reference clock, in NTP short format (16 bits int + 16 bits frac)
    uint32_t rootDelay = 0;

    // == Root Dispersion: 32 bits (Bytes 8-11) ==
    // Total dispersion to the reference clock, in NTP short format
    uint32_t rootDispersion = 0;

    // == Reference ID: 32 bits (Bytes 12-15) ==
    // Identifier of specific server or reference clock (depends on stratum)
    uint32_t refId = 0;

    // == Reference Timestamp: 64 bits (Bytes 16-23) ==
    // Time when system clock was last set or corrected (seconds + fractional seconds)
    uint32_t refTimeSec = 0;
    uint32_t refTimeFrac = 0;

    // == Origin Timestamp: 64 bits (Bytes 24-31) ==
    // Time at client when request departed (seconds + fractional seconds)
    uint32_t origTimeSec = 0;
    uint32_t origTimeFrac = 0;

    // == Receive Timestamp: 64 bits (Bytes 32-39) ==
    // Time at server when request arrived (seconds + fractional seconds)
    uint32_t recvTimeSec = 0;
    uint32_t recvTimeFrac = 0;

    // == Transmit Timestamp: 64 bits (Bytes 40-47) ==
    // Time at server when response left (seconds + fractional seconds)
    uint32_t txTimeSec = 0;
    uint32_t txTimeFrac = 0;

    // == payload extension field ==//
    //NtpExtensionField payload; // breaks normal ntp if included
};

struct NtpPacketWithExtension {
    NtpPacket packet;        // Existing NTP packet data
    NtpExtensionField ext;   // Extension field data (will be filled dynamically)
};

#pragma pack(pop) // this restores padding to normal. 

class NtpClient {
public:
    explicit NtpClient(const std::string& server_ip = "127.0.0.1", int port = 6969) {

        this->server_ip = server_ip;
        this->port = port;

        initWinsock();
        createSocket();
        setupServerAddress();
    }

    //deconstructor at close for socket hanldineg
    ~NtpClient() {
        if (this->sock != INVALID_SOCKET) {
            closesocket(this->sock);
        }
        WSACleanup();
    }

    std::time_t getTime() {
        sendRequest();
        receiveResponse();
        printRawBuffer();
        return parseTime();
    }

    void print_debug_info() {
        //debug info
        std::cout << "== DEBUG INFO == \n";
        std::cout << "\tSERVER: " << this->server_ip << "\n\t" << "PORT: " << this->port << "\n";
    }

private:
    SOCKET sock = INVALID_SOCKET;
    std::string server_ip;
    int port;
    sockaddr_in server_addr;
    //char buffer[48]{};
    NtpPacket packet{}; //{} init's with all 0's
    NtpExtensionField ext{};
    NtpPacketWithExtension packetWithExtension{}; //used as size placeholder. packet and ext are where the data actualyl is. Prolly worth fixing later

    static constexpr uint64_t NTP_TIMESTAMP_DELTA = 2208988800ULL;

    void initWinsock() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
            throw std::runtime_error("WSAStartup failed.");
        }
    }

    void createSocket() {
        this->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (this->sock == INVALID_SOCKET) {
            WSACleanup();
            throw std::runtime_error("Socket creation failed.");
        }
    }

    void setupServerAddress() {
        memset(&this->server_addr, 0, sizeof(this->server_addr));
        this->server_addr.sin_family = AF_INET;
        this->server_addr.sin_port = htons(port);
        inet_pton(AF_INET, server_ip.c_str(), &this->server_addr.sin_addr);
    }

    void sendRequest() {
        int sent = sendto(
            this->sock,
            reinterpret_cast<char*>(&this->packet),
            sizeof(this->packet),
            0,
            (sockaddr*)&this->server_addr,
            sizeof(this->server_addr)
        );
        if (sent == SOCKET_ERROR) {
            throw std::runtime_error("sendto failed.");
        }
    }

    void receiveResponse() {
    //    sockaddr_in from{};
    //    int from_len = sizeof(from);

    //    int received = recvfrom(
    //        this->sock,
    //        reinterpret_cast<char*>(&this->packet),
    //        sizeof(this->packet),
    //        0,
    //        (sockaddr*)&from,
    //        &from_len
    //    );

    //}
        sockaddr_in from{};
        int from_len = sizeof(from);


        //note, this expects it to be 60 bytes every time. If it's not, it will either hang or fail
        int received = recvfrom(
            this->sock,
            reinterpret_cast<char*>(&this->packet),
            //60, // buffer size, sizeof(this->packetWithExtension) is 48, not full thing
            sizeof(this->packetWithExtension),
            0,
            (sockaddr*)&from,
            &from_len
        );

        if (received == SOCKET_ERROR) {
            throw std::runtime_error("recvfrom failed.");
        }

        // Check if there's an extension field in the received data
        if (received >= sizeof(NtpPacket)) {
            const uint8_t* rawData = reinterpret_cast<const uint8_t*>(&this->packet);

            parse_extension_field(rawData);

            //// Assuming the extension starts right after the regular NTP packet structure
            //size_t extensionStart = sizeof(NtpPacket);

            //if (received > extensionStart) {
            //    std::cout << "Extension Field Discovered\n";
            //    std::cout << "Parsing Extension Field\n";

            //    //parse_extension_field

            //// If data is beyond the normal NTP packet size, parse the extension into the struct
            //    NtpExtensionField extension;
            //    memcpy(&extension, rawData + extensionStart, sizeof(NtpExtensionField));
            //    this->ext = extension;  // Store extension field
            //}


            //std::cout << "Extension Length: " << this->ext.length << "\n";
            //std::cout << "Extension Type: " << this->ext.type << "\n";
            //std::cout << "Extension Data: " << this->ext.data << "\n";
        }
    }

    std::time_t parseTime() const {
        uint32_t seconds = ntohl(this->packet.txTimeSec);
        return static_cast<std::time_t>(seconds - NTP_TIMESTAMP_DELTA);
    }
    

    void parse_extension_field(const uint8_t* rawData) {        
        //std::cout << "Parsing Extension Field\n";
        //size_t extensionStart = sizeof(NtpPacket);

        //// parse_extension_field

        ////// If data is beyond the normal NTP packet size, parse the extension into the struct
        //NtpExtensionField extension;
        //memcpy(&extension, rawData + extensionStart, sizeof(NtpExtensionField));
        //this->ext = extension;  // Store extension field

        ////these 2 print wrong
        ////std::cout << "Extension Length: " << this->ext.length << "\n";
        //std::cout << "Extension Length: " << ntohs(this->ext.length) << "\n"; //need to use ntohs to get back into little endian from net order (big endian)
        //std::cout << "Extension Type: " << ntohs(this->ext.type) << "\n";
        ////this prints correctly(ish, AAAAAAAA and a pi symbol)
        //std::cout << "Extension Data: " << this->ext.data << "\n";

        std::cout << "Parsing Extension Field\n";

        // Create a "walking" pointer to the beginning of the extension data
        const uint8_t* p = rawData + sizeof(NtpPacket);

        // Pull type out (2 bytes, uint16_t)
        uint16_t raw_type;
        memcpy(&raw_type, p, sizeof(uint16_t));
        this->ext.type = ntohs(raw_type);
        p += sizeof(uint16_t); // Advance the pointer by 2 bytes

        // Pull length out (2 bytes, uint16_t)
        uint16_t raw_length;
        memcpy(&raw_length, p, sizeof(uint16_t));
        this->ext.length = ntohs(raw_length);
        p += sizeof(uint16_t); // Advance the pointer by 2 bytes

        //pull rest ot bytes, which is data field
        if (this->ext.length < 4) {
            // This is an invalid length, handle error
            std::cerr << "Error: Invalid extension length " << this->ext.length << "\n";
            return;
        }
        //strip out the 4 byte headers
        size_t data_payload_size = this->ext.length - 4;

        // 3. Copy the raw data payload. No endian conversion needed for raw bytes.
        // Add a safety check to prevent a buffer overflow on this->ext.data, 
        // we make sure that at the specified size is not smaller than the data, which would overflow.
        // std::min would limit the copy size to the buffer's capacity to prevent an overflow.
        size_t bytes_to_copy = (std::min)(data_payload_size, sizeof(this->ext.data)); //need to () std::min as it conflicts with win macros, () hides from preprocessor
        memcpy(this->ext.data, p, bytes_to_copy);

        std::cout << "Extension Type: " << this->ext.type << "\n";
        std::cout << "Extension Length: " << this->ext.length << "\n";
        std::cout << "Extension Data: " << this->ext.data << "\n";

    }


    void printRawBuffer() const {
        const uint8_t* raw = reinterpret_cast<const uint8_t*>(&this->packet);
        //constexpr size_t size = sizeof(NtpPacket);
        constexpr size_t size = sizeof(NtpPacketWithExtension);

        std::cout << "NTP Packet Raw Buffer (hex):\n";
        for (size_t i = 0; i < size; ++i) {
            printf("%02X ", raw[i]);
            if ((i + 1) % 8 == 0) std::cout << "\n"; // group by 8 bytes
        }
    }

};

int main() {
    try {
        NtpClient client;
        client.print_debug_info();
        std::time_t t = client.getTime();
        //std::string t_string = client.getTime();
        std::cout << "NTP Time (UTC): " << std::ctime(&t);
    }
    catch (const std::exception& e) {
        std::cerr << "NTP Client error: " << e.what() << '\n';
    }

    return 0;
}

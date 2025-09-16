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
    explicit NtpClient(const std::string& server_ip = "127.0.0.1", int port = 6969)
        : server_ip(server_ip), port(port) {
        initWinsock();
        createSocket();
        setupServerAddress();
    }

    ~NtpClient() {
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
        }
        WSACleanup();
    }

    // A sample method to demonstrate sending an extension and getting a response
    void sendAndReceiveExtension() {
        prepareRequestPacket("Hello NTP!"); // Prepare a request with extension data
        sendRequest();
        receiveResponse();
        printResponse();
    }

    std::time_t getServerTime() const {
        // Return 0 if the timestamp is not set (i.e., no response received yet)
        if (packet_buffer.packet.txTimeSec == 0) {
            return 0;
        }
        return static_cast<std::time_t>(packet_buffer.packet.txTimeSec - NTP_TIMESTAMP_DELTA);
    }

    std::string getExtensionData() const {
        // The extension length must be at least 4 bytes (for type and length fields)
        if (packet_buffer.ext.length < 4) {
            return ""; // No valid data, return empty string
        }

        // Calculate the actual size of the data payload
        size_t data_payload_size = packet_buffer.ext.length - 4;

        // Create and return a string from the raw byte data
        return std::string(
            reinterpret_cast<const char*>(packet_buffer.ext.data),
            data_payload_size
        );
    }


private:
    // --- Member Variables ---
    SOCKET sock = INVALID_SOCKET;
    std::string server_ip;
    int port;
    sockaddr_in server_addr;

    // GOOD: One struct to hold all packet data for both sending and receiving.
    NtpPacketWithExtension packet_buffer{};

    static constexpr uint64_t NTP_TIMESTAMP_DELTA = 2208988800ULL;

    // --- Core Networking (Unchanged) ---
    void initWinsock() {
        WSADATA wsaData;
        // Request Winsock version 2.2
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed.");
        }
    }

    /**
     * @brief Creates the client's UDP socket.
     */
    void createSocket() {
        // Create a socket for UDP over IPv4
        this->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        // Check for errors
        if (this->sock == INVALID_SOCKET) {
            WSACleanup(); // Clean up Winsock resources
            throw std::runtime_error("Socket creation failed with error: " + std::to_string(WSAGetLastError()));
        }
    }

    /**
     * @brief Fills the sockaddr_in struct with the server's IP and port.
     */
    void setupServerAddress() {
        // Zero out the structure to prevent any garbage data
        memset(&this->server_addr, 0, sizeof(this->server_addr));

        // Set the address family to IPv4
        this->server_addr.sin_family = AF_INET;

        // Set the port, converting it to network byte order
        this->server_addr.sin_port = htons(this->port);

        // Convert the IP address string to the binary format required by the struct
        int result = inet_pton(AF_INET, this->server_ip.c_str(), &this->server_addr.sin_addr);
        if (result <= 0) {
            if (result == 0) {
                throw std::runtime_error("Invalid IP address string provided.");
            }
            else {
                throw std::runtime_error("inet_pton failed with error: " + std::to_string(WSAGetLastError()));
            }
        }
    }

    // --- Packet Preparation and Sending ---
    void prepareRequestPacket(const std::string& data) {
        // 1. Clear the buffer
        memset(&packet_buffer, 0, sizeof(packet_buffer));

        // 2. Fill the main NTP packet fields
        packet_buffer.packet.li_vn_mode = 0b00100011; // VN=4, Mode=3 (client)
        // ... set other necessary NTP fields ...

        // 3. Fill the extension field
        packet_buffer.ext.type = 0xABCD; // Your custom type
        size_t data_size = data.length();
        size_t bytes_to_copy = (std::min)(data_size, sizeof(packet_buffer.ext.data));
        memcpy(packet_buffer.ext.data, data.c_str(), bytes_to_copy);

        // The length includes its own header (4 bytes) plus the data payload
        packet_buffer.ext.length = 4 + bytes_to_copy;
    }

    void sendRequest() {
        // IMPORTANT: Convert to network byte order right before sending
        convertToNetworkOrder(packet_buffer);

        int sent = sendto(
            sock,
            reinterpret_cast<char*>(&packet_buffer),
            sizeof(packet_buffer), // Send the entire structure
            0,
            (sockaddr*)&server_addr,
            sizeof(server_addr)
        );

        // Optional: Convert back to host order if you need to reuse the struct
        convertToHostOrder(packet_buffer);

        if (sent == SOCKET_ERROR) {
            throw std::runtime_error("sendto failed.");
        }
    }

    // --- Receiving and Parsing ---
    void receiveResponse() {
        sockaddr_in from{};
        int from_len = sizeof(from);

        int received = recvfrom(
            sock,
            reinterpret_cast<char*>(&packet_buffer),
            sizeof(packet_buffer), // Receive into the entire structure
            0,
            (sockaddr*)&from,
            &from_len
        );

        if (received == SOCKET_ERROR) {
            throw std::runtime_error("recvfrom failed with error: " + std::to_string(WSAGetLastError()));
        }

        // IMPORTANT: Convert from network byte order right after receiving
        convertToHostOrder(packet_buffer);
    }

    void printResponse() {
        std::cout << "== NTP Response Received ==\n";

        // Now you can just access the members directly!
        std::cout << "Stratum: " << static_cast<int>(packet_buffer.packet.stratum) << "\n";

        uint32_t seconds = packet_buffer.packet.txTimeSec;
        std::cout << "Timestamp (seconds since 1900): " << seconds << "\n";

        // Check if the extension field has a valid length
        if (packet_buffer.ext.length >= 4) {
            std::cout << "Extension Type: 0x" << std::hex << packet_buffer.ext.type << std::dec << "\n";
            std::cout << "Extension Length: " << packet_buffer.ext.length << "\n";
            // Print data safely, ensuring null termination for cout
            std::string data_str(
                reinterpret_cast<char*>(packet_buffer.ext.data),
                packet_buffer.ext.length - 4
            );
            std::cout << "Extension Data: \"" << data_str << "\"\n";
        }
    }

    // --- Helper Functions for Endian Conversion ---
    void convertToNetworkOrder(NtpPacketWithExtension& p) {
        // Packet
        p.packet.rootDelay = htonl(p.packet.rootDelay);
        p.packet.rootDispersion = htonl(p.packet.rootDispersion);
        p.packet.refId = htonl(p.packet.refId);
        p.packet.refTimeSec = htonl(p.packet.refTimeSec);
        p.packet.refTimeFrac = htonl(p.packet.refTimeFrac);
        p.packet.origTimeSec = htonl(p.packet.origTimeSec);
        p.packet.origTimeFrac = htonl(p.packet.origTimeFrac);
        p.packet.recvTimeSec = htonl(p.packet.recvTimeSec);
        p.packet.recvTimeFrac = htonl(p.packet.recvTimeFrac);
        p.packet.txTimeSec = htonl(p.packet.txTimeSec);
        p.packet.txTimeFrac = htonl(p.packet.txTimeFrac);
        // Extension
        p.ext.type = htons(p.ext.type);
        p.ext.length = htons(p.ext.length);
    }

    void convertToHostOrder(NtpPacketWithExtension& p) {
        // This is the exact same logic, as ntoh/hton are often identical macros
        // But it's good practice to have separate functions for clarity
        convertToNetworkOrder(p);
    }
};

// main() is the entry point of your program
int main() {
    try {
        // Create an instance of the client.
        // We'll use a real public NTP server pool and the standard NTP port (123).
        NtpClient client("127.0.0.1", 6969);

        // Call the method that prepares, sends, and receives the packet.
        client.sendAndReceiveExtension();
        std::string extension_data = client.getExtensionData();
        std::cout << extension_data << std::endl;

        std::time_t server_time = client.getServerTime();
        std::cout << "✅ Success! Retrieved data from the client.\n";
        std::cout << "Server Time (from getter): " << ctime(&server_time);
    
    }


    catch (const std::exception& e) {
        // If anything goes wrong (socket creation, send/receive), an exception is thrown.
        // We catch it here and print the error message.
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1; // Indicate failure
    }

    return 0; // Indicate success
}

//int main() {
//    try {
//        NtpClient client;
//        client.print_debug_info();
//        std::time_t t = client.getTime();
//        //std::string t_string = client.getTime();
//        std::cout << "NTP Time (UTC): " << std::ctime(&t);
//    }
//    catch (const std::exception& e) {
//        std::cerr << "NTP Client error: " << e.what() << '\n';
//    }
//
//    return 0;
//}

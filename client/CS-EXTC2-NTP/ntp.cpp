
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include <iomanip>

#include <winsock2.h> //for htons
/*
* 
* NTP Packet Reference

 *    Byte     | Length | Field Name              | Description
 * ------------|--------|-------------------------|-----------------------------------------------------
 *      0      |   1    | LI | VN | Mode          | Leap Indicator (2 bits), Version (3 bits), Mode (3 bits)
 *      1      |   1    | Stratum                 | Stratum level of the local clock
 *      2      |   1    | Poll                    | Max interval between messages (log2 seconds)
 *      3      |   1    | Precision               | Precision of local clock (log2 seconds)

 *      4      |   4    | Root Delay              | Total round trip delay to the reference clock (signed 16.16 fixed-point)
 *      8      |   4    | Root Dispersion         | Nominal error relative to the reference clock (unsigned 16.16 fixed-point)
 *     12      |   4    | Reference ID            | Reference clock identifier (IP, ASCII, or KISS code)

 *     16      |   8    | Reference Timestamp     | Time when the system clock was last set or corrected
 *     24      |   8    | Originate Timestamp     | Time request departed the client for the server
 *     32      |   8    | Receive Timestamp       | Time request arrived at the server
 *     40      |   8    | Transmit Timestamp      | Time reply departed the server for client
 

 * LI (Leap Indicator): Warns of impending leap second (00 = no warning)
 * VN (Version Number): NTP version (e.g., 4 for NTPv4)
 * Mode: 3 = client, 4 = server, 5 = broadcast, etc.

 * Stratum:
 *   0 = unspecified or invalid
 *   1 = primary server (e.g., GPS, atomic clock)
 *   2-15 = secondary server
 *   16+ = reserved

 * Timestamps (64-bit):
 *   First 32 bits: seconds since Jan 1, 1900 (NTP epoch)
 *   Last 32 bits: fractional seconds

*/

class NTPPacket {
public:
    // Define the struct inside the class
#pragma pack(push, 1)
    struct PacketData {
        uint8_t li_vn_mode;
        uint8_t stratum;
        uint8_t poll;
        int8_t precision;
        uint32_t root_delay;
        uint32_t root_dispersion;
        uint32_t reference_id;
        uint64_t reference_ts;
        uint64_t originate_ts;
        uint64_t receive_ts;
        uint64_t transmit_ts;
    };
#pragma pack(pop)

    static constexpr size_t BASE_PACKET_SIZE = sizeof(PacketData);

    NTPPacket(uint8_t li = 0, uint8_t version = 4, uint8_t mode = 3) {
        std::memset(&packet_, 0, sizeof(PacketData));

        //packet things & bitshift reference

        //Note, bitshift reference cuz C++ does it weird w syntax
        /*
        1 << 0  = 00000001 (1)
        1 << 1  = 00000010 (2)
        1 << 2  = 00000100 (4)
        1 << 3  = 00001000 (8)
        1 << 4  = 00010000 (16)
        1 << 5  = 00100000 (32)
        1 << 6  = 01000000 (64)
        1 << 7  = 10000000 (128)

        Basically, 1,then X number of zero's get put to the left

        For the li_vn_mode: 
        Bits:   7   6   5   4   3   2   1   0
               [LI][LI][VN][VN][VN][MO][MO][MO]


        The below line does this:
        */
        uint8_t _li = (li << 6);             //XX000000
        uint8_t _version = (version << 3);   //00XXX000
        uint8_t _mode = mode;                //000000XX
        
        /*
        then, | XOR's then, which combies into the following struct:
        Bits:   7   6   5   4   3   2   1   0
               [LI][LI][VN][VN][VN][MO][MO][MO]
        */
        packet_.li_vn_mode = _li | _version | _mode;
        packet_.stratum = 0;                //distance from ref clock. Fine to be 0
        packet_.poll = 6;                   //Max interval (seconds) between successive messages. Not sure if it matters or not
        packet_.precision = -20;            //how precise clock is. -20: 1/1048576 of a second. -6: 1/64 seconds
        // Other fields remain zero
    }

    std::vector<uint8_t> getPacket() const {
        std::vector<uint8_t> packetVector(sizeof(PacketData));
        std::memcpy(packetVector.data(), &packet_, sizeof(PacketData));
        //if extension, add in extra bytes to end
        packetVector.insert(packetVector.end(), extension_.begin(), extension_.end());
        return packetVector;
    }

    void addExtensionField(const std::array<uint8_t, 4>& fieldType, const std::vector<uint8_t>& value) {
        //first 4 are the size, rest is the vector/rest of extension field
        size_t extLength = 2 + 2 + value.size(); //2 bytes of field type, 2 of length, size of data
        //RFC 7822 defines NTP ext feilds, which specify 2 bytes of ext type, and 2 of length. To look legit, we should follow that

        //size_t paddedLength = (extLength + 3) & ~3;
        /*
            The field length needs to be padded to a multiple of 4 bytes. We can use this calculation to mathematically round up to the nearest multiple of 4.            
            This hurts my brain a bit.

            If extLength = 1,
                1 + 3 = 4
                4 / 4 = 1       
                1 * 4 = 4       padded length

            If extLength = 2,
                2 + 3 = 5
                5 / 4 = 1       int, so decimal gets chopped off
                1 * 4 = 4       padded length

            If extLength = 3,
                3 + 3 = 6
                6 / 4 = 1       int, so decimal gets chopped off
                1 * 4 = 4       padded length

            If extLength = 4,
                4 + 3 = 7
                7 / 4 = 1       int, so decimal gets chopped off
                1 * 4 = 4       padded length
        
        */
        size_t paddedLength = ((extLength + 3) / 4) * 4;

        //resize to size of full new legth, and the 0 is all new values are 0, 0 fills the rest here
        extension_.resize(paddedLength, 0);

        // Copy fieldType/extensionTpe (2 bytes)
        std::memcpy(extension_.data(), fieldType.data(), 2);

        // Copy extension length (2 bytes) in network byte order (big endian)
        //need to convert length to net order due to RFC/sending data over the line standards.
        uint16_t netLength = htons(static_cast<uint16_t>(extLength));
        std::memcpy(extension_.data() + 2, &netLength, 2);

        //then get rest of data into it
        std::memcpy(extension_.data() + 4, value.data(), value.size());
    }

    void printPacket() const {
        auto packet = getPacket();
        std::cout << "Packet (" << packet.size() << " bytes):" << std::endl;
        for (size_t i = 0; i < packet.size(); ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(packet[i]) << " ";
            if ((i + 1) % 8 == 0) std::cout << std::endl;
        }

        std::cout << "li_vn_mode" << packet_.li_vn_mode << std::endl;
    }

private:
    PacketData packet_;               // Struct instance
    std::vector<uint8_t> extension_;  // For extension fields
};

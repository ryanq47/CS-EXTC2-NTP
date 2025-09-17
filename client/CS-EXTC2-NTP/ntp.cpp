
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include <iomanip>

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
        packet_.stratum = 0;
        packet_.poll = 6;
        packet_.precision = -20;
        // Other fields remain zero
    }

    std::vector<uint8_t> getPacket() const {
        std::vector<uint8_t> packetVector(sizeof(PacketData));
        std::memcpy(packetVector.data(), &packet_, sizeof(PacketData));
        //if extension, add in extra bytes to end
        packetVector.insert(packetVector.end(), extension_.begin(), extension_.end());
        return packetVector;
    }

    void addExtensionField(const std::array<uint8_t, 4>& fieldType,
        const std::vector<uint8_t>& value) {
        size_t extLength = 4 + value.size();
        size_t paddedLength = (extLength + 3) & ~3;
        extension_.resize(paddedLength, 0);
        std::memcpy(extension_.data(), fieldType.data(), 4);
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

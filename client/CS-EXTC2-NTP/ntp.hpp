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
#pragma once

#include <array>
#include <cstdint>
#include <vector>

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


/**
 * @brief NTPPacket class - represents an NTP packet with optional extension fields.
 */
class NTPPacket {
public:

    static constexpr size_t BASE_PACKET_SIZE = sizeof(PacketData);

    NTPPacket(uint8_t li = 0, uint8_t version = 4, uint8_t mode = 3);

/**
* @brief Gets current packet bytes and returns it
*
*
* @return std::vector<uint8_t>, a vector of the packet bytes.
*/

    std::vector<uint8_t> getPacket() const;

/**
* @brief Adds an extension field to the current packet. This is where data is hidden
*
*
* @param fieldType std::array<uint8_t, 2>& fieldType, the "type" of extension it is. Ex: `std::array<uint8_t, 2> myFieldArray = {1,2};`
* @param data std::vector<uint8_t>& data, the data that is added to the extension field. Ex: `std::vector<uint8_t> packetData = {10,20,30,40};` Using uint8_t as this can hold one byte of any value, which is perfect for tunneling data.
* @return void
*/
    void addExtensionField(const std::array<uint8_t, 2>& fieldType, const std::vector<uint8_t>& data);

/**
* @brief Prints the current packet buffer to the terminal
*
* @return void
*/
    void printPacket() const;

private:
    PacketData packet_;
    std::vector<uint8_t> extension_;
};

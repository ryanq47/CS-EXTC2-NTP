#include "ntp.hpp"
#include <vector>
#include <array>

class NTPPacketParser {
public:

    static constexpr size_t BASE_PACKET_SIZE = sizeof(PacketData);

    NTPPacketParser(std::vector<uint8_t> ntpPacket);
    std::vector<uint8_t> getExtensionData();

private:
    //move this into a class var so it can be accessed everywhere
    PacketData packetStruct{};

    //internal only method to extract extenion out
    void _extractExtension();
    //extension private vars
    //uint8_t _extensionType      = 0;
    std::array<uint8_t, 2> _extensionField;
    uint16_t _extensionLength    = 0;
    std::vector<uint8_t> _extensionData;
    std::vector<uint8_t> _extension; //holds the full extension
    std::vector<uint8_t> _ntpPacket; //raw ntp packet
};
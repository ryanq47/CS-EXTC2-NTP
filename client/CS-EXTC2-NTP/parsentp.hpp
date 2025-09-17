#include "ntp.hpp"
#include <vector>

class NTPPacketParser {
public:

    static constexpr size_t BASE_PACKET_SIZE = sizeof(PacketData);

    NTPPacketParser(std::vector<uint8_t> ntpPacket);

private:
    //move this into a class var so it can be accessed everywhere
    PacketData packetStruct;
};


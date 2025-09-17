/*
A class for parsing NTP packets & exposing easy to use methods with them

Ex:

    std::vector<uint8_t> getPayload() const; //returns bytes of payload from extension field

    // Timestamps helper functions
    std::string getReferenceTime() const;   //normal NTP stuff
    std::string getOriginateTime() const;   //normal NTP stuff
    std::string getReceiveTime() const;    //normal NTP stuff
    std::string getTransmitTime() const;   //normal NTP stuff

*/

/*
Notes for parsing, use vector.at(), as it does bounds checking. This prevents 
weird out of bounds problems if packets get malformed.

also remember - .at/vectors are 0 indexed.

*/

#include "ntp.hpp"
#include "parsentp.hpp"
#include <iostream>

NTPPacketParser::NTPPacketParser(std::vector<uint8_t> ntpPacket) {
    //weird but, packet not full size???? says its 4.
    std::cout << "Parsing packet of size " << ntpPacket.size() << std::endl;
    std::cout << "Struct size" << sizeof(PacketData) << std::endl;
    if (ntpPacket.size() < sizeof(PacketData)) {
        std::cerr << "Packet size is too small!" << std::endl;
        return;
    }

    //move this into a class var so it can be accessed everywhere
    PacketData data;

    // Manually copy bytes from packet into struct fields, rather than casting, for manual control/just incase the packet is malformed, etc.
    std::memcpy(&data.li_vn_mode, ntpPacket.data(), sizeof(data.li_vn_mode));
    std::memcpy(&data.stratum, ntpPacket.data() + 1, sizeof(data.stratum));
    std::memcpy(&data.poll, ntpPacket.data() + 2, sizeof(data.poll));
    std::memcpy(&data.precision, ntpPacket.data() + 3, sizeof(data.precision));
    std::memcpy(&data.root_delay, ntpPacket.data() + 4, sizeof(data.root_delay));
    std::memcpy(&data.root_dispersion, ntpPacket.data() + 8, sizeof(data.root_dispersion));
    std::memcpy(&data.reference_id, ntpPacket.data() + 12, sizeof(data.reference_id));
    std::memcpy(&data.reference_ts, ntpPacket.data() + 16, sizeof(data.reference_ts));
    std::memcpy(&data.originate_ts, ntpPacket.data() + 24, sizeof(data.originate_ts));
    std::memcpy(&data.receive_ts, ntpPacket.data() + 32, sizeof(data.receive_ts));
    std::memcpy(&data.transmit_ts, ntpPacket.data() + 40, sizeof(data.transmit_ts));

    // Now you can access the fields directly
    std::cout << "li_vn_mode: " << static_cast<int>(data.li_vn_mode) << std::endl;
    std::cout << "stratum: " << static_cast<int>(data.stratum) << std::endl;
    std::cout << "root_delay: " << data.root_delay << std::endl;
    std::cout << "reference_ts: " << data.reference_ts << std::endl;


    //also - don't bother adding the ext field to the struct,just manually extract the extension field based on length, etc. rather than adding complications with struct stuff

}
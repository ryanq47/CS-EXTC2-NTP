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
    std::cout << "Parsing packet of size " << ntpPacket.size() << std::endl;
    std::cout << "Struct size" << sizeof(PacketData) << std::endl;
    if (ntpPacket.size() < sizeof(PacketData)) {
        std::cerr << "Packet size is too small!" << std::endl;
        return;
    }

    // Manually copy bytes from packet into struct fields, rather than casting, for manual control/just incase the packet is malformed, etc.
    std::memcpy(&this->packetStruct.li_vn_mode, ntpPacket.data(), sizeof(this->packetStruct.li_vn_mode));
    std::memcpy(&this->packetStruct.stratum, ntpPacket.data() + 1, sizeof(this->packetStruct.stratum));
    std::memcpy(&this->packetStruct.poll, ntpPacket.data() + 2, sizeof(this->packetStruct.poll));
    std::memcpy(&this->packetStruct.precision, ntpPacket.data() + 3, sizeof(this->packetStruct.precision));
    std::memcpy(&this->packetStruct.root_delay, ntpPacket.data() + 4, sizeof(this->packetStruct.root_delay));
    std::memcpy(&this->packetStruct.root_dispersion, ntpPacket.data() + 8, sizeof(this->packetStruct.root_dispersion));
    std::memcpy(&this->packetStruct.reference_id, ntpPacket.data() + 12, sizeof(this->packetStruct.reference_id));
    std::memcpy(&this->packetStruct.reference_ts, ntpPacket.data() + 16, sizeof(this->packetStruct.reference_ts));
    std::memcpy(&this->packetStruct.originate_ts, ntpPacket.data() + 24, sizeof(this->packetStruct.originate_ts));
    std::memcpy(&this->packetStruct.receive_ts, ntpPacket.data() + 32, sizeof(this->packetStruct.receive_ts));
    std::memcpy(&this->packetStruct.transmit_ts, ntpPacket.data() + 40, sizeof(this->packetStruct.transmit_ts));

    // Now you can access the fields directly
    std::cout << "li_vn_mode: " << static_cast<int>(this->packetStruct.li_vn_mode) << std::endl;
    std::cout << "stratum: " << static_cast<int>(this->packetStruct.stratum) << std::endl;
    std::cout << "root_delay: " << this->packetStruct.root_delay << std::endl;
    std::cout << "reference_ts: " << this->packetStruct.reference_ts << std::endl;


    //also - don't bother adding the ext field to the struct,just manually extract the extension field based on length, etc. rather than adding complications with struct stuff

}

void NTPPacketParser::_extractExtension() {
    /*
    Extract extension from packet
    

    Store type, payload, and size in class var, 

    Not meant to be called from outside of this class. is a private method
    */


}
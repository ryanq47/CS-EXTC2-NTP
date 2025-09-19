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
#include "constants.hpp"
#include <iostream>
#include "helpers.hpp"

NTPPacketParser::NTPPacketParser(std::vector<uint8_t> ntpPacket) {
    std::cout << "[?] Parsing packet of size: " << ntpPacket.size() << std::endl;
    std::cout << "[?] Struct size:\t" << sizeof(PacketData) << std::endl;
    if (ntpPacket.size() < sizeof(PacketData)) {
        std::cerr << "Packet size is too small!" << std::endl;
        return;
    }

    //Setup vars
    _ntpPacket = std::move(ntpPacket); //move the ntpPacket into the internal var
    //also init our struct
    //packetStruct = PacketData();

    //works fine without needing to actually parse the NTP packet,but could be useful in the future.Access violation otherwise.
    // Manually copy bytes from packet into struct fields, rather than casting, for manual control/just incase the packet is malformed, etc.
    //std::memcpy(&this->packetStruct.li_vn_mode, ntpPacket.data(), sizeof(this->packetStruct.li_vn_mode));
    //std::memcpy(&this->packetStruct.stratum, ntpPacket.data() + 1, sizeof(this->packetStruct.stratum));
    //std::memcpy(&this->packetStruct.poll, ntpPacket.data() + 2, sizeof(this->packetStruct.poll));
    //std::memcpy(&this->packetStruct.precision, ntpPacket.data() + 3, sizeof(this->packetStruct.precision));
    //std::memcpy(&this->packetStruct.root_delay, ntpPacket.data() + 4, sizeof(this->packetStruct.root_delay));
    //std::memcpy(&this->packetStruct.root_dispersion, ntpPacket.data() + 8, sizeof(this->packetStruct.root_dispersion));
    //std::memcpy(&this->packetStruct.reference_id, ntpPacket.data() + 12, sizeof(this->packetStruct.reference_id));
    //std::memcpy(&this->packetStruct.reference_ts, ntpPacket.data() + 16, sizeof(this->packetStruct.reference_ts));
    //std::memcpy(&this->packetStruct.originate_ts, ntpPacket.data() + 24, sizeof(this->packetStruct.originate_ts));
    //std::memcpy(&this->packetStruct.receive_ts, ntpPacket.data() + 32, sizeof(this->packetStruct.receive_ts));
    //std::memcpy(&this->packetStruct.transmit_ts, ntpPacket.data() + 40, sizeof(this->packetStruct.transmit_ts));

    //// Now you can access the fields directly
    //std::cout << "li_vn_mode: " << static_cast<int>(this->packetStruct.li_vn_mode) << std::endl;
    //std::cout << "stratum: " << static_cast<int>(this->packetStruct.stratum) << std::endl;
    //std::cout << "root_delay: " << this->packetStruct.root_delay << std::endl;
    //std::cout << "reference_ts: " << this->packetStruct.reference_ts << std::endl;


    //also - don't bother adding the ext field to the struct,just manually extract the extension field based on length, etc. rather than adding complications with struct stuff
    _extractExtension();
}

std::vector<uint8_t> NTPPacketParser::getExtensionData() {
    return this->_extensionData;
}

std::vector<uint8_t> NTPPacketParser::getExtension() {
    return this->_extension;
}

void NTPPacketParser::_extractExtension() {
    /*
    Extract extension from packet


    Store type, payload, and size in class var,

    Not meant to be called from outside of this class. is a private method
    */

    /*
    Alrighty extension breakdown:

    Bytes 0 & 1: Type of extension (store in this->_extensionType)

    Bytes 2 & 3: Length of extension (specific to this packet only, not overall chunk) (store in this->_extensionLength)

    Bytes 3-LENGTH: The rest of the extension (which is 4 byte padded, will need to remove padding properly...) (store in this->_extensionData)

    */

    //need to pull from packet var. if null, or somethign, error


    //copy payload into vector, so it's easier to access/more readable
    this->_extension.insert(
        this->_extension.begin(),           //insert at beginning of extension array
        this->_ntpPacket.begin() + 48,      //use 48 bytes ofset, as packet bytes noramlly are 48 bytes
        this->_ntpPacket.end()              //keep copying to end of packet, which *should* be end of extension field. Only one extension field per packet.
    );

    std::cout << "[?] Extension Field:\t";
    printHexVector(this->_extension);


    // Extract the first 2 bytes for the extension type (NTP extension type)
    uint8_t extensionFieldByte0 = this->_extension[0]; //first 2 bytes are extension type
    uint8_t extensionFieldByte1 = this->_extension[1];

    this->_extensionField[0] = extensionFieldByte0;
    this->_extensionField[1] = extensionFieldByte1;

    std::cout << "[?] Extension Type:\t"
        << std::hex << static_cast<int>(extensionFieldByte0) << " "
        << std::hex << static_cast<int>(extensionFieldByte1)
        << std::endl;

    //get length
    uint8_t extensionFieldByte2 = this->_extension[2]; // Next 2 bytes are Extension Length
    uint8_t extensionFieldByte3 = this->_extension[3];

    //do some byte magic to turn this into a uint16 for size
    this->_extensionLength = (static_cast<uint16_t>(extensionFieldByte2) << 8) | static_cast<uint16_t>(extensionFieldByte3);
    std::cout << "[?] Extension Length:\t" << this->_extensionLength << std::endl;


    //copy extension data (the payload) into the _extensionData vec
    this->_extensionData.insert(
        this->_extensionData.begin(),       //insert at beginning of extensinoData array
        this->_extension.begin() + 4,       //2 bytes for type, 2 bytes for length, rest for payload
        this->_extension.end()              //keep copying to end of packet, which *should* be end of extension field. Only one extension field per packet.
    );

    std::cout << "[?] Extension Data:\t";
    printHexVector(this->_extensionData);
}
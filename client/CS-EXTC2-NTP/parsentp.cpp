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
#include <unordered_map>
#include <cstdint>
#include "client.hpp"

// Forward declare your Session class
class ClientSession;

using SessionID = uint32_t;
/*
Can't use vector, so need to use uint32 as its ahshable

//so do this to convert
uint32_t sessionID = 0;
std::memcpy(&sessionID, packet.data() + 4, 4);

*/

// Declare as extern in a header (globals.hpp)
extern std::unordered_map<SessionID, ClientSession> sessions;
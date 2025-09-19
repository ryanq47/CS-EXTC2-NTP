#include <vector>
#include <random>
#include <cstdint>
/*
Meant to hold client sessions while packets come in

*/
class ClientSession {

public:

	//append to dataBuffer()

private:

	//hold packet inobund data
	std::vector<uint8_t> dataBuffer;

};



uint32_t generateClientID() {
    static std::random_device rd;  // Non-deterministic seed
    static std::mt19937 gen(rd()); // Mersenne Twister RNG
    static std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

    return dist(gen);
}

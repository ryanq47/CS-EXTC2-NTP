#include <iostream>
#include <vector>
#include <iomanip> // for std::setw and std::setfill

void printHexVector(const std::vector<uint8_t>& vec) {
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)vec[i] << " ";
    }
    std::cout << std::dec << std::endl; // Reset back to decimal
}

//for specificalyl printing a packet with a newline after 8 hex chars
void printHexVectorPacket(const std::vector<uint8_t>& vec) {
    std::cout << "Packet ------------------" << std::endl;
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)vec[i] << " ";
        if ((i + 1) % 8 == 0)
            std::cout << std::endl;
    }
    if (vec.size() % 8 != 0)
        std::cout << std::endl; // Final newline if not already printed
    std::cout << std::dec; // Reset back to decimal
    std::cout << "-------------------------" << std::endl;

}
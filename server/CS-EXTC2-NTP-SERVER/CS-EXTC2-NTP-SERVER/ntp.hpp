#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct PacketData {
    uint8_t li_vn_mode;
    uint8_t stratum;
    uint8_t poll;
    int8_t precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t reference_id;
    uint64_t reference_ts;
    uint64_t originate_ts;
    uint64_t receive_ts;
    uint64_t transmit_ts;
};
#pragma pack(pop)

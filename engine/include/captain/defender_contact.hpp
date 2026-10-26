#pragma once

#include <array>
#include <cstdint>

namespace captain {
struct DefenderContactInput {
    std::uint16_t incoming_power;
    std::uint16_t defender_power;
    std::uint8_t random_first;
    std::uint8_t random_second;
    std::uint8_t range;
    std::uint8_t offset;
    bool weakened;
    bool eligible;
    std::uint8_t context;
    std::uint8_t defender_slot;
    std::uint8_t subtype;
    std::array<std::uint8_t, 4> thresholds;
};

struct DefenderContactResult {
    std::uint16_t defender_power;
    std::uint8_t strength;
    std::uint8_t loss;
    std::uint8_t score;
    std::uint8_t result_code;
    std::uint16_t incoming_power;
    std::uint8_t subtype;
    bool special;
};

DefenderContactResult calculate_defender_contact(const DefenderContactInput& input);
}

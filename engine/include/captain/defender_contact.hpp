#pragma once

#include <array>
#include <cstdint>

namespace captain {
struct DefenderRollInput {
    std::uint16_t base;
    std::uint8_t random_first;
    std::uint8_t random_second;
    std::uint8_t range;
    std::uint8_t offset;
    bool weakened;
    bool eligible;
    std::uint8_t context;
    std::uint8_t defender_slot;
    std::uint8_t subtype;
};
struct DefenderRollResult {
    std::uint16_t base;
    std::uint8_t strength;
    std::uint8_t subtype;
    bool special;
};
DefenderRollResult roll_defender_strength(const DefenderRollInput& input);
std::uint8_t classify_score(std::uint8_t score, const std::array<std::uint8_t, 4>& thresholds);

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

// Comparison/classification only. Reports candidate loss without applying it;
// the selected-defender caller preserves incoming power at this stage.
DefenderContactResult compare_defender_contact(const DefenderContactInput& input);

// Isolated caller continuation: applies the candidate loss for result1.
DefenderContactResult calculate_defender_contact(const DefenderContactInput& input);
}

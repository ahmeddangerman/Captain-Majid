#pragma once

#include <cstdint>

namespace captain {
std::uint8_t standard_skill_index(std::uint8_t base, std::uint8_t level, std::uint8_t modifier);

struct ActionPowerInput {
    std::uint16_t base;
    std::uint8_t random_first;
    std::uint8_t random_second;
    bool weakened;
    std::uint8_t subtype;
    bool incoming_carry;
};
struct ActionPowerResult {
    std::uint16_t base;
    std::uint16_t multiplier;
    std::uint16_t power;
    std::uint8_t subtype;
};
ActionPowerResult calculate_action_power(const ActionPowerInput& input);
}

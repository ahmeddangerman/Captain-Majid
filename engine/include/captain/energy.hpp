#pragma once

#include <cstdint>

namespace captain {
struct EnergyInput {
    std::uint16_t stamina;
    std::uint16_t cost;
    std::uint8_t player_id;
    std::uint8_t side;
    std::uint8_t action;
    std::uint8_t subtype;
    bool skip_home_side; // Select the defensive wrapper that skips deduction for side zero.
    bool incoming_carry = false;
};

struct EnergyResult {
    std::uint16_t stamina;
    std::uint16_t cost;
    bool weakened;
    bool carry; // Original outgoing carry, distinct from the weakened flag.
};

EnergyResult calculate_energy(const EnergyInput& input);
}

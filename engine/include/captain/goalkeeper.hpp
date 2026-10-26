#pragma once

#include "captain/defender_contact.hpp"

namespace captain {
struct GoalkeeperInput {
    std::uint16_t incoming_power;
    DefenderRollInput roll; // Eligibility is derived from critical_threshold, not roll.eligible.
    std::uint8_t critical_threshold;
    std::uint8_t stored_timer;
    std::uint8_t elapsed;
    std::uint8_t stored_penalty;
    std::array<std::uint8_t, 4> thresholds;
};
struct GoalkeeperResult {
    std::uint16_t base;
    std::uint8_t strength;
    std::uint8_t ratio;
    std::uint8_t state_adjustment;
    std::uint8_t score;
    std::uint8_t result_code;
    std::uint8_t subtype;
    bool special;
};
GoalkeeperResult calculate_goalkeeper(const GoalkeeperInput& input);
}

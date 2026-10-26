#include "captain/goalkeeper.hpp"

namespace captain {
GoalkeeperResult calculate_goalkeeper(const GoalkeeperInput& input) {
    auto variation = input.roll;
    variation.eligible = variation.random_first > input.critical_threshold;
    const auto roll = roll_defender_strength(variation);
    const auto numerator = static_cast<std::uint16_t>(std::uint32_t(input.incoming_power) * 192);
    const unsigned quotient = roll.strength ? numerator / roll.strength : 65535;
    const auto ratio = static_cast<std::uint8_t>(quotient > 255 ? 255 : quotient);
    const unsigned timer = input.stored_timer > input.elapsed ? input.stored_timer - input.elapsed : 0;
    const unsigned penalty = timer + input.stored_penalty;
    const auto adjustment = static_cast<std::uint8_t>(penalty > 255 ? 255 : penalty);
    const unsigned total = unsigned(ratio) + adjustment;
    const auto score = static_cast<std::uint8_t>(total > 255 ? 255 : total);
    return {roll.base, roll.strength, ratio, adjustment, score,
            classify_score(score, input.thresholds), roll.subtype, roll.special};
}
}

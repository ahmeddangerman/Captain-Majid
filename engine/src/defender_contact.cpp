#include "captain/defender_contact.hpp"
#include "captain/action_strength.hpp"

#include <stdexcept>

namespace captain {
std::uint8_t classify_score(std::uint8_t score, const std::array<std::uint8_t, 4>& thresholds) {
    if (thresholds.back() != 0) {
        throw std::invalid_argument("Zero final threshold required");
    }
    for (std::size_t i = 1; i < thresholds.size(); ++i) {
        if (thresholds[i] > thresholds[i - 1]) {
            throw std::invalid_argument("Thresholds must be descending");
        }
    }
    std::uint8_t code = 0;
    while (score < thresholds[code]) ++code;
    return code;
}

DefenderRollResult roll_defender_strength(const DefenderRollInput& input) {
    if (!input.range) throw std::invalid_argument("Nonzero variation range required");
    const unsigned average = (unsigned(input.random_first) + input.random_second) / 2;
    const bool special = input.eligible && !input.weakened &&
        ((input.context == 4 && (input.defender_slot == 0 || input.defender_slot == 11)) ||
         input.random_second >= 248);
    const unsigned sample = average > input.range ? 1 + (average - 1) % input.range : average;
    const unsigned multiplier = special ? 256 + (average & 127) : sample + input.offset;
    const auto base = static_cast<std::uint16_t>(input.base >> (input.weakened ? 4 : 0));
    const std::uint32_t product = static_cast<std::uint32_t>(base) * multiplier;
    const auto strength = static_cast<std::uint8_t>(
        (product & 0x00FF0000u) ? 255 : (product >> 8) & 255);
    const auto subtype = static_cast<std::uint8_t>(input.subtype | (special ? 128 : 0));
    return {base, strength, subtype, special};
}

DefenderContactResult calculate_defender_contact(const DefenderContactInput& input) {
    const auto roll = roll_defender_strength({input.defender_power, input.random_first,
        input.random_second, input.range, input.offset, input.weakened, input.eligible,
        input.context, input.defender_slot, input.subtype});
    const auto strength = roll.strength;
    const auto loss = static_cast<std::uint8_t>(strength / 4);
    const auto numerator = static_cast<std::uint16_t>(std::uint32_t(input.incoming_power) * 64);
    const unsigned quotient = strength ? numerator / strength : 65535;
    const auto score = static_cast<std::uint8_t>(quotient > 255 ? 255 : quotient);
    const auto code = classify_score(score, input.thresholds);
    const auto remaining = code == 1 ? reduce_action_strength(input.incoming_power, loss)
                                     : input.incoming_power;
    return {roll.base, strength, loss, score, code, remaining, roll.subtype, roll.special};
}
}

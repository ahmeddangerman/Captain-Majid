#include "captain/defender_contact.hpp"
#include "captain/action_strength.hpp"

#include <stdexcept>

namespace captain {
DefenderContactResult calculate_defender_contact(const DefenderContactInput& input) {
    if (input.range == 0 || input.thresholds.back() != 0) {
        throw std::invalid_argument("Nonzero variation range and zero final threshold required");
    }
    for (std::size_t i = 1; i < input.thresholds.size(); ++i) {
        if (input.thresholds[i] > input.thresholds[i - 1]) {
            throw std::invalid_argument("Thresholds must be descending");
        }
    }

    const unsigned average = (unsigned(input.random_first) + input.random_second) / 2;
    const bool special = input.eligible && !input.weakened &&
        ((input.context == 4 && (input.defender_slot == 0 || input.defender_slot == 11)) ||
         input.random_second >= 248);
    const unsigned sample = average > input.range ? 1 + (average - 1) % input.range : average;
    const unsigned multiplier = special ? 256 + (average & 127) : sample + input.offset;
    const auto base = static_cast<std::uint16_t>(input.defender_power >> (input.weakened ? 4 : 0));
    const std::uint32_t product = static_cast<std::uint32_t>(base) * multiplier;
    const auto strength = static_cast<std::uint8_t>(
        (product & 0x00FF0000u) ? 255 : (product >> 8) & 255);
    const auto loss = static_cast<std::uint8_t>(strength / 4);
    const auto numerator = static_cast<std::uint16_t>(std::uint32_t(input.incoming_power) * 64);
    const unsigned quotient = strength ? numerator / strength : 65535;
    const auto score = static_cast<std::uint8_t>(quotient > 255 ? 255 : quotient);
    std::uint8_t code = 0;
    while (score < input.thresholds[code]) ++code;
    const auto remaining = code == 1 ? reduce_action_strength(input.incoming_power, loss)
                                     : input.incoming_power;
    const auto subtype = static_cast<std::uint8_t>(input.subtype | (special ? 128 : 0));
    return {base, strength, loss, score, code, remaining, subtype, special};
}
}

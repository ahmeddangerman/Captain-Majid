#include "captain/shot_power.hpp"

namespace captain {
std::uint8_t standard_skill_index(std::uint8_t base, std::uint8_t level, std::uint8_t modifier) {
    const auto doubled = static_cast<std::uint8_t>(unsigned(level) * 2);
    const auto leveled = static_cast<std::uint8_t>(unsigned(base) + doubled + (level >> 7));
    const auto capped = leveled > 191 ? 191 : leveled;
    const auto modified = static_cast<std::uint8_t>(unsigned(capped) + modifier);
    return modified > 191 ? 191 : modified;
}

ActionPowerResult calculate_action_power(const ActionPowerInput& input) {
    auto subtype = input.subtype;
    if (!input.weakened && input.random_first < 8) subtype |= 128;
    const bool carry = input.weakened ? input.incoming_carry : input.random_first >= 8;
    const unsigned sum = unsigned(input.random_first) + input.random_second + (carry ? 1 : 0);
    const unsigned average = sum / 2;
    const unsigned multiplier = (subtype & 128 ? 256 + (average & 127) : (average | 128)) + (sum & 1);
    const auto base = static_cast<std::uint16_t>(input.base >> (input.weakened ? 4 : 0));
    const auto power = static_cast<std::uint16_t>((std::uint32_t(base) * multiplier) >> 8);
    return {base, static_cast<std::uint16_t>(multiplier), power, subtype};
}
}

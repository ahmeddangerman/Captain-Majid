#include "captain/reference_random.hpp"

namespace captain {
ReferenceRandomInputs sample_reference_random(const std::array<std::uint8_t, 2048>& ram,
                                             std::uint8_t index, bool carry) {
    return {ram[0x300u + index], ram[0x700u + index], ram[(0x780u + index) & 0x7FFu], carry};
}

ReferenceRandomResult advance_reference_random(ReferenceRandomState state, ReferenceRandomInputs inputs) {
    const unsigned source_sum = unsigned(inputs.player_byte) + inputs.sound_byte + (inputs.carry ? 1 : 0);
    // Two rotations preserve both the source-sum carry and the old first byte's high bits.
    const unsigned rotated = ((unsigned(state.first) << 2) & 255u) |
                             (source_sum > 255 ? 2u : 0u) | (state.first >> 7);
    const unsigned mixed = ((source_sum & 255u) ^ 255u) + rotated + ((state.first >> 6) & 1u);
    const auto first = static_cast<std::uint8_t>(mixed);
    // A 6502 subtraction borrows when its incoming carry is clear.
    const int difference = int(first) - inputs.tail_byte - (mixed > 255 ? 0 : 1);
    const unsigned next = unsigned(static_cast<std::uint8_t>(difference)) + state.index +
                          (difference >= 0 ? 1u : 0u);
    return {{static_cast<std::uint8_t>(unsigned(state.index) + 1), first,
             static_cast<std::uint8_t>(next)}, next > 255};
}
}

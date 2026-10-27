#pragma once

#include <array>
#include <cstdint>

namespace captain {
struct ReferenceRandomState {
    std::uint8_t index;
    std::uint8_t first;
    std::uint8_t second;
};
struct ReferenceRandomInputs {
    std::uint8_t player_byte;
    std::uint8_t sound_byte;
    std::uint8_t tail_byte;
    bool carry;
};
struct ReferenceRandomResult {
    ReferenceRandomState state;
    bool carry;
};

ReferenceRandomInputs sample_reference_random(const std::array<std::uint8_t, 2048>& ram,
                                             std::uint8_t index, bool carry);
ReferenceRandomResult advance_reference_random(ReferenceRandomState state, ReferenceRandomInputs inputs);
}

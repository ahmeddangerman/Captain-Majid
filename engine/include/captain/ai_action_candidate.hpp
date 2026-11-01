#pragma once
#include <array>
#include <cstdint>
namespace captain {
// Eight packed candidates for an already selected AI category. This does not
// select the category or apply the later action/subtype adjustments.
std::uint8_t choose_ai_action_candidate(const std::array<std::uint8_t,4>& candidates,
                                       std::uint8_t random);
}

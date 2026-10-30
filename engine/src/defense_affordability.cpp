#include "captain/defense_affordability.hpp"

namespace captain {
bool can_select_defense_move(std::uint16_t stamina, std::uint16_t cost, std::uint8_t subtype) {
    const auto difference = static_cast<std::uint16_t>(unsigned(stamina) - unsigned(cost));
    return subtype == 0 || !(difference & 0x8000u);
}
}

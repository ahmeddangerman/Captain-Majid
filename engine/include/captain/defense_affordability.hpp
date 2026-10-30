#pragma once
#include <cstdint>

namespace captain {
// Numeric menu check only; cost loading, stamina deduction and UI are separate.
bool can_select_defense_move(std::uint16_t stamina, std::uint16_t cost, std::uint8_t subtype);
}

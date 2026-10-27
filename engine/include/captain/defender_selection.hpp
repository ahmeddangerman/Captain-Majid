#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace captain {
struct DefenderSelectionPlayer {
    std::uint8_t x;
    std::uint8_t y;
    bool status_nonzero;
};

struct DefenderSelectionInput {
    std::uint8_t possession_side; // The ROM's $05FB: 0 or 11.
    std::uint8_t ball_x;
    std::uint8_t ball_y;
    std::uint8_t radius;
    std::array<DefenderSelectionPlayer, 20> players; // IDs 1–10, then 12–21.
};

// Mirrors the ROM selector at $E4D7. Returns opposing field-player IDs in
// scan order when both coordinate deltas are strictly smaller than radius.
std::vector<std::uint8_t> select_field_defenders(const DefenderSelectionInput& input);
}

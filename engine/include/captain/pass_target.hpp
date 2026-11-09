#pragma once
#include <array>
#include <cstdint>

namespace captain {
struct OrdinaryPassTargetInitialization {
    std::uint8_t ball_x, ball_y, cell, cursor, ball_mode, attack_subtype;
};
// Ordinary target setup after an explicitly acknowledged pass-type selection.
OrdinaryPassTargetInitialization initialize_ordinary_pass_target(std::uint16_t carrier_x, std::uint16_t carrier_y);
std::uint8_t advance_ordinary_pass_cursor(std::uint8_t cursor, std::uint8_t held);
struct OrdinaryPassCandidates {
    std::array<std::uint8_t, 4> ids{};
    std::uint8_t count = 0, allied_count = 0;
};
OrdinaryPassCandidates select_ordinary_pass_candidates(
    std::uint8_t carrier, std::uint8_t cursor, const std::array<std::array<std::uint8_t, 2>, 22>& positions);
// Original $E059 target update for a supplied field recipient or sentinel255.
// Position bytes are ignored for sentinel255; unsupported player IDs reject.
std::uint8_t resolve_pass_target_cell(std::uint8_t recipient,std::uint8_t previous_cell,
                                      std::uint8_t recipient_x,std::uint8_t recipient_y);
}

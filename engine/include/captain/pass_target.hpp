#pragma once
#include <cstdint>

namespace captain {
struct OrdinaryPassTargetInitialization {
    std::uint8_t ball_x, ball_y, cell, cursor, ball_mode, attack_subtype;
};
// Ordinary target setup after an explicitly acknowledged pass-type selection.
OrdinaryPassTargetInitialization initialize_ordinary_pass_target(std::uint16_t carrier_x, std::uint16_t carrier_y);
// Original $E059 target update for a supplied field recipient or sentinel255.
// Position bytes are ignored for sentinel255; unsupported player IDs reject.
std::uint8_t resolve_pass_target_cell(std::uint8_t recipient,std::uint8_t previous_cell,
                                      std::uint8_t recipient_x,std::uint8_t recipient_y);
}

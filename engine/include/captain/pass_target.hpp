#pragma once
#include <cstdint>

namespace captain {
// Original $E059 target update for a supplied field recipient or sentinel255.
// Position bytes are ignored for sentinel255; unsupported player IDs reject.
std::uint8_t resolve_pass_target_cell(std::uint8_t recipient,std::uint8_t previous_cell,
                                      std::uint8_t recipient_x,std::uint8_t recipient_y);
}

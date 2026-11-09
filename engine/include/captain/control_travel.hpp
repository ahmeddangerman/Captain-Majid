#pragma once
#include "captain/phase_session.hpp"
#include <cstdint>

namespace captain {
// The E3CA..E406 travel update, after its caller has supplied the loaded speed.
std::uint16_t advance_control_travel(std::uint16_t travel, std::uint16_t speed,
                                     std::uint8_t side, std::uint8_t held,
                                     std::uint8_t facing);
// Apply only this original task fragment with explicitly supplied speed/input.
// Does not run a frame, input poll, sound, random or presentation.
PhaseSessionState advance_session_control_travel(const PhaseSessionState& input,
                                                 std::uint16_t speed, std::uint8_t held);
}

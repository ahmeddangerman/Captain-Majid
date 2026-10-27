#pragma once
#include "captain/phase_session.hpp"
#include <istream>
#include <ostream>

namespace captain {
// Private represented phase/clock state, not a complete game save or protocol.
PhaseSessionState load_phase_session(std::istream& input);
void write_phase_session(std::ostream& output, const PhaseSessionState& state);
}

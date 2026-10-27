#pragma once
#include "captain/field_phase.hpp"
#include <istream>
#include <ostream>

namespace captain {
// Bounded private development snapshot, not a complete game save/network format.
FieldPhaseState load_field_state(std::istream& input);
void write_field_state(std::ostream& output, const FieldPhaseState& state);
}

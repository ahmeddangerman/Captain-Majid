#pragma once
#include "captain/phase_session.hpp"
namespace captain {
// Explicit acknowledgement of the pending original presentation; numeric
// state is retained. Keeper-area winner handling remains a saved boundary.
PhaseSessionState continue_session_clearance(const PhaseSessionState& input);
}

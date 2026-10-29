#pragma once
#include "captain/phase_session.hpp"
namespace captain {
// Prepared DD1C dispatch through normal8509 or stunned8900, before presentation.
PhaseSessionState prepare_session_keeper_entry(const PhaseSessionState& input);
// Original8900..8917: explicit descending thresholds, final threshold zero.
PhaseSessionState resolve_session_stunned_keeper(const PhaseSessionState& input,
 const std::array<std::uint8_t,2>& thresholds);
}

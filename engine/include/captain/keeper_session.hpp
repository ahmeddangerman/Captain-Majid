#pragma once
#include "captain/phase_session.hpp"
#include "captain/shot_content.hpp"
namespace captain {
// Prepared DD1C dispatch through normal8509 or stunned8900, before presentation.
PhaseSessionState prepare_session_keeper_entry(const PhaseSessionState& input);
// Original8900..8917: explicit descending thresholds, final threshold zero.
PhaseSessionState resolve_session_stunned_keeper(const PhaseSessionState& input,
 const std::array<std::uint8_t,2>& thresholds);
// Caller supplies/acknowledges state after keeper choice presentation, before852F.
PhaseSessionState resolve_prepared_session_keeper(const PhaseSessionState& input,
 std::uint8_t action,std::uint8_t subtype,std::uint8_t opponent_team,
 const ShotContent& content);
}

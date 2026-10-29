#pragma once
#include "captain/phase_session.hpp"
#include "captain/kickoff_formation.hpp"
namespace captain {
// Explicit prepared boundary after original earlier reset/team selection and before DAE9.
PhaseSessionState prepare_session_kickoff_formation(const PhaseSessionState& input,
 std::uint8_t selected_side,const KickoffFormationContent& content);
PhaseSessionState prepare_session_kickoff_receiver(const PhaseSessionState& input,
 std::uint8_t random,const std::array<std::uint8_t,5>& candidates);
PhaseSessionState prepare_session_kickoff_ball(const PhaseSessionState& input);
}

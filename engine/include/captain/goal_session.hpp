#pragma once
#include "captain/phase_session.hpp"
#include "captain/shot_content.hpp"
namespace captain {
// Explicit baseline capture; post-keeper presentation acknowledged separately.
PhaseSessionState prepare_session_scored_goal(const PhaseSessionState& input,
 MatchScoreContext baseline,const ShotContent& content);
}

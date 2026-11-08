#pragma once
#include "captain/phase_session.hpp"
#include "captain/shot_content.hpp"
namespace captain {
// Bounded result0 goal for the sampled opponent shot.
PhaseSessionState prepare_sampled_opponent_goal(const PhaseSessionState& input,const ShotContent& content);
PhaseSessionState acknowledge_sampled_opponent_goal_presentation(const PhaseSessionState& input);
PhaseSessionState begin_sampled_opponent_goal_restart(const PhaseSessionState& input);
// Explicit baseline capture; post-keeper presentation acknowledged separately.
PhaseSessionState prepare_session_scored_goal(const PhaseSessionState& input,
 MatchScoreContext baseline,const ShotContent& content);
// Caller supplies state after goal presentations and the original post-goal menu.
PhaseSessionState continue_prepared_goal_restart(const PhaseSessionState& input);
}

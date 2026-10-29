#include "captain/goal_session.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState prepare_session_scored_goal(const PhaseSessionState& input,MatchScoreContext baseline,const ShotContent& content) {
 validate_phase_session(input);
 if((input.phase!=FieldSessionPhase::keeper_decided && input.phase!=FieldSessionPhase::stunned_keeper_decided) ||
    input.keeper_decision->result_code>1 ||
    (input.keeper_decision->result_code==1 && (input.recovery->attack_subtype&63u)>=3) || !content.experience)
  throw std::invalid_argument("Goal preparation requires a scored keeper result and experience content");
 if(input.score && (input.score->left_goals!=baseline.left_goals || input.score->right_goals!=baseline.right_goals || input.score->kickoff!=baseline.kickoff))
  throw std::invalid_argument("Captured score does not match the saved baseline");
 auto state=input;
 const auto reward=award_player_experience(*state.experience,state.current.field.roster[state.current.field.ball_owner].type,0,*content.experience);
 state.experience=reward.state;
 baseline.left_goals=static_cast<std::uint8_t>(unsigned(baseline.left_goals)+1u);
 state.score=baseline;state.phase=FieldSessionPhase::goal_celebration_pending;
 validate_phase_session(state);return state;
}
PhaseSessionState continue_prepared_goal_restart(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::goal_celebration_pending || input.current.clock.ball_mode!=0)
  throw std::invalid_argument("Post-goal restart requires acknowledged presentations/menu and ball mode zero");
 auto state=input;state.current.field.side=11;
 auto& keeper=state.current.field.keepers[1];keeper.timer=0;keeper.penalty=0;keeper.stun=0;
 state.score->kickoff=4;state.shot_flight.reset();state.ball_trajectory.reset();state.keeper_decision.reset();
 state.phase=FieldSessionPhase::goal_kickoff_pending;validate_phase_session(state);return state;
}

}

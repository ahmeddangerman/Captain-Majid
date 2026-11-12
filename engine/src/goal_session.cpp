#include "captain/goal_session.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState prepare_sampled_opponent_goal(const PhaseSessionState& input,const ShotContent& content) {
 validate_phase_session(input);
 const bool player19_return=input.phase==FieldSessionPhase::sampled_player19_return_keeper_compared;
 const bool score24=input.phase==FieldSessionPhase::sampled_score23_keeper_compared;
 const auto scorer=input.current.field.ball_owner;
 if((input.phase!=FieldSessionPhase::sampled_shot_keeper_compared && !player19_return && !score24) || !content.experience || scorer>21 || input.current.field.roster[scorer].type!=23)
  throw std::invalid_argument("Unsupported sampled opponent goal");
 const auto reward=award_player_experience(*input.experience,input.current.field.roster[scorer].type,0,*content.experience);
 if(!reward.slot || *reward.slot!=0 || reward.award!=15)
  throw std::invalid_argument("Unverified sampled opponent goal reward");
 auto state=input;state.experience=reward.state;++state.score->right_goals;
 state.recovery->opposing_player=player19_return ? 5 : (score24 && (scorer==17 || input.score->left_goals==3) ? 4 : 1);state.control->facing=(player19_return || (score24 && input.score->left_goals!=3)) ? 0 : 64;state.control->motion_requested=3;
 if(score24)state.area_defender_cursor=0;
 state.phase=score24 ? FieldSessionPhase::sampled_score24_goal_scored : player19_return ? FieldSessionPhase::sampled_player19_return_goal_scored : FieldSessionPhase::sampled_opponent_goal_scored;validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_opponent_goal_presentation(const PhaseSessionState& input) {
 validate_phase_session(input);
 const bool player19_return=input.phase==FieldSessionPhase::sampled_player19_return_goal_scored;
 const bool score24=input.phase==FieldSessionPhase::sampled_score24_goal_scored;
 if(input.phase!=FieldSessionPhase::sampled_opponent_goal_scored && !player19_return && !score24)
  throw std::invalid_argument("Unsupported sampled opponent goal presentation");
 auto state=input;state.current.field.side=0;state.control->motion_requested=0;
 state.phase=score24 ? FieldSessionPhase::sampled_score24_goal_presented : player19_return ? FieldSessionPhase::sampled_player19_return_goal_presented : FieldSessionPhase::sampled_opponent_goal_presented;validate_phase_session(state);return state;
}
PhaseSessionState begin_sampled_opponent_goal_restart(const PhaseSessionState& input) {
 validate_phase_session(input);
 const bool player19_return=input.phase==FieldSessionPhase::sampled_player19_return_goal_presented;
 const bool score24=input.phase==FieldSessionPhase::sampled_score24_goal_presented;
 if(input.phase!=FieldSessionPhase::sampled_opponent_goal_presented && !player19_return && !score24)
  throw std::invalid_argument("Unsupported sampled opponent goal restart");
 auto state=input;auto& keeper=state.current.field.keepers[0];keeper.timer=0;keeper.penalty=0;keeper.stun=0;
 state.phase=score24 ? FieldSessionPhase::sampled_score24_goal_restart : player19_return ? FieldSessionPhase::sampled_player19_return_goal_restart : FieldSessionPhase::sampled_opponent_goal_restart;validate_phase_session(state);return state;
}
PhaseSessionState prepare_session_scored_goal(const PhaseSessionState& input,MatchScoreContext baseline,const ShotContent& content) {
 validate_phase_session(input);
 const bool returned=input.phase==FieldSessionPhase::ordinary_return_goal_area_keeper_compared;
 if((input.phase!=FieldSessionPhase::keeper_decided && input.phase!=FieldSessionPhase::stunned_keeper_decided && !returned) ||
    input.keeper_decision->result_code>1 ||
    (input.keeper_decision->result_code==1 && (input.recovery->attack_subtype&63u)>=3) || !content.experience)
  throw std::invalid_argument("Goal preparation requires a scored keeper result and experience content");
 if(returned && input.control->motion_requested!=3)
  throw std::invalid_argument("Returned goal requires the observed presentation handoff");
 if(input.score && (input.score->left_goals!=baseline.left_goals || input.score->right_goals!=baseline.right_goals || input.score->kickoff!=baseline.kickoff))
  throw std::invalid_argument("Captured score does not match the saved baseline");
 auto state=input;
 const auto reward=award_player_experience(*state.experience,state.current.field.roster[state.current.field.ball_owner].type,0,*content.experience);
 state.experience=reward.state;
 if(state.current.field.side==0)baseline.left_goals=static_cast<std::uint8_t>(unsigned(baseline.left_goals)+1u);
 else baseline.right_goals=static_cast<std::uint8_t>(unsigned(baseline.right_goals)+1u);
 state.score=baseline;state.phase=returned ? FieldSessionPhase::ordinary_return_goal_area_goal_scored : FieldSessionPhase::goal_celebration_pending;
 validate_phase_session(state);return state;
}
PhaseSessionState continue_prepared_goal_restart(const PhaseSessionState& input) {
 validate_phase_session(input);
 const bool returned=input.phase==FieldSessionPhase::ordinary_return_goal_area_goal_scored;
 if((input.phase!=FieldSessionPhase::goal_celebration_pending && !returned) || input.current.clock.ball_mode!=0 ||
    (returned && (input.control->motion_requested!=0 || input.control->facing!=0)))
  throw std::invalid_argument("Post-goal restart requires acknowledged presentations/menu and ball mode zero");
 auto state=input;state.current.field.side=static_cast<std::uint8_t>(input.current.field.side^11u);
 auto& keeper=state.current.field.keepers[state.current.field.side==0?0:1];keeper.timer=0;keeper.penalty=0;keeper.stun=0;
 state.score->kickoff=4;state.shot_flight.reset();state.ball_trajectory.reset();state.keeper_decision.reset();
 if(returned)state.area_contact.reset();
 state.phase=FieldSessionPhase::goal_kickoff_pending;validate_phase_session(state);return state;
}

}

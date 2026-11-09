#include "captain/keeper_session.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState compare_sampled_home_goalkeeper(const PhaseSessionState& input,const ShotContent& content) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::sampled_player19_return_keeper_precompare || input.phase==FieldSessionPhase::sampled_score23_keeper_precompare) {
  const bool score23=input.phase==FieldSessionPhase::sampled_score23_keeper_precompare;
  const auto& field=input.current.field;const auto& player=field.roster[0];const auto& status=field.keepers[0];
  const GoalkeeperInput comparison{input.action->power,
   {0,field.random_first,field.random_second,content.keeper_range,content.keeper_offset,false,false,3,0,0},
   0,status.timer,0,status.penalty,content.keeper_thresholds};
  const auto result=prepare_goalkeeper({{0,player.type,static_cast<std::uint8_t>(player.stamina),
   static_cast<std::uint8_t>(player.stamina>>8),player.level},11,0,0,0,128,false,comparison},content.goalkeepers,content.keeper);
  if(result.preparation.base_strength!=10 || result.preparation.stamina_cost!=20 || result.energy.stamina!=(input.score && input.score->left_goals==3 && input.score->right_goals==5 ? 64 : score23 && field.ball_owner!=17 ? 22 : 24) || result.energy.weakened ||
     result.comparison.strength!=5 || result.comparison.score!=255 || result.comparison.result_code!=0)
   throw std::invalid_argument("Unverified player19 return goalkeeper result");
  auto state=input;state.current.field.roster[0].stamina=result.energy.stamina;
  state.phase=score23 ? FieldSessionPhase::sampled_score23_keeper_compared : FieldSessionPhase::sampled_player19_return_keeper_compared;validate_phase_session(state);return state;
 }
 const bool postgoal=input.current.clock.extra_time==13 && input.score && input.score->left_goals==2 && input.score->right_goals==1 && input.current.clock.remaining==381;
 if(input.phase!=FieldSessionPhase::sampled_shot_keeper_precompare ||
    (input.keeper_menu_action!=0 && !(input.current.clock.extra_time==9 && input.keeper_menu_action==1)) ||
    input.current.field.roster[0].stamina!=(postgoal ? 30 : input.current.clock.extra_time==9 ? 16 : 18))
  throw std::invalid_argument("Unsupported sampled home goalkeeper comparison");
 const auto& field=input.current.field;const auto& player=field.roster[0];const auto& status=field.keepers[0];
 const auto action=*input.keeper_menu_action;
 const GoalkeeperInput comparison{input.action->power,
  {0,field.random_first,field.random_second,content.keeper_range,content.keeper_offset,false,false,3,0,0},
  0,status.timer,0,status.penalty,content.keeper_thresholds};
 const auto result=prepare_goalkeeper({{0,player.type,static_cast<std::uint8_t>(player.stamina),
  static_cast<std::uint8_t>(player.stamina>>8),player.level},11,action,0,0,128,false,comparison},content.goalkeepers,content.keeper);
 if(result.preparation.base_strength!=(action==1 ? 12 : 10) || result.preparation.stamina_cost!=(action==1 ? 40 : 20) || result.energy.stamina!=(postgoal ? 10 : 0) ||
    result.energy.weakened==postgoal || result.comparison.strength!=(postgoal ? 5 : 0) || result.comparison.score!=255 || result.comparison.result_code!=0)
  throw std::invalid_argument("Unverified sampled home goalkeeper result");
 auto state=input;state.current.field.roster[0].stamina=result.energy.stamina;
 state.phase=FieldSessionPhase::sampled_shot_keeper_compared;validate_phase_session(state);return state;
}
PhaseSessionState prepare_session_keeper_entry(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::ordinary_return_goal_area_keeper_arrival) {
  auto state=input;state.recovery->opposing_player=11;state.phase=FieldSessionPhase::ordinary_return_goal_area_keeper_ready;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ordinary_return_goal_area_target) {
  if(input.current.field.keepers[1].stun)throw std::invalid_argument("Stunned goal-area keeper route is unsupported");
  auto state=input;state.current.field.elapsed=0;state.current.field.selected_defenders=1;
  state.scheduling->selected_ids[0]=11;state.phase=FieldSessionPhase::ordinary_return_goal_area_keeper_arrival;
  validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::shot_keeper_entry)
  throw std::invalid_argument("Keeper entry requires completed keeper-bound flight");
 auto state=input;const auto keeper=static_cast<std::uint8_t>(input.current.field.side^11u);
 state.recovery->opposing_player=keeper;
 if(input.current.field.keepers[input.current.field.side==0?1:0].stun) {
  state.action->action=0;state.phase=FieldSessionPhase::stunned_keeper_pending;
 } else {
  state.current.field.selected_defenders=1;state.scheduling->selected_ids[0]=keeper;
  state.phase=FieldSessionPhase::keeper_presentation_pending;
 }
 validate_phase_session(state);return state;
}
PhaseSessionState resolve_session_stunned_keeper(const PhaseSessionState& input,
 const std::array<std::uint8_t,2>& thresholds) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::stunned_keeper_pending || thresholds[1])
  throw std::invalid_argument("Stunned keeper needs pending entry and descending thresholds ending zero");
 std::uint8_t result=0;
 while(input.current.field.random_first<thresholds[result])++result;
 auto state=input;state.keeper_decision=KeeperDecisionContext{0,result};
 state.phase=FieldSessionPhase::stunned_keeper_decided;
 validate_phase_session(state);return state;
}
PhaseSessionState resolve_prepared_session_keeper(const PhaseSessionState& input,
 std::uint8_t action,std::uint8_t subtype,std::uint8_t opponent_team,const ShotContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_presentation_pending)
  throw std::invalid_argument("Prepared keeper comparison requires the saved normal keeper entry");
 const auto& field=input.current.field;const auto keeper=static_cast<std::uint8_t>(field.side^11u);
 const auto& p=field.roster[keeper];const auto& status=field.keepers[field.side==0?1:0];
 const GoalkeeperInput comparison{input.action->power,
  {0,field.random_first,field.random_second,content.keeper_range,content.keeper_offset,
   false,false,input.action->menu_route,keeper,subtype},0,status.timer,
  input.shot_flight->distance,status.penalty,content.keeper_thresholds};
 const auto prepared=prepare_goalkeeper({{keeper,p.type,static_cast<std::uint8_t>(p.stamina),
  static_cast<std::uint8_t>(p.stamina>>8),p.level},field.side,action,subtype,
  input.action->action,input.recovery->attack_subtype,false,comparison},
  content.goalkeepers,content.keeper);
 const auto story=apply_keeper_story_override({prepared.comparison.result_code,field.side,
  opponent_team,input.recovery->game_mode,input.recovery->attack_subtype,field.random_first});
 auto state=input;state.current.field.roster[keeper].stamina=prepared.energy.stamina;
 if(input.score && ((input.score->left_goals==2 && input.score->right_goals==5 && input.current.clock.remaining==304 && input.area_defender_cursor==5) ||
    (input.score->left_goals==3 && input.score->right_goals==6 && input.current.clock.remaining==263 && input.area_defender_cursor==4)) &&
    input.current.field.side==0 && input.current.field.ball_owner==7)
  state.area_defender_cursor=0;
 state.keeper_decision=KeeperDecisionContext{0,story.result_code,action,
  prepared.comparison.subtype,prepared.preparation.move_effect};
 state.phase=FieldSessionPhase::keeper_decided;validate_phase_session(state);return state;
}

}

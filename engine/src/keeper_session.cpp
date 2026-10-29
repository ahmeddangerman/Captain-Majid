#include "captain/keeper_session.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState prepare_session_keeper_entry(const PhaseSessionState& input) {
 validate_phase_session(input);
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
 state.keeper_decision=KeeperDecisionContext{0,story.result_code,action,
  prepared.comparison.subtype,prepared.preparation.move_effect};
 state.phase=FieldSessionPhase::keeper_decided;validate_phase_session(state);return state;
}

}

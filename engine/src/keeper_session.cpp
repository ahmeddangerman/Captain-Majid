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
}

#include "captain/match_client.hpp"
#include <limits>
#include <stdexcept>

namespace captain {
void ClientButtons::press(std::uint8_t mask) {
 pressed_=static_cast<std::uint8_t>(pressed_ | (mask & ~held_));
 held_=static_cast<std::uint8_t>(held_ | mask);
}
void ClientButtons::release(std::uint8_t mask) { held_=static_cast<std::uint8_t>(held_ & ~mask); }
void ClientButtons::clear() { held_=pressed_=0; }
ClientButtonSample ClientButtons::take() { const ClientButtonSample sample{held_,pressed_};pressed_=0;return sample; }

MatchInput client_input(const MatchRuntimeState& state,std::uint8_t held,std::uint8_t pressed) {
 if(state.last_sequence==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("Input sequence exhausted");
 MatchInput input{MatchInputKind::tick,state.last_sequence+1,held,pressed};
 const auto phase=state.session.phase;
 if(phase==FieldSessionPhase::attack_choice_pending && state.session.current.field.side==0 && state.session.action && state.session.action->menu_route==0) {
  if(state.session.action->action==0 || state.session.action->action==1){
   input.kind=MatchInputKind::action;input.action=state.session.action->action==0 ? MatchAction::shot : MatchAction::pass;
   input.held=input.pressed=0;return input;
  }
 }
 bool acknowledge=phase==FieldSessionPhase::attack_prepared || phase==FieldSessionPhase::selected_attack_presentation ||
  phase==FieldSessionPhase::ball_arrival_callback || phase==FieldSessionPhase::keeper_presentation_pending ||
  phase==FieldSessionPhase::keeper_decided || phase==FieldSessionPhase::goal_celebration_pending ||
  phase==FieldSessionPhase::ball_recovery || phase==FieldSessionPhase::keeper_punch_recovery ||
  phase==FieldSessionPhase::recovered_owner || phase==FieldSessionPhase::kickoff_ball_placed;
 if(phase==FieldSessionPhase::ordinary_pass_prepared)
  acknowledge=state.pass_stage==MatchPassStage::awaiting_presentation || state.pass_stage==MatchPassStage::flight_arrived;
 if(state.session.current.field.side==11 && (phase==FieldSessionPhase::field_contact_pending || phase==FieldSessionPhase::field)) {
  if(!state.defense_menu)acknowledge=phase==FieldSessionPhase::field_contact_pending;
  else {const auto stage=state.defense_menu->stage;
   acknowledge=stage==MatchDefenseStage::attack_prepared || stage==MatchDefenseStage::dispatch_pending || stage==MatchDefenseStage::ai_flight_arrived;
  }
 }
 if(acknowledge){input.kind=MatchInputKind::presentation_complete;input.held=input.pressed=0;}
 return input;
}
}

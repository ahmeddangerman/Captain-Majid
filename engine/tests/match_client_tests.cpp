#include "captain/match_client.hpp"
#include <cassert>
#include <limits>
#include <stdexcept>

int main() {
 using namespace captain;
 ClientButtons keys;keys.press(128);keys.press(128);auto first=keys.take();
 assert(first.held==128 && first.pressed==128);auto repeat=keys.take();assert(repeat.held==128 && repeat.pressed==0);
 keys.release(128);keys.press(128);auto second=keys.take();assert(second.held==128 && second.pressed==128);
 keys.press(1);keys.clear();auto unfocused=keys.take();assert(!unfocused.held && !unfocused.pressed);
 MatchRuntimeState state{};state.mode=MatchTimingMode::provisional_demo;state.last_sequence=17;
 state.session.phase=FieldSessionPhase::control_ready;
 auto input=client_input(state,1,128);
 assert(input.kind==MatchInputKind::tick && input.sequence==18 && input.held==1 && input.pressed==128);
 state.session.phase=FieldSessionPhase::human_action_menu;
 input=client_input(state,8,8);assert(input.kind==MatchInputKind::tick && input.held==8 && input.pressed==8);
 state.session.phase=FieldSessionPhase::attack_choice_pending;state.session.action=ActionContext{};
 input=client_input(state,1,128);assert(input.kind==MatchInputKind::action && input.action==MatchAction::shot && !input.held && !input.pressed && !input.target);
 state.session.action->action=1;input=client_input(state,1,128);assert(input.kind==MatchInputKind::action && input.action==MatchAction::pass);
 state.session.action->action=2;input=client_input(state,0,0);assert(input.kind==MatchInputKind::tick);
 for(const auto phase:{FieldSessionPhase::attack_prepared,FieldSessionPhase::selected_attack_presentation,
     FieldSessionPhase::ball_arrival_callback,FieldSessionPhase::keeper_presentation_pending,
     FieldSessionPhase::keeper_decided,FieldSessionPhase::goal_celebration_pending,
     FieldSessionPhase::ball_recovery,FieldSessionPhase::keeper_punch_recovery,
     FieldSessionPhase::recovered_owner,FieldSessionPhase::kickoff_ball_placed}) {
  state.session.phase=phase;input=client_input(state,1,128);
  assert(input.kind==MatchInputKind::presentation_complete && input.sequence==18 && !input.held && !input.pressed);
 }
 state.session.phase=FieldSessionPhase::ordinary_pass_prepared;
 for(const auto stage:{MatchPassStage::awaiting_presentation,MatchPassStage::flight_arrived}){
  state.pass_stage=stage;assert(client_input(state,0,0).kind==MatchInputKind::presentation_complete);
 }
 state.pass_stage=MatchPassStage::flight_cell;assert(client_input(state,1,128).kind==MatchInputKind::tick);
 state.session.phase=FieldSessionPhase::field_contact_pending;state.session.current.field.side=11;
 assert(client_input(state,0,0).kind==MatchInputKind::presentation_complete);
 state.defense_menu=MatchDefenseMenu{};
 for(const auto stage:{MatchDefenseStage::attack_prepared,MatchDefenseStage::dispatch_pending,MatchDefenseStage::ai_flight_arrived}){
  state.defense_menu->stage=stage;assert(client_input(state,0,0).kind==MatchInputKind::presentation_complete);
 }
 state.defense_menu->stage=MatchDefenseStage::polling;input=client_input(state,4,128);
 assert(input.kind==MatchInputKind::tick && input.held==4 && input.pressed==128);
 state.session.current.field.side=0;state.defense_menu.reset();assert(client_input(state,0,0).kind==MatchInputKind::tick);
 state.last_sequence=std::numeric_limits<std::uint64_t>::max();bool rejected=false;
 try{(void)client_input(state,0,0);}catch(const std::overflow_error&){rejected=true;}assert(rejected);
}

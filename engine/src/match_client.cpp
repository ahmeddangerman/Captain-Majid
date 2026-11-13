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
ClientButtonSample ClientButtons::take() { const ClientButtonSample sample{static_cast<std::uint8_t>(held_ | pressed_),pressed_};pressed_=0;return sample; }

MatchInput client_input(const MatchRuntimeState& state,std::uint8_t held,std::uint8_t pressed) {
 if(state.last_sequence==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("Input sequence exhausted");
 MatchInput input{MatchInputKind::tick,state.last_sequence+1,held,pressed};
 const auto phase=state.session.phase;
 if(phase==FieldSessionPhase::keeper_presentation_pending && state.session.current.field.side==11 && state.session.action && state.session.action->menu_route==4)return input;
 if(phase==FieldSessionPhase::attack_choice_pending && state.session.current.field.side==0 && state.session.action && state.session.action->menu_route==0) {
  if(state.session.action->action==0 || state.session.action->action==1){
   input.kind=MatchInputKind::action;input.action=state.session.action->action==0 ? MatchAction::shot : MatchAction::pass;
   input.held=input.pressed=0;return input;
  }
 }
 if(phase==FieldSessionPhase::attack_choice_pending && state.session.current.field.ball_owner==5 && state.session.current.field.ball_x==71 && state.session.current.field.ball_y==109 && state.session.current.clock.remaining==1 && state.session.action && state.session.action->menu_route==2 && state.session.action->action==1){input.kind=MatchInputKind::action;input.action=MatchAction::pass;input.held=input.pressed=0;return input;}
 if(phase==FieldSessionPhase::attack_choice_pending && (state.session.current.field.ball_owner==8 || state.session.current.field.ball_owner==6 || state.session.current.field.ball_owner==4 || state.session.current.field.ball_owner==3) && state.session.action && state.session.action->menu_route==1 && state.session.action->action==0){input.kind=MatchInputKind::action;input.action=MatchAction::shot;input.held=input.pressed=0;return input;}
 bool acknowledge=phase==FieldSessionPhase::observed_period_display || phase==FieldSessionPhase::observed_period_dispatch || phase==FieldSessionPhase::observed_interval_stamina || phase==FieldSessionPhase::observed_interval_restart || phase==FieldSessionPhase::attack_prepared || phase==FieldSessionPhase::selected_attack_presentation ||
  phase==FieldSessionPhase::ball_arrival_callback || phase==FieldSessionPhase::keeper_presentation_pending ||
  phase==FieldSessionPhase::keeper_decided || phase==FieldSessionPhase::goal_celebration_pending ||
  phase==FieldSessionPhase::ball_recovery || phase==FieldSessionPhase::keeper_punch_recovery ||
  phase==FieldSessionPhase::recovered_owner || phase==FieldSessionPhase::kickoff_ball_placed;
 if(state.mode==MatchTimingMode::provisional_demo && (phase==FieldSessionPhase::ordinary_pass_defender_compared || phase==FieldSessionPhase::ordinary_pass_next_defender_compared ||
    phase==FieldSessionPhase::ordinary_pass_dispatch_clock || phase==FieldSessionPhase::ordinary_pass_flight_arrival ||
    phase==FieldSessionPhase::ordinary_pass_recipient_positioned || phase==FieldSessionPhase::ordinary_free_pass_contest_winner))acknowledge=true;
 if(phase==FieldSessionPhase::player8_area_shot_prepared || phase==FieldSessionPhase::player4_area_shot_prepared || phase==FieldSessionPhase::player6_area_shot_prepared || phase==FieldSessionPhase::player3_area_shot_prepared || phase==FieldSessionPhase::player3_area_contact_compared || phase==FieldSessionPhase::player3_area_shot_field_moved || phase==FieldSessionPhase::player3_area_keeper_ready || phase==FieldSessionPhase::player3_area_keeper_decided || phase==FieldSessionPhase::player3_area_goal_scored || phase==FieldSessionPhase::player8_area_recovered_owner || phase==FieldSessionPhase::player4_area_recovered_owner || (phase==FieldSessionPhase::keeper_area_prepared && state.session.action && state.session.action->menu_route==1))acknowledge=true;
 if(((state.session.current.field.ball_owner==19 && state.session.current.field.ball_x==52 && state.session.current.field.ball_y==124) || (state.session.current.field.ball_owner==16 && state.session.current.field.ball_x==68 && state.session.current.field.ball_y==141) || (state.session.current.field.ball_owner==17 && state.session.current.field.ball_x==70 && state.session.current.field.ball_y==104 && state.session.current.clock.remaining==170)) && (phase==FieldSessionPhase::keeper_area_defense_initial || phase==FieldSessionPhase::keeper_area_defense_preparation || phase==FieldSessionPhase::keeper_area_attack_prepared))acknowledge=true;
 if(state.session.current.field.side==0 && state.session.current.field.ball_owner==5 && state.session.current.field.ball_x==71 && state.session.current.field.ball_y==109 && state.session.current.clock.remaining==0 && (phase==FieldSessionPhase::keeper_area_contact_added_time || phase==FieldSessionPhase::keeper_area_contact_clock_done))acknowledge=true;
 if(phase==FieldSessionPhase::keeper_area_prepared && state.session.current.field.side==0 && state.session.current.field.ball_owner==5 && state.session.current.field.ball_x==71 && state.session.current.field.ball_y==109 && state.session.current.clock.remaining==1)acknowledge=true;
 if(phase==FieldSessionPhase::keeper_area_attack_prepared && state.session.action && state.session.action->menu_route==1 && !state.session.current.field.selected_defenders)acknowledge=true;
 if(phase==FieldSessionPhase::keeper_area_attack_continuation && state.session.action && state.session.action->menu_route==1 && !state.session.current.field.selected_defenders)acknowledge=true;
 if(phase==FieldSessionPhase::area_ball_arrival_callback && state.session.action && state.session.action->menu_route==1 && state.session.current.field.ball_owner==17)acknowledge=true;
 if(phase==FieldSessionPhase::area_pass_defenders_pending && state.session.action && state.session.action->menu_route==1 && state.session.current.field.ball_owner==17)acknowledge=true;
 if(phase==FieldSessionPhase::area_pass_variant_restored && state.session.action && state.session.action->menu_route==1 && state.session.current.field.ball_owner==17)acknowledge=true;
 if(phase==FieldSessionPhase::area_pass_defender_captured && state.session.action && state.session.action->menu_route==1 && state.session.current.field.ball_owner==1)acknowledge=true;
 if((phase==FieldSessionPhase::ordinary_return_shot_prepared || phase==FieldSessionPhase::ordinary_return_shot_defender_compared) && state.session.current.field.ball_owner==1 && state.session.current.field.ball_x==116 && state.session.current.field.ball_y==100)acknowledge=true;
 if(phase==FieldSessionPhase::ordinary_return_shot_flight_arrival && state.session.shot_flight && state.session.shot_flight->loose_ball && state.session.shot_flight->destination==221)acknowledge=true;
 if(phase==FieldSessionPhase::keeper_possession && state.pass_stage==MatchPassStage::flight_arrived)acknowledge=true;
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

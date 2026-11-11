#include "captain/match_runtime.hpp"
#include "captain/control_session.hpp"
#include "captain/ball_step.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>
namespace captain {
namespace {
void advance_demo_random(MatchRuntimeState& state) {
 auto random=state.demo_rng;random^=random<<13;random^=random>>17;random^=random<<5;
 state.demo_rng=random;state.session.current.field.random_first=static_cast<std::uint8_t>(random);
 state.session.current.field.random_second=static_cast<std::uint8_t>(random>>8);
}
}
const char* capability_name(UnsupportedCapability c) {
 switch(c) {case UnsupportedCapability::content:return "content";case UnsupportedCapability::initialization:return "match initialization";case UnsupportedCapability::input_sequence:return "input sequence";case UnsupportedCapability::ball_fraction:return "ball fractional position is unknown";case UnsupportedCapability::random_state:return "random index is not present in development snapshot";case UnsupportedCapability::timing:return "live timing and random scheduling";case UnsupportedCapability::action:return "action routing";case UnsupportedCapability::defender_contact:return "pass-flight defender contact";case UnsupportedCapability::defensive_prompt:return "opponent contact defensive prompt";case UnsupportedCapability::presentation:return "presentation completion";case UnsupportedCapability::phase:return "session phase";} return "unknown";
}
MatchBallView match_ball_view(const MatchRuntimeState& state) {
 const bool flight=state.pass_stage==MatchPassStage::flight_ready || state.pass_stage==MatchPassStage::flight_cell || state.pass_stage==MatchPassStage::flight_arrived;
 if(flight){
  if(!state.pass_trajectory || !state.pass_cell)throw std::invalid_argument("Missing runtime flight ball context");
  return {static_cast<std::uint8_t>(state.pass_trajectory->x>>8),static_cast<std::uint8_t>(state.pass_trajectory->y>>8),*state.pass_cell,true};
 }
 const auto& field=state.session.current.field;return {field.ball_x,field.ball_y,field.ball_cell,false};
}
MatchStepResult create_match(const MatchConfig& config,const MatchContent& content) {
 MatchStepResult result{};
 if(!content.data){result.unsupported=UnsupportedCapability::content;return result;}
 if(!config.initial || config.clock || (config.mode!=MatchTimingMode::original_backed && config.mode!=MatchTimingMode::provisional_demo) || (config.mode==MatchTimingMode::original_backed && config.seed) || (config.mode==MatchTimingMode::provisional_demo && !config.seed)){result.unsupported=UnsupportedCapability::initialization;return result;}
 validate_phase_session(*config.initial);
 result.state.session=*config.initial;
 result.state.random_index=config.random_index;
 result.state.mode=config.mode;result.state.demo_rng=config.seed;result.state.ball_fraction=config.ball_fraction;
 project_field_random_memory(result.state.session.current.field,result.state.random_memory);
 return result;
}
MatchStepResult step_match(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(!content.data){result.unsupported=UnsupportedCapability::content;return result;}
 if(event.sequence<=input.last_sequence){result.unsupported=UnsupportedCapability::input_sequence;return result;}
 if(input.session.phase==FieldSessionPhase::player_control && event.kind==MatchInputKind::tick) {
  auto prepared=prepare_session_control(input.session);
  result.state.session=prepared.state;result.state.last_sequence=event.sequence;
  if(result.state.session.current.field.ball_x!=input.session.current.field.ball_x || result.state.session.current.field.ball_y!=input.session.current.field.ball_y)result.state.ball_fraction.reset();
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);
  for(const auto& request:prepared.presentation) result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && event.kind==MatchInputKind::tick && input.session.current.field.side==0 &&
    (input.session.phase==FieldSessionPhase::action_menu_pending || input.session.phase==FieldSessionPhase::field_contact_pending || input.session.phase==FieldSessionPhase::human_action_menu)) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->action_menu){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  auto menu=(next.session.phase==FieldSessionPhase::action_menu_pending || next.session.phase==FieldSessionPhase::field_contact_pending)
    ? open_human_action_menu(next.session,*content.data->action_menu)
    : poll_human_action_menu(next.session,event.pressed,*content.data->action_menu);
  next.session=menu.state;next.last_sequence=event.sequence;
  project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);
  for(const auto& request:menu.presentation)result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::control_ready && event.kind==MatchInputKind::tick) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->movement || !content.data->control_movement){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  auto poll=poll_session_control(next.session,event.held,event.pressed,content.data->field_players,*content.data->movement,*content.data->control_movement);
  next.session=poll.state;next.last_sequence=event.sequence;
  if((poll.moved_player && *poll.moved_player==input.session.current.field.ball_owner) || next.session.current.field.ball_x!=input.session.current.field.ball_x || next.session.current.field.ball_y!=input.session.current.field.ball_y)next.ball_fraction.reset();
  project_field_random_memory(next.session.current.field,next.random_memory);
  result.state=std::move(next);
  for(const auto& request:poll.presentation)result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::attack_choice_pending &&
    input.session.current.field.side==0 && input.session.action && input.session.action->action==1 &&
    input.session.action->menu_route==0 && event.kind==MatchInputKind::action && event.action==MatchAction::pass && !event.target) {
  result.state.session=begin_ordinary_human_pass_target(input.session);
  if(result.state.session.current.field.ball_x!=input.session.current.field.ball_x || result.state.session.current.field.ball_y!=input.session.current.field.ball_y)result.state.ball_fraction.reset();
  result.state.last_sequence=event.sequence;
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_human_pass_target && event.kind==MatchInputKind::tick) {
  if(event.pressed&64u || ((event.pressed&128u) && (!input.session.ordinary_pass_candidates || input.session.ordinary_pass_candidates->candidates.allied_count>1))) {
   result.unsupported=UnsupportedCapability::action;return result;
  }
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  auto next=input;advance_demo_random(next);
  next.session=(event.pressed&128u) ? confirm_ordinary_human_pass_target(next.session)
    : poll_ordinary_human_pass_cursor(next.session,event.held,event.pressed).state;
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);
  result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_human_pass_confirmed &&
    !input.session.current.field.selected_defenders && event.kind==MatchInputKind::tick) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(content.data->field_players.skills.empty() || content.data->attack.moves[1].empty()) {result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  next.session=prepare_ordinary_unopposed_pass(next.session,content.data->field_players,content.data->attack);
  next.pass_stage=MatchPassStage::awaiting_presentation;
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);
  result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::awaiting_presentation) {
  if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
  if(!input.session.recovery || input.session.recovery->ball_variant || input.session.current.field.selected_defenders) {
   result.unsupported=UnsupportedCapability::phase;return result;
  }
  result.state.session.recovery->ball_variant=1;
  validate_phase_session(result.state.session);
  result.state.pass_stage=MatchPassStage::handoff;result.state.last_sequence=event.sequence;
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::handoff && event.kind==MatchInputKind::tick) {
  validate_phase_session(input.session);
  if(!content.pass_distance){result.unsupported=UnsupportedCapability::content;return result;}
  const auto& field=input.session.current.field;
  if(field.elapsed || !input.session.ordinary_pass_recipient || !input.session.human_pass_cursor ||
     *input.session.ordinary_pass_recipient==255 || *input.session.ordinary_pass_recipient<1 || *input.session.ordinary_pass_recipient>10) {
   result.unsupported=UnsupportedCapability::phase;return result;
  }
  const auto recipient=*input.session.ordinary_pass_recipient;
  const auto& player=field.motion[field_motion_index(recipient)];
  const auto destination=resolve_pass_target_cell(recipient,*input.session.human_pass_cursor,
      static_cast<std::uint8_t>(player.x>>8),static_cast<std::uint8_t>(player.y>>8));
  if(destination==255){result.unsupported=UnsupportedCapability::action;return result;}
  const auto distance=calculate_pass_distance(field.ball_x,field.ball_y,destination,*content.pass_distance);
  result.state.pass_target=MatchPassTarget{recipient,destination,255,distance};
  result.state.pass_stage=MatchPassStage::target_ready;result.state.last_sequence=event.sequence;
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::target_ready && event.kind==MatchInputKind::tick) {
  validate_phase_session(input.session);
  if(!content.data->movement || !content.ball_velocity){result.unsupported=UnsupportedCapability::content;return result;}
  if(!input.ball_fraction){result.unsupported=UnsupportedCapability::ball_fraction;return result;}
  if(!input.pass_target || !input.session.ordinary_pass_recipient || input.pass_target->recipient!=*input.session.ordinary_pass_recipient ||
     input.pass_target->destination>=240 || input.pass_target->counter!=255 || input.session.current.field.ball_cell==input.pass_target->destination){
   result.unsupported=UnsupportedCapability::phase;return result;
  }
  const auto& field=input.session.current.field;
  BallTrajectoryContext trajectory{static_cast<std::uint16_t>((unsigned(field.ball_x)<<8)|input.ball_fraction->x),
    static_cast<std::uint16_t>((unsigned(field.ball_y)<<8)|input.ball_fraction->y),0,0,0,1};
  trajectory.direction=calculate_player_direction({trajectory.x,trajectory.y,input.pass_target->destination,0},field.ball_cell,content.data->movement->geometry);
  const auto velocity=calculate_ball_velocity(trajectory.direction,*content.ball_velocity);
  trajectory.vx=velocity.x;trajectory.vy=velocity.y;
  result.state.pass_trajectory=trajectory;result.state.pass_cell=field.ball_cell;result.state.session.recovery->x_jitter_control=trajectory.direction;
  result.state.session.saved_ball_flight_flag=1;result.state.pass_stage=MatchPassStage::flight_ready;
  result.state.last_sequence=event.sequence;validate_phase_session(result.state.session);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::flight_ready && event.kind==MatchInputKind::tick) {
  validate_phase_session(input.session);
  if(!input.pass_target || !input.pass_trajectory || !input.pass_cell){result.unsupported=UnsupportedCapability::phase;return result;}
  const auto& t=*input.pass_trajectory;
  const auto step=advance_ball_to_next_cell({t.x,t.y,{t.vx,t.vy},*input.pass_cell,input.pass_target->destination});
  result.state.pass_trajectory->x=step.x;result.state.pass_trajectory->y=step.y;result.state.pass_cell=step.cell;
  result.state.pass_stage=step.event==BallStepEvent::cell_changed ? MatchPassStage::flight_cell : MatchPassStage::flight_arrived;
  result.state.last_sequence=event.sequence;return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::flight_cell && event.kind==MatchInputKind::tick) {
  validate_phase_session(input.session);
  if(!input.pass_cell || !input.pass_target || !input.pass_trajectory){result.unsupported=UnsupportedCapability::phase;return result;}
  const auto& field=input.session.current.field;FlightDefenderSelectionInput selection{};
  selection.possession_side=field.side;selection.ball_cell=*input.pass_cell;selection.random_first=field.random_first;
  selection.random_second=field.random_second;selection.threshold=input.pass_target->counter;
  for(unsigned i=0;i<field.motion.size();++i){const auto& player=field.motion[i];
   selection.players[i]={static_cast<std::uint8_t>(player.x>>8),static_cast<std::uint8_t>(player.y>>8),player.status!=0};}
  if(!select_flight_defenders(selection).empty()){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  result.state.pass_stage=MatchPassStage::flight_ready;result.state.last_sequence=event.sequence;return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::flight_arrived) {
  if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
  validate_phase_session(input.session);
  if(!input.pass_target || !input.pass_trajectory || !input.pass_cell || !input.session.ordinary_pass_recipient ||
     input.pass_target->recipient!=*input.session.ordinary_pass_recipient || input.pass_target->counter!=255 || input.pass_trajectory->flags!=1 ||
     *input.pass_cell!=input.pass_target->destination || input.session.current.field.selected_defenders){result.unsupported=UnsupportedCapability::phase;return result;}
  const auto recipient=input.pass_target->recipient;
  if(recipient<1 || recipient>10){result.unsupported=UnsupportedCapability::action;return result;}
  const auto& player=input.session.current.field.motion[field_motion_index(recipient)];
  const auto x=static_cast<std::uint8_t>(input.pass_trajectory->x>>8),y=static_cast<std::uint8_t>(input.pass_trajectory->y>>8);
  if((player.x>>8)!=x || (player.y>>8)!=y){result.unsupported=UnsupportedCapability::phase;return result;}
  auto& state=result.state;state.session.current.field.ball_owner=recipient;state.session.current.field.ball_x=x;state.session.current.field.ball_y=y;state.session.current.field.ball_cell=*input.pass_cell;
  state.session.phase=FieldSessionPhase::player_control;state.session.human_pass_cursor.reset();state.session.human_pass_steps=0;
  state.session.ordinary_pass_candidates.reset();state.session.ordinary_pass_recipient.reset();state.session.ordinary_pass_defenses.reset();
  state.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(input.pass_trajectory->x),static_cast<std::uint8_t>(input.pass_trajectory->y)};
  state.pass_target.reset();state.pass_trajectory.reset();state.pass_cell.reset();state.pass_stage=MatchPassStage::none;
  state.last_sequence=event.sequence;validate_phase_session(state.session);project_field_random_memory(state.session.current.field,state.random_memory);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::control_clock_pending && event.kind==MatchInputKind::tick) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->world_step){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  auto clock=advance_control_tick(next.session,content.data->field_players,content.data->goalkeepers,*content.data->world_step);
  next.session=clock.state;next.last_sequence=event.sequence;
  project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);
  for(const auto& request:clock.presentation)result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::field_round_pending && event.kind==MatchInputKind::tick) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->movement || !content.data->control_movement || !content.data->world_step){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  std::array<LogicalButtons,22> buttons{};for(auto& item:buttons)item.held=event.held;buttons[0].pressed=event.pressed;
  auto round=advance_control_field_round(next.session,content.data->field_players,*content.data->movement,*content.data->control_movement,*content.data->world_step,buttons);
  next.session=round.state;next.last_sequence=event.sequence;next.ball_fraction.reset();
  project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);
  for(const auto& request:round.presentation)result.events.push_back({request});
  return result;
 }
 if(input.session.phase==FieldSessionPhase::field_contact_pending && input.session.current.field.side==11){
  result.unsupported=UnsupportedCapability::defensive_prompt;return result;
 }
 if(input.session.phase==FieldSessionPhase::field){result.unsupported=UnsupportedCapability::initialization;return result;}
 if(event.kind==MatchInputKind::action)result.unsupported=UnsupportedCapability::action;
 else if(event.kind==MatchInputKind::presentation_complete)result.unsupported=UnsupportedCapability::presentation;
 else if(input.session.phase==FieldSessionPhase::control_ready)result.unsupported=input.random_index ? UnsupportedCapability::timing : UnsupportedCapability::random_state;
 else result.unsupported=UnsupportedCapability::phase;
 return result;
}
}

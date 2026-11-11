#include "captain/match_runtime.hpp"
#include "captain/control_session.hpp"
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
 switch(c) {case UnsupportedCapability::content:return "content";case UnsupportedCapability::initialization:return "match initialization";case UnsupportedCapability::input_sequence:return "input sequence";case UnsupportedCapability::random_state:return "random index is not present in development snapshot";case UnsupportedCapability::timing:return "live timing and random scheduling";case UnsupportedCapability::action:return "action routing";case UnsupportedCapability::presentation:return "presentation completion";case UnsupportedCapability::phase:return "session phase";} return "unknown";
}
MatchStepResult create_match(const MatchConfig& config,const MatchContent& content) {
 MatchStepResult result{};
 if(!content.data){result.unsupported=UnsupportedCapability::content;return result;}
 if(!config.initial || config.clock || (config.mode!=MatchTimingMode::original_backed && config.mode!=MatchTimingMode::provisional_demo) || (config.mode==MatchTimingMode::original_backed && config.seed) || (config.mode==MatchTimingMode::provisional_demo && !config.seed)){result.unsupported=UnsupportedCapability::initialization;return result;}
 validate_phase_session(*config.initial);
 result.state.session=*config.initial;
 result.state.random_index=config.random_index;
 result.state.mode=config.mode;result.state.demo_rng=config.seed;
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
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);
  for(const auto& request:prepared.presentation) result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && event.kind==MatchInputKind::tick &&
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
  project_field_random_memory(next.session.current.field,next.random_memory);
  result.state=std::move(next);
  for(const auto& request:poll.presentation)result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::attack_choice_pending &&
    input.session.current.field.side==0 && input.session.action && input.session.action->action==1 &&
    input.session.action->menu_route==0 && event.kind==MatchInputKind::action && event.action==MatchAction::pass && !event.target) {
  result.state.session=begin_ordinary_human_pass_target(input.session);
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
 if(input.session.phase==FieldSessionPhase::field){result.unsupported=UnsupportedCapability::initialization;return result;}
 if(event.kind==MatchInputKind::action)result.unsupported=UnsupportedCapability::action;
 else if(event.kind==MatchInputKind::presentation_complete)result.unsupported=UnsupportedCapability::presentation;
 else if(input.session.phase==FieldSessionPhase::control_ready)result.unsupported=input.random_index ? UnsupportedCapability::timing : UnsupportedCapability::random_state;
 else result.unsupported=UnsupportedCapability::phase;
 return result;
}
}

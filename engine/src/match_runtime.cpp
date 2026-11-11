#include "captain/match_runtime.hpp"
#include "captain/control_session.hpp"
#include <stdexcept>
namespace captain {
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
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::control_ready && event.kind==MatchInputKind::tick) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->movement || !content.data->control_movement){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;auto random=next.demo_rng;
  random^=random<<13;random^=random>>17;random^=random<<5;
  next.demo_rng=random;next.session.current.field.random_first=static_cast<std::uint8_t>(random);
  next.session.current.field.random_second=static_cast<std::uint8_t>(random>>8);
  auto poll=poll_session_control(next.session,event.held,event.pressed,content.data->field_players,*content.data->movement,*content.data->control_movement);
  next.session=poll.state;next.last_sequence=event.sequence;
  project_field_random_memory(next.session.current.field,next.random_memory);
  result.state=std::move(next);
  for(const auto& request:poll.presentation)result.events.push_back({request});
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

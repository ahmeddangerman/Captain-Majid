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
 if(!config.initial || config.seed || config.clock){result.unsupported=UnsupportedCapability::initialization;return result;}
 validate_phase_session(*config.initial);
 result.state.session=*config.initial;
 result.state.random_index=config.random_index;
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
 if(input.session.phase==FieldSessionPhase::field){result.unsupported=UnsupportedCapability::initialization;return result;}
 if(event.kind==MatchInputKind::action)result.unsupported=UnsupportedCapability::action;
 else if(event.kind==MatchInputKind::presentation_complete)result.unsupported=UnsupportedCapability::presentation;
 else if(input.session.phase==FieldSessionPhase::control_ready)result.unsupported=input.random_index ? UnsupportedCapability::timing : UnsupportedCapability::random_state;
 else result.unsupported=UnsupportedCapability::phase;
 return result;
}
}

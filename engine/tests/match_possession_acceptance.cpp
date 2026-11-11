#include "captain/match_runtime.hpp"
#include <iostream>
#include <array>
#include <string>

// Explicitly failing milestone acceptance, invoked manually, not part of CTest.
// Synthetic fixtures test runtime wiring; they are not original startup evidence.
int main(int argc,char** argv) {
 const bool demo=argc==2 && std::string(argv[1])=="--demo";
 if(argc>1 && !demo){std::cerr<<"Usage: match-possession-acceptance [--demo]\n";return 2;}
 if(demo)std::cerr<<"PROVISIONAL DEMO: simplified timing and random values; synthetic content.\n";
 using namespace captain;
 unsigned failures=0;
 const std::array<std::uint8_t,3> owners{{3,7,15}};
 const std::array<std::uint8_t,3> targets{{7,3,19}};
 unsigned fixture=0;
 for (const auto owner : owners) {
  const auto target=targets[fixture++];
  PhaseSessionState initial{};initial.phase=FieldSessionPhase::player_control;
  initial.experience=ExperienceState{};initial.recovery=RecoveryContext{};
  initial.control=ControlContext{};initial.scheduling=SchedulingContext{};
  initial.current.field.side=owner<11?0:11;initial.current.field.ball_owner=owner;
  initial.current.field.ball_x=124;initial.current.field.ball_y=124;
  for(auto& player:initial.current.field.motion){player.x=124u<<8;player.y=124u<<8;}
  initial.current.field.motion[field_motion_index(target)].x=140u<<8;
  MatchConfig config{};config.initial=initial;config.random_index=0; // Synthetic metadata, not an original capture.
  MatchContent content{};content.data=ShotContent{};
  if(demo){config.mode=MatchTimingMode::provisional_demo;config.seed=1;content.data->movement=FieldPhaseContent{};content.data->control_movement=ControlMovementContent{};}
  try {
   auto started=create_match(config,content);
   if(started.unsupported) {std::cerr<<"owner "<<unsigned(owner)<<": create: "<<capability_name(*started.unsupported)<<'\n';++failures;continue;}
   auto control=step_match(started.state,{MatchInputKind::tick,1},content);
   if(control.unsupported) {std::cerr<<"owner "<<unsigned(owner)<<": control: "<<capability_name(*control.unsupported)<<'\n';++failures;continue;}
   auto poll=step_match(control.state,{MatchInputKind::tick,2},content);
   if(poll.unsupported) {std::cerr<<"owner "<<unsigned(owner)<<": first control poll: "<<capability_name(*poll.unsupported)<<'\n';++failures;continue;}
   MatchInput request{MatchInputKind::action,3};request.action=MatchAction::pass;request.target=target;
   auto action=step_match(poll.state,request,content);
   if(action.unsupported) {std::cerr<<"owner "<<unsigned(owner)<<": action: "<<capability_name(*action.unsupported)<<'\n';++failures;continue;}
   auto state=action.state;bool completed=false;
   for(std::uint64_t sequence=4;sequence<2004;++sequence) {
    auto next=step_match(state,{MatchInputKind::tick,sequence},content);
    if(next.unsupported) {std::cerr<<"owner "<<unsigned(owner)<<": pass progression: "<<capability_name(*next.unsupported)<<'\n';break;}
    state=next.state;
    if(state.session.phase==FieldSessionPhase::control_ready && state.session.current.field.ball_owner==target){completed=true;break;}
   }
   if(!completed){std::cerr<<"owner "<<unsigned(owner)<<": pass did not reach target "<<unsigned(target)<<" and resumed control\n";++failures;}
  } catch(const std::exception& error) {std::cerr<<"owner "<<unsigned(owner)<<": "<<error.what()<<'\n';++failures;}
 }
 std::cerr<<failures<<" possession scenarios incomplete\n";
 return failures?1:0;
}

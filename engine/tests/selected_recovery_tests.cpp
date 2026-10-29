#include "captain/recovery_session.hpp"
#include "captain/control_session.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
namespace {
void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid selected recovery accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){
 using namespace captain;
 PhaseSessionState s{};s.phase=FieldSessionPhase::selected_ball_recovery;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.shot_progress=ShotContactProgress{1,2,12,0,0};
 s.current.field.ball_owner=2;s.current.field.ball_x=120;s.current.field.ball_y=100;
 s.current.field.selected_defenders=2;s.scheduling->selected_ids={12,13,0,0,0};s.recovery->opposing_player=12;
 s.current.clock.remaining=449;s.experience->totals[0]=7;
 for(auto& m:s.current.field.motion){m.x=(160<<8)|23;m.y=(140<<8)|45;}
 s.current.field.motion[field_motion_index(1)].x=(120<<8)|23;
 s.current.field.motion[field_motion_index(1)].y=(100<<8)|45;
 FieldPhaseContent movement{};PlayerProfileMappings mappings{};
 std::vector<RecoveryDefender> choices{{12,0},{13,0}};
 auto run=[&]{return advance_session_selected_recovery(s,choices,mappings,movement);};
 const auto before=bytes(s);auto r=run();
 check(static_cast<unsigned>(r.state.phase)==31 && r.state.current.field.ball_owner==1 &&
  r.state.current.field.selected_defenders==2 && r.state.scheduling->selected_ids[1]==13,
  "Nearest recovery preserves original selected prefix");
 check(r.state.current.clock.remaining==449 && r.state.experience->totals[0]==7 && bytes(s)==before,
  "No repeated clock/reward/input mutation");
 auto resumed=continue_session_selected_recovery(r.state);
 auto ready=prepare_session_control(resumed);
 check(ready.state.phase==FieldSessionPhase::control_ready && ready.state.current.field.selected_defenders==0 &&
  !ready.state.shot_progress,"Recovered possession returns to control and setup clears prefix");
 choices[1].defense_action=5;r=run();
 check(r.recipient->direct_defender_capture && r.state.current.field.ball_owner==13 &&
  r.state.current.field.side==11 && r.request_possession_sound,"Unvisited action5 suffix receives ball");
 const auto& owner=r.state.current.field.motion[field_motion_index(13)];
 check(owner.x==((120<<8)|23) && owner.y==((100<<8)|45),"Recipient fractional coordinates preserved");
 auto encoded=bytes(r.state);check(encoded.size()==370 && encoded[10]=='e',"Session40 format");
 std::istringstream saved(encoded);check(bytes(load_phase_session(saved))==encoded,"Side11 recovered-owner snapshot");
 encoded[10]='A';rejects([&]{std::istringstream bad(encoded);load_phase_session(bad);});
 resumed=continue_session_selected_recovery(r.state);check(resumed.current.field.selected_defenders==2,"Prepared tail retains prefix");
 auto prior=s;s=r.state;rejects(run);s=prior;
 choices[0].defense_action=5;rejects(run);choices[0].defense_action=0;
 choices[1].player_id=14;rejects(run);choices[1].player_id=13;
 choices.pop_back();rejects(run);choices.push_back({13,0});
 s.current.field.ball_x=48;s.current.field.random_second=129;r=run();
 check(static_cast<unsigned>(r.state.phase)==32 && !r.recipient && r.state.current.field.ball_x==40 &&
  r.state.current.field.selected_defenders==2,"Outside-field jitter retains pending selected state");
 encoded=bytes(r.state);std::istringstream boundary(encoded);check(bytes(load_phase_session(boundary))==encoded,"Boundary snapshot");
 rejects([&]{continue_session_selected_recovery(r.state);});
 s=prior;r=run();r.state.recovery->game_mode=4;r.state.current.field.roster[1].type=2;
 r.state.current.field.ball_x=140;r.state.current.field.motion[0].x=140<<8;
 rejects([&]{continue_session_selected_recovery(r.state);});
 std::cout<<"Selected recovery checks passed\n";
}

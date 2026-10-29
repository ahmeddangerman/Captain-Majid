#include "captain/restart_route.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
namespace {
void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid restart routing accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){
 using namespace captain;
 PhaseSessionState s{};s.phase=FieldSessionPhase::selected_restart_entry;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.shot_progress=ShotContactProgress{1,4,12,0,0};
 s.current.field.ball_owner=2;s.current.field.ball_x=120;s.current.field.ball_y=100;
 s.current.field.selected_defenders=1;s.scheduling->selected_ids={12,0,0,0,0};
 s.current.clock.remaining=449;s.recovery->ball_variant=7;s.current.field.motion[0].status=9;
 const auto before=bytes(s);auto r=route_session_selected_restart(s);
 check(static_cast<unsigned>(r.state.phase)==38 && r.restart.route==FieldRestartRoute::field_restart &&
  r.state.recovery->ball_variant==0 && r.state.current.field.motion[0].status==9 &&
  r.state.current.clock.remaining==449 && bytes(s)==before,"Route clears situation without repeating reset/time");
 auto original=s;s=r.state;rejects([&]{route_session_selected_restart(s);});s=original;
 for(unsigned y:{95,96,159,160})for(unsigned x:{60,79,80,175,176,190}){
  s.current.field.ball_x=x;s.current.field.ball_y=y;r=route_session_selected_restart(s);
  const bool goal=x>=176 && y>=96 && y<160;
  check(static_cast<unsigned>(r.state.phase)==(goal?39u:38u),"Restart coordinate thresholds");
 }
 auto encoded=bytes(r.state);check(encoded.size()==370 && encoded[10]=='S',"Session28 format");
 std::istringstream saved(encoded);check(bytes(load_phase_session(saved))==encoded,"Restart route snapshot");
 encoded[10]='E';rejects([&]{std::istringstream bad(encoded);load_phase_session(bad);});
 auto legacy=before;legacy[10]='E';legacy.resize(336);std::istringstream old(legacy);check(bytes(load_phase_session(old))==before,"Version14 restart entry remains readable");
 std::cout<<"Selected restart route checks passed\n";
}

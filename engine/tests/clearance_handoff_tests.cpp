#include "captain/clearance_handoff.hpp"
#include "captain/recovery_session.hpp"
#include "captain/control_session.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
namespace {
void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid handoff accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){
 using namespace captain;
 PhaseSessionState s{};s.phase=FieldSessionPhase::clearance_race_winner;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.shot_progress=ShotContactProgress{1,3,12,0,0};
 s.clearance_flight=ClearanceFlightContext{4};s.clearance_race=ClearanceRaceContext{4,13,1};
 s.current.field.side=11;s.current.field.ball_owner=13;s.current.field.ball_x=120;s.current.field.ball_y=100;
 s.current.field.elapsed=1;s.current.field.selected_defenders=1;s.scheduling->selected_ids={12,0,0,0,0};
 s.recovery->opposing_player=12;s.current.clock.remaining=449;s.experience->totals[0]=7;
 for(auto& p:s.current.field.roster)p={1,0,123};
 for(auto& m:s.current.field.motion){m.x=(160<<8)|23;m.y=(140<<8)|45;}
 s.current.field.motion[0].x=(120<<8)|23;s.current.field.motion[0].y=(100<<8)|45;
 const auto before=bytes(s);auto r=continue_session_clearance(s);
 check(r.phase==FieldSessionPhase::player_control && !r.shot_progress && !r.clearance_flight && !r.clearance_race &&
  r.current.field.selected_defenders==1 && r.current.field.ball_owner==13 && r.current.clock.remaining==449,
  "Winner prepared presentation returns to control without erasing selected prefix early");
 check(bytes(s)==before,"Input preserved");rejects([&]{continue_session_clearance(r);});
 auto original=s;s.current.field.ball_x=60;rejects([&]{continue_session_clearance(s);});s=original;
 s.phase=FieldSessionPhase::clearance_race_timeout;s.clearance_race->remaining=0;s.current.field.ball_owner=2;
 s.current.field.random_second=1;s.current.field.random_first=128;
 r=continue_session_clearance(s);
 check(static_cast<unsigned>(r.phase)==36 && r.current.field.ball_x==120 && r.current.field.random_second==1,
  "Timeout resumes before movement with no second jitter");
 auto encoded=bytes(r);check(encoded.size()==370 && encoded[10]=='b',"Session37 format");
 std::istringstream saved(encoded);r=load_phase_session(saved);
 rejects([&]{continue_session_clearance(r);});
 FieldPhaseContent movement{};movement.profiles.speeds.resize(1);movement.geometry.direction_thresholds.fill(65535);
 PlayerProfileMappings mappings{};std::vector<RecoveryDefender> choices{{12,0}};
 auto recovered=advance_session_clearance_recovery(r,choices,mappings,movement);
 check(static_cast<unsigned>(recovered.state.phase)==37 && recovered.state.current.field.ball_x==120 &&
  recovered.state.current.field.ball_y==100 && recovered.state.current.field.ball_owner==1 &&
  recovered.state.current.field.side==0 && recovered.state.current.field.elapsed==0 &&
  recovered.state.current.clock.remaining==449 && recovered.state.experience->totals[0]==7,
  "Timeout recovery skips jitter and preserves time/rewards");
 check(recovered.state.current.field.selected_defenders==1 && recovered.state.current.field.roster[2].stamina==123,
  "Prefix and energy retained");
 auto control=continue_session_selected_recovery(recovered.state);
 check(control.phase==FieldSessionPhase::player_control && !control.shot_progress && !control.clearance_flight && !control.clearance_race,
  "Recovered clearance prepared tail rejoins control");
 check(prepare_session_control(control).state.current.field.selected_defenders==0,"Control setup owns prefix clear");
 rejects([&]{advance_session_clearance_recovery(recovered.state,choices,mappings,movement);});
 choices[0].defense_action=5;rejects([&]{advance_session_clearance_recovery(r,choices,mappings,movement);});
 encoded=bytes(recovered.state);std::istringstream recovered_save(encoded);check(bytes(load_phase_session(recovered_save))==encoded,"Recovered clearance roundtrip");
 encoded[10]='D';rejects([&]{std::istringstream bad(encoded);load_phase_session(bad);});
 auto legacy=before;legacy[10]='D';legacy.resize(336);std::istringstream old(legacy);check(bytes(load_phase_session(old))==before,"Version13 winner stays readable");
 std::cout<<"Clearance handoff checks passed\n";
}

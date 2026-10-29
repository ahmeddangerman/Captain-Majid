#include "captain/clearance_race.hpp"
#include "captain/phase_session_io.hpp"
#include "captain/defender_selection.hpp"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
namespace {
void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid clearance race accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){
 using namespace captain;
 PhaseSessionState s{};s.phase=FieldSessionPhase::capture_clearance_flight;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.shot_progress=ShotContactProgress{1,3,12,0,0};
 s.clearance_flight=ClearanceFlightContext{4};s.current.field.side=11;s.current.field.ball_owner=2;
 s.current.field.ball_x=60;s.current.field.ball_y=90;s.current.field.ball_cell=field_cell_at(60,90);
 s.current.field.selected_defenders=1;s.scheduling->selected_ids={12,0,0,0,0};s.recovery->opposing_player=12;
 s.current.clock.remaining=449;s.experience->totals[0]=7;
 for(auto& p:s.current.field.roster)p={1,0,123};
 for(auto& m:s.current.field.motion){m.x=(160<<8)|23;m.y=(140<<8)|45;}
 for(auto id:{1,2,12,13}) {auto& m=s.current.field.motion[field_motion_index(id)];m.x=(60<<8)|23;m.y=(90<<8)|45;}
 FieldPhaseContent movement{};movement.profiles.speeds.resize(1);movement.geometry.direction_thresholds.fill(65535);
 PlayerProfileMappings mappings{};ClearanceRaceContent race{};
 auto run=[&]{return resolve_session_clearance_race(s,mappings,movement,race);};
 const auto before=bytes(s);auto r=run();
 check(r.winner==13 && r.rounds==1 && static_cast<unsigned>(r.state.phase)==34 &&
  r.state.current.field.ball_owner==13 && r.state.current.field.side==11 && !r.request_side_music && r.pending_script==28,
  "Active candidate wins first; carrier/opposing player excluded");
 check(r.state.current.field.motion[field_motion_index(13)].x==((60<<8)|23) &&
  r.state.current.clock.remaining==449 && r.state.experience->totals[0]==7 &&
  r.state.current.field.elapsed==1 && bytes(s)==before,"Fractions/time/experience/input preserved");
 auto encoded=bytes(r.state);check(encoded.size()==382 && encoded[10]=='m',"Session48 format");
 std::istringstream saved(encoded);check(bytes(load_phase_session(saved))==encoded,"Winner snapshot");
 auto original=s;s=r.state;rejects(run);s=original;
 s.current.field.motion[field_motion_index(13)].x=80<<8;r=run();
 check(r.winner==1 && r.request_side_music && r.pending_script==62 && r.state.current.field.side==0,
  "Other-side winner requests music and script62");
 s.current.field.motion[field_motion_index(1)].x=80<<8;r=run();
 check(!r.winner && r.rounds==4 && static_cast<unsigned>(r.state.phase)==35 && r.pending_script==52 &&
  r.state.current.field.ball_owner==2,"Timeout retains transitional owner");
 s.clearance_flight->distance_adjustment=0;r=run();check(r.rounds==256,"Zero distance counter wraps through256 rounds");
 encoded=bytes(r.state);std::istringstream timeout(encoded);check(bytes(load_phase_session(timeout))==encoded,"Timeout snapshot");
 encoded[10]='C';rejects([&]{std::istringstream bad(encoded);load_phase_session(bad);});
 s=original;s.current.field.roster[1].type=0;r=run();check(r.winner==13,"Unused other candidate profile is not loaded");
 s.current.field.motion[field_motion_index(13)].x=80<<8;rejects(run);
 auto legacy=before;legacy.resize(332);legacy[10]='C';std::istringstream old(legacy);
 check(bytes(load_phase_session(old))==before,"Version12 setup remains readable");
 // Distinct nonzero columns protect selector35(active) vs36(other).
 s=original;movement.geometry.axis_signs.fill(1);race.speeds[0]={2048,256};
 auto& active=s.current.field.motion[field_motion_index(13)];active.x=52<<8;active.y=82<<8;
 r=run();check(r.winner==13 && r.rounds==1,"Active selector35 uses its own nonzero speed column");
 s=original;race.speeds[0]={256,2048};
 for(auto id:{1,13}){auto& p=s.current.field.motion[field_motion_index(id)];p.x=52<<8;p.y=82<<8;}
 r=run();check(r.winner==1 && r.rounds==1,"Other selector36 uses its own nonzero speed column");
 std::cout<<"Clearance race checks passed\n";
}

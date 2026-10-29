#include "captain/ball_flight_session.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
namespace {void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid flight session accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}}
int main(){try{
 using namespace captain;PhaseSessionState s{};s.phase=FieldSessionPhase::shot_flight_pending;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.shot_progress=ShotContactProgress{1,0,12,0,0};
 s.shot_flight=ShotFlightContext{18,221,255,true};s.current.field.ball_owner=2;
 s.current.field.ball_x=120;s.current.field.ball_y=100;s.current.field.ball_cell=110;
 s.current.field.selected_defenders=1;s.current.field.elapsed=1;s.scheduling->selected_ids={12,0,0,0,0};
 s.recovery->opposing_player=12;s.current.clock.remaining=448;
 MovementGeometryContent g{};g.direction_thresholds.fill(65535);BallVelocityContent v{};v.quarter_wave[63]=256;
 const auto source=bytes(s);auto ready=begin_prepared_session_ball_flight(s,64,192,g,v);
 check(static_cast<unsigned>(ready.phase)==47 && !ready.shot_progress && ready.current.field.selected_defenders==0 &&
  ready.scheduling->selected_ids[0]==0 && ready.current.field.elapsed==0 && ready.ball_trajectory->vx==256 &&
  ready.ball_trajectory->vy==0 && ready.ball_trajectory->x==((120<<8)|64) && ready.current.clock.remaining==448 &&
  bytes(s)==source,"Saved numeric flight setup preserves fractions and clears original prefix/elapsed");
 auto encoded=bytes(ready);check(encoded.size()==361 && encoded[10]=='O',"Session24 layout");
 std::istringstream saved(encoded);ready=load_phase_session(saved);
 auto r=advance_prepared_session_ball_cell(ready);
 check(static_cast<unsigned>(r.state.phase)==48 && r.step.additions==8 &&
  r.state.current.field.ball_x==128 && r.state.current.field.ball_cell==122 &&
  r.state.ball_trajectory->x==((128<<8)|64),"Saved flight advances to cell callback with fractional position");
 auto cell=r.state;cell.current.field.random_first=100;
 auto& defender=cell.current.field.motion[field_motion_index(12)];defender.x=128<<8;defender.y=100<<8;
 const auto before_cell=bytes(cell);auto continued=resolve_session_ball_cell(cell);
 check(static_cast<unsigned>(continued.phase)==47 && continued.current.field.selected_defenders==1 &&
  continued.scheduling->selected_ids[0]==12 && bytes(cell)==before_cell && continued.current.clock.remaining==448,
  "Cell callback accumulates matching field defender and resumes numeric flight without time or input writes");
 rejects([&]{resolve_session_ball_cell(continued);});
 auto next=advance_prepared_session_ball_cell(continued);
 check(next.state.current.field.selected_defenders==1 && next.state.scheduling->selected_ids[0]==12,
  "Selected flight prefix survives next numeric cell step");
 rejects([&]{advance_prepared_session_ball_cell(r.state);});
 rejects([&]{begin_prepared_session_ball_flight(ready,64,192,g,v);});
 s.shot_flight->destination=122;ready=begin_prepared_session_ball_flight(s,64,192,g,v);r=advance_prepared_session_ball_cell(ready);
 check(static_cast<unsigned>(r.state.phase)==49 && r.state.current.field.ball_x==132 &&
  r.state.ball_trajectory->x==((132<<8)|64),"Saved arrival snaps high byte only");
 s.shot_flight->destination=110;ready=begin_prepared_session_ball_flight(s,64,192,g,v);
 check(static_cast<unsigned>(ready.phase)==50 && ready.ball_trajectory->x==((120<<8)|64),"Already arrived does not snap or project");
 s.shot_flight->destination=221;s.current.field.selected_defenders=0;s.scheduling->selected_ids.fill(0);s.shot_progress=ShotContactProgress{};
 ready=begin_prepared_session_ball_flight(s,64,192,g,v);check(static_cast<unsigned>(ready.phase)==47,"Unopposed flight setup");
 auto legacy=source;legacy[10]='J';legacy.resize(344);std::istringstream old(legacy);
 check(bytes(load_phase_session(old))==source,"LegacyJ remains readable");
 for(unsigned length=344;length<355;++length){auto bad=encoded;bad.resize(length);rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 for(auto offset:{344u,353u,354u}){auto bad=encoded;bad[offset]=2;rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 for(unsigned offset=345;offset<355;++offset){auto bad=source;bad[offset]=1;rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 encoded[10]='J';rejects([&]{std::istringstream bad(encoded);load_phase_session(bad);});
 auto corrupt=ready;corrupt.ball_trajectory.reset();rejects([&]{bytes(corrupt);});
 corrupt=ready;corrupt.ball_trajectory->x^=256;rejects([&]{bytes(corrupt);});
 std::cout<<"Saved ball flight session checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

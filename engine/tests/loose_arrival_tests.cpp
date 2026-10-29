#include "captain/ball_flight_session.hpp"
#include "captain/recovery_session.hpp"
#include "captain/control_session.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
namespace {void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid arrival accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}}
int main(){try{
 using namespace captain;PhaseSessionState s{};s.phase=FieldSessionPhase::ball_arrival_callback;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};
 s.current.field.ball_owner=2;s.current.field.ball_x=156;s.current.field.ball_y=124;s.current.field.ball_cell=161;
 s.shot_flight=ShotFlightContext{18,161,255,true};s.ball_trajectory=BallTrajectoryContext{156<<8,124<<8,256,0,0,0};
 s.recovery->opposing_player=12;s.current.clock.remaining=448;s.current.field.random_first=1;
 for(auto& p:s.current.field.roster){p={1,0,123};}
 for(auto& m:s.current.field.motion){m.x=156<<8;m.y=124<<8;}
 const auto source=bytes(s);auto routed=continue_session_unopposed_arrival(s);
 check(static_cast<unsigned>(routed.phase)==51 && bytes(s)==source && routed.current.clock.remaining==448,"Loose arrival branch preserves numeric inputs");
 rejects([&]{continue_session_unopposed_arrival(routed);});
 auto encoded=bytes(routed);check(encoded.size()==370 && encoded[10]=='c',"Session38 layout");
 encoded[10]='K';rejects([&]{std::istringstream bad(encoded);load_phase_session(bad);});
 PlayerProfileMappings primary{};FieldPhaseContent movement{};movement.profiles.speeds.resize(1);movement.geometry.direction_thresholds.fill(65535);
 auto r=advance_session_shot_loose_recovery(routed,primary,movement);
 check(r.state.phase==FieldSessionPhase::recovered_owner && !r.state.shot_flight && !r.state.ball_trajectory &&
  r.state.current.field.ball_x==156 && r.state.current.field.ball_y==124 && r.state.current.field.ball_owner==1 &&
  r.state.current.clock.remaining==448 && !r.special_owner_cleared,"Recovery skips jitter/cleanup and clears flight state only at ownership handoff");
 check(continue_session_recovery(r.state).phase==FieldSessionPhase::player_control,"Ordinary loose flight rejoins control");
 rejects([&]{advance_session_shot_loose_recovery(r.state,primary,movement);});
 auto opposing_input=continue_session_unopposed_arrival(s);opposing_input.current.field.random_first=128;
 auto opposing=advance_session_shot_loose_recovery(opposing_input,primary,movement);
 check(opposing.state.phase==FieldSessionPhase::recovered_owner && opposing.state.current.field.side==11 &&
  opposing.state.current.field.ball_owner==13 && opposing.state.current.clock.remaining==448,
  "Opposing recovery tie uses generic recovered-owner state without stale shot constraints");
 check(prepare_session_control(continue_session_recovery(opposing.state)).state.phase==FieldSessionPhase::control_ready,
  "Opposing recovery also rejoins native control-ready");
 s.shot_flight->loose_ball=false;routed=continue_session_unopposed_arrival(s);
 check(static_cast<unsigned>(routed.phase)==52,"Keeper bound flight remains at keeper entry");
 rejects([&]{advance_session_shot_loose_recovery(routed,primary,movement);});
 s.shot_flight->loose_ball=true;s.current.field.selected_defenders=1;s.scheduling->selected_ids[0]=12;
 rejects([&]{continue_session_unopposed_arrival(s);});
 s.current.field.selected_defenders=0;s.scheduling->selected_ids.fill(0);s.phase=FieldSessionPhase::ball_already_arrived;s.ball_trajectory->vx=0;
 check(static_cast<unsigned>(continue_session_unopposed_arrival(s).phase)==51,"Already arrived routes without arrival callback");
 std::cout<<"Loose flight arrival checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

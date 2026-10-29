#include "captain/attack_clock.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
namespace {
void check(bool x,const char*m){if(!x){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool x=false;try{f();}catch(const std::invalid_argument&){x=true;}check(x,"Invalid prepared attack accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){try{
 using namespace captain;PhaseSessionState s{};s.phase=FieldSessionPhase::selected_attack_presentation;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.shot_progress=ShotContactProgress{1,0,12,0,0};
 s.current.field.ball_owner=2;s.current.field.ball_x=120;s.current.field.ball_y=100;
 s.current.field.selected_defenders=1;s.scheduling->selected_ids={12,0,0,0,0};s.recovery->opposing_player=12;
 s.current.clock.remaining=449;s.current.field.roster[2].stamina=123;s.experience->totals[2]=33;
 ContactPreludeContent c{};c.field_elapsed[1]=1;c.defender_status[0]=6;
 auto run=[&]{return continue_prepared_session_attack(s,c);};const auto source=bytes(s);auto r=run();
 check(static_cast<unsigned>(r.state.phase)==45 && r.state.current.clock.remaining==448 &&
  r.state.current.field.motion[field_motion_index(12)].status==6 && r.state.current.field.elapsed==1,
  "Prepared clock and defender status");
 check(bytes(s)==source && r.state.current.field.roster[2].stamina==123 && r.state.experience->totals[2]==33 &&
  r.state.current.field.ball_owner==2,"Source/energy/rewards/owner preserved");
 auto initial=s;s=r.state;rejects(run);rejects([&]{resume_session_attack_clock(s);});s=initial;
 s.current.clock.remaining=1;s.current.clock.extra_time=5;r=run();
 check(static_cast<unsigned>(r.state.phase)==44,"Attack added time pauses");
 auto encoded=bytes(r.state);check(encoded.size()==355 && encoded[10]=='K',"Session20 format");
 std::istringstream saved(encoded);s=load_phase_session(saved);rejects(run);
 s=resume_session_attack_clock(s);check(static_cast<unsigned>(s.phase)==45 &&
  s.current.field.elapsed==1 && s.current.clock.extra_time==5,"Resume does not repeat prelude");
 rejects([&]{resume_session_attack_clock(s);});
 encoded[10]='H';rejects([&]{std::istringstream bad(encoded);load_phase_session(bad);});
 s=initial;auto legacy=source;legacy[10]='H';legacy.resize(339);std::istringstream old(legacy);
 check(bytes(load_phase_session(old))==source,"LegacyH reads");
 s.current.clock.remaining=0;s.current.clock.extra_time=0;r=run();
 check(r.state.phase==FieldSessionPhase::period_end,"Period end stops attack");
 rejects([&]{resume_session_attack_clock(r.state);});
 s=initial;s.control->last_carrier_cell=1;rejects(run);
 s=initial;s.recovery->opposing_player=13;rejects(run);
 s=initial;s.current.field.selected_defenders=0;s.scheduling->selected_ids.fill(0);s.shot_progress=ShotContactProgress{};rejects(run);
 c.field_elapsed[5]=2;c.defender_status[2]=9;
 auto unopposed=continue_prepared_session_attack(s,c,2);
 check(static_cast<unsigned>(unopposed.state.phase)==45 && unopposed.state.current.clock.remaining==447 &&
  unopposed.state.current.field.motion[field_motion_index(12)].status==9 &&
  unopposed.state.shot_progress->cursor==0,"Unopposed uses explicit captured defending action");
 rejects([&]{continue_prepared_session_attack(s,c,7);});
 s.current.clock.remaining=1;s.current.clock.extra_time=5;unopposed=continue_prepared_session_attack(s,c,2);
 encoded=bytes(unopposed.state);std::istringstream zero_save(encoded);
 check(static_cast<unsigned>(resume_session_attack_clock(load_phase_session(zero_save)).phase)==45,
  "Unopposed clock pause saves/resumes without inventing defender prefix");
 std::cout<<"Prepared attack clock checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

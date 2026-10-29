#include "captain/restart_options.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
namespace {
void check(bool v,const char* m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid restart options accepted");}
std::string bytes(const captain::PhaseSessionState& s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
captain::PhaseSessionState roundtrip(const captain::PhaseSessionState& s){std::istringstream i(bytes(s));return captain::load_phase_session(i);}
}
int main(){try{
 using namespace captain;
 PhaseSessionState s{};s.phase=FieldSessionPhase::selected_field_restart;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.shot_progress=ShotContactProgress{1,4,12,0,0};
 s.current.field.ball_owner=2;s.current.field.ball_x=120;s.current.field.ball_y=100;
 s.current.field.selected_defenders=1;s.scheduling->selected_ids={12,0,0,0,0};
 s.current.clock.remaining=449;s.current.clock.ball_mode=133;s.current.field.motion[0].status=9;
 s.current.field.roster[2].stamina=711;s.experience->totals[2]=512;
 for(bool goal:{false,true}){
  s.phase=goal?FieldSessionPhase::selected_goal_restart:FieldSessionPhase::selected_field_restart;
  auto source=bytes(s);auto r=open_prepared_restart_options(s);
  check(r.state.phase==FieldSessionPhase::restart_options && r.state.restart_options->goal_area==goal &&
   r.state.restart_options->cursor==4 && r.state.current.clock.ball_mode==0,"Open prepared options");
  check(bytes(s)==source,"Open mutates source");
  check(r.presentation.size()==5 && r.presentation[0].kind==ControlPresentationKind::clear_overlay &&
   r.presentation[1].id==0 && r.presentation[2].id==1 && r.presentation[3].id==51 &&
   r.presentation[4].kind==ControlPresentationKind::sprite && r.presentation[4].id==4,"Options opening requests");
  auto expected=s;expected.phase=r.state.phase;expected.restart_options=r.state.restart_options;expected.current.clock.ball_mode=0;
  check(bytes(expected)==bytes(r.state),"Menu changes time/owner/status/stamina/rewards/action");
  auto saved=roundtrip(r.state);check(bytes(saved).size()==372 && bytes(saved)[10]=='j',"Session45 format");
  auto versionG=bytes(saved);versionG[10]='G';versionG.resize(339);std::istringstream previous(versionG);
  check(bytes(load_phase_session(previous))==bytes(saved),"VersionG options rewrites canonically asH");
  rejects([&]{open_prepared_restart_options(saved);});
  auto untouched=bytes(saved);auto no=poll_prepared_restart_options(saved,0,128);
  check(bytes(no.state)==untouched && no.presentation.empty(),"Pressed A alone must not confirm");
  auto toggle=poll_prepared_restart_options(saved,0,12);
  check(toggle.state.restart_options->cursor==68 && toggle.presentation.size()==1 && toggle.presentation[0].id==68,"Both vertical bits toggle once");
  auto back=poll_prepared_restart_options(roundtrip(toggle.state),0,8);
  check(back.state.restart_options->cursor==4,"Saved options toggle back");
  auto done=poll_prepared_restart_options(saved,128,0);
  check(done.state.phase==FieldSessionPhase::restart_options_done && done.state.restart_options->goal_area==goal &&
   done.state.restart_options->cursor==4 && done.presentation.size()==3,"Held A returns to correct caller");
  rejects([&]{poll_prepared_restart_options(done.state,128,0);});
  auto tactics=poll_prepared_restart_options(saved,128,4);
  check(tactics.state.phase==FieldSessionPhase::restart_tactics && tactics.state.restart_options->cursor==0 &&
   tactics.presentation.size()==4 && tactics.presentation.front().id==68,"Toggle precedes confirm and clears cursor at tactical boundary");
  check(bytes(roundtrip(tactics.state))==bytes(tactics.state),"Tactical boundary save");
  rejects([&]{poll_prepared_restart_options(tactics.state,128,0);});
  auto corrupt=saved;corrupt.restart_options.reset();rejects([&]{bytes(corrupt);});
  corrupt=s;corrupt.restart_options=RestartOptionsContext{};rejects([&]{bytes(corrupt);});
  corrupt=saved;corrupt.restart_options->cursor=5;rejects([&]{bytes(corrupt);});
  auto old=source;old[10]='F';old.resize(336);std::istringstream legacy(old);
  check(bytes(load_phase_session(legacy))==source,"LegacyF restart remains readable");
  auto bad=bytes(saved);bad[10]='F';rejects([&]{std::istringstream i(bad);load_phase_session(i);});
  bad=bytes(saved);bad[337]=2;rejects([&]{std::istringstream i(bad);load_phase_session(i);});
  bad=source;bad[337]=1;rejects([&]{std::istringstream i(bad);load_phase_session(i);});
  check(bytes(saved)==untouched,"Polling mutates source");
 }
 s.phase=FieldSessionPhase::selected_restart_entry;rejects([&]{open_prepared_restart_options(s);});
 std::cout<<"Prepared restart options checks passed\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

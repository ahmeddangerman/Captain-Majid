#include "captain/selected_dispatch.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
namespace {
void check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool caught=false;try{f();}catch(const std::invalid_argument&){caught=true;}check(caught,"Invalid dispatch accepted");}
std::string bytes(const captain::PhaseSessionState& s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){
 using namespace captain;
 PhaseSessionState s{};s.phase=FieldSessionPhase::shot_dispatch_ready;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.shot_progress=ShotContactProgress{1,2,12,0,0};
 s.current.field.ball_owner=2;s.current.field.ball_x=95;s.current.field.ball_y=100;
 s.current.field.elapsed=3;s.current.field.selected_defenders=1;s.scheduling->selected_ids={12,0,0,0,0};
 s.current.clock.remaining=449;s.current.field.random_first=100;
 for(auto& p:s.current.field.roster)p={1,0,200};
 s.current.field.roster[2].stamina=123;
 for(auto& m:s.current.field.motion){m.x=120<<8;m.y=100<<8;}
 PlayerProfileMappings primary{};FieldPhaseContent movement{};movement.profiles.speeds.resize(1);
 movement.geometry.direction_thresholds.fill(65535);
 ExperienceContent experience{};experience.stage_bases[0]=4;experience.reward_factors[2]=64;
 auto run=[&]{return resolve_session_selected_result(s,primary,movement,experience);};
 const auto before=bytes(s);auto r=run();
 check(static_cast<unsigned>(r.state.phase)==28 && r.state.shot_progress &&
  r.state.current.field.ball_owner==2 && r.state.current.field.ball_x==95 &&
  r.state.current.field.selected_defenders==1 && r.reward && r.state.experience->totals[0]==1,
  "Selected result2 retains recovery inputs with one reward");
 check(r.state.current.clock.remaining==449 && r.state.current.field.elapsed==0 &&
  r.state.current.field.roster[2].stamina==123 && r.movement.players.size()==19,
  "Dispatch advances movement without clock or attacker debit");
 check(bytes(s)==before,"Input preserved");
 auto encoded=bytes(r.state);check(encoded.size()==370 && encoded[10]=='b',"Session37 format");
 std::istringstream saved(encoded);check(bytes(load_phase_session(saved))==encoded,"Dispatch snapshot roundtrip");
 encoded[10]='9';rejects([&]{std::istringstream old(encoded);load_phase_session(old);});
 auto initial=s;s=r.state;rejects(run);s=initial;
 s.shot_progress->result_code=3;s.shot_progress->defense_action=2;r=run();
 check(r.state.phase==FieldSessionPhase::player_control && !r.state.shot_progress &&
  r.state.current.field.ball_owner==12 && r.state.current.field.side==11 && r.request_possession_sound &&
  r.state.current.field.selected_defenders==1 && r.state.experience->totals[0]==1,
  "Action2 capture transfers after movement and reward");
 s.shot_progress->defense_action=0;r=run();
 check(static_cast<unsigned>(r.state.phase)==29 && r.state.current.field.ball_owner==2 &&
  r.state.current.field.side==0 && !r.request_possession_sound,"Action0 awaits animation before transfer");
 s.shot_progress->result_code=4;r=run();
 check(static_cast<unsigned>(r.state.phase)==30 && !r.reward && r.state.experience->totals[0]==0,
  "Restart entry has no defender reward");
 // Both nonterminal result codes share8408 after movement and before script8.
 for(unsigned result:{0u,1u}) {
  s=initial;s.shot_progress->result_code=result;const auto entry=bytes(s);r=run();
  check(static_cast<unsigned>(r.state.phase)==43 && !r.reward && !r.request_possession_sound &&
   r.state.current.field.elapsed==0 && r.state.current.clock.remaining==449 &&
   r.state.experience->totals[0]==0 && r.state.current.field.ball_owner==2 &&
   r.state.current.field.selected_defenders==1 && bytes(s)==entry,"Nonterminal passes to attacking presentation without reward/time/owner writes");
  auto saved_result=bytes(r.state);std::istringstream pending(saved_result);
  check(bytes(load_phase_session(pending))==saved_result,"Nonterminal dispatch save");
  auto legacy_new=saved_result;legacy_new[10]='G';rejects([&]{std::istringstream bad(legacy_new);load_phase_session(bad);});
  auto corrupt=r.state;corrupt.shot_progress->result_code=2;rejects([&]{bytes(corrupt);});
  corrupt=r.state;corrupt.current.field.selected_defenders=2;corrupt.scheduling->selected_ids[1]=13;
  rejects([&]{bytes(corrupt);});
  s=r.state;rejects(run);
 }
 s=initial;s.current.field.selected_defenders=0;s.scheduling->selected_ids.fill(0);
 s.shot_progress=ShotContactProgress{};r=run();
 check(static_cast<unsigned>(r.state.phase)==43 && !r.reward,"Unopposed shot dispatch");
 s=initial;
 s.shot_progress->result_code=2;experience.player_mappings[1]=255;rejects(run);
 check(bytes(s)==before,"Reached reward errors preserve original state");
 // A phase27 session9 checkpoint remains readable and rewrites canonically.
 encoded=before;encoded.resize(330);encoded[10]='9';std::istringstream legacy(encoded);
 check(bytes(load_phase_session(legacy))==before,"Version9 phase27 remains readable");
 std::cout<<"Selected result dispatch checks passed\n";
}

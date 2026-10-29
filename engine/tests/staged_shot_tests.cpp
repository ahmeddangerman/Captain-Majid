#include "captain/staged_shot.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
namespace {
void check(bool ok,const char* message) { if (!ok) { std::cerr<<message<<'\n';std::exit(1); } }
template<class F> void rejects(F call) { bool rejected=false;try{call();}catch(const std::invalid_argument&){rejected=true;}check(rejected,"Invalid staged shot accepted"); }
std::string encode(const captain::PhaseSessionState& s) {std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main() {
 using namespace captain;
 PhaseSessionState s{};s.phase=FieldSessionPhase::attack_prepared;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};
 s.current.field.ball_owner=2;s.current.field.ball_x=95;s.current.field.ball_y=100;
 s.current.field.elapsed=1;s.current.clock.remaining=451;s.action->power=45;
 s.current.field.selected_defenders=2;s.scheduling->selected_ids={12,13,0,0,0};
 for(auto& p:s.current.field.roster)p={1,0,200};
 s.current.field.roster[2].stamina=123;
 FieldPlayerProfiles profiles{};profiles.skills.resize(1);
 FieldDefenseContent defense{};for(auto& row:defense.skill_selectors)row.fill(4);
 for(auto& row:defense.moves)row={{7,0,30,0}};
 defense.strength_curve.fill(240);
 FieldComparisonContent comparison{};comparison.eligibility.resize(1);comparison.variation={{100,100}};
 ContactPreludeContent prelude{};prelude.field_elapsed[0]=2;prelude.field_elapsed[1]=1;prelude.defender_status[0]=6;
 FieldDefenseChoice c{};c.player_id=12;c.comparison.random_first=100;c.comparison.random_second=100;
 auto step=[&]{return step_session_shot_defender(s,c,false,profiles,defense,comparison,prelude);};
 const auto before=encode(s);auto r=step();
 check(r.state.current.clock.remaining==450 && r.state.current.field.elapsed==2 &&
  r.state.current.field.motion[field_motion_index(12)].status==6 &&
  r.state.current.field.motion[field_motion_index(2)].status==0 &&
  r.state.current.field.roster[2].stamina==123,"Failed defender applies its clock/status without attacker debit");
 check(encode(s)==before,"Input preserved");
 s=r.state;rejects(step);c.player_id=13;r=step();
 check(r.state.current.clock.remaining==449 && r.state.current.field.elapsed==3,"Second defender follows first");
 auto dispatched=dispatch_session_shot(r.state,2);
 check(dispatched.state.current.clock.remaining==447 && dispatched.state.current.field.elapsed==5 &&
  static_cast<unsigned>(dispatched.state.phase)==27,"Exhausted prefix adds final clock once");
 rejects([&]{dispatch_session_shot(dispatched.state,2);});
 // No selected defenders: dedicated one-unit final clock, no carrier status.
 PhaseSessionState initial{};std::istringstream init(before);initial=load_phase_session(init);
 s=initial;s.current.field.selected_defenders=0;s.scheduling->selected_ids.fill(0);
 r=dispatch_session_shot(s,1);
 check(r.state.current.clock.remaining==450 && r.state.current.field.elapsed==2,"No defender timing");
 // Pause after first failed contact and save its exact cursor; second remains unvisited.
 s=initial;s.current.clock.remaining=1;s.current.clock.extra_time=5;c.player_id=12;
 r=step();check(static_cast<unsigned>(r.state.phase)==24,"Failed contact added-time pause");
 auto encoded=encode(r.state);check(encoded.size()==382 && encoded[10]=='n',"Session49 layout");
 std::istringstream stored(encoded);s=load_phase_session(stored);
 check(encode(s)==encoded,"Paused shot roundtrip");rejects(step);
 s=resume_session_shot_clock(s);c.player_id=13;r=step();
 check(r.state.current.clock.extra_time==4 && r.state.current.field.elapsed==3,"Resume skips first contact timing");
 r=dispatch_session_shot(r.state,2);check(r.state.current.clock.extra_time==2,"Final step subtracts once");
 // Final dispatch itself may pause; resume must not reapply it.
 s=initial;s.current.field.selected_defenders=0;s.scheduling->selected_ids.fill(0);
 s.current.clock.remaining=1;s.current.clock.extra_time=5;r=dispatch_session_shot(s,1);
 check(static_cast<unsigned>(r.state.phase)==26,"Final clock pause");s=resume_session_shot_clock(r.state);
 check(s.current.field.elapsed==2 && s.current.clock.extra_time==5 && static_cast<unsigned>(s.phase)==27,"Final resume no duplicate time");
 rejects([&]{resume_session_shot_clock(s);});
 // Terminal contact stops before later defense and final dispatch.
 s=initial;s.action->power=1;c.player_id=12;comparison.thresholds={255,220,180,0};r=step();
 check(static_cast<unsigned>(r.state.phase)==27 && r.state.current.clock.remaining==449 &&
  r.state.current.field.ball_owner==2 && r.state.current.field.roster[13].stamina==200,"Terminal comparison pauses before dispatch handlers");
 rejects([&]{dispatch_session_shot(r.state,2);});
 // Terminal added time saves/resumes to dispatch without repeating its comparison.
 s=initial;s.action->power=1;s.current.clock.remaining=1;s.current.clock.extra_time=5;
 r=step();check(static_cast<unsigned>(r.state.phase)==24,"Terminal contact added-time pause");
 encoded=encode(r.state);std::istringstream terminal_saved(encoded);s=load_phase_session(terminal_saved);
 s=resume_session_shot_clock(s);
 check(static_cast<unsigned>(s.phase)==27 && s.shot_progress->result_code==3 &&
  s.current.field.elapsed==3 && s.current.clock.extra_time==5 && s.current.field.ball_owner==2,
  "Terminal added-time resume reaches dispatch once");
 // An interruption clears all statuses, then9095 gives the carrier status5.
 comparison.thresholds={0,0,0,0};s=initial;c.comparison.random_first=0;
 s.current.field.keepers[0].stun=8;s.current.field.keepers[1].penalty=9;r=step();
 check(static_cast<unsigned>(r.state.phase)==27 && r.state.current.field.keepers[0].stun==0 &&
  r.state.current.field.keepers[1].penalty==0 && r.state.current.field.motion[field_motion_index(2)].status==5 &&
  r.state.current.clock.remaining==449,"Interruption reset before carrier status/clock");
 // Zero main time and insufficient extra time reaches period end, with no continuation.
 s=initial;s.current.clock.remaining=0;s.current.clock.extra_time=0;c.comparison.random_first=100;r=step();
 check(r.state.phase==FieldSessionPhase::period_end,"Period end suspends shot");
 rejects([&]{resume_session_shot_clock(r.state);});
 encoded=encode(r.state);encoded[10]='8';rejects([&]{std::istringstream bad(encoded);load_phase_session(bad);});
 std::cout<<"Staged shot timing checks passed\n";
}

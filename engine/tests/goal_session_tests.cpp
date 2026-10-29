#include "captain/goal_session.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
namespace {
void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid goal preparation accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){try{
 using namespace captain;PhaseSessionState s{};s.phase=FieldSessionPhase::keeper_decided;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.keeper_decision=KeeperDecisionContext{};
 s.current.field.ball_owner=2;s.current.field.ball_x=204;s.current.field.ball_y=124;s.current.field.ball_cell=233;
 s.current.field.selected_defenders=1;s.scheduling->selected_ids[0]=11;s.recovery->opposing_player=11;
 s.shot_flight=ShotFlightContext{18,233,255,false};s.ball_trajectory=BallTrajectoryContext{204<<8,124<<8,256,0,0,0};
 s.current.clock.remaining=448;for(auto& p:s.current.field.roster)p={1,0,123};
 ShotContent content{};content.experience=ExperienceContent{};content.experience->reward_factors.fill(64);content.experience->stage_bases.fill(100);
 const auto original=bytes(s);const auto goal=prepare_session_scored_goal(s,{255,4,2},content);
 check(static_cast<unsigned>(goal.phase)==59 && goal.current.field.side==0 && goal.current.field.ball_owner==2 &&
  goal.current.clock.remaining==448 && goal.experience->totals[0]==25 && bytes(s)==original,
  "Scorer rewarded once before celebration without possession/time mutation");
 check(goal.score && goal.score->left_goals==0 && goal.score->right_goals==4 && goal.score->kickoff==2,"Goal increments one score byte with wrap and retains kickoff");
 auto saved=bytes(goal);check(saved.size()==365 && saved[10]=='Q',"Scored goal snapshot26");std::istringstream stream(saved);
 check(bytes(load_phase_session(stream))==saved,"Scored goal round-trip");
 auto legacy=original;legacy[10]='O';legacy.resize(361);std::istringstream old(legacy);check(bytes(load_phase_session(old))==original,"Legacy O decision remains readable");
 auto forbidden=saved;forbidden[10]='O';forbidden.resize(361);rejects([&]{std::istringstream i(forbidden);load_phase_session(i);});
 for(unsigned length=361;length<365;++length){auto bad=saved;bad.resize(length);rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 auto bad=saved;bad[361]=2;rejects([&]{std::istringstream i(bad);load_phase_session(i);});
 bad=original;bad[362]=1;rejects([&]{std::istringstream i(bad);load_phase_session(i);});
 auto existing=s;existing.score=MatchScoreContext{1,2,0};rejects([&]{prepare_session_scored_goal(existing,{0,2,0},content);});
 auto stunned=s;stunned.phase=FieldSessionPhase::stunned_keeper_decided;stunned.current.field.keepers[1].stun=3;
 stunned.current.field.selected_defenders=0;stunned.scheduling->selected_ids.fill(0);
 check(prepare_session_scored_goal(stunned,{1,2,0},content).score->left_goals==2,"Stunned keeper goal also saves without a selected prefix");
 auto prepared=goal;prepared.current.clock.ball_mode=0;prepared.current.field.keepers[1]={42,9,50,3};
 // A stunned goal has no selected keeper; its saved status must agree before the menu.
 prepared.current.field.selected_defenders=0;prepared.scheduling->selected_ids.fill(0);
 const auto handoff=continue_prepared_goal_restart(prepared);
 check(static_cast<unsigned>(handoff.phase)==60 && handoff.current.field.side==11 &&
  handoff.current.field.ball_owner==2 && handoff.score->kickoff==4 && handoff.score->left_goals==0 &&
  handoff.current.field.keepers[1].timer==0 && handoff.current.field.keepers[1].penalty==0 &&
  handoff.current.field.keepers[1].stun==0 && handoff.current.field.keepers[1].penalty_duration==9 &&
  handoff.current.clock.remaining==448 && !handoff.shot_flight && !handoff.ball_trajectory && !handoff.keeper_decision,
  "Post-goal restart flips side, clears keeper and prepares kickoff once");
 auto encoded=bytes(handoff);std::istringstream hs(encoded);check(bytes(load_phase_session(hs))==encoded,"Restart save round-trip");
 auto old_goal=bytes(goal);old_goal[10]='P';std::istringstream pg(old_goal);check(bytes(load_phase_session(pg))==bytes(goal),"Legacy P goal reads");
 auto old_restart=encoded;old_restart[10]='P';rejects([&]{std::istringstream i(old_restart);load_phase_session(i);});
 auto normal=goal;normal.current.clock.ball_mode=0;normal.current.field.keepers[1]={42,9,50,0};
 const auto normal_restart=continue_prepared_goal_restart(normal);
 check(normal_restart.current.field.selected_defenders==1 && normal_restart.scheduling->selected_ids[0]==11,"Normal keeper prefix preserved through restart");
 auto malformed=normal_restart;malformed.scheduling->selected_ids[0]=12;rejects([&]{validate_phase_session(malformed);});
 rejects([&]{continue_prepared_goal_restart(handoff);});
 auto wrong=prepared;wrong.current.clock.ball_mode=1;rejects([&]{continue_prepared_goal_restart(wrong);});
 rejects([&]{prepare_session_scored_goal(goal,{0,4,2},content);});
 s.keeper_decision->result_code=1;s.recovery->attack_subtype=3;
 rejects([&]{prepare_session_scored_goal(s,{0,0,0},content);});
 s.recovery->attack_subtype=128;check(static_cast<unsigned>(prepare_session_scored_goal(s,{0,0,0},content).phase)==59,"Masked ordinary subtype result1 still scores");
 std::cout<<"Scored goal preparation checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

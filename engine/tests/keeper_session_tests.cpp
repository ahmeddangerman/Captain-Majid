#include "captain/keeper_session.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
namespace {
void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid keeper operation accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){try{
 using namespace captain;PhaseSessionState s{};s.phase=FieldSessionPhase::shot_keeper_entry;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};
 s.current.field.ball_owner=2;s.current.field.ball_x=204;s.current.field.ball_y=124;s.current.field.ball_cell=233;
 s.shot_flight=ShotFlightContext{18,233,255,false};s.ball_trajectory=BallTrajectoryContext{204<<8,124<<8,256,0,0,0};
 s.recovery->opposing_player=12;s.current.clock.remaining=448;
 s.current.field.keepers[0].stun=19;s.current.field.keepers[1].timer=41;
 for(auto& p:s.current.field.roster)p={1,0,123};
 const auto original=bytes(s);auto normal=prepare_session_keeper_entry(s);
 check(static_cast<unsigned>(normal.phase)==53 && normal.current.field.selected_defenders==1 &&
  normal.scheduling->selected_ids[0]==11 && normal.recovery->opposing_player==11,
  "Ready opposing keeper becomes the sole selected defender");
 auto expected=s;expected.phase=static_cast<FieldSessionPhase>(53);expected.current.field.selected_defenders=1;
 expected.scheduling->selected_ids[0]=11;expected.recovery->opposing_player=11;
 check(bytes(normal)==bytes(expected) && bytes(s)==original,"Keeper setup changes only original writes/phase");
 rejects([&]{prepare_session_keeper_entry(normal);});
 s.current.field.keepers[1].stun=3;auto stunned=prepare_session_keeper_entry(s);
 check(static_cast<unsigned>(stunned.phase)==54 && stunned.current.field.selected_defenders==0 &&
  stunned.recovery->opposing_player==11,"Stunned opposing keeper bypasses selection");
 for(auto random:{0,15,16,255}){
  auto input=stunned;input.current.field.random_first=static_cast<std::uint8_t>(random);
  const auto result=resolve_session_stunned_keeper(input,{16,0});
  check(static_cast<unsigned>(result.phase)==55,"Stunned classification saved before display12");
  const auto saved=bytes(result);check(saved.size()==370 && saved[10]=='a' && saved[355]==1 && saved[356]==0 &&
   saved[357]==(random>=16 ? 0 : 1),"Saved result matches each original threshold boundary");
  std::istringstream stream(saved);check(bytes(load_phase_session(stream))==saved,"Keeper decision round-trip");
  check(result.current.field.random_first==random && result.current.clock.remaining==448 &&
   result.current.field.keepers[1].timer==41 && result.action->power==0,"Decision does not repeat timing, energy or random");
  rejects([&]{resolve_session_stunned_keeper(result,{16,0});});
 }
 rejects([&]{resolve_session_stunned_keeper(normal,{16,0});});
 rejects([&]{resolve_session_stunned_keeper(stunned,{0,16});});
 rejects([&]{resolve_session_stunned_keeper(stunned,{16,1});});
 auto malformed=normal;malformed.scheduling->selected_ids[0]=12;rejects([&]{validate_phase_session(malformed);});
 auto legacy=bytes(s);legacy[10]='L';legacy.resize(355);std::istringstream old(legacy);
 check(bytes(load_phase_session(old))==bytes(s),"Old L keeper entry remains readable");
 auto fresh=bytes(normal);fresh[10]='L';fresh.resize(355);rejects([&]{std::istringstream i(fresh);load_phase_session(i);});
 auto decided=resolve_session_stunned_keeper(stunned,{16,0});const auto valid=bytes(decided);
 for(auto offset:{355,356,357}){auto bad=valid;bad[offset]=offset==355?2:3;rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 for(unsigned length=355;length<358;++length){auto bad=valid;bad.resize(length);rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 auto unused=bytes(stunned);unused[357]=1;rejects([&]{std::istringstream i(unused);load_phase_session(i);});
 ShotContent content{};content.goalkeepers.skills.resize(1);content.goalkeepers.skills[0].fill(20);
 content.keeper.skill_selectors={25,26,25,28};content.keeper.strength_curve.fill(240);
 for(auto& moves:content.keeper.moves)moves={{0,0,10,0}};
 content.keeper_range=255;content.keeper_offset=128;content.keeper_thresholds={200,180,100,0};
 auto before=normal;before.action->power=100;
 const auto decision=resolve_prepared_session_keeper(before,0,0,4,content);
 check(decision.keeper_decision->result_code==1,"Keeper score/adjustment classifies comparison");
 check(static_cast<unsigned>(decision.phase)==56,"Ready keeper comparison persists before presentation9");
 check(decision.current.field.roster[11].stamina==123 && decision.current.clock.remaining==448 &&
  decision.action->power==100 && decision.current.field.random_first==0,"Keeper comparison preserves attack/stamina/time/random");
 rejects([&]{resolve_prepared_session_keeper(decision,0,0,4,content);});
 auto story_input=before;story_input.recovery->game_mode=1;story_input.recovery->attack_subtype=3;
 check(resolve_prepared_session_keeper(story_input,0,0,5,content).keeper_decision->result_code==2,"Raw subtype and explicit opponent activate keeper story override");
 story_input.current.field.random_first=128;
 check(resolve_prepared_session_keeper(story_input,0,0,5,content).keeper_decision->result_code==3,"Keeper story override preserves random sign decision");
 rejects([&]{resolve_prepared_session_keeper(before,4,0,4,content);});
 const auto decided_bytes=bytes(decision);std::istringstream decision_stream(decided_bytes);
 check(bytes(load_phase_session(decision_stream))==decided_bytes,"Normal keeper decision round-trip");
 auto old_m=bytes(stunned);old_m[10]='M';old_m.resize(358);std::istringstream mstream(old_m);
 check(bytes(load_phase_session(mstream))==bytes(stunned),"Legacy M entry loads without new move metadata");
 auto old_decided=bytes(decided);old_decided[10]='M';old_decided.resize(358);std::istringstream old_decided_stream(old_decided);
 check(bytes(load_phase_session(old_decided_stream))==bytes(decided),"Legacy M stunned decision retains result without new move metadata");
 for(unsigned length=358;length<361;++length){auto bad=decided_bytes;bad.resize(length);rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 auto invalid_decision=decision;invalid_decision.keeper_decision->result_code=4;rejects([&]{validate_phase_session(invalid_decision);});
 invalid_decision=decision;invalid_decision.keeper_decision->defense_action=4;rejects([&]{validate_phase_session(invalid_decision);});
 invalid_decision=decision;invalid_decision.current.field.keepers[1].stun=1;rejects([&]{validate_phase_session(invalid_decision);});
 const auto before_errors=bytes(before);auto missing=content;missing.goalkeepers.skills.clear();rejects([&]{resolve_prepared_session_keeper(before,0,0,4,missing);});
 check(bytes(before)==before_errors,"Error calls retain immutable source");
 auto forbidden=decided_bytes;forbidden[10]='M';forbidden.resize(358);rejects([&]{std::istringstream i(forbidden);load_phase_session(i);});
 for(auto offset:{358,359,360}){auto bad=bytes(stunned);bad[offset]=1;rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 auto extra_decision=bytes(decided);extra_decision[358]=1;rejects([&]{std::istringstream i(extra_decision);load_phase_session(i);});
 std::cout<<"Saved keeper entry checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

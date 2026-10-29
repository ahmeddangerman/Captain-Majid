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
  const auto saved=bytes(result);check(saved.size()==358 && saved[10]=='M' && saved[355]==1 && saved[356]==0 &&
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
 std::cout<<"Saved keeper entry checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

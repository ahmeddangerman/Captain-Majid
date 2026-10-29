#include "captain/approach_session.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
namespace {
void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid approach accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){try{
 using namespace captain;PhaseSessionState s{};s.phase=FieldSessionPhase::shot_approach_ready;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.shot_progress=ShotContactProgress{1,0,12,0,0};
 s.current.field.ball_owner=2;s.current.field.ball_x=120;s.current.field.ball_y=100;
 s.current.field.selected_defenders=1;s.scheduling->selected_ids={12,0,0,0,0};s.recovery->opposing_player=12;
 s.current.clock.remaining=448;s.current.field.random_first=101;s.current.field.roster[2].stamina=123;
 ShotApproachContent c{};c.outer_distance.fill(2);c.inner_distance.fill(3);
 const auto before=bytes(s);auto r=prepare_session_shot_approach(s,c);
 check(static_cast<unsigned>(r.state.phase)==46 && r.state.shot_flight->distance==18 &&
  r.state.shot_flight->destination==221 && r.state.shot_flight->counter==255 && r.state.shot_flight->loose_ball &&
  r.state.current.field.random_first==101,"Outer ordinary shot loose destination");
 check(bytes(s)==before && r.state.current.clock.remaining==448 && r.state.current.field.ball_x==120 &&
  r.state.current.field.ball_y==100 && r.state.current.field.roster[2].stamina==123,"Approach preserves inputs/ball/clock/stamina");
 auto expected=s;expected.phase=r.state.phase;expected.shot_flight=r.state.shot_flight;
 expected.current.field.random_first=r.state.current.field.random_first;
 check(bytes(expected)==bytes(r.state),"Approach changes no other represented state including ball cell");
 auto encoded=bytes(r.state);check(encoded.size()==370 && encoded[10]=='V',"Session31 layout");
 std::istringstream saved(encoded);check(bytes(load_phase_session(saved))==encoded,"Flight setup snapshot");
 rejects([&]{prepare_session_shot_approach(r.state,c);});
 auto corrupt=s;corrupt.shot_flight=ShotFlightContext{};rejects([&]{bytes(corrupt);});
 corrupt=r.state;corrupt.shot_flight.reset();rejects([&]{bytes(corrupt);});
 for(unsigned length=339;length<344;++length){auto bad=encoded;bad.resize(length);rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 for(auto change: {std::pair<unsigned,unsigned>{339,2},{343,2},{341,240},{342,254}}){
  auto bad=encoded;bad[change.first]=change.second;rejects([&]{std::istringstream i(bad);load_phase_session(i);});
 }
 for(unsigned offset=340;offset<344;++offset){auto bad=before;bad[offset]=1;rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 encoded[10]='I';rejects([&]{std::istringstream bad(encoded);load_phase_session(bad);});
 auto legacy=before;legacy[10]='I';legacy.resize(339);std::istringstream old(legacy);
 check(bytes(load_phase_session(old))==before,"VersionI reads with absent flight context");
 s.current.field.ball_x=160;r=prepare_session_shot_approach(s,c);
 check(r.state.shot_flight->distance==27 && !r.state.shot_flight->loose_ball &&
  r.state.shot_flight->destination==234 && r.state.current.field.random_first==50,"Keeper-bound shot consumes exactly one random bit");
 s.current.field.selected_defenders=0;s.scheduling->selected_ids.fill(0);s.shot_progress=ShotContactProgress{};
 r=prepare_session_shot_approach(s,c);check(r.state.shot_progress->cursor==0,"Unopposed shot approach");
 std::cout<<"Session shot approach checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

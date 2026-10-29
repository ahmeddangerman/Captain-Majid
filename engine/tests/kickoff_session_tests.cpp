#include "captain/kickoff_session.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
namespace {
void check(bool b,const char*m){if(!b){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool caught=false;try{f();}catch(const std::invalid_argument&){caught=true;}check(caught,"Invalid kickoff input accepted");}
std::string bytes(const captain::PhaseSessionState& s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){try{using namespace captain;
 PhaseSessionState s{};s.phase=FieldSessionPhase::goal_kickoff_pending;s.experience=ExperienceState{};
 s.recovery=RecoveryContext{};s.recovery->opposing_player=11;s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.score=MatchScoreContext{3,2,4};
 s.current.field.side=11;s.current.field.ball_owner=2;s.current.field.ball_x=204;s.current.field.ball_y=124;s.current.field.ball_cell=233;
 s.current.field.keepers[0]={0x12,0x34,0x56,0};s.current.field.keepers[1]={0,0,0,0};
 for(auto& m:s.current.field.motion){m.x=0x1267;m.y=0x3439;}
 KickoffFormationContent c{};c.on_ball.fill(0);c.off_ball.fill(13);
 auto original=bytes(s);auto placed=prepare_session_kickoff_formation(s,0,c);
 check(static_cast<unsigned>(placed.phase)==61 && placed.current.field.side==0 && placed.current.field.ball_owner==8,
  "Saved formation enters the selected kickoff side and original initial owner");
 check(placed.current.field.keepers[0].timer==0x12 && placed.current.field.keepers[0].penalty_duration==0x34 &&
  placed.current.field.keepers[0].penalty==0 && placed.current.field.keepers[1].penalty_duration==0x3c &&
  placed.kickoff_formation && placed.kickoff_formation->keeper_y[0]==0x54 && placed.kickoff_formation->keeper_y[1]==0x5c,
  "Saved keeper aliases and both previously unrepresented Y bytes match formation");
 check(placed.current.field.motion[0].x==0x3467 && placed.current.field.motion[0].y==0x5439 &&
  placed.score->left_goals==3 && bytes(s)==original,"Formation preserves fractions, score and input");
 auto saved=bytes(placed);std::istringstream stream(saved);check(bytes(load_phase_session(stream))==saved,"Kickoff formation roundtrip");
 auto old=original;old[10]='Q';old.resize(365);std::istringstream legacy(old);
 check(bytes(load_phase_session(legacy))==original,"Legacy Q kickoff state still loads");
 auto old_new=saved;old_new[10]='Q';old_new.resize(365);rejects([&]{std::istringstream i(old_new);load_phase_session(i);});
 for(unsigned n=365;n<368;++n){auto bad=saved;bad.resize(n);rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 auto bad=saved;bad[365]=2;rejects([&]{std::istringstream i(bad);load_phase_session(i);});
 bad=original;bad[366]=1;rejects([&]{std::istringstream i(bad);load_phase_session(i);});
 auto missing=placed;missing.kickoff_formation.reset();rejects([&]{validate_phase_session(missing);});
 rejects([&]{prepare_session_kickoff_formation(placed,0,c);});
 auto mirrored=prepare_session_kickoff_formation(s,11,c);
 check(mirrored.current.field.ball_owner==19 && mirrored.kickoff_formation->keeper_y[0]==0xa4 &&
  mirrored.kickoff_formation->keeper_y[1]==0xac,"Opposing kickoff mirrors both keeper positions");
 rejects([&]{prepare_session_kickoff_formation(s,1,c);});
 auto malformed=c;malformed.on_ball[0]=240;rejects([&]{prepare_session_kickoff_formation(s,0,malformed);});
 std::cout<<"Saved kickoff formation checks passed\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

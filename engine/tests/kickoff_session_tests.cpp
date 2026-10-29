#include "captain/kickoff_session.hpp"
#include "captain/control_session.hpp"
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
 const std::array<std::uint8_t,5> candidates{5,6,7,9,10};
 auto receiver=prepare_session_kickoff_receiver(placed,7,candidates);
 check(static_cast<unsigned>(receiver.phase)==62 && receiver.kickoff_receiver && *receiver.kickoff_receiver==7 &&
       receiver.current.field.ball_owner==8 && bytes(placed)==saved,"Receiver is saved without changing the current carrier");
 auto receiver_bytes=bytes(receiver);std::istringstream receiver_stream(receiver_bytes);
 check(bytes(load_phase_session(receiver_stream))==receiver_bytes,"Kickoff receiver roundtrip");
 auto ball=prepare_session_kickoff_ball(receiver);
 check(static_cast<unsigned>(ball.phase)==63 && ball.current.field.ball_x==52 && ball.current.field.ball_y==84 &&
       ball.current.field.ball_cell==0 && ball.current.field.ball_owner==8 && *ball.kickoff_receiver==7 &&
       bytes(receiver)==receiver_bytes,"Ball starts at current carrier, before presentation and receiver transfer");
 auto ball_bytes=bytes(ball);std::istringstream ball_stream(ball_bytes);
 check(bytes(load_phase_session(ball_stream))==ball_bytes,"Kickoff ball roundtrip");
 auto handoff=continue_prepared_session_kickoff(ball);
 check(static_cast<unsigned>(handoff.phase)==64 && handoff.current.field.ball_owner==7 &&
       handoff.control->controlled_player==1 && handoff.current.field.ball_x==52 &&
       handoff.current.field.ball_y==84 && bytes(ball)==ball_bytes,
       "Acknowledged kickoff display transfers carrier and resets controlled player");
 auto handoff_bytes=bytes(handoff);std::istringstream handoff_stream(handoff_bytes);
 check(bytes(load_phase_session(handoff_stream))==handoff_bytes,"Kickoff handoff roundtrip");
 auto control_ready=prepare_session_control(handoff).state;
 check(control_ready.phase==FieldSessionPhase::control_ready && control_ready.current.field.ball_owner==7 &&
       control_ready.kickoff_formation && !control_ready.kickoff_receiver &&
       control_ready.kickoff_formation->keeper_y==handoff.kickoff_formation->keeper_y,
       "Kickoff control setup retains goalkeeper positions and consumes receiver context");
 auto control_bytes=bytes(control_ready);std::istringstream control_stream(control_bytes);
 check(bytes(load_phase_session(control_stream))==control_bytes && bytes(handoff)==handoff_bytes,
       "Kickoff control state roundtrips without mutating its input");
 auto old_control=control_bytes;old_control[10]='U';
 rejects([&]{std::istringstream i(old_control);load_phase_session(i);});
 auto past_control=control_ready;past_control.phase=FieldSessionPhase::control_clock_pending;
 rejects([&]{validate_phase_session(past_control);});
 auto t_legacy=ball_bytes;t_legacy[10]='T';std::istringstream legacy_t(t_legacy);
 check(bytes(load_phase_session(legacy_t))==ball_bytes,"Legacy T ball state loads");
 auto t_new=handoff_bytes;t_new[10]='T';rejects([&]{std::istringstream i(t_new);load_phase_session(i);});
 auto invalid_handoff=handoff;invalid_handoff.control->controlled_player=2;
 rejects([&]{validate_phase_session(invalid_handoff);});
 invalid_handoff=handoff;invalid_handoff.current.field.ball_owner=8;
 rejects([&]{validate_phase_session(invalid_handoff);});
 rejects([&]{continue_prepared_session_kickoff(handoff);});
 auto s_legacy=receiver_bytes;s_legacy[10]='S';std::istringstream legacy_s(s_legacy);
 check(bytes(load_phase_session(legacy_s))==receiver_bytes,"Legacy S receiver loads");
 auto s_new=ball_bytes;s_new[10]='S';rejects([&]{std::istringstream i(s_new);load_phase_session(i);});
 auto invalid_ball=ball;invalid_ball.current.field.ball_cell=1;rejects([&]{validate_phase_session(invalid_ball);});
 rejects([&]{prepare_session_kickoff_ball(ball);});
 auto r_legacy=saved;r_legacy[10]='R';r_legacy.resize(368);std::istringstream legacy_r(r_legacy);
 check(bytes(load_phase_session(legacy_r))==saved,"Legacy R formation loads");
 auto r_new=receiver_bytes;r_new[10]='R';r_new.resize(368);
 rejects([&]{std::istringstream i(r_new);load_phase_session(i);});
 for(unsigned n=368;n<370;++n){auto truncated=receiver_bytes;truncated.resize(n);rejects([&]{std::istringstream i(truncated);load_phase_session(i);});}
 auto invalid_receiver=receiver;invalid_receiver.kickoff_receiver=11;rejects([&]{validate_phase_session(invalid_receiver);});
 invalid_receiver=receiver;invalid_receiver.kickoff_receiver.reset();rejects([&]{validate_phase_session(invalid_receiver);});
 rejects([&]{prepare_session_kickoff_receiver(receiver,0,candidates);});
 auto invalid_candidates=candidates;invalid_candidates[0]=0;
 rejects([&]{prepare_session_kickoff_receiver(placed,0,invalid_candidates);});
 auto bad_receiver=receiver_bytes;bad_receiver[368]=2;rejects([&]{std::istringstream i(bad_receiver);load_phase_session(i);});
 bad_receiver=saved;bad_receiver[369]=1;rejects([&]{std::istringstream i(bad_receiver);load_phase_session(i);});
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

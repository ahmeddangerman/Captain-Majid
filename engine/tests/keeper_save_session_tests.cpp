#include "captain/keeper_save_session.hpp"
#include "captain/recovery_session.hpp"
#include "captain/action_menu.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
namespace {
void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid keeper save accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){try{
 using namespace captain;PhaseSessionState s{};s.phase=FieldSessionPhase::keeper_decided;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.keeper_decision=KeeperDecisionContext{0,2,0,0,0};
 s.current.field.ball_owner=2;s.current.field.ball_x=204;s.current.field.ball_y=124;s.current.field.ball_cell=233;
 s.current.field.selected_defenders=1;s.scheduling->selected_ids[0]=11;s.recovery->opposing_player=11;
 s.shot_flight=ShotFlightContext{18,233,255,false};s.ball_trajectory=BallTrajectoryContext{204<<8,124<<8,256,0,0,0};
 s.current.clock.remaining=448;s.current.field.keepers[1]={250,9,49,0};
 for(auto& p:s.current.field.roster)p={1,0,123};
 for(auto& m:s.current.field.motion){m.x=156<<8;m.y=124<<8;}
 ShotContent content{};content.experience=ExperienceContent{};content.experience->reward_factors.fill(64);
 content.experience->stage_bases.fill(100);content.punch_intervals.fill(8);
 const auto original=bytes(s);const auto rebound=resolve_session_keeper_save(s,content);
 auto kickoff=s;kickoff.kickoff_formation=KickoffFormationContext{{124,132}};
 auto kickoff_rebound=resolve_session_keeper_save(kickoff,content).state;
 check(kickoff_rebound.phase==FieldSessionPhase::ball_recovery && kickoff_rebound.kickoff_formation &&
       !bytes(kickoff_rebound).empty(),"Kickoff rebound handoff preserves keeper coordinates");
 for(auto code:{2,3})for(auto action:{0,1}) {
  auto input=kickoff;input.keeper_decision->result_code=code;input.keeper_decision->defense_action=action;
  auto output=resolve_session_keeper_save(input,content).state;auto saved=bytes(output);
  std::istringstream saved_stream(saved);check(bytes(load_phase_session(saved_stream))==saved,"Kickoff save handoff roundtrip");
  check(output.kickoff_formation->keeper_y==input.kickoff_formation->keeper_y,"Keeper coordinates retained after save");
  auto prior=saved;prior[10]='f';prior.resize(370);rejects([&]{std::istringstream i(prior);load_phase_session(i);});
  rejects([&]{resolve_session_keeper_save(output,content);});
 }
 auto prior_decision=bytes(kickoff);prior_decision[10]='f';prior_decision.resize(370);std::istringstream prior_stream(prior_decision);
 check(bytes(load_phase_session(prior_stream))==bytes(kickoff),"Legacy f kickoff keeper decision loads");
 check(rebound.event==ShotEvent::rebound && rebound.state.phase==FieldSessionPhase::ball_recovery &&
  rebound.state.current.field.selected_defenders==0 && rebound.state.current.field.keepers[1].timer==1 &&
  rebound.state.current.field.keepers[1].penalty==127 && rebound.state.current.field.keepers[1].penalty_duration==4,
  "Rebound commits timer/penalty and clears queue for original jitter recovery");
 check(rebound.state.experience->totals[0]==25 && bytes(s)==original && rebound.state.current.clock.remaining==448,
  "One keeper reward, immutable input and unchanged clock");
 check(!rebound.state.shot_flight && !rebound.state.ball_trajectory && !rebound.state.keeper_decision,"Resolved keeper contexts cleared");
 rejects([&]{resolve_session_keeper_save(rebound.state,content);});
 s.keeper_decision->result_code=3;const auto caught=resolve_session_keeper_save(s,content);
 check(caught.event==ShotEvent::keeper_catch && static_cast<unsigned>(caught.state.phase)==57 &&
  caught.state.current.field.side==11 && caught.state.current.field.ball_owner==11 &&
  caught.state.current.field.selected_defenders==1 && caught.state.scheduling->selected_ids[0]==11 &&
  caught.request_possession_sound,"Catch commits keeper ownership and retains original selected keeper prefix");
 auto no_flight=caught.state;no_flight.shot_flight=ShotFlightContext{0,0,255,true};no_flight.shot_flight.reset();
 check(bytes(no_flight)==bytes(caught.state),"Keeper possession must not read a cleared flight context");
 auto encoded=bytes(caught.state);std::istringstream stream(encoded);check(bytes(load_phase_session(stream))==encoded,"Keeper ownership save round-trip");
 s.keeper_decision->defense_action=1;const auto punched=resolve_session_keeper_save(s,content);
 check(punched.event==ShotEvent::keeper_punch && static_cast<unsigned>(punched.state.phase)==58 &&
  punched.state.current.field.ball_x==200 && punched.state.current.field.ball_y==80 &&
  punched.state.current.field.elapsed==1 && punched.state.current.field.selected_defenders==0 &&
  punched.state.current.field.ball_cell==233,"Punch commits placement/elapsed and preserves the prior cell during prepared recovery");
 auto punch_recovery=punched.state;punch_recovery.current.field.ball_x=156;punch_recovery.current.field.ball_y=124;
 PlayerProfileMappings primary{};FieldPhaseContent movement{};movement.profiles.speeds.resize(1);movement.geometry.direction_thresholds.fill(65535);
 auto prepared_rebound=kickoff_rebound;prepared_rebound.current.field.ball_x=156;
 prepared_rebound.control->motion_requested=2;
 auto rebound_owner=advance_session_recovery(prepared_rebound,primary,movement).state;
 check(rebound_owner.phase==FieldSessionPhase::recovered_owner && rebound_owner.kickoff_formation &&
       rebound_owner.control->motion_requested==2,"Rebound jitter recovery keeps its original animation state");
 auto outside_rebound=prepared_rebound;outside_rebound.current.field.ball_x=0;outside_rebound.current.field.ball_y=0;
 const auto outside_bytes=bytes(outside_rebound);
 rejects([&]{advance_session_recovery(outside_rebound,primary,movement);});
 check(bytes(outside_rebound)==outside_bytes,"Unsupported kickoff boundary exit leaves the source intact");
 auto kickoff_punch=punch_recovery;kickoff_punch.kickoff_formation=KickoffFormationContext{{124,132}};
 kickoff_punch.control->motion_requested=2;
 auto kickoff_recovered=advance_session_keeper_punch_recovery(kickoff_punch,primary,movement).state;
 check(kickoff_recovered.phase==FieldSessionPhase::recovered_owner && kickoff_recovered.kickoff_formation &&
       !bytes(kickoff_recovered).empty(),"Keeper punch recovery retains kickoff coordinates");
 check(kickoff_recovered.control->motion_requested==0 && kickoff_punch.control->motion_requested==2,
       "Prepared punch recovery captures animation acknowledgement without input mutation");
 auto kickoff_control=continue_session_recovery(kickoff_recovered);
 check(kickoff_control.phase==FieldSessionPhase::player_control && kickoff_control.kickoff_formation &&
       !bytes(kickoff_control).empty(),"Recovered kickoff ball returns to control with keeper coordinates");
 auto keeper_area=kickoff_recovered;keeper_area.current.field.ball_x=204;keeper_area.current.field.ball_y=124;
 auto area_result=continue_session_recovery(keeper_area);
 check(area_result.phase==FieldSessionPhase::keeper_interaction && area_result.kickoff_formation,
       "Recovered kickoff ball pauses at keeper-area interaction");
 auto area_setup=prepare_session_keeper_area(area_result);
 check(static_cast<unsigned>(area_setup.phase)==65 && area_setup.action->menu_route==1 &&
       area_setup.kickoff_formation && !bytes(area_setup).empty(),"Keeper-area setup saves its opposing-side menu route");
 const auto area_bytes=bytes(area_setup);std::istringstream area_stream(area_bytes);
 check(bytes(load_phase_session(area_stream))==area_bytes,"Keeper-area setup roundtrip");
 auto old_area=area_bytes;old_area[10]='i';old_area.resize(370);
 rejects([&]{std::istringstream old(old_area);load_phase_session(old);});
 auto old_area_entry=bytes(area_result);old_area_entry[10]='i';old_area_entry.resize(370);
 std::istringstream old_area_stream(old_area_entry);
 check(bytes(load_phase_session(old_area_stream))==bytes(area_result),"Legacy i keeper-area entry loads");
 for(unsigned length=370;length<372;++length) {
  auto truncated=area_bytes;truncated.resize(length);
  rejects([&]{std::istringstream input(truncated);load_phase_session(input);});
 }
 auto invalid_area=area_bytes;invalid_area[370]=2;
 rejects([&]{std::istringstream input(invalid_area);load_phase_session(input);});
 invalid_area=area_bytes;invalid_area[371]=0;
 rejects([&]{std::istringstream input(invalid_area);load_phase_session(input);});
 auto missing_area=area_setup;missing_area.keeper_area.reset();rejects([&]{validate_phase_session(missing_area);});
 auto outside_area=area_result;outside_area.current.field.ball_x=120;
 rejects([&]{prepare_session_keeper_area(outside_area);});
 rejects([&]{prepare_session_keeper_area(area_setup);});
 auto own_area=area_result;own_area.current.field.ball_x=56;own_area.current.field.ball_cell=17;
 auto own_setup=prepare_session_keeper_area(own_area);
 auto own_menu=open_prepared_keeper_area_menu(own_setup,64,ActionMenuContent{}).state;
 check(own_menu.phase==FieldSessionPhase::human_action_menu && own_menu.action->menu_route==2 &&
       own_menu.keeper_area && own_menu.control->facing==64 && !bytes(own_menu).empty(),
       "Prepared own-area menu retains its route and captured facing");
 auto own_confirmed=poll_human_action_menu(own_menu,129,ActionMenuContent{}).state;
 check(own_confirmed.phase==FieldSessionPhase::attack_choice_pending && own_confirmed.keeper_area &&
       !bytes(own_confirmed).empty(),"Own-area human choice retains route context after confirmation");
 auto choice_bytes=bytes(own_confirmed);std::istringstream choice_stream(choice_bytes);
 check(bytes(load_phase_session(choice_stream))==choice_bytes,"Area confirmation roundtrip");
 auto prior_choice=choice_bytes;prior_choice[10]='k';
 rejects([&]{std::istringstream old(prior_choice);load_phase_session(old);});
 auto prior_menu=bytes(own_menu);prior_menu[10]='k';std::istringstream prior_menu_stream(prior_menu);
 check(bytes(load_phase_session(prior_menu_stream))==bytes(own_menu),"Legacy k area menu loads");
 rejects([&]{prepare_session_basic_shot(own_confirmed,FieldPlayerProfiles{},FieldDefenseContent{});});
 auto defense_area=area_result;defense_area.current.field.side=11;defense_area.current.field.ball_owner=12;defense_area.control->controlled_player=1;
 auto defense_setup=prepare_session_keeper_area(defense_area);defense_setup.action->cursor=211;
 auto defense_menu=open_prepared_keeper_area_menu(defense_setup,0,ActionMenuContent{}).state;
 check(defense_menu.phase==FieldSessionPhase::keeper_area_defense_pending && defense_menu.action->cursor==211 && defense_menu.keeper_area,
       "Defensive entry preserves cursor and stops before choices");
 for(const auto& output:{own_menu,defense_menu}) {
  auto saved=bytes(output);std::istringstream input(saved);
  check(bytes(load_phase_session(input))==saved,"Prepared area menu roundtrip");
  saved[10]='j';rejects([&]{std::istringstream old(saved);load_phase_session(old);});
  rejects([&]{open_prepared_keeper_area_menu(output,0,ActionMenuContent{});});
 }
 auto prior_setup=bytes(own_setup);prior_setup[10]='j';std::istringstream prior_setup_stream(prior_setup);
 check(bytes(load_phase_session(prior_setup_stream))==bytes(own_setup),"Legacy j area setup loads");
 rejects([&]{open_prepared_keeper_area_menu(own_setup,17,ActionMenuContent{});});
 rejects([&]{open_prepared_keeper_area_menu(area_setup,64,ActionMenuContent{});});
 for(const auto& output:{kickoff_control,area_result}) {
  auto saved=bytes(output);std::istringstream input(saved);
  check(bytes(load_phase_session(input))==saved,"Recovery continuation roundtrip");
  saved[10]='h';saved.resize(370);rejects([&]{std::istringstream old(saved);load_phase_session(old);});
 }
 auto pending_story=kickoff_recovered;pending_story.recovery->game_mode=4;
 pending_story.current.field.roster[pending_story.current.field.ball_owner].type=2;
 rejects([&]{continue_session_recovery(pending_story);});
 auto recovered_bytes=bytes(kickoff_recovered);std::istringstream recovered_stream(recovered_bytes);
 check(bytes(load_phase_session(recovered_stream))==recovered_bytes,"Kickoff punch recovered-owner roundtrip");
 auto old_recovered=recovered_bytes;old_recovered[10]='g';old_recovered.resize(370);
 rejects([&]{std::istringstream i(old_recovered);load_phase_session(i);});
 auto old_punch=bytes(kickoff_punch);old_punch[10]='g';old_punch.resize(370);std::istringstream old_punch_stream(old_punch);
 check(bytes(load_phase_session(old_punch_stream))==bytes(kickoff_punch),"Legacy g kickoff punch handoff loads");
 rejects([&]{advance_session_keeper_punch_recovery(kickoff_recovered,primary,movement);});
 const auto recovered=advance_session_keeper_punch_recovery(punch_recovery,primary,movement);
 check(recovered.state.phase==FieldSessionPhase::recovered_owner && recovered.state.current.field.ball_x==156 &&
  recovered.state.current.field.ball_y==124 && recovered.state.current.field.ball_cell==233 &&
  recovered.state.current.clock.remaining==448 && recovered.state.current.field.elapsed==0 && !recovered.special_owner_cleared,
  "Punch recovery acknowledges movement but skips a second jitter and clock/reward debit");
 rejects([&]{advance_session_keeper_punch_recovery(recovered.state,primary,movement);});
 auto legacy=original;legacy[10]='N';legacy.resize(361);std::istringstream old(legacy);check(bytes(load_phase_session(old))==original,"N keeper decisions still readable");
 auto forbidden=encoded;forbidden[10]='N';rejects([&]{std::istringstream i(forbidden);load_phase_session(i);});
 auto invalid=caught.state;invalid.current.field.ball_owner=12;rejects([&]{validate_phase_session(invalid);});
 invalid=punched.state;invalid.current.field.elapsed=0;rejects([&]{validate_phase_session(invalid);});
 auto missing=content;missing.experience.reset();rejects([&]{resolve_session_keeper_save(s,missing);});
 s.keeper_decision->result_code=0;rejects([&]{resolve_session_keeper_save(s,content);});
 std::cout<<"Keeper save handoff checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

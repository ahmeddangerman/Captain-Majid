#include "captain/keeper_save_session.hpp"
#include "captain/recovery_session.hpp"
#include "captain/action_menu.hpp"
#include "captain/area_ai_session.hpp"
#include "captain/phase_session_io.hpp"
#include "captain/ball_flight_session.hpp"
#include "captain/defender_selection.hpp"
#include "captain/clearance_race.hpp"
#include "captain/control_session.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
namespace {
void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void reject_test(F f,int line){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}if(!v){std::cerr<<"Expected invalid state rejection at test line "<<line<<'\n';std::exit(1);}}
#define rejects(f) reject_test(f,__LINE__)
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
 auto prior_choice=choice_bytes;prior_choice[10]='k';prior_choice.resize(372);
 rejects([&]{std::istringstream old(prior_choice);load_phase_session(old);});
 auto prior_menu=bytes(own_menu);prior_menu[10]='k';prior_menu.resize(372);std::istringstream prior_menu_stream(prior_menu);
 check(bytes(load_phase_session(prior_menu_stream))==bytes(own_menu),"Legacy k area menu loads");
 rejects([&]{prepare_session_basic_shot(own_confirmed,FieldPlayerProfiles{},FieldDefenseContent{});});
 auto defense_area=area_result;defense_area.current.field.side=11;defense_area.current.field.ball_owner=12;defense_area.control->controlled_player=1;
 auto defense_setup=prepare_session_keeper_area(defense_area);defense_setup.action->cursor=211;
 auto defense_menu=open_prepared_keeper_area_menu(defense_setup,0,ActionMenuContent{}).state;
 check(defense_menu.phase==FieldSessionPhase::keeper_area_defense_pending && defense_menu.action->cursor==211 && defense_menu.keeper_area,
       "Defensive entry preserves cursor and stops before choices");
 auto empty_defense=begin_keeper_area_defense(defense_menu,7);
 check(static_cast<unsigned>(empty_defense.phase)==68 && empty_defense.action->cursor==211 &&
       !bytes(empty_defense).empty(),"Empty defensive prefix returns without initializing choices");
 auto selected_defense=defense_menu;selected_defense.current.field.selected_defenders=1;
 selected_defense.scheduling->selected_ids={1,0,0,0,0};
 auto initialized_defense=begin_keeper_area_defense(selected_defense,7);
 check(static_cast<unsigned>(initialized_defense.phase)==67 && initialized_defense.action->cursor==0 &&
       !bytes(initialized_defense).empty(),"Nonempty defensive prefix initializes its choice state");
 check(initialized_defense.area_defense && initialized_defense.area_defense->blink==7 &&
       initialized_defense.area_defense->actions[3]==255 && initialized_defense.area_defense->subtypes[0]==0,
       "Defense initialization stores captured blink and unselected canonical slots");
 AreaDefenseMenuContent defense_content{};defense_content.titles[2]=42;
 auto prompt=open_keeper_area_defense_prompt(initialized_defense,defense_content);
 check(static_cast<unsigned>(prompt.state.phase)==69 && prompt.presentation.size()==3 &&
       prompt.presentation[0].id==18 && prompt.presentation[1].id==29 && prompt.presentation[1].slot==0 &&
       prompt.presentation[2].id==42 && !bytes(prompt.state).empty(),"Defense prompt emits count, indexed blank icon and private title");
 auto four_defenders=initialized_defense;four_defenders.current.field.selected_defenders=4;
 four_defenders.scheduling->selected_ids={1,2,3,4,0};
 auto four_prompt=open_keeper_area_defense_prompt(four_defenders,defense_content);
 check(four_prompt.presentation.size()==6 && four_prompt.presentation[0].id==21 &&
       four_prompt.presentation[4].slot==3,"Defensive prompt retains each icon slot");
 auto prompt_bytes=bytes(prompt.state);std::istringstream prompt_stream(prompt_bytes);
 check(bytes(load_phase_session(prompt_stream))==prompt_bytes,"Defensive prompt roundtrip");
 auto old_prompt=prompt_bytes;old_prompt[10]='m';old_prompt.resize(382);
 rejects([&]{std::istringstream old(old_prompt);load_phase_session(old);});
 auto old_initial=bytes(initialized_defense);old_initial[10]='m';old_initial.resize(382);std::istringstream old_initial_stream(old_initial);
 check(bytes(load_phase_session(old_initial_stream))==bytes(initialized_defense),"Legacy m initial defense loads");
 rejects([&]{open_keeper_area_defense_prompt(prompt.state,defense_content);});
 defense_content.directions[2][0]=4;defense_content.field_icons=std::array<std::uint8_t,7>{10,11,12,13,55,15,16};
 auto defense_poll=poll_keeper_area_defense(prompt.state,1,defense_content);
 check(static_cast<unsigned>(defense_poll.state.phase)==70 && defense_poll.state.area_defense->actions[0]==4 &&
       defense_poll.state.area_defense->blink==13 && defense_poll.presentation[0].id==55 &&
       !bytes(defense_poll.state).empty(),"Defensive direction stores action and emits its first blink");
 auto defense_confirm=poll_keeper_area_defense(defense_poll.state,131,defense_content);
 check(static_cast<unsigned>(defense_confirm.state.phase)==71 && defense_confirm.confirmed &&
       defense_confirm.state.area_defense->pressed==131 && defense_confirm.state.recovery->opposing_player==1 &&
       defense_confirm.state.current.field.roster[1].stamina==defense_poll.state.current.field.roster[1].stamina,
       "Defensive confirm retains its sample and defender without charging stamina");
 auto back_state=four_prompt.state;back_state.phase=FieldSessionPhase::keeper_area_defense_poll;
 back_state.action->cursor=1;back_state.area_defense->actions={4,2,255,255};
 auto back_result=poll_keeper_area_defense(back_state,65,defense_content);
 check(back_result.state.action->cursor==0 && back_result.presentation.size()==2 &&
       back_result.presentation[0].slot==1 && back_result.state.area_defense->actions[0]==4,
       "Back navigation restores the old slot and takes precedence over direction");
 for(const auto& output:{defense_poll.state,defense_confirm.state}) {
  auto saved=bytes(output);std::istringstream stream(saved);
  check(bytes(load_phase_session(stream))==saved,"Defensive input state roundtrip");
  saved[10]='n';saved.resize(382);
  rejects([&]{std::istringstream old(saved);load_phase_session(old);});
 }
 auto missing_icons=defense_content;missing_icons.field_icons.reset();
 rejects([&]{poll_keeper_area_defense(prompt.state,1,missing_icons);});
 rejects([&]{poll_keeper_area_defense(defense_confirm.state,0,defense_content);});
 auto missing_sample=defense_confirm.state;missing_sample.area_defense->pressed=0;
 rejects([&]{validate_phase_session(missing_sample);});
 auto plain_confirm=defense_confirm.state;plain_confirm.area_defense->pressed=128;
 auto registered=prepare_keeper_area_defense(plain_confirm,defense_content);
 check(static_cast<unsigned>(registered.state.phase)==70 && registered.state.action->cursor==1 &&
       registered.state.area_defense->pressed==0 && registered.presentation.size()==1 &&
       registered.presentation[0].id==55 && registered.presentation[0].slot==0,
       "Ordinary last defender registers then waits for the next confirmation");
 check(registered.state.current.field.roster[1].stamina==defense_confirm.state.current.field.roster[1].stamina,
       "Menu registration does not consume defender stamina");
 auto completed=poll_keeper_area_defense(registered.state,128,defense_content);
 auto ai_input=completed.state;ai_input.current.field.random_first=0;
 ai_input.current.field.roster[ai_input.current.field.ball_owner].type=35;
 ai_input.current.field.roster[ai_input.current.field.ball_owner].stamina=0;
 AiAreaChoiceContent ai_data{};ai_data.profile_rows={0};ai_data.cells[0]={5,32};ai_data.rows.resize(1);ai_data.rows[0][44]=0x10;
 auto ai_chosen=choose_session_area_action(ai_input,ai_data);
 check(static_cast<unsigned>(ai_chosen.phase)==78 && ai_chosen.action->action==6 && ai_chosen.area_ai_target &&
       !ai_chosen.area_ai_target->recipient,"Saved opponent area choice advances phase");
 auto ai_saved=bytes(ai_chosen);std::istringstream ai_stream(ai_saved);
 check(bytes(load_phase_session(ai_stream))==ai_saved,"Area AI choice roundtrip");
 rejects([&]{choose_session_area_action(ai_chosen,ai_data);});
 ai_saved[10]='t';ai_saved.resize(393);rejects([&]{std::istringstream old(ai_saved);load_phase_session(old);});
 ai_data.rows[0][44]=0x20;ai_data.recipient_profile_rows={0};ai_data.recipient_rows.resize(1);ai_data.recipient_rows[0][0]=8;
 ai_input.current.field.random_second=0;const auto before_ai=bytes(ai_input);
 const auto targeted_ai=choose_session_area_action(ai_input,ai_data);
 check(targeted_ai.action->action==1 && targeted_ai.area_ai_target->recipient==13 && bytes(ai_input)==before_ai,
       "AI recipient choice commits target without mutating input");
 auto target_bytes=bytes(targeted_ai);std::istringstream target_stream(target_bytes);
 check(bytes(load_phase_session(target_stream))==target_bytes,"AI target and cell roundtrip");
 auto bad_target=targeted_ai;bad_target.area_ai_target->cell^=1;
 rejects([&]{validate_phase_session(bad_target);});
 auto missing_ai=targeted_ai;missing_ai.area_ai_target.reset();rejects([&]{validate_phase_session(missing_ai);});
 ShotContent ai_attack_data{};ai_attack_data.field_players.alternate={0};ai_attack_data.field_players.skills.resize(1);
 ai_attack_data.field_players.skills[0].fill(20);ai_attack_data.area_attack6=FieldDefenseContent{};
 ai_attack_data.area_attack6->skill_selectors[0]={1,1,1};ai_attack_data.area_attack6->moves[0]={{4,0,7,0},{5,0,8,0}};
 ai_attack_data.area_attack6->strength_curve.fill(64);
 const auto prepared_area=prepare_session_area_attack(ai_chosen,ai_attack_data);
 check(static_cast<unsigned>(prepared_area.phase)==79 && prepared_area.area_ai_target->cost==7 &&
       prepared_area.action->move_effect==4 && (prepared_area.recovery->attack_subtype&128u) &&
       prepared_area.current.field.roster[prepared_area.current.field.ball_owner].stamina==0,
       "Saved area attack commits cost/effect/power without stamina debit");
 auto prepared_bytes=bytes(prepared_area);std::istringstream prepared_stream(prepared_bytes);
 check(bytes(load_phase_session(prepared_stream))==prepared_bytes,"Prepared area attack roundtrip");
 rejects([&]{prepare_session_area_attack(prepared_area,ai_attack_data);});
 auto prior_prepared=prepared_bytes;prior_prepared[10]='u';prior_prepared.resize(396);
 rejects([&]{std::istringstream old(prior_prepared);load_phase_session(old);});
 const auto area_entry=begin_prepared_area_defense(prepared_area);
 check(static_cast<unsigned>(area_entry.state.phase)==80 && area_entry.state.control->motion_requested==2 &&
       area_entry.presentation.size()==1 && area_entry.presentation[0].id==14 &&
       area_entry.state.current.clock.remaining==prepared_area.current.clock.remaining,"Prepared presentation enters area defense without clock debit");
 auto entry_bytes=bytes(area_entry.state);std::istringstream entry_stream(entry_bytes);
 check(bytes(load_phase_session(entry_stream))==entry_bytes,"Area defense entry roundtrip");
 rejects([&]{begin_prepared_area_defense(area_entry.state);});
 entry_bytes[10]='v';rejects([&]{std::istringstream old(entry_bytes);load_phase_session(old);});
 auto second_variant=prepared_area;second_variant.recovery->ball_variant=2;second_variant.control->motion_requested=2;
 const auto second_entry=begin_prepared_area_defense(second_variant);
 check(second_entry.state.control->motion_requested==0,"Second area variant clears prior motion instead of requesting ground motion");
 const auto announced=announce_prepared_area_defenders(area_entry.state,64);
 check(static_cast<unsigned>(announced.state.phase)==81 && announced.state.control->motion_requested==16 &&
       announced.presentation.size()==4 && announced.presentation[1].kind==ControlPresentationKind::wait_frames &&
       announced.presentation[1].id==20,"Area defender announcement emits script and explicit waits");
 auto announced_bytes=bytes(announced.state);std::istringstream announced_stream(announced_bytes);
 check(bytes(load_phase_session(announced_stream))==announced_bytes,"Announced defenders roundtrip");
 rejects([&]{announce_prepared_area_defenders(announced.state,64);});
 rejects([&]{announce_prepared_area_defenders(area_entry.state,1);});
 ai_attack_data.field_players.ordinary.fill(0);
 ai_attack_data.defense.skill_selectors[4]={1,1,1};
 ai_attack_data.defense.moves[4]={{3,0,9,0}};ai_attack_data.defense.strength_curve.fill(64);
 ai_attack_data.area_comparison=AreaComparisonContent{};
 for(auto& pair:ai_attack_data.area_comparison->variation)pair={100,100};
 ai_attack_data.area_comparison->thresholds={200,100,50,0};
 ai_attack_data.field_comparison.eligibility.resize(10);
 auto contact_input=announced.state;const auto contact_id=contact_input.scheduling->selected_ids[0];
 contact_input.current.field.roster[contact_id].type=1;
 contact_input.current.field.roster[contact_id].stamina=100;
 const auto contact_before=bytes(contact_input);
 const auto contact=resolve_session_area_defender(contact_input,ai_attack_data);
 check(static_cast<unsigned>(contact.phase)==82 && contact.area_contact &&
       contact.current.field.roster[contact_id].stamina==91 &&
       contact.area_contact->cost==9 && contact.area_contact->effect==3 &&
       contact.recovery->opposing_player==contact_id && bytes(contact_input)==contact_before,
       "Saved area defender commits stamina and result without mutating input");
 auto contact_bytes=bytes(contact);std::istringstream contact_stream(contact_bytes);
 check(bytes(load_phase_session(contact_stream))==contact_bytes,"Area contact context roundtrip");
 rejects([&]{resolve_session_area_defender(contact,ai_attack_data);});
 auto no_contact=contact;no_contact.area_contact.reset();rejects([&]{validate_phase_session(no_contact);});
 auto wrong_contact=contact;wrong_contact.area_contact->cursor=4;rejects([&]{validate_phase_session(wrong_contact);});
 auto bad_encoding=contact_bytes;bad_encoding[398]=2;
 rejects([&]{std::istringstream bad(bad_encoding);load_phase_session(bad);});
 bad_encoding=contact_bytes;bad_encoding[411]=2;
 rejects([&]{std::istringstream bad(bad_encoding);load_phase_session(bad);});
 auto legacy_announced=announced_bytes;legacy_announced[10]='x';legacy_announced.resize(398);
 std::istringstream legacy_announced_stream(legacy_announced);
 check(bytes(load_phase_session(legacy_announced_stream))==announced_bytes,"Legacy x announcement loads without contact context");
 auto missing_area_data=ai_attack_data;missing_area_data.area_comparison.reset();
 rejects([&]{resolve_session_area_defender(contact_input,missing_area_data);});
 check(bytes(contact_input)==contact_before,"Failed area calculation leaves input unchanged");
 ContactPreludeContent area_clock{};area_clock.field_elapsed.fill(3);area_clock.defender_status.fill(2);
 auto clock_input=contact;clock_input.current.clock.remaining=449;
 const auto clocked=advance_session_area_contact_clock(clock_input,area_clock);
 check(static_cast<unsigned>(clocked.state.phase)==83 && clocked.state.current.clock.remaining==446 &&
       clocked.state.current.field.roster[contact_id].stamina==91 && clocked.prelude && clocked.prelude->elapsed_step==3,
       "Area contact clock advances status/time without another stamina debit");
 rejects([&]{advance_session_area_contact_clock(clocked.state,area_clock);});
 clock_input.current.clock.remaining=1;clock_input.current.clock.extra_time=5;
 const auto paused=advance_session_area_contact_clock(clock_input,area_clock);
 check(static_cast<unsigned>(paused.state.phase)==84,"Area contact preserves added-time pause");
 const auto clock_resumed=resume_session_area_contact_clock(paused.state);
 auto clock_resumed_expected=paused.state;clock_resumed_expected.phase=static_cast<FieldSessionPhase>(83);
 check(bytes(clock_resumed)==bytes(clock_resumed_expected),"Area clock resume changes only its phase");
 rejects([&]{resume_session_area_contact_clock(clock_resumed);});
 clock_input.current.clock.remaining=0;clock_input.current.clock.extra_time=0;
 const auto ended=advance_session_area_contact_clock(clock_input,area_clock);
 check(static_cast<unsigned>(ended.state.phase)==85,"Area contact retains period-end boundary");
 for(const auto& timed:{clocked.state,paused.state,ended.state}) {
  const auto saved=bytes(timed);std::istringstream encoded_clock(saved);
  check(bytes(load_phase_session(encoded_clock))==saved,"Area clock boundary roundtrip");
  auto old_clock=saved;old_clock[10]='y';
  rejects([&]{std::istringstream old(old_clock);load_phase_session(old);});
 }
 auto previous_clock=bytes(clocked.state);previous_clock[10]='z';previous_clock.resize(412);
 std::istringstream previous_clock_stream(previous_clock);
 check(bytes(load_phase_session(previous_clock_stream))==bytes(clocked.state),"Legacy z cursor0 clock snapshot loads");
 auto previous_contact=contact_bytes;previous_contact[10]='y';previous_contact.resize(412);
 std::istringstream previous_contact_stream(previous_contact);
 check(bytes(load_phase_session(previous_contact_stream))==contact_bytes,"Legacy y compared defender loads");
 auto malformed_pause=paused.state;malformed_pause.current.clock.remaining=1;
 rejects([&]{validate_phase_session(malformed_pause);});
 malformed_pause=paused.state;malformed_pause.current.clock.extra_time=0;
 rejects([&]{validate_phase_session(malformed_pause);});
 auto result_input=clocked.state;result_input.area_contact->result=1;result_input.area_sequence_result=1;
 const auto presented_result=present_session_area_contact_result(result_input);
 check(static_cast<unsigned>(presented_result.state.phase)==86 && presented_result.presentation.size()==2 &&
       presented_result.presentation[0].id==17 && presented_result.presentation[1].id==18 &&
       presented_result.state.control->motion_requested==16 &&
       presented_result.state.current.field.roster[contact_id].stamina==91,
       "Prepared nonterminal result requests scripts17/18 without repeating numeric effects");
 rejects([&]{present_session_area_contact_result(presented_result.state);});
 result_input.area_contact->result=2;result_input.area_sequence_result=2;
 const auto terminal_presented=present_session_area_contact_result(result_input);
 check(terminal_presented.presentation[0].id==16 && terminal_presented.state.control->motion_requested==0 &&
       terminal_presented.state.control->facing==0,"Terminal result script16 clears facing and motion");
 for(const auto& output:{presented_result.state,terminal_presented.state}) {
  auto saved=bytes(output);std::istringstream input(saved);
  check(bytes(load_phase_session(input))==saved,"Presented area result roundtrip");
  saved[10]='z';saved.resize(412);
  rejects([&]{std::istringstream old(saved);load_phase_session(old);});
 }
 auto chain=result_input;chain.area_contact->result=1;chain.area_sequence_result=1;chain.current.field.selected_defenders=4;
 chain.scheduling->selected_ids={1,2,3,4,0};chain.action->cursor=4;
 chain.area_defense->actions={4,4,4,4};chain.area_defense->subtypes={0,0,0,0};
 chain.action->power=65535;
 for(unsigned id=2;id<=4;++id){chain.current.field.roster[id].type=1;chain.current.field.roster[id].stamina=100;}
 auto loop=advance_session_area_defender(present_session_area_contact_result(chain).state);
 check(static_cast<unsigned>(loop.state.phase)==87 && loop.state.area_defender_cursor==1 && !loop.state.area_contact &&
       loop.presentation.size()==1 && loop.presentation[0].kind==ControlPresentationKind::wait_frames &&
       loop.presentation[0].id==1,"Nonterminal result advances once and requests next-defender wait");
 rejects([&]{advance_session_area_defender(loop.state);});
 rejects([&]{skip_session_area_defender(loop.state);});
 for(unsigned cursor=1;cursor<4;++cursor) {
  const auto resolved=resolve_session_area_defender(loop.state,ai_attack_data);
  check(resolved.area_contact->cursor==cursor && resolved.current.field.roster[cursor+1].stamina==91,
        "Later selected defender consumes its own stamina exactly once");
  if(cursor==1) {
   const auto current=bytes(resolved);
   for(const auto version:{'y','z'}) {
    auto downgraded=current;downgraded[10]=version;downgraded.resize(412);
    rejects([&]{std::istringstream old(downgraded);load_phase_session(old);});
   }
   std::istringstream later_saved(current);
   check(bytes(load_phase_session(later_saved))==current,"Later defender cursor roundtrips in the new format");
  }
  loop=advance_session_area_defender(present_session_area_contact_result(advance_session_area_contact_clock(resolved,area_clock).state).state);
 }
 check(static_cast<unsigned>(loop.state.phase)==88 && loop.state.area_defender_cursor==4 && !loop.state.area_contact &&
       loop.presentation.empty(),"Full defender prefix stops before final clock without a fifth wait");
 rejects([&]{resolve_session_area_defender(loop.state,ai_attack_data);});
 auto skipped=announced.state;skipped.current.field.selected_defenders=4;skipped.action->cursor=4;
 skipped.scheduling->selected_ids={1,2,3,4,0};skipped.area_defense->actions={5,6,5,6};skipped.area_defense->subtypes={0,0,0,0};
 const auto skip_clock=skipped.current.clock.remaining;const auto skip_energy=skipped.current.field.roster;
 for(unsigned cursor=0;cursor<4;++cursor){
  const auto step=skip_session_area_defender(skipped);skipped=step.state;
  check(skipped.area_defender_cursor==cursor+1 && step.presentation.size()==(cursor<3?1u:0u),"Skip advances exactly one selected slot");
 }
 check(static_cast<unsigned>(skipped.phase)==88 && skipped.current.clock.remaining==skip_clock &&
       skipped.current.field.roster[1].stamina==skip_energy[1].stamina && skipped.recovery->opposing_player==4,
       "All-skip prefix ends without contact, stamina or clock effects");
 rejects([&]{skip_session_area_defender(skipped);});
 chain.area_contact->result=2;chain.area_sequence_result=2;
 const auto terminal_exit=advance_session_area_defender(present_session_area_contact_result(chain).state);
 check(static_cast<unsigned>(terminal_exit.state.phase)==89 && !terminal_exit.state.area_defender_cursor &&
       terminal_exit.state.area_contact->result==2 && terminal_exit.presentation.empty() &&
       terminal_exit.state.current.field.roster[2].stamina==100,"Terminal result stops before later defenders");
 for(const auto& saved_state:{loop.state,skipped,terminal_exit.state}){
  const auto saved=bytes(saved_state);std::istringstream input(saved);
  check(bytes(load_phase_session(input))==saved,"Area sequence cursor and exit roundtrip");
 }
 const auto prefix_before=bytes(loop.state);
 const auto prefix_clock=calculate_area_prefix_clock(loop.state,3);
 check(prefix_clock.state.clock.remaining==loop.state.current.clock.remaining-3 &&
       prefix_clock.state.field.elapsed==static_cast<std::uint8_t>(loop.state.current.field.elapsed+3) &&
       bytes(loop.state)==prefix_before,"Area prefix clock preview is pure and advances time once");
 rejects([&]{calculate_area_prefix_clock(contact,3);});
 ai_attack_data.area_prefix_elapsed=std::array<std::uint8_t,7>{};
 (*ai_attack_data.area_prefix_elapsed)[loop.state.action->action]=3;
 const auto profiled=advance_profiled_area_prefix_clock(loop.state,ai_attack_data);
 check(bytes(profiled.state)==bytes(advance_session_area_prefix_clock(loop.state,3).state),"Original timing content selects saved attack duration");
 auto missing_prefix_content=ai_attack_data;missing_prefix_content.area_prefix_elapsed.reset();
 rejects([&]{advance_profiled_area_prefix_clock(loop.state,missing_prefix_content);});
 rejects([&]{preview_area_result_dispatch(loop.state,ai_attack_data);});
 auto prefix_input=loop.state;prefix_input.current.clock.remaining=449;
 const auto prefix_saved=advance_session_area_prefix_clock(prefix_input,3);
 check(static_cast<unsigned>(prefix_saved.state.phase)==90 && prefix_saved.state.current.clock.remaining==446,"Saved prefix clock advances once");
 rejects([&]{advance_session_area_prefix_clock(prefix_saved.state,3);});
 rejects([&]{preview_area_result_dispatch(prefix_saved.state,ai_attack_data);});
 auto dispatch_data=ai_attack_data;dispatch_data.movement=movement;
 dispatch_data.experience=ExperienceContent{};dispatch_data.experience->stage_bases[0]=4;dispatch_data.experience->reward_factors[1]=64;
 auto ready_dispatch=prefix_saved.state;ready_dispatch.current.field.elapsed=0;
 const auto ready_bytes=bytes(ready_dispatch);
 const auto dispatch_preview=preview_area_result_dispatch(ready_dispatch,dispatch_data);
 check(dispatch_preview.route==0 && dispatch_preview.movement.players.empty() && bytes(ready_dispatch)==ready_bytes,"Zero-time area dispatch preview preserves input");
 auto attack_dispatch=prefix_saved.state;attack_dispatch.area_sequence_result=1;
 attack_dispatch.current.field.roster[attack_dispatch.current.field.ball_owner].type=1;attack_dispatch.experience=ExperienceState{};
 const auto attack_dispatch_before=bytes(attack_dispatch);
 const auto continued=advance_session_area_result_dispatch(attack_dispatch,dispatch_data);
 check(continued.route==0 && continued.state.phase==FieldSessionPhase::keeper_area_attack_continuation &&
       continued.state.current.field.ball_owner==attack_dispatch.current.field.ball_owner &&
       continued.state.current.field.elapsed==0 && !continued.reward && bytes(attack_dispatch)==attack_dispatch_before,
       "Final area result 0/1 commits movement once and preserves possession at attack continuation");
 check(continued.state.area_defense && continued.state.area_defense->actions[0]==attack_dispatch.area_defense->actions[0],
       "Area action6 retains defensive choices needed by a later recovery");
 auto action4_source=prefix_saved.state;action4_source.action->action=4;action4_source.recovery->attack_subtype=0;
 action4_source.current.field.roster[action4_source.current.field.ball_owner].type=1;action4_source.experience=ExperienceState{};
 for(unsigned i=0;i<action4_source.current.field.selected_defenders;++i){action4_source.area_defense->actions[i]=5;action4_source.area_defense->subtypes[i]=0;}
 const auto action4_dispatch=advance_session_area_result_dispatch(action4_source,dispatch_data);
 check(action4_dispatch.state.area_defense && action4_dispatch.state.area_defense->actions[0]==5,
       "Area action4 dispatch retains the earlier defender choices");
 const auto action4_bytes=bytes(action4_dispatch.state);std::istringstream action4_stream(action4_bytes);
 check(static_cast<unsigned char>(action4_bytes[10])==131 && bytes(load_phase_session(action4_stream))==action4_bytes,
       "Area action4 retained choices survive snapshot roundtrip");
 auto action4_old=action4_bytes;action4_old[10]=static_cast<char>(130);
 rejects([&]{std::istringstream old(action4_old);load_phase_session(old);});
 const auto action4_encounter=continue_session_area_action4(action4_dispatch.state,0,0,&*dispatch_data.experience);
 check(action4_encounter.experience->totals[0]==1,"Area action4 awards carrier experience once");
 rejects([&]{continue_session_area_action4(action4_dispatch.state,0,0,nullptr);});
 check(action4_encounter.phase==FieldSessionPhase::area_action4_encounters_pending &&
       action4_encounter.current.field.selected_defenders==action4_dispatch.state.current.field.selected_defenders &&
       action4_encounter.recovery->ball_variant==0 && bytes(action4_dispatch.state)==action4_bytes,
       "Area action4 retains defenders choosing action5 and clears the ball variant");
 auto action4_skips=action4_dispatch.state;for(unsigned i=0;i<action4_skips.current.field.selected_defenders;++i)action4_skips.area_defense->actions[i]=6;
 const auto action4_control=continue_session_area_action4(action4_skips,0,0,&*dispatch_data.experience);
 check(action4_control.phase==FieldSessionPhase::area_action4_control_pending &&
       action4_control.current.field.selected_defenders==action4_skips.current.field.selected_defenders,
       "Area action4 without choice5 preserves the original prefix before control");
 auto action4_legacy=action4_dispatch.state;action4_legacy.area_defense.reset();
 const auto legacy4_bytes=bytes(action4_legacy);std::istringstream legacy4_stream(legacy4_bytes);
 check(static_cast<unsigned char>(legacy4_bytes[10])==130 && bytes(load_phase_session(legacy4_stream))==legacy4_bytes,
       "Legacy action4 continuation remains readable without invented choices");
 rejects([&]{continue_session_area_action4(action4_legacy,0,0,&*dispatch_data.experience);});
 rejects([&]{continue_session_area_action4(action4_encounter,0,0,&*dispatch_data.experience);});
 AreaAction6DestinationContent area6_content{};for(auto& p:area6_content.positions)p={80,100};
 const auto area6_before=bytes(continued.state);
 const auto area6_destination=continue_session_area_action6(continued.state,64,2,area6_content,&*dispatch_data.experience);
 check(area6_destination.experience->totals[0]==1,"Area action6 awards carrier experience once");
 rejects([&]{continue_session_area_action6(continued.state,64,2,area6_content,nullptr);});
 check(area6_destination.phase==FieldSessionPhase::area_action6_destination_pending &&
       area6_destination.current.field.ball_x==175 && area6_destination.current.field.ball_y==100 &&
       area6_destination.current.field.ball_cell==field_cell_at(175,100) &&
       area6_destination.area_ai_target->cell==area6_destination.current.field.ball_cell &&
       bytes(continued.state)==area6_before,"Area action6 selects mirrored destination without mutating input");
 const auto area6_bytes=bytes(area6_destination);std::istringstream area6_stream(area6_bytes);
 check(bytes(load_phase_session(area6_stream))==area6_bytes,"Area action6 destination snapshot roundtrips");
 auto area6_old=area6_bytes;area6_old[10]=static_cast<char>(130);
 rejects([&]{std::istringstream old(area6_old);load_phase_session(old);});
 auto area6_bad=area6_content;for(auto& p:area6_bad.positions)p={0,0};
 rejects([&]{continue_session_area_action6(continued.state,64,2,area6_bad,&*dispatch_data.experience);});
 rejects([&]{continue_session_area_action6(area6_destination,64,2,area6_content,&*dispatch_data.experience);});
 auto area6_race_input=area6_destination;area6_race_input.recovery->opposing_player=1;
 for(auto& p:area6_race_input.current.field.motion){p.x=100<<8;p.y=120<<8;}
 for(auto id:{2,13}){auto& p=area6_race_input.current.field.motion[field_motion_index(id)];
  p.x=static_cast<std::uint16_t>(unsigned(area6_race_input.current.field.ball_x)<<8);
  p.y=static_cast<std::uint16_t>(unsigned(area6_race_input.current.field.ball_y)<<8);}
 FieldPhaseContent area6_race_movement{};area6_race_movement.profiles.speeds.resize(1);
 area6_race_movement.geometry.direction_thresholds.fill(65535);
 PlayerProfileMappings area6_race_profiles{};ClearanceRaceContent area6_race_data{};
 const auto race_before=bytes(area6_race_input);
 const auto area6_race=resolve_session_area_action6_race(area6_race_input,area6_race_profiles,area6_race_movement,area6_race_data);
 check(area6_race.winner==13 && area6_race.state.phase==FieldSessionPhase::area_action6_race_winner &&
       area6_race.state.current.field.ball_owner==13 && bytes(area6_race_input)==race_before,
       "Area action6 race commits the first eligible active-side winner");
 const auto area6_race_bytes=bytes(area6_race.state);std::istringstream area6_race_stream(area6_race_bytes);
 check(bytes(load_phase_session(area6_race_stream))==area6_race_bytes,"Area action6 race snapshot roundtrips");
 const auto area6_control=continue_session_area_action6_winner(area6_race.state,64,0);
 check(area6_control.phase==FieldSessionPhase::area_action6_control_pending && area6_control.control->facing==64 &&
       area6_control.area_ai_target && area6_control.area_defense && area6_control.clearance_race &&
       bytes(area6_race.state)==area6_race_bytes,"Area action6 winner reaches saved control entry preserving prior context");
 const auto area6_control_bytes=bytes(area6_control);std::istringstream area6_control_stream(area6_control_bytes);
 check(bytes(load_phase_session(area6_control_stream))==area6_control_bytes,"Area action6 control handoff roundtrips");
 const auto area6_ready=prepare_session_area_action6_control(area6_control);
 check(area6_ready.state.phase==FieldSessionPhase::area_action6_control_ready && area6_ready.state.control->decision_timer==10 &&
       area6_ready.state.area_defense && area6_ready.state.clearance_race && bytes(area6_control)==area6_control_bytes,
       "Area action6 enters prepared control while retaining original area and race context");
 const auto area6_ready_bytes=bytes(area6_ready.state);std::istringstream area6_ready_stream(area6_ready_bytes);
 check(bytes(load_phase_session(area6_ready_stream))==area6_ready_bytes,"Area action6 prepared control snapshot roundtrips");
 rejects([&]{prepare_session_area_action6_control(area6_ready.state);});
 rejects([&]{continue_session_area_action6_winner(area6_control,64,0);});
 auto timeout_input=area6_race_input;
 for(auto& p:timeout_input.current.field.motion){p.x=100<<8;p.y=120<<8;}
 const auto area6_timeout=resolve_session_area_action6_race(timeout_input,area6_race_profiles,area6_race_movement,area6_race_data);
 check(area6_timeout.state.phase==FieldSessionPhase::area_action6_race_timeout && !area6_timeout.winner,
       "Area action6 race reaches timeout when no candidate moves");
 const auto timeout_before=bytes(area6_timeout.state);
 const auto area6_recovered=advance_session_area_action6_recovery(area6_timeout.state,area6_race_profiles,area6_race_movement);
 check(area6_recovered.state.phase==FieldSessionPhase::area_action6_recovered_owner && area6_recovered.recipient &&
       area6_recovered.state.area_defense && area6_recovered.state.clearance_race && bytes(area6_timeout.state)==timeout_before,
       "Area action6 timeout recovery preserves selected choices and race metadata");
 const auto area6_recovered_bytes=bytes(area6_recovered.state);std::istringstream area6_recovered_stream(area6_recovered_bytes);
 check(bytes(load_phase_session(area6_recovered_stream))==area6_recovered_bytes,"Area action6 recovered-owner snapshot roundtrips");
 const auto area6_recovered_control=continue_session_area_action6_recovery(area6_recovered.state,64,0);
 check(area6_recovered_control.phase==FieldSessionPhase::area_action6_recovered_control &&
       area6_recovered_control.clearance_race && area6_recovered_control.area_defense && bytes(area6_recovered.state)==area6_recovered_bytes,
       "Area action6 recovered owner reaches control handoff with retained context");
 const auto area6_recovered_control_bytes=bytes(area6_recovered_control);std::istringstream area6_recovered_control_stream(area6_recovered_control_bytes);
 check(bytes(load_phase_session(area6_recovered_control_stream))==area6_recovered_control_bytes,"Area action6 recovered control snapshot roundtrips");
 rejects([&]{continue_session_area_action6_recovery(area6_recovered_control,64,0);});
 auto legacy_timeout=area6_timeout.state;legacy_timeout.area_defense.reset();
 rejects([&]{advance_session_area_action6_recovery(legacy_timeout,area6_race_profiles,area6_race_movement);});
 rejects([&]{advance_session_area_action6_recovery(area6_recovered.state,area6_race_profiles,area6_race_movement);});
 rejects([&]{resolve_session_area_action6_race(area6_race.state,area6_race_profiles,area6_race_movement,area6_race_data);});
 auto area_flight_input=continued.state;
 area_flight_input.action->action=1;area_flight_input.recovery->attack_subtype=0;area_flight_input.area_defense.reset();
 area_flight_input.current.field.ball_x=204;area_flight_input.current.field.ball_y=124;
 area_flight_input.current.field.ball_cell=field_cell_at(204,124);
 area_flight_input.current.field.motion[field_motion_index(13)].x=128<<8;
 area_flight_input.current.field.motion[field_motion_index(13)].y=124<<8;
 area_flight_input.area_ai_target->recipient=13;
 area_flight_input.area_ai_target->cell=field_cell_at(128,124);
 BallVelocityContent area_velocity{};area_velocity.quarter_wave.fill(100);
 AreaFlightAcknowledgement ack{64,192,4,64,2,0x1234,0xfedc};
 const auto flight_before=bytes(area_flight_input);
 const auto area_flight=begin_prepared_area_action_flight(area_flight_input,ack,movement.geometry,area_velocity,&*dispatch_data.experience);
 check(area_flight.experience->totals[0]==1,"Area flight awards carrier experience once before clearing the selected prefix");
 rejects([&]{begin_prepared_area_action_flight(area_flight_input,ack,movement.geometry,area_velocity,nullptr);});
 auto undefended_flight=area_flight_input;undefended_flight.current.field.selected_defenders=0;undefended_flight.scheduling->selected_ids.fill(0);
 undefended_flight.action->cursor=0;undefended_flight.area_defender_cursor=0;
 check(begin_prepared_area_action_flight(undefended_flight,ack,movement.geometry,area_velocity,nullptr).experience->totals[0]==0,
       "Area continuation without selected defenders skips the experience award");
 check(area_flight.phase==FieldSessionPhase::area_ball_advance_ready && area_flight.ball_trajectory->flags==1 &&
       area_flight.shot_flight->counter==255 && area_flight.shot_flight->distance==4 &&
       area_flight.area_ai_target->recipient==13 && area_flight.current.field.selected_defenders==0 &&
       bytes(area_flight_input)==flight_before,"Area flight setup retains recipient and commits flag1 after acknowledgement");
 auto area_flight_bytes=bytes(area_flight);std::istringstream area_flight_stream(area_flight_bytes);
 check(static_cast<unsigned char>(area_flight_bytes[10])==131 && bytes(load_phase_session(area_flight_stream))==area_flight_bytes,
       "Area flight snapshot version65 roundtrips");
 const auto flight_step=advance_prepared_area_ball_cell(area_flight);
 check((flight_step.state.phase==FieldSessionPhase::area_ball_cell_callback ||
        flight_step.state.phase==FieldSessionPhase::area_ball_arrival_callback) &&
       flight_step.state.ball_trajectory->flags==1 && bytes(area_flight)==area_flight_bytes,
       "Area numeric flight commits to a separate callback boundary without mutating input");
 const auto step_bytes=bytes(flight_step.state);std::istringstream step_stream(step_bytes);
 check(bytes(load_phase_session(step_stream))==step_bytes,"Area flight callback snapshot roundtrips");
 rejects([&]{advance_prepared_area_ball_cell(flight_step.state);});
 check(flight_step.state.phase==FieldSessionPhase::area_ball_cell_callback,"Area flight fixture reaches a new-cell callback");
 auto cell_input=flight_step.state;cell_input.current.field.random_first=1;cell_input.current.field.random_second=0;
 for(unsigned id=1;id<=10;++id){auto& p=cell_input.current.field.motion[field_motion_index(id)];
  p.x=static_cast<std::uint16_t>(unsigned(cell_input.current.field.ball_x)<<8);
  p.y=static_cast<std::uint16_t>(unsigned(cell_input.current.field.ball_y)<<8);p.status=0;}
 const auto cell_before=bytes(cell_input);
 const auto cell_done=resolve_session_area_ball_cell(cell_input);
 check(cell_done.phase==FieldSessionPhase::area_ball_advance_ready && cell_done.current.field.selected_defenders==4 &&
       cell_done.scheduling->selected_ids[0]==1 && cell_done.scheduling->selected_ids[3]==4 && bytes(cell_input)==cell_before,
       "Area cell callback accumulates opposing defenders with original capacity");
 rejects([&]{resolve_session_area_ball_cell(cell_done);});
 const auto next_area_step=advance_prepared_area_ball_cell(cell_done);
 check(next_area_step.state.current.field.selected_defenders==4,"Area flight preserves selected defenders across subsequent steps");
 auto downgraded_flight=area_flight_bytes;downgraded_flight[10]=static_cast<char>(130);
 rejects([&]{std::istringstream old(downgraded_flight);load_phase_session(old);});
 auto at_target=area_flight_input;at_target.current.field.motion[field_motion_index(13)].x=204<<8;
 at_target.current.field.motion[field_motion_index(13)].y=124<<8;at_target.area_ai_target->cell=field_cell_at(204,124);
 at_target.action->auxiliary|=128;
 const auto area_arrived=begin_prepared_area_action_flight(at_target,ack,movement.geometry,area_velocity,&*dispatch_data.experience);
 check(area_arrived.phase==FieldSessionPhase::area_ball_already_arrived && area_arrived.shot_flight->counter==38 &&
       area_arrived.ball_trajectory->vx==0x1234 && area_arrived.ball_trajectory->vy==0xfedc,
       "Area early return preserves captured stale velocity and special counter");
 const auto arrived_bytes=bytes(area_arrived);std::istringstream arrived_stream(arrived_bytes);
 check(bytes(load_phase_session(arrived_stream))==arrived_bytes,"Area early return snapshot preserves stale velocity");
 auto arrived_old=arrived_bytes;arrived_old[10]=static_cast<char>(130);
 rejects([&]{std::istringstream old(arrived_old);load_phase_session(old);});
 const auto received=continue_area_already_arrived(area_arrived);
 check(received.phase==FieldSessionPhase::area_pass_received && received.current.field.ball_owner==13 &&
       received.current.field.side==area_arrived.current.field.side && received.ball_trajectory->vx==0x1234 &&
       bytes(area_arrived)==arrived_bytes,"Area early return transfers to saved recipient without changing input");
 const auto received_bytes=bytes(received);std::istringstream received_stream(received_bytes);
 check(bytes(load_phase_session(received_stream))==received_bytes,"Area pass-received snapshot roundtrips");
 auto shifted_recipient=received;shifted_recipient.current.field.motion[field_motion_index(13)].x=200<<8;
 shifted_recipient.current.field.motion[field_motion_index(13)].y=120<<8;
 const auto pass_before=bytes(shifted_recipient);const auto keeper_entry=continue_area_pass_received(shifted_recipient);
 check(keeper_entry.phase==FieldSessionPhase::area_pass_keeper_entry && keeper_entry.current.field.ball_x==200 &&
       keeper_entry.current.field.ball_y==120 && (keeper_entry.ball_trajectory->x&255)==64 &&
       (keeper_entry.ball_trajectory->y&255)==192 && bytes(shifted_recipient)==pass_before,
       "Area pass handoff positions the ball at the recipient while preserving fractions");
 const auto keeper_entry_bytes=bytes(keeper_entry);std::istringstream keeper_entry_stream(keeper_entry_bytes);
 check(bytes(load_phase_session(keeper_entry_stream))==keeper_entry_bytes,"Area pass keeper-entry snapshot roundtrips");
 const auto keeper_prepared=prepare_area_pass_keeper(keeper_entry);
 check(keeper_prepared.phase==FieldSessionPhase::area_pass_keeper_prepared && keeper_prepared.keeper_area &&
       keeper_prepared.action->action==1 && keeper_prepared.action->menu_route==2 &&
       keeper_prepared.shot_flight->counter==255 && keeper_prepared.ball_trajectory->flags==1 &&
       bytes(keeper_entry)==keeper_entry_bytes,"Area pass reaches prepared keeper menu while retaining prior flight context");
 const auto keeper_prepared_bytes=bytes(keeper_prepared);std::istringstream keeper_prepared_stream(keeper_prepared_bytes);
 check(bytes(load_phase_session(keeper_prepared_stream))==keeper_prepared_bytes,"Prepared area pass keeper snapshot roundtrips");
 rejects([&]{prepare_area_pass_keeper(keeper_prepared);});
 rejects([&]{continue_area_pass_received(keeper_entry);});
 rejects([&]{continue_area_already_arrived(received);});
 rejects([&]{continue_area_already_arrived(area_flight);});
 auto normal_arrival=area_arrived;normal_arrival.phase=FieldSessionPhase::area_ball_arrival_callback;
 check(continue_area_already_arrived(normal_arrival).phase==FieldSessionPhase::area_pass_received,
       "Unopposed normal area arrival reaches the same recipient handoff");
 auto defended_arrival=normal_arrival;defended_arrival.current.field.selected_defenders=1;defended_arrival.scheduling->selected_ids[0]=1;
 const auto defended_before=bytes(defended_arrival);
 const auto defender_entry=continue_area_already_arrived(defended_arrival);
 check(static_cast<unsigned>(defender_entry.phase)==114 && defender_entry.current.field.ball_owner==defended_arrival.current.field.ball_owner &&
       defender_entry.current.field.selected_defenders==1 && defender_entry.scheduling->selected_ids[0]==1 &&
       bytes(defended_arrival)==defended_before,"Defended area arrival stops at the first defender without transferring possession");
 const auto defender_entry_bytes=bytes(defender_entry);std::istringstream defender_entry_stream(defender_entry_bytes);
 check(bytes(load_phase_session(defender_entry_stream))==defender_entry_bytes,"Defended area arrival entry survives a snapshot roundtrip");
 rejects([&]{continue_area_already_arrived(defender_entry);});
 const auto defender_announced=acknowledge_area_pass_defender_announcement(defender_entry);
 check(static_cast<unsigned>(defender_announced.phase)==115 && defender_announced.recovery->ball_variant==defender_entry.recovery->ball_variant &&
       defender_announced.scheduling->selected_ids[0]==1 && defender_announced.current.field.ball_owner==defender_entry.current.field.ball_owner &&
       bytes(defender_entry)==defender_entry_bytes,"Acknowledged area defender announcement preserves the temporary flag and owner");
 const auto pass_announced_bytes=bytes(defender_announced);std::istringstream pass_announced_stream(pass_announced_bytes);
 check(bytes(load_phase_session(pass_announced_stream))==pass_announced_bytes,"Acknowledged defender announcement snapshot roundtrips");
 rejects([&]{acknowledge_area_pass_defender_announcement(defender_announced);});
 const auto prepared_pass_defender=prepare_area_pass_defender(defender_announced,50);
 check(static_cast<unsigned>(prepared_pass_defender.phase)==116 && prepared_pass_defender.recovery->opposing_player==1 &&
       prepared_pass_defender.recovery->ball_variant==0 && prepared_pass_defender.area_pass_contact &&
       prepared_pass_defender.area_pass_contact->saved_ball_variant==defender_announced.recovery->ball_variant &&
       prepared_pass_defender.area_pass_contact->captured_cost==50 && bytes(defender_announced)==pass_announced_bytes,
       "Prepared pass defender preserves the temporarily stacked ball variant");
 const auto prepared_pass_bytes=bytes(prepared_pass_defender);std::istringstream prepared_pass_stream(prepared_pass_bytes);
 check(static_cast<unsigned char>(prepared_pass_bytes[10])==132 && bytes(load_phase_session(prepared_pass_stream))==prepared_pass_bytes,
       "Prepared pass defender survives version66 snapshot roundtrip");
 auto short_pass=prepared_pass_bytes;short_pass.pop_back();rejects([&]{std::istringstream old(short_pass);load_phase_session(old);});
 auto downgraded_pass=prepared_pass_bytes;downgraded_pass[10]=static_cast<char>(131);
 rejects([&]{std::istringstream old(downgraded_pass);load_phase_session(old);});
 rejects([&]{prepare_area_pass_defender(prepared_pass_defender,50);});
 auto pass_compare_input=prepared_pass_defender;pass_compare_input.action->power=1;
 pass_compare_input.current.field.random_first=0;pass_compare_input.current.field.random_second=0;
 const AreaPassComparisonContent pass_comparison{1,64,0,{100,50,25,0}};
 const auto pass_comparison_before=bytes(pass_compare_input);
 const auto compared_pass=compare_area_pass_defender(pass_compare_input,256,0,pass_comparison);
 check(static_cast<unsigned>(compared_pass.phase)==117 && compared_pass.area_contact && compared_pass.area_contact->strength==64 &&
       compared_pass.area_contact->loss==16 && compared_pass.area_contact->score==1 && compared_pass.area_contact->result==3 &&
       compared_pass.action->menu_route==0 &&
       compared_pass.area_pass_contact->saved_ball_variant==pass_compare_input.area_pass_contact->saved_ball_variant &&
       bytes(pass_compare_input)==pass_comparison_before,"Pass defender comparison saves original strength, score and result without applying the outcome");
 const auto compared_pass_bytes=bytes(compared_pass);std::istringstream compared_pass_stream(compared_pass_bytes);
 check(bytes(load_phase_session(compared_pass_stream))==compared_pass_bytes,"Compared pass defender snapshot roundtrips");
 rejects([&]{compare_area_pass_defender(compared_pass,256,0,pass_comparison);});
 rejects([&]{compare_area_pass_defender(pass_compare_input,256,6,pass_comparison);});
 const auto restored_pass=restore_area_pass_ball_variant(compared_pass);
 check(static_cast<unsigned>(restored_pass.phase)==118 && restored_pass.recovery->ball_variant==compared_pass.area_pass_contact->saved_ball_variant &&
       restored_pass.area_contact->result==3 && restored_pass.current.field.ball_owner==compared_pass.current.field.ball_owner &&
       bytes(compared_pass)==compared_pass_bytes,"Pass contact restores the stacked ball variant before handling the result");
 const auto restored_pass_bytes=bytes(restored_pass);std::istringstream restored_pass_stream(restored_pass_bytes);
 check(bytes(load_phase_session(restored_pass_stream))==restored_pass_bytes,"Restored pass contact snapshot roundtrips");
 rejects([&]{restore_area_pass_ball_variant(restored_pass);});
 const auto pass_handler=acknowledge_area_pass_result(restored_pass,64,0);
 check(static_cast<unsigned>(pass_handler.phase)==119 && pass_handler.control->facing==64 && pass_handler.control->motion_requested==0 &&
       pass_handler.area_contact->result==3 && pass_handler.recovery->ball_variant==restored_pass.recovery->ball_variant &&
       bytes(restored_pass)==restored_pass_bytes,"Area pass result acknowledgement preserves the result and restored ball variant");
 const auto pass_handler_bytes=bytes(pass_handler);std::istringstream pass_handler_stream(pass_handler_bytes);
 check(bytes(load_phase_session(pass_handler_stream))==pass_handler_bytes,"Area pass result handler entry snapshot roundtrips");
 rejects([&]{acknowledge_area_pass_result(pass_handler,64,0);});
 rejects([&]{acknowledge_area_pass_result(restored_pass,1,0);});
 auto blocked_pass_input=pass_handler;blocked_pass_input.area_contact->result=2;blocked_pass_input.area_sequence_result=2;
 blocked_pass_input.current.field.roster[1].type=1;blocked_pass_input.current.field.motion[field_motion_index(1)].x=120<<8;
 blocked_pass_input.current.field.motion[field_motion_index(1)].y=120<<8;
 auto block_experience=*dispatch_data.experience;block_experience.reward_factors[2]=64;
 const auto blocked_pass_before=bytes(blocked_pass_input);
 const auto blocked_pass=advance_area_pass_blocked(blocked_pass_input,block_experience);
 check(static_cast<unsigned>(blocked_pass.phase)==124 && blocked_pass.current.field.ball_x==120 && blocked_pass.current.field.ball_y==120 &&
       blocked_pass.current.field.ball_cell==blocked_pass_input.current.field.ball_cell &&
       (blocked_pass.ball_trajectory->x&255)==(blocked_pass_input.ball_trajectory->x&255) &&
       blocked_pass.experience->totals[0]==blocked_pass_input.experience->totals[0]+1 && bytes(blocked_pass_input)==blocked_pass_before,
       "Blocked pass rewards defender and moves ball while retaining the original stale cell");
 const auto blocked_pass_bytes=bytes(blocked_pass);std::istringstream blocked_pass_stream(blocked_pass_bytes);
 check(bytes(load_phase_session(blocked_pass_stream))==blocked_pass_bytes,"Blocked pass snapshot roundtrips");
 rejects([&]{advance_area_pass_blocked(blocked_pass,block_experience);});
 auto terminal3_input=pass_handler;terminal3_input.current.clock.remaining=100;terminal3_input.current.clock.extra_time=0;
 ContactPreludeContent terminal_clock{};terminal_clock.field_elapsed[4]=2;
 const auto terminal3_before=bytes(terminal3_input);
 const auto terminal3=advance_area_pass_terminal(terminal3_input,terminal_clock,block_experience);
 check(static_cast<unsigned>(terminal3.phase)==125 && terminal3.current.clock.remaining==98 && terminal3.current.field.elapsed==2 &&
       terminal3.experience->totals[0]==terminal3_input.experience->totals[0]+1 && bytes(terminal3_input)==terminal3_before,
       "Captured pass result awards the defender and advances terminal time once");
 auto terminal2_input=blocked_pass;terminal2_input.current.clock.remaining=100;terminal2_input.current.clock.extra_time=0;
 const auto terminal2=advance_area_pass_terminal(terminal2_input,terminal_clock,block_experience);
 check(static_cast<unsigned>(terminal2.phase)==125 && terminal2.current.clock.remaining==98 && terminal2.current.field.ball_cell==blocked_pass.current.field.ball_cell &&
       terminal2.experience->totals[0]==terminal2_input.experience->totals[0]+1,"Blocked pass applies the second original defender reward and terminal time");
 const auto terminal2_bytes=bytes(terminal2);std::istringstream terminal2_stream(terminal2_bytes);
 check(bytes(load_phase_session(terminal2_stream))==terminal2_bytes,"Terminal pass clock snapshot roundtrips");
 rejects([&]{advance_area_pass_terminal(terminal2,terminal_clock,block_experience);});
 const auto blocked_recovery=enter_area_pass_blocked_recovery(terminal2);
 check(static_cast<unsigned>(blocked_recovery.phase)==134 && !blocked_recovery.current.field.selected_defenders &&
       blocked_recovery.current.field.elapsed==2 && blocked_recovery.current.field.ball_cell==terminal2.current.field.ball_cell &&
       bytes(terminal2)==terminal2_bytes,"Blocked pass clears the defender queue before recovery without changing its stale cell");
 const auto blocked_recovery_bytes=bytes(blocked_recovery);std::istringstream blocked_recovery_stream(blocked_recovery_bytes);
 check(bytes(load_phase_session(blocked_recovery_stream))==blocked_recovery_bytes,"Blocked pass recovery entry snapshot roundtrips");
 rejects([&]{enter_area_pass_blocked_recovery(blocked_recovery);});
 auto pass_recovery_fixture=blocked_recovery;pass_recovery_fixture.current.field.random_first=0;pass_recovery_fixture.current.field.random_second=0;
 for(auto& actor:pass_recovery_fixture.current.field.motion){actor.x=120<<8;actor.y=120<<8;actor.status=0;}
 const auto pass_recovery_before=bytes(pass_recovery_fixture);
 const auto pass_recovered=advance_session_area_pass_blocked_recovery(pass_recovery_fixture,area6_race_profiles,area6_race_movement);
 check(static_cast<unsigned>(pass_recovered.state.phase)==135 && pass_recovered.recipient && !pass_recovered.state.current.field.elapsed &&
       pass_recovered.state.area_pass_contact && bytes(pass_recovery_fixture)==pass_recovery_before,
       "Blocked pass recovery selects a new owner while preserving pass history");
 const auto pass_recovered_bytes=bytes(pass_recovered.state);std::istringstream pass_recovered_stream(pass_recovered_bytes);
 check(bytes(load_phase_session(pass_recovered_stream))==pass_recovered_bytes,"Blocked pass recovered owner snapshot roundtrips");
 rejects([&]{advance_session_area_pass_blocked_recovery(pass_recovered.state,area6_race_profiles,area6_race_movement);});
 const auto pass_recovery_control=continue_session_area_pass_blocked_recovery(pass_recovered.state,0,0);
 check(static_cast<unsigned>(pass_recovery_control.phase)==137 && pass_recovery_control.current.field.ball_owner==pass_recovered.state.current.field.ball_owner &&
       pass_recovery_control.control->facing==0 && bytes(pass_recovered.state)==pass_recovered_bytes,
       "Ordinary blocked pass recovery reaches control entry after presentation acknowledgement");
 rejects([&]{continue_session_area_pass_blocked_recovery(pass_recovery_control,0,0);});
 const auto pass_recovery_ready=prepare_session_area_pass_blocked_control(pass_recovery_control);
 check(static_cast<unsigned>(pass_recovery_ready.state.phase)==138 && pass_recovery_ready.state.current.field.ball_owner==pass_recovery_control.current.field.ball_owner &&
       pass_recovery_ready.state.control->decision_timer==10 && pass_recovery_ready.state.area_pass_contact,
       "Blocked pass recovered owner reaches prepared player control");
 const auto pass_recovery_ready_bytes=bytes(pass_recovery_ready.state);std::istringstream pass_recovery_ready_stream(pass_recovery_ready_bytes);
 check(bytes(load_phase_session(pass_recovery_ready_stream))==pass_recovery_ready_bytes,"Blocked pass prepared control snapshot roundtrips");
 rejects([&]{prepare_session_area_pass_blocked_control(pass_recovery_ready.state);});
 ControlMovementContent blocked_input_content{};
 const auto blocked_poll=poll_session_area_pass_capture(pass_recovery_ready.state,0,0,area6_race_profiles,area6_race_movement,blocked_input_content);
 check(static_cast<unsigned>(blocked_poll.state.phase)==139 && blocked_poll.state.control->decision_timer==9 && blocked_poll.state.area_pass_contact,
       "Blocked pass recovered owner accepts an idle control poll");
 const auto blocked_menu=poll_session_area_pass_capture(pass_recovery_ready.state,64,64,area6_race_profiles,area6_race_movement,blocked_input_content);
 check(static_cast<unsigned>(blocked_menu.state.phase)==140 && blocked_menu.menu_requested,"Blocked pass recovered owner can request the action menu");
 const auto terminal3_bytes=bytes(terminal3);const auto captured_pass=capture_area_pass_defender(terminal3);
 check(static_cast<unsigned>(captured_pass.state.phase)==128 && captured_pass.state.current.field.ball_owner==1 &&
       captured_pass.state.current.field.side==0 && captured_pass.request_possession_sound &&
       bytes(terminal3)==terminal3_bytes,"Captured pass transfers possession to the defender and requests the side-change sound");
 const auto captured_pass_bytes=bytes(captured_pass.state);std::istringstream captured_pass_stream(captured_pass_bytes);
 check(bytes(load_phase_session(captured_pass_stream))==captured_pass_bytes,"Captured pass snapshot roundtrips");
 rejects([&]{capture_area_pass_defender(captured_pass.state);});
 const auto pass_capture_control=continue_area_pass_capture(captured_pass.state,area6_race_profiles,area6_race_movement,64,0);
 check(static_cast<unsigned>(pass_capture_control.phase)==129 && pass_capture_control.current.field.ball_owner==1 &&
       pass_capture_control.current.field.side==0 && !pass_capture_control.current.field.elapsed &&
       pass_capture_control.control->facing==64 && bytes(captured_pass.state)==captured_pass_bytes,
       "Captured area pass advances field movement before control entry");
 const auto capture_control_bytes=bytes(pass_capture_control);std::istringstream capture_control_stream(capture_control_bytes);
 check(bytes(load_phase_session(capture_control_stream))==capture_control_bytes,"Captured pass control entry snapshot roundtrips");
 rejects([&]{continue_area_pass_capture(pass_capture_control,area6_race_profiles,area6_race_movement,64,0);});
 const auto pass_capture_ready=prepare_session_area_pass_capture_control(pass_capture_control);
 check(static_cast<unsigned>(pass_capture_ready.state.phase)==130 && pass_capture_ready.state.current.field.ball_owner==1 &&
       !pass_capture_ready.state.current.field.selected_defenders && pass_capture_ready.state.control->decision_timer==10 &&
       pass_capture_ready.state.area_pass_contact && bytes(pass_capture_control)==capture_control_bytes,
       "Captured pass prepares player control while retaining prior pass context");
 const auto pass_ready_bytes=bytes(pass_capture_ready.state);std::istringstream pass_ready_stream(pass_ready_bytes);
 check(bytes(load_phase_session(pass_ready_stream))==pass_ready_bytes,"Captured pass prepared control snapshot roundtrips");
 rejects([&]{prepare_session_area_pass_capture_control(pass_capture_ready.state);});
 ControlMovementContent pass_input_content{};
 const auto pass_poll=poll_session_area_pass_capture(pass_capture_ready.state,0,0,area6_race_profiles,area6_race_movement,pass_input_content);
 check(static_cast<unsigned>(pass_poll.state.phase)==131 && pass_poll.state.control->decision_timer==9 &&
       pass_poll.state.area_pass_contact && bytes(pass_capture_ready.state)==pass_ready_bytes,"Captured pass accepts an idle control poll");
 const auto pass_menu_poll=poll_session_area_pass_capture(pass_capture_ready.state,64,64,area6_race_profiles,area6_race_movement,pass_input_content);
 check(static_cast<unsigned>(pass_menu_poll.state.phase)==132 && pass_menu_poll.menu_requested && pass_menu_poll.state.area_pass_contact,
       "Captured pass control input reaches the action menu boundary");
 ActionMenuContent pass_menu_content{};
 const auto reopened_pass_menu=open_area_pass_human_action_menu(pass_menu_poll.state,pass_menu_content);
 check(reopened_pass_menu.state.phase==FieldSessionPhase::human_action_menu && reopened_pass_menu.state.current.field.ball_owner==1 &&
       !reopened_pass_menu.state.area_pass_contact && !reopened_pass_menu.state.ball_trajectory,
       "Opening a new action menu completes the prior pass context");
 rejects([&]{open_area_pass_human_action_menu(reopened_pass_menu.state,pass_menu_content);});
 auto pass_move_input=pass_capture_ready.state;pass_move_input.control->decision_timer=0;
 const auto pass_move_poll=poll_session_area_pass_capture(pass_move_input,1,0,area6_race_profiles,area6_race_movement,pass_input_content);
 check(static_cast<unsigned>(pass_move_poll.state.phase)==133 && pass_move_poll.decision_step && pass_move_poll.moved_player==1 &&
       pass_move_poll.state.area_pass_contact,"Captured pass timer expiry moves to the control-clock boundary");
 GoalkeeperProfiles pass_keepers{};pass_keepers.skills.resize(1);
 WorldStepContent pass_world{};pass_world.stamina_curve.fill(1000);
 const auto pass_control_tick=advance_area_pass_control_tick(pass_move_poll.state,dispatch_data.field_players,pass_keepers,pass_world);
 check(static_cast<unsigned>(pass_control_tick.state.phase)==142 && pass_control_tick.state.current.field.elapsed==1 &&
       pass_control_tick.state.area_pass_contact,"Captured pass control tick reaches the field-round boundary");
 std::array<LogicalButtons,22> pass_world_inputs{};
 const auto pass_field_round=advance_area_pass_field_round(pass_control_tick.state,area6_race_profiles,area6_race_movement,
  pass_input_content,pass_world,pass_world_inputs);
 check(static_cast<unsigned>(pass_field_round.state.phase)==131 && pass_field_round.consumed_polls==22 &&
       pass_field_round.state.area_pass_contact,"Captured pass field round returns to the control poll");
 pass_world_inputs[0]={64,64};
 const auto pass_world_menu=advance_area_pass_field_round(pass_control_tick.state,area6_race_profiles,area6_race_movement,
  pass_input_content,pass_world,pass_world_inputs);
 check(static_cast<unsigned>(pass_world_menu.state.phase)==132 && pass_world_menu.consumed_polls==1,
       "Captured pass field round stops when the action menu is requested");
 auto pass_nonterminal_input=pass_handler;pass_nonterminal_input.area_contact->result=1;pass_nonterminal_input.area_sequence_result=1;
 pass_nonterminal_input.current.clock.remaining=100;pass_nonterminal_input.current.clock.extra_time=0;
 ContactPreludeContent pass_clock{};pass_clock.field_elapsed[5]=1;pass_clock.defender_status[2]=7;
 const auto nonterminal_before=bytes(pass_nonterminal_input);
 const auto pass_nonterminal=advance_area_pass_nonterminal(pass_nonterminal_input,pass_clock,*dispatch_data.experience);
 check(static_cast<unsigned>(pass_nonterminal.phase)==120 && pass_nonterminal.action->power==0 &&
       pass_nonterminal.current.clock.remaining==99 && pass_nonterminal.current.field.elapsed==1 &&
       pass_nonterminal.current.field.motion[field_motion_index(1)].status==7 &&
       pass_nonterminal.experience->totals[0]==pass_nonterminal_input.experience->totals[0]+1 &&
       bytes(pass_nonterminal_input)==nonterminal_before,"Nonterminal pass result applies loss, carrier experience and defender clock once");
 const auto pass_nonterminal_bytes=bytes(pass_nonterminal);std::istringstream pass_nonterminal_stream(pass_nonterminal_bytes);
 check(bytes(load_phase_session(pass_nonterminal_stream))==pass_nonterminal_bytes,"Nonterminal pass result snapshot roundtrips");
 rejects([&]{advance_area_pass_nonterminal(pass_nonterminal,pass_clock,*dispatch_data.experience);});
 auto pass_added=pass_nonterminal_input;pass_added.current.clock.remaining=1;pass_added.current.clock.extra_time=5;
 const auto waiting_pass=advance_area_pass_nonterminal(pass_added,pass_clock,*dispatch_data.experience);
 const auto resumed_pass=resume_area_pass_added_time(waiting_pass);
 check(static_cast<unsigned>(waiting_pass.phase)==121 && resumed_pass.phase==FieldSessionPhase::area_pass_nonterminal_done &&
       resumed_pass.current.clock.remaining==waiting_pass.current.clock.remaining && resumed_pass.experience->totals==waiting_pass.experience->totals,
       "Area pass added-time acknowledgement resumes without reapplying clock or reward");
 rejects([&]{resume_area_pass_added_time(resumed_pass);});
 auto pass_added_script=waiting_pass;pass_added_script.current.clock.script_flags|=128;
 pass_added_script.control->travel_accumulator=42;pass_added_script.control->display_parameter=9;pass_added_script.control->motion_requested=2;
 rejects([&]{resume_area_pass_added_time(pass_added_script);});
 auto two_pass_defenders=pass_nonterminal;two_pass_defenders.current.field.selected_defenders=2;
 two_pass_defenders.scheduling->selected_ids[1]=2;const auto two_pass_before=bytes(two_pass_defenders);
 const auto next_pass_defender=advance_area_pass_defender(two_pass_defenders);
 check(next_pass_defender.phase==FieldSessionPhase::area_pass_defenders_pending && next_pass_defender.area_defender_cursor==1 &&
       next_pass_defender.current.field.elapsed==1 && next_pass_defender.scheduling->selected_ids[1]==2 &&
       !next_pass_defender.area_contact && !next_pass_defender.area_pass_contact && bytes(two_pass_defenders)==two_pass_before,
       "Nonterminal pass result advances to the next queued defender without losing elapsed time");
 const auto second_pass_announced=acknowledge_area_pass_defender_announcement(next_pass_defender);
 const auto second_pass_prepared=prepare_area_pass_defender(second_pass_announced,50);
 const auto second_pass_compared=compare_area_pass_defender(second_pass_prepared,256,0,pass_comparison);
 check(second_pass_prepared.recovery->opposing_player==2 && second_pass_compared.area_contact->cursor==1 &&
       second_pass_compared.current.field.elapsed==1,"A later queued pass defender keeps the cursor and accumulated time");
 const auto second_pass_bytes=bytes(second_pass_compared);std::istringstream second_pass_stream(second_pass_bytes);
 check(bytes(load_phase_session(second_pass_stream))==second_pass_bytes,"Later queued pass defender comparison roundtrips");
 const auto pass_defenders_done=advance_area_pass_defender(pass_nonterminal);
 check(static_cast<unsigned>(pass_defenders_done.phase)==123 && pass_defenders_done.area_defender_cursor==1 &&
       !pass_defenders_done.current.field.selected_defenders && !pass_defenders_done.current.field.elapsed &&
       pass_defenders_done.current.field.ball_owner==pass_nonterminal.current.field.ball_owner,
       "Last nonterminal pass defender clears selection and accumulated time before recipient transfer");
 const auto pass_queue_received=continue_area_already_arrived(pass_defenders_done);
 check(pass_queue_received.phase==FieldSessionPhase::area_pass_received && pass_queue_received.current.field.ball_owner==pass_defenders_done.area_ai_target->recipient &&
       !pass_queue_received.area_defender_cursor,"Exhausted pass queue transfers possession to the recipient");
 rejects([&]{advance_area_pass_defender(pass_defenders_done);});
 auto invalid_pass_comparison=pass_comparison;invalid_pass_comparison.range=0;
 rejects([&]{compare_area_pass_defender(pass_compare_input,256,0,invalid_pass_comparison);});
 check(bytes(pass_compare_input)==pass_comparison_before,"Rejected pass comparison leaves its input unchanged");
 rejects([&]{begin_prepared_area_action_flight(area_flight,ack,movement.geometry,area_velocity,&*dispatch_data.experience);});
 rejects([&]{advance_prepared_session_ball_cell(area_flight);});
 auto bad_area_facing=ack;bad_area_facing.facing=1;
 rejects([&]{begin_prepared_area_action_flight(area_flight_input,bad_area_facing,movement.geometry,area_velocity,&*dispatch_data.experience);});
 auto bad_area_cell=area_flight_input;bad_area_cell.current.field.ball_cell=0;
 rejects([&]{begin_prepared_area_action_flight(bad_area_cell,ack,movement.geometry,area_velocity,&*dispatch_data.experience);});
 check(bytes(area_flight_input)==flight_before,"Rejected area flight preserves source snapshot");
 auto all_skipped_timed=advance_session_area_prefix_clock(skipped,0).state;
 const auto all_skipped_dispatch=advance_session_area_result_dispatch(all_skipped_timed,dispatch_data);
 check(all_skipped_dispatch.route==0 && all_skipped_dispatch.state.area_sequence_result==0 &&
       all_skipped_dispatch.state.phase==FieldSessionPhase::keeper_area_attack_continuation,
       "All skipped defenders retain the original initial result and continue the attack");
 auto unknown_sequence=prefix_saved.state;unknown_sequence.area_sequence_result=255;
 rejects([&]{advance_session_area_result_dispatch(unknown_sequence,dispatch_data);});
 auto legacy_unknown_bytes=bytes(prefix_saved.state);legacy_unknown_bytes[10]=static_cast<char>(129);legacy_unknown_bytes.pop_back();
 std::istringstream legacy_unknown_input(legacy_unknown_bytes);const auto legacy_unknown=load_phase_session(legacy_unknown_input);
 check(legacy_unknown.area_sequence_result==255,"Version63 exhausted prefixes remain readable without inventing the last result");
 rejects([&]{advance_session_area_result_dispatch(legacy_unknown,dispatch_data);});
 auto direct_area=terminal_exit.state;direct_area.area_contact->result=3;direct_area.area_sequence_result=3;direct_area.area_contact->defense_action=2;
 direct_area.area_defense->actions[direct_area.area_contact->cursor]=2;
 direct_area.recovery->opposing_player=direct_area.scheduling->selected_ids[direct_area.area_contact->cursor];
 direct_area.current.field.roster[direct_area.recovery->opposing_player].type=1;
 direct_area.experience=ExperienceState{};dispatch_data.experience=ExperienceContent{};
 dispatch_data.experience->stage_bases[0]=4;dispatch_data.experience->reward_factors[2]=64;
 const auto direct_area_before=bytes(direct_area);
 const auto direct_capture=advance_session_area_result_dispatch(direct_area,dispatch_data);
 check(direct_capture.route==3 && direct_capture.state.phase==FieldSessionPhase::player_control &&
       direct_capture.state.current.field.ball_owner==1 && direct_capture.state.current.field.side==0 &&
       direct_capture.state.current.field.selected_defenders==direct_area.current.field.selected_defenders &&
       direct_capture.reward && direct_capture.reward->award==1 && direct_capture.state.experience->totals[0]==1 &&
       direct_capture.request_possession_sound && bytes(direct_area)==direct_area_before,
       "Area result3/action2 awards once then transfers possession directly");
 rejects([&]{advance_session_area_result_dispatch(direct_capture.state,dispatch_data);});
 auto loose_area=direct_area;loose_area.area_contact->defense_action=4;loose_area.area_defense->actions[loose_area.area_contact->cursor]=4;
 const auto loose_ready=advance_session_area_result_dispatch(loose_area,dispatch_data);
 check(loose_ready.route==2 && loose_ready.state.phase==FieldSessionPhase::keeper_area_recovery_ready &&
       loose_ready.state.current.field.side==loose_area.current.field.side && loose_ready.reward &&
       loose_ready.state.experience->totals[0]==1,"Area result3/action4 awards once and waits for selected recovery");
 auto nonreward_loose=loose_area;nonreward_loose.area_contact->result=2;nonreward_loose.area_sequence_result=2;
 const auto result2_ready=advance_session_area_result_dispatch(nonreward_loose,dispatch_data);
 check(result2_ready.route==2 && !result2_ready.reward && result2_ready.state.experience->totals[0]==0,
       "Area result2 enters recovery without the result3 experience award");
 PlayerProfileMappings area_recovery_profiles{};FieldPhaseContent area_recovery_movement=movement;
 auto recover_area=loose_ready.state;recover_area.current.field.random_first=0;recover_area.current.field.random_second=0;
 recover_area.current.field.ball_x=204;recover_area.current.field.ball_y=100;
 for(auto& motion:recover_area.current.field.motion){motion.x=120<<8;motion.y=120<<8;}
 recover_area.current.field.motion[field_motion_index(2)].x=204<<8;
 recover_area.current.field.motion[field_motion_index(2)].y=100<<8;
 const auto recovered_area=advance_session_area_result_recovery(recover_area,area_recovery_profiles,area_recovery_movement);
 check(recovered_area.state.phase==FieldSessionPhase::keeper_area_recovered_owner &&
       recovered_area.state.current.field.ball_owner==2 && recovered_area.state.current.field.side==0 &&
       recovered_area.state.current.field.selected_defenders==recover_area.current.field.selected_defenders &&
       recovered_area.state.experience->totals[0]==1 && recovered_area.recipient,
       "Area loose recovery chooses possession once and does not repeat experience");
 const auto area_before=bytes(recovered_area.state);
 const auto area_keeper=continue_session_recovery(recovered_area.state);
 check(area_keeper.phase==FieldSessionPhase::keeper_interaction &&
       area_keeper.current.field.selected_defenders==recovered_area.state.current.field.selected_defenders &&
       area_keeper.scheduling->selected_ids==recovered_area.state.scheduling->selected_ids &&
       bytes(recovered_area.state)==area_before,
       "Area recovered-owner keeper gate preserves selected players and input");
 auto area_keeper_bytes=bytes(area_keeper);std::istringstream area_keeper_input(area_keeper_bytes);
 check(bytes(load_phase_session(area_keeper_input))==area_keeper_bytes,"Area keeper gate snapshot roundtrip");
 auto area_keeper_old=area_keeper_bytes;area_keeper_old[10]=static_cast<char>(129);area_keeper_old.pop_back();
 rejects([&]{std::istringstream old(area_keeper_old);load_phase_session(old);});
 auto area_ordinary=recovered_area.state;area_ordinary.current.field.ball_x=100;area_ordinary.current.field.ball_y=120;
 rejects([&]{continue_session_recovery(area_ordinary);});
 const auto ordinary_before=bytes(area_ordinary);
 const auto ordinary_tail=continue_session_area_ordinary_recovery(area_ordinary,64);
 check(ordinary_tail.phase==FieldSessionPhase::player_control && ordinary_tail.control->facing==64 &&
       ordinary_tail.current.field.selected_defenders==area_ordinary.current.field.selected_defenders &&
       ordinary_tail.scheduling->selected_ids==area_ordinary.scheduling->selected_ids &&
       bytes(area_ordinary)==ordinary_before,"Area ordinary prepared tail preserves selection and captures facing");
 const auto ordinary_bytes=bytes(ordinary_tail);std::istringstream ordinary_input(ordinary_bytes);
 check(bytes(load_phase_session(ordinary_input))==ordinary_bytes,"Area ordinary control handoff roundtrip");
 auto story_area=area_ordinary;story_area.recovery->game_mode=4;story_area.current.field.side=0;
 story_area.current.field.roster[story_area.current.field.ball_owner].type=2;
 story_area.current.field.motion[field_motion_index(story_area.current.field.ball_owner)].x=128<<8;
 rejects([&]{continue_session_area_ordinary_recovery(story_area,64);});
 const auto story_before=bytes(story_area);
 const auto story_tail=continue_session_area_story_recovery(story_area,64);
 check(story_tail.phase==FieldSessionPhase::story_transition && story_tail.control->facing==64 &&
       story_tail.current.field.selected_defenders==story_area.current.field.selected_defenders &&
       story_tail.scheduling->selected_ids==story_area.scheduling->selected_ids && bytes(story_area)==story_before,
       "Area story prepared tail preserves selection and captures facing");
 const auto story_bytes=bytes(story_tail);std::istringstream story_input(story_bytes);
 check(bytes(load_phase_session(story_input))==story_bytes,"Area story handoff snapshot roundtrip");
 auto story_old=story_bytes;story_old[10]=static_cast<char>(129);story_old.pop_back();
 rejects([&]{std::istringstream old(story_old);load_phase_session(old);});
 auto story_kickoff=story_area;story_kickoff.kickoff_formation=KickoffFormationContext{{124,132}};
 const auto story_kickoff_tail=continue_session_area_story_recovery(story_kickoff,0);
 check(story_kickoff_tail.kickoff_formation && story_kickoff_tail.control->facing==0,
       "Area story handoff retains kickoff keeper coordinates");
 rejects([&]{continue_session_area_story_recovery(area_ordinary,64);});
 rejects([&]{continue_session_area_story_recovery(recovered_area.state,64);});
 rejects([&]{continue_session_area_story_recovery(story_area,1);});
 rejects([&]{continue_session_area_story_recovery(story_tail,64);});
 rejects([&]{continue_session_area_ordinary_recovery(recovered_area.state,64);});
 rejects([&]{continue_session_area_ordinary_recovery(area_ordinary,1);});
 rejects([&]{continue_session_area_ordinary_recovery(ordinary_tail,64);});
 rejects([&]{continue_session_recovery(area_keeper);});
 const auto recovered_result2=advance_session_area_result_recovery(result2_ready.state,area_recovery_profiles,area_recovery_movement);
 check(recovered_result2.state.phase==FieldSessionPhase::keeper_area_recovered_owner &&
       recovered_result2.state.experience->totals[0]==0,"Area result2 recovers without an experience award");
 auto boundary_area=loose_ready.state;boundary_area.current.field.random_first=0;boundary_area.current.field.random_second=3;
 const auto boundary_recovery=advance_session_area_result_recovery(boundary_area,area_recovery_profiles,area_recovery_movement);
 check(boundary_recovery.state.phase==FieldSessionPhase::boundary_restart &&
       boundary_recovery.state.current.field.selected_defenders==0 && !boundary_recovery.recipient,
       "Area recovery clears its selected prefix when jitter takes the ball out of bounds");
 auto no_area_reward=loose_area;auto missing_reward=dispatch_data;missing_reward.experience.reset();
 const auto no_reward_before=bytes(no_area_reward);
 rejects([&]{advance_session_area_result_dispatch(no_area_reward,missing_reward);});
 check(bytes(no_area_reward)==no_reward_before,"Missing reached area reward content rejects atomically");
 for(const auto& item:{continued.state,all_skipped_dispatch.state,direct_capture.state,loose_ready.state,
                       recovered_area.state,recovered_result2.state}) {
  const auto saved=bytes(item);std::istringstream input(saved);
  check(saved.size()==414 && static_cast<unsigned char>(saved[10])==(item.area_defense && item.phase==FieldSessionPhase::keeper_area_attack_continuation ? 131 : 130) &&
        bytes(load_phase_session(input))==saved,"Final area result and possession snapshots roundtrip in version64");
 }
 auto terminal_dispatch=terminal_exit.state;terminal_dispatch.current.field.elapsed=0;
 check(preview_area_result_dispatch(terminal_dispatch,dispatch_data).route==2,"Terminal result2 routes to recovery");
 terminal_dispatch.area_contact->result=3;terminal_dispatch.area_sequence_result=3;
 check(preview_area_result_dispatch(terminal_dispatch,dispatch_data).route==3,"Terminal result3 routes to original terminal handler");
 prefix_input.current.clock.remaining=1;prefix_input.current.clock.extra_time=5;
 const auto prefix_pause=advance_session_area_prefix_clock(prefix_input,3);
 check(static_cast<unsigned>(prefix_pause.state.phase)==91,"Saved prefix clock retains added time");
 auto prefix_resume=resume_session_area_prefix_clock(prefix_pause.state);
 check(static_cast<unsigned>(prefix_resume.phase)==90 && prefix_resume.current.clock.remaining==0,"Prefix clock resumes without another debit");
 rejects([&]{resume_session_area_prefix_clock(prefix_resume);});
 prefix_input.current.clock.remaining=0;prefix_input.current.clock.extra_time=0;
 const auto prefix_end=advance_session_area_prefix_clock(prefix_input,3);
 check(static_cast<unsigned>(prefix_end.state.phase)==92,"Saved prefix clock retains period end");
 for(const auto& item:{prefix_saved.state,prefix_pause.state,prefix_end.state}) {
  auto raw=bytes(item);std::istringstream current(raw);
  check(bytes(load_phase_session(current))==raw,"Prefix clock snapshot roundtrip");
  raw[10]=static_cast<char>(128);rejects([&]{std::istringstream old(raw);load_phase_session(old);});
 }
 auto bad_cursor=loop.state;bad_cursor.area_defender_cursor=3;rejects([&]{validate_phase_session(bad_cursor);});
 auto old_contact=contact_bytes;old_contact[10]='x';old_contact.resize(398);
 rejects([&]{std::istringstream old(old_contact);load_phase_session(old);});
 check(static_cast<unsigned>(completed.state.phase)==72 && completed.presentation.size()==1 &&
       completed.presentation[0].kind==ControlPresentationKind::clear_overlay,
       "Full-prefix confirmation returns without reading beyond the selected slots");
 auto backed=poll_keeper_area_defense(registered.state,64,defense_content);
 check(backed.state.action->cursor==0 && backed.presentation.size()==1,
       "Back from the full prefix returns to its last defender without a nonexistent icon");
 rejects([&]{poll_keeper_area_defense(registered.state,1,defense_content);});
 auto pending_four=poll_keeper_area_defense(back_result.state,128,defense_content);
 auto next_defender=prepare_keeper_area_defense(pending_four.state,defense_content);
 check(next_defender.state.action->cursor==1 && next_defender.presentation.size()==2 &&
       next_defender.presentation[1].id==42 && next_defender.state.area_defense->blink==0,
       "Registered ordinary choice advances to the next subject prompt");
 for(const auto& output:{registered.state,completed.state,next_defender.state}) {
  auto saved=bytes(output);std::istringstream stream(saved);
  check(bytes(load_phase_session(stream))==saved,"Registered defensive choices roundtrip");
 }
 for(const auto& output:{registered.state,completed.state}) {
  auto saved=bytes(output);saved[10]='o';
  rejects([&]{std::istringstream old(saved);load_phase_session(old);});
 }
 auto old_pending=bytes(plain_confirm);old_pending[10]='o';old_pending.resize(383);std::istringstream old_pending_stream(old_pending);
 check(bytes(load_phase_session(old_pending_stream))==bytes(plain_confirm),"Legacy o pending preparation stays readable");
 auto special_pending=defense_confirm.state;special_pending.area_defense->actions[0]=2;
 const auto special_before=bytes(special_pending);
 rejects([&]{prepare_keeper_area_defense(special_pending,defense_content);});
 check(bytes(special_pending)==special_before,"Unsupported special preparation leaves its input unchanged");
 auto mixed_final=defense_confirm.state;mixed_final.area_defense->pressed=129;
 const auto mixed_before=bytes(mixed_final);
 rejects([&]{prepare_keeper_area_defense(mixed_final,defense_content);});
 check(bytes(mixed_final)==mixed_before,"Unrepresented final direction leaves pending state unchanged");
 rejects([&]{prepare_keeper_area_defense(plain_confirm,missing_icons);});
 auto incomplete_full=registered.state;incomplete_full.area_defense->actions[0]=255;
 rejects([&]{validate_phase_session(incomplete_full);});
 auto early_complete=next_defender.state;early_complete.phase=FieldSessionPhase::keeper_area_defense_complete;
 rejects([&]{validate_phase_session(early_complete);});
 rejects([&]{prepare_keeper_area_defense(registered.state,defense_content);});
 rejects([&]{poll_keeper_area_defense(completed.state,128,defense_content);});
 auto legacy_partial=bytes(next_defender.state);legacy_partial[10]='o';legacy_partial.resize(383);std::istringstream legacy_partial_stream(legacy_partial);
 check(bytes(load_phase_session(legacy_partial_stream))==bytes(next_defender.state),"Legacy o partial70 remains readable");
 auto mixed_partial=pending_four.state;mixed_partial.area_defense->pressed=129;
 check(prepare_keeper_area_defense(mixed_partial,defense_content).state.action->cursor==1,
       "A with direction registers a nonfinal slot before its next prompt");
 auto mixed_back=plain_confirm;mixed_back.area_defense->pressed=193;
 check(prepare_keeper_area_defense(mixed_back,defense_content).state.action->cursor==0,
       "A plus B takes precedence over direction after final registration");
 check(!keeper_area_defense_special_option(plain_confirm,defense_content),"Ordinary action bypasses special data");
 auto option_pending=plain_confirm;option_pending.area_defense->actions[0]=2;
 option_pending.current.field.roster[1].type=10;
 defense_content.special_options=AreaDefenseSpecialOptions{};
 defense_content.special_options->field_action2.resize(35);
 check(!keeper_area_defense_special_option(option_pending,defense_content),"Missing rule means no special option");
 check(prepare_keeper_area_defense(option_pending,defense_content).state.action->cursor==1,
       "Action2 with no special option registers ordinary subtype0");
 defense_content.special_options->field_action2[10]=AreaDefenseSpecialOption{7,std::uint8_t{20}};
 for(unsigned i=1;i<=10;++i)option_pending.current.field.roster[i].type=10;
 check(!keeper_area_defense_special_option(option_pending,defense_content),"Required teammate absent suppresses option");
 option_pending.current.field.roster[10].type=20;
 check(keeper_area_defense_special_option(option_pending,defense_content)==7,"Required teammate in slot10 enables option");
 rejects([&]{prepare_keeper_area_defense(option_pending,defense_content);});
 defense_content.special_cursor=std::array<std::uint8_t,2>{22,33};
 auto special_open=open_keeper_area_special(option_pending,defense_content);
 check(static_cast<unsigned>(special_open.state.phase)==73 && special_open.state.area_defense->pressed==0 &&
       special_open.presentation.size()==1 && special_open.presentation[0].id==12 &&
       special_open.special_cursor && special_open.special_cursor->tile==22 &&
       special_open.special_cursor->x==33 && special_open.special_cursor->attribute==3,
       "Special menu opens with display and captured cursor metadata");
 auto special_encoded=bytes(special_open.state);std::istringstream special_stream(special_encoded);
 auto legacy_q=special_encoded;legacy_q[10]='q';legacy_q.resize(385);std::istringstream legacy_q_stream(legacy_q);
 check(bytes(load_phase_session(legacy_q_stream))==special_encoded,"Legacy q chooser opening remains readable");
 defense_content.special_cursor_y_base=100;
 auto special_down=poll_keeper_area_special(special_open.state,4,defense_content);
 check(static_cast<unsigned>(special_down.state.phase)==74 && special_down.state.area_special_cursor==1 &&
       special_down.special_cursor_y && *special_down.special_cursor_y==100,"Chooser direction updates cursor after displaying its old position");
 auto special_pick=poll_keeper_area_special(special_down.state,128,defense_content);
 check(static_cast<unsigned>(special_pick.state.phase)==75 && special_pick.state.area_special_cursor==1 &&
       special_pick.state.area_special_pressed==128 && special_pick.special_cursor_y && *special_pick.special_cursor_y==248,
       "Chooser confirmation preserves input and selected special option");
 auto special_cancel=poll_keeper_area_special(special_open.state,64,defense_content);
 check(special_cancel.state.area_special_pressed==64 && static_cast<unsigned>(special_cancel.state.phase)==75,
       "Chooser cancellation preserves the B sample");
 auto cancel_inspected=inspect_keeper_area_special(special_cancel.state,FieldPlayerProfiles{},FieldDefenseContent{});
 check(static_cast<unsigned>(cancel_inspected.state.phase)==76 &&
       cancel_inspected.state.area_special_pressed==64 && cancel_inspected.presentation.empty(),
       "Cancellation reaches the pre-redraw boundary without loading a move");
 check(cancel_inspected.state.area_special_decision && cancel_inspected.state.area_special_decision->status==2 &&
       cancel_inspected.state.area_special_decision->cost==0,"Cancellation stores canonical empty inspection data");
 auto canceled_redraw=redraw_keeper_area_special(cancel_inspected.state,defense_content);
 check(static_cast<unsigned>(canceled_redraw.state.phase)==77 && canceled_redraw.state.action->cursor==0 &&
       canceled_redraw.state.area_special_pressed==64 && canceled_redraw.presentation.size()==4,
       "Canceled special redraw keeps cursor and sample before button tail");
 FieldPlayerProfiles special_profiles{};special_profiles.skills.resize(1);
 FieldDefenseContent special_moves{};special_moves.skill_selectors[2][1]=1;
 special_moves.moves[2].push_back({3,0,50,0});special_moves.moves[2].push_back({4,0,180,0});
 auto cost_input=special_pick.state;cost_input.area_special_option=1;cost_input.recovery->ball_variant=1;
 cost_input.current.field.roster[1].type=1;cost_input.current.field.roster[1].level=0;
 cost_input.current.field.roster[1].stamina=179;
 auto denied=inspect_keeper_area_special(cost_input,special_profiles,special_moves);
 check(denied.state.area_special_decision && denied.state.area_special_decision->status==1 &&
       denied.state.area_special_decision->subtype==1 && denied.state.area_special_decision->cost==180 &&
       denied.state.area_special_decision->effect==4 && denied.presentation.size()==1 &&
       denied.presentation[0].kind==ControlPresentationKind::script && denied.presentation[0].id==61 &&
       denied.state.current.field.roster[1].stamina==179,"Unavailable stamina requests original warning without debit");
 auto enough=cost_input;enough.current.field.roster[1].stamina=180;
 auto accepted=inspect_keeper_area_special(enough,special_profiles,special_moves);
 check(accepted.state.area_special_decision->status==0 && accepted.presentation.empty() &&
       accepted.state.current.field.roster[1].stamina==180,"Affordable special reaches registration boundary without debit");
 const auto prior_accept=bytes(accepted.state);
 rejects([&]{redraw_keeper_area_special(accepted.state,defense_content);});
 check(bytes(accepted.state)==prior_accept,"Missing special icon leaves decision unchanged");
 defense_content.action2_special_icon=77;
 auto selected_redraw=redraw_keeper_area_special(accepted.state,defense_content);
 check(static_cast<unsigned>(selected_redraw.state.phase)==77 && selected_redraw.state.action->cursor==1 &&
       selected_redraw.state.area_defense->subtypes[0]==1 && selected_redraw.presentation.size()==3 &&
       selected_redraw.presentation[2].id==77,"Accepted last special stores subtype and its icon");
 auto full_tail=continue_keeper_area_special(selected_redraw.state,defense_content);
 check(static_cast<unsigned>(full_tail.state.phase)==70 && full_tail.state.action->cursor==1 &&
       full_tail.state.area_defense->subtypes[0]==1 && !full_tail.state.area_special_option &&
       !full_tail.state.area_special_decision,"Full accepted tail retains special subtype and clears completed context");
 auto cancel_tail=continue_keeper_area_special(canceled_redraw.state,defense_content);
 check(static_cast<unsigned>(cancel_tail.state.phase)==70 && cancel_tail.state.action->cursor==0,
       "Canceled first slot returns to defensive input wait");
 auto full_back=selected_redraw.state;full_back.area_special_pressed=192;
 auto no_icons=defense_content;no_icons.field_icons.reset();no_icons.action2_special_icon.reset();
 auto full_back_result=continue_keeper_area_special(full_back,no_icons);
 check(full_back_result.state.action->cursor==0 && full_back_result.state.area_defense->subtypes[0]==1 &&
       full_back_result.presentation.size()==1,"Full A+B tail uses B without requesting an icon");
 auto full_direction=selected_redraw.state;full_direction.area_special_pressed=129;
 const auto before_direction=bytes(full_direction);
 rejects([&]{continue_keeper_area_special(full_direction,defense_content);});
 check(bytes(full_direction)==before_direction,"Unsupported full-prefix direction rejects atomically");
 auto nonfinal_accepted=accepted.state;nonfinal_accepted.current.field.selected_defenders=2;
 nonfinal_accepted.scheduling->selected_ids[1]=2;nonfinal_accepted.area_special_pressed=193;
 auto nonfinal_redraw=redraw_keeper_area_special(nonfinal_accepted,defense_content);
 auto nonfinal_tail=continue_keeper_area_special(nonfinal_redraw.state,defense_content);
 check(nonfinal_tail.state.action->cursor==1 && nonfinal_tail.state.area_defense->subtypes[0]==1 &&
       nonfinal_tail.state.area_defense->actions[1]==255 && nonfinal_tail.presentation.size()==1 &&
       nonfinal_tail.presentation[0].id==defense_content.titles[2],"Nonfinal accepted path ignores B/direction");
 auto denied_buttons=denied.state;denied_buttons.area_special_pressed=193;
 auto denied_redraw=redraw_keeper_area_special(denied_buttons,defense_content);
 auto denied_tail=continue_keeper_area_special(denied_redraw.state,defense_content);
 check(denied_tail.state.action->cursor==0 && denied_tail.state.area_defense->actions[0]==2 &&
       denied_tail.presentation.size()==1,"Denied nonfinal path ignores B/direction");
 auto previous_cancel=canceled_redraw.state;previous_cancel.current.field.selected_defenders=2;
 previous_cancel.scheduling->selected_ids[1]=2;previous_cancel.action->cursor=1;
 previous_cancel.recovery->opposing_player=2;previous_cancel.area_defense->actions[0]=2;
 previous_cancel.area_defense->subtypes[0]=1;previous_cancel.area_defense->actions[1]=2;
 auto previous_tail=continue_keeper_area_special(previous_cancel,defense_content);
 check(previous_tail.state.action->cursor==0 && previous_tail.state.area_defense->subtypes[0]==1 &&
       previous_tail.presentation.size()==2,"Canceled B preserves earlier special subtype");
 const auto encoded_redraw=bytes(selected_redraw.state);std::istringstream redraw_stream(encoded_redraw);
 check(bytes(load_phase_session(redraw_stream))==encoded_redraw,"Special redraw snapshot roundtrip");
 auto invalid_old_redraw=encoded_redraw;invalid_old_redraw[10]='s';
 rejects([&]{std::istringstream old(invalid_old_redraw);load_phase_session(old);});
 auto resumed=selected_redraw.state;resumed.phase=FieldSessionPhase::keeper_area_defense_poll;
 resumed.area_special_option.reset();resumed.area_special_decision.reset();resumed.area_special_cursor=0;resumed.area_special_pressed=0;
 resumed.action->cursor=0;resumed.area_defense->blink=0;resumed.action->flash_counter=0;
 defense_content.directions[2][0]=2;defense_content.directions[2][1]=4;
 auto same=poll_keeper_area_defense(resumed,1,defense_content);
 check(same.state.area_defense->subtypes[0]==1 && same.presentation[0].id==77,
       "Same action preserves and blinks the special icon");
 auto changed=poll_keeper_area_defense(resumed,2,defense_content);
 check(changed.state.area_defense->subtypes[0]==0 && changed.state.area_defense->actions[0]==4,
       "Changing action clears the old special subtype");
 auto old_subtype_snapshot=bytes(resumed);old_subtype_snapshot[10]='s';
 rejects([&]{std::istringstream old(old_subtype_snapshot);load_phase_session(old);});
 auto unsupported=accepted.state;unsupported.area_special_option=127;unsupported.area_special_decision->subtype=127;
 rejects([&]{redraw_keeper_area_special(unsupported,defense_content);});
 auto normal=cost_input;normal.area_special_cursor=0;normal.current.field.roster[1].stamina=0;
 auto normal_result=inspect_keeper_area_special(normal,special_profiles,special_moves);
 check(normal_result.state.area_special_decision->status==0 && normal_result.state.area_special_decision->subtype==0 &&
       normal_result.state.area_special_decision->cost==50,"Ordinary subtype is accepted even with insufficient stamina");
 auto inconsistent_decision=denied.state;inconsistent_decision.area_special_decision->status=0;
 rejects([&]{validate_phase_session(inconsistent_decision);});
 auto oversized_cost=accepted.state;oversized_cost.area_special_decision->cost=1024;
 rejects([&]{validate_phase_session(oversized_cost);});
 auto invalid_id=accepted.state;invalid_id.recovery->opposing_player=255;invalid_id.scheduling->selected_ids[0]=255;
 rejects([&]{validate_phase_session(invalid_id);});
 for(const auto& output:{cancel_inspected.state,denied.state,accepted.state,normal_result.state}) {
  auto saved=bytes(output);std::istringstream stream(saved);
  check(bytes(load_phase_session(stream))==saved,"Special inspection snapshot roundtrip");
  saved[10]='r';saved.resize(387);
  rejects([&]{std::istringstream old(saved);load_phase_session(old);});
  rejects([&]{inspect_keeper_area_special(output,special_profiles,special_moves);});
 }
 auto missing_moves=cost_input;
 const auto before_inspection=bytes(missing_moves);
 rejects([&]{inspect_keeper_area_special(missing_moves,special_profiles,FieldDefenseContent{});});
 check(bytes(missing_moves)==before_inspection,"Missing move content leaves pending selection unchanged");
 auto special_both=poll_keeper_area_special(special_open.state,196,defense_content);
 check(special_both.state.area_special_cursor==1 && special_both.state.area_special_pressed==196,
       "Chooser moves before A and A takes priority over B");
 auto special_up_down=poll_keeper_area_special(special_down.state,12,defense_content);
 check(special_up_down.state.area_special_cursor==0,"Chooser prioritizes up when both directions are pressed");
 for(const auto& output:{special_down.state,special_pick.state,special_cancel.state}) {
  auto saved=bytes(output);std::istringstream stream(saved);
  check(bytes(load_phase_session(stream))==saved,"Chooser sampled input roundtrip");
  saved[10]='q';saved.resize(385);
  rejects([&]{std::istringstream old(saved);load_phase_session(old);});
 }
 auto invalid_special_cursor=special_down.state;invalid_special_cursor.area_special_cursor=2;
 rejects([&]{validate_phase_session(invalid_special_cursor);});
 auto missing_exit=special_pick.state;missing_exit.area_special_pressed=0;
 rejects([&]{validate_phase_session(missing_exit);});
 rejects([&]{poll_keeper_area_special(special_pick.state,0,defense_content);});
 auto missing_y=defense_content;missing_y.special_cursor_y_base.reset();
 rejects([&]{poll_keeper_area_special(special_open.state,0,missing_y);});
 check(bytes(load_phase_session(special_stream))==special_encoded,"Special chooser roundtrip");
 check(special_open.state.area_special_option && *special_open.state.area_special_option==7 &&
       special_open.state.current.field.roster[1].stamina==option_pending.current.field.roster[1].stamina,
       "Chooser saves option and leaves stamina unchanged");
 auto missing_special=special_open.state;missing_special.area_special_option.reset();
 rejects([&]{validate_phase_session(missing_special);});
 auto malformed_special=special_encoded;malformed_special[383]=2;
 rejects([&]{std::istringstream bad(malformed_special);load_phase_session(bad);});
 auto absent_special=special_encoded;absent_special[383]=0;absent_special[384]=7;
 rejects([&]{std::istringstream bad(absent_special);load_phase_session(bad);});
 auto truncated_special=special_encoded;truncated_special.pop_back();
 rejects([&]{std::istringstream bad(truncated_special);load_phase_session(bad);});
 auto old_special=special_encoded;old_special[10]='p';old_special.resize(383);
 rejects([&]{std::istringstream old(old_special);load_phase_session(old);});
 auto no_cursor=defense_content;no_cursor.special_cursor.reset();
 rejects([&]{open_keeper_area_special(option_pending,no_cursor);});
 rejects([&]{open_keeper_area_special(special_open.state,defense_content);});
 defense_content.special_options->field_action2[10]=AreaDefenseSpecialOption{3,std::nullopt};
 check(keeper_area_defense_special_option(option_pending,defense_content)==3,"Unconditional option needs no teammate");
 defense_content.special_options->field_action2[10]=AreaDefenseSpecialOption{5,std::uint8_t{10}};
 check(keeper_area_defense_special_option(option_pending,defense_content)==5,"Original teammate scan includes the selected actor");
 for(const auto required:{std::uint8_t{0},std::uint8_t{255}}) {
  defense_content.special_options->field_action2[10]=AreaDefenseSpecialOption{0,required};
  option_pending.current.field.roster[10].type=required;
  check(keeper_area_defense_special_option(option_pending,defense_content)==0,
        "Raw teammate type0/255 and subtype0 are distinct from no condition or option");
  auto zero_option=open_keeper_area_special(option_pending,defense_content);
  check(zero_option.state.area_special_option && *zero_option.state.area_special_option==0,
        "Chooser preserves option zero distinctly from an absent option");
  auto zero_bytes=bytes(zero_option.state);std::istringstream zero_stream(zero_bytes);
  check(bytes(load_phase_session(zero_stream))==zero_bytes,"Chooser option zero survives a snapshot roundtrip");
  rejects([&]{prepare_keeper_area_defense(option_pending,defense_content);});
 }
 auto missing_options=defense_content;missing_options.special_options.reset();
 rejects([&]{keeper_area_defense_special_option(option_pending,missing_options);});
 option_pending.current.field.roster[1].type=35;
 rejects([&]{keeper_area_defense_special_option(option_pending,defense_content);});
 for(unsigned action=0;action<2;++action) {
  auto bypass=plain_confirm;bypass.area_defense->actions[0]=action;
  check(!keeper_area_defense_special_option(bypass,missing_options) &&
        prepare_keeper_area_defense(bypass,missing_options).state.action->cursor==1,
        "Nonzero area variant bypasses special options for field actions0/1");
 }
 for(const auto& output:{empty_defense,initialized_defense}) {
  auto saved=bytes(output);std::istringstream stream(saved);
  check(bytes(load_phase_session(stream))==saved,"Defense entry roundtrip");
  saved[10]='l';saved.resize(372);
  rejects([&]{std::istringstream old(saved);load_phase_session(old);});
  rejects([&]{begin_keeper_area_defense(output,0);});
 }
 auto old_defense_entry=bytes(defense_menu);old_defense_entry[10]='l';old_defense_entry.resize(372);
 std::istringstream old_defense_stream(old_defense_entry);
 check(bytes(load_phase_session(old_defense_stream))==bytes(defense_menu),"Legacy l defensive entry loads");
 auto initialized_bytes=bytes(initialized_defense);
 for(unsigned size=372;size<414;++size) {
  auto truncated=initialized_bytes;truncated.resize(size);
  rejects([&]{std::istringstream stream(truncated);load_phase_session(stream);});
 }
 for(auto offset:{372,373,377}) {
  auto invalid=initialized_bytes;invalid[offset]=offset==372 ? 2 : 1;
  rejects([&]{std::istringstream stream(invalid);load_phase_session(stream);});
 }
 auto missing_defense=initialized_defense;missing_defense.area_defense.reset();rejects([&]{validate_phase_session(missing_defense);});
 for(const auto& output:{own_menu,defense_menu}) {
  auto saved=bytes(output);std::istringstream input(saved);
  check(bytes(load_phase_session(input))==saved,"Prepared area menu roundtrip");
  saved[10]='j';saved.resize(372);rejects([&]{std::istringstream old(saved);load_phase_session(old);});
  rejects([&]{open_prepared_keeper_area_menu(output,0,ActionMenuContent{});});
 }
 auto prior_setup=bytes(own_setup);prior_setup[10]='j';prior_setup.resize(372);std::istringstream prior_setup_stream(prior_setup);
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

#include "captain/ball_flight_session.hpp"
#include "captain/keeper_session.hpp"
#include "captain/recovery_session.hpp"
#include "captain/phase_session_io.hpp"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
namespace {void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid flight session accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}}
int main(){try{
 using namespace captain;PhaseSessionState s{};s.phase=FieldSessionPhase::shot_flight_pending;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.shot_progress=ShotContactProgress{1,0,12,0,0};
 s.shot_flight=ShotFlightContext{18,221,255,true};s.current.field.ball_owner=2;
 s.current.field.ball_x=120;s.current.field.ball_y=100;s.current.field.ball_cell=110;
 s.current.field.selected_defenders=1;s.current.field.elapsed=1;s.scheduling->selected_ids={12,0,0,0,0};
 s.recovery->opposing_player=12;s.current.clock.remaining=448;
 MovementGeometryContent g{};g.direction_thresholds.fill(65535);BallVelocityContent v{};v.quarter_wave[63]=256;
 auto retained=s;retained.saved_ball_flight_flag=1;
 auto retained_flight=begin_prepared_session_ball_flight(retained,64,192,g,v);
 check(retained_flight.ball_trajectory->flags==1,"New shot retains the earlier original flight flag");
 auto retained_bytes=bytes(retained_flight);std::istringstream retained_stream(retained_bytes);
 check(load_phase_session(retained_stream).ball_trajectory->flags==1,"Retained flag survives saved shot flight");
 retained.shot_flight->destination=122;
 auto retained_arrival=advance_prepared_session_ball_cell(begin_prepared_session_ball_flight(retained,64,192,g,v)).state;
 const auto retained_arrival_bytes=bytes(retained_arrival);
 check(continue_session_unopposed_arrival(retained_arrival).phase==FieldSessionPhase::shot_loose_recovery &&
       bytes(retained_arrival)==retained_arrival_bytes,
       "Retained-flag empty loose arrival reaches recovery without changing its input");
 auto retained_keeper=retained_arrival;retained_keeper.shot_flight->loose_ball=false;
 check(continue_session_unopposed_arrival(retained_keeper).phase==FieldSessionPhase::shot_keeper_entry,
       "Retained-flag empty keeper arrival reaches the saved keeper entry");
 auto kickoff_contested=retained_arrival;kickoff_contested.saved_ball_flight_flag=0;kickoff_contested.ball_trajectory->flags=0;
 kickoff_contested.shot_flight->destination=209;kickoff_contested.current.field.ball_x=188;kickoff_contested.current.field.ball_y=124;
 kickoff_contested.current.field.ball_cell=209;kickoff_contested.ball_trajectory->x=188u<<8;kickoff_contested.ball_trajectory->y=124u<<8;
 kickoff_contested.current.field.ball_owner=7;kickoff_contested.current.field.random_first=0;kickoff_contested.current.field.random_second=128;
 kickoff_contested.current.field.selected_defenders=2;kickoff_contested.scheduling->selected_ids={19,14,0,0,0};
 kickoff_contested.score=MatchScoreContext{0,1,4};kickoff_contested.kickoff_formation=KickoffFormationContext{{124,132}};
 const auto kickoff_contested_before=bytes(kickoff_contested);
 const auto kickoff_defenders=continue_session_unopposed_arrival(kickoff_contested);
 check(kickoff_defenders.phase==FieldSessionPhase::shot_pass_defenders_pending && kickoff_defenders.area_defender_cursor==0 &&
       !kickoff_defenders.saved_ball_flight_flag && kickoff_defenders.scheduling->selected_ids[0]==19 && bytes(kickoff_contested)==kickoff_contested_before,
       "Sampled flag-zero loose shot saves the queued defenders");
 const auto kickoff_announcement=acknowledge_area_pass_defender_announcement(kickoff_defenders);
 check(kickoff_announcement.phase==FieldSessionPhase::shot_pass_defender_announced && kickoff_announcement.current.field.selected_defenders==2,
       "Sampled flag-zero loose shot acknowledges the first defender");
 const auto kickoff_announcement_bytes=bytes(kickoff_announcement);std::istringstream kickoff_announcement_stream(kickoff_announcement_bytes);
 check(bytes(load_phase_session(kickoff_announcement_stream))==kickoff_announcement_bytes,"Flag-zero defender announcement snapshot roundtrips");
 const auto loose_prepared=prepare_session_sampled_loose_shot_defender(kickoff_announcement);
 check(static_cast<unsigned>(loose_prepared.phase)==188 && loose_prepared.recovery->opposing_player==19 &&
       loose_prepared.recovery->ball_variant==0 && loose_prepared.scheduling->selected_ids[1]==14 &&
       bytes(kickoff_announcement)==kickoff_announcement_bytes,
       "Flag-zero shot prepares the original first defender before comparison");
 const auto loose_prepared_bytes=bytes(loose_prepared);std::istringstream loose_prepared_stream(loose_prepared_bytes);
 check(bytes(load_phase_session(loose_prepared_stream))==loose_prepared_bytes,"Prepared flag-zero defender snapshot roundtrips");
 auto loose_comparison_input=loose_prepared;loose_comparison_input.action->power=11;loose_comparison_input.action->menu_route=0;
 FlightContactContent loose_content{};loose_content.defense_actions={0,2};loose_content.eligibility[0]=128;
 loose_content.variation[0]={192,64};loose_content.thresholds[0]={154,96,48,0};
 const auto loose_comparison_before=bytes(loose_comparison_input);
 const auto loose_compared=compare_session_sampled_loose_shot_defender(loose_comparison_input,19,loose_content);
 check(static_cast<unsigned>(loose_compared.phase)==189 && loose_compared.area_contact && loose_compared.area_contact->defender_power==19 &&
       loose_compared.area_contact->defense_action==0 && loose_compared.area_contact->strength==9 && loose_compared.area_contact->loss==2 &&
       loose_compared.area_contact->score==78 && loose_compared.area_contact->result==2 && loose_compared.area_sequence_result==2 &&
       bytes(loose_comparison_input)==loose_comparison_before,
       "Sampled first loose-shot defender comparison matches the original result2 arithmetic");
 const auto loose_compared_bytes=bytes(loose_compared);std::istringstream loose_compared_stream(loose_compared_bytes);
 check(bytes(load_phase_session(loose_compared_stream))==loose_compared_bytes,"Compared loose-shot defender snapshot roundtrips");
 auto loose_result_input=loose_compared;loose_result_input.current.clock.remaining=445;loose_result_input.current.clock.extra_time=0;
 loose_result_input.current.field.roster[19].type=1;loose_result_input.current.field.motion[field_motion_index(19)].x=140u<<8;
 loose_result_input.current.field.motion[field_motion_index(19)].y=140u<<8;loose_result_input.experience->totals[0]=21;
 ContactPreludeContent loose_result_clock{};loose_result_clock.field_elapsed[0]=2;
 ExperienceContent loose_result_experience{};loose_result_experience.stage_bases[0]=4;loose_result_experience.reward_factors[2]=64;
 const auto loose_result_before=bytes(loose_result_input);
 const auto loose_recovery=recover_session_sampled_loose_shot_result2(loose_result_input,loose_result_clock,loose_result_experience);
 check(loose_recovery.phase==FieldSessionPhase::ball_recovery && loose_recovery.current.field.ball_x==140 && loose_recovery.current.field.ball_y==140 &&
       loose_recovery.current.field.ball_cell==209 && loose_recovery.current.field.elapsed==2 && loose_recovery.current.clock.remaining==443 &&
       loose_recovery.current.field.selected_defenders==0 && loose_recovery.experience->totals[0]==23 &&
       loose_recovery.control->facing==64 && loose_recovery.control->motion_requested==0 &&
       !loose_recovery.shot_flight && !loose_recovery.ball_trajectory && !loose_recovery.area_contact && bytes(loose_result_input)==loose_result_before,
       "Sampled loose-shot result2 awards both rewards, advances the clock and enters recovery");
 const auto loose_recovery_bytes=bytes(loose_recovery);std::istringstream loose_recovery_stream(loose_recovery_bytes);
 check(bytes(load_phase_session(loose_recovery_stream))==loose_recovery_bytes,"Sampled loose-shot recovery snapshot roundtrips");
 auto loose_owner=loose_recovery;loose_owner.phase=FieldSessionPhase::recovered_owner;loose_owner.current.field.ball_owner=8;
 loose_owner.current.field.elapsed=0;loose_owner.recovery->x_jitter_control=128;loose_owner.control->facing=64;
 const auto loose_owner_before=bytes(loose_owner);const auto loose_control=continue_session_recovery(loose_owner);
 check(loose_control.phase==FieldSessionPhase::player_control && loose_control.control->facing==0 && bytes(loose_owner)==loose_owner_before,
       "Sampled loose-shot recovery acknowledges the original facing reset before control");
 rejects([&]{recover_session_sampled_loose_shot_result2(loose_recovery,loose_result_clock,loose_result_experience);});
 rejects([&]{compare_session_sampled_loose_shot_defender(loose_compared,19,loose_content);});
 rejects([&]{compare_session_sampled_loose_shot_defender(loose_comparison_input,20,loose_content);});
 auto unknown_loose_content=loose_content;unknown_loose_content.variation[0][0]=191;
 rejects([&]{compare_session_sampled_loose_shot_defender(loose_comparison_input,19,unknown_loose_content);});
 rejects([&]{prepare_session_sampled_loose_shot_defender(loose_prepared);});
 rejects([&]{prepare_area_pass_defender(kickoff_announcement,0);});
 auto unobserved_loose=kickoff_contested;unobserved_loose.scheduling->selected_ids[1]=15;
 rejects([&]{continue_session_unopposed_arrival(unobserved_loose);});
 auto defended_retained=retained_arrival;defended_retained.current.field.selected_defenders=5;
 defended_retained.scheduling->selected_ids={13,14,15,16,17};
 const auto defended_before=bytes(defended_retained);
 const auto defended_entry=continue_session_unopposed_arrival(defended_retained);
 check(static_cast<unsigned>(defended_entry.phase)==150 && defended_entry.area_defender_cursor==0 &&
       defended_entry.current.field.selected_defenders==5 && defended_entry.scheduling->selected_ids[4]==17 &&
       bytes(defended_retained)==defended_before,"Retained-flag defended arrival saves all five queued defenders");
 const auto defended_bytes=bytes(defended_entry);std::istringstream defended_stream(defended_bytes);
 check(bytes(load_phase_session(defended_stream))==defended_bytes,"Retained-flag defended arrival snapshot roundtrips");
 const auto announced_shot_pass=acknowledge_area_pass_defender_announcement(defended_entry);
 check(static_cast<unsigned>(announced_shot_pass.phase)==151 && announced_shot_pass.current.field.selected_defenders==5 &&
       announced_shot_pass.area_defender_cursor==0 && bytes(defended_entry)==defended_bytes,
       "Shot pass defender announcement preserves the five-player prefix");
 const auto announced_bytes=bytes(announced_shot_pass);std::istringstream announced_stream(announced_bytes);
 check(bytes(load_phase_session(announced_stream))==announced_bytes,"Shot pass announcement snapshot roundtrips");
 rejects([&]{acknowledge_area_pass_defender_announcement(announced_shot_pass);});
 const auto prepared_shot_pass=prepare_area_pass_defender(announced_shot_pass,50);
 check(static_cast<unsigned>(prepared_shot_pass.phase)==152 && prepared_shot_pass.recovery->opposing_player==13 &&
       prepared_shot_pass.area_pass_contact && prepared_shot_pass.area_pass_contact->saved_ball_variant==0 &&
       prepared_shot_pass.area_pass_contact->captured_cost==50 && bytes(announced_shot_pass)==announced_bytes,
       "Shot pass preparation retains the zero stacked ball variant and selected defender");
 const auto prepared_bytes=bytes(prepared_shot_pass);std::istringstream prepared_stream(prepared_bytes);
 check(bytes(load_phase_session(prepared_stream))==prepared_bytes,"Shot pass preparation snapshot roundtrips");
 rejects([&]{prepare_area_pass_defender(prepared_shot_pass,50);});
 AreaPassComparisonContent zero_range{0,0,0,{192,128,64,0}};
 const auto compared_shot_pass=compare_area_pass_defender(prepared_shot_pass,32,0,zero_range);
 check(static_cast<unsigned>(compared_shot_pass.phase)==153 && compared_shot_pass.area_contact &&
       compared_shot_pass.area_contact->defender_power==32 && compared_shot_pass.area_contact->strength==0 &&
       compared_shot_pass.area_contact->loss==0 && compared_shot_pass.area_contact->score==255 &&
       compared_shot_pass.area_contact->result==0 && bytes(prepared_shot_pass)==prepared_bytes,
       "Zero-range shot pass comparison saves the observed original result");
 const auto compared_bytes=bytes(compared_shot_pass);std::istringstream compared_stream(compared_bytes);
 check(bytes(load_phase_session(compared_stream))==compared_bytes,"Shot pass comparison snapshot roundtrips");
 const auto restored_shot_pass=restore_area_pass_ball_variant(compared_shot_pass);
 check(static_cast<unsigned>(restored_shot_pass.phase)==154 && restored_shot_pass.recovery->ball_variant==0 &&
       bytes(compared_shot_pass)==compared_bytes,"Shot pass restores the stacked zero variant without mutating input");
 const auto restored_bytes=bytes(restored_shot_pass);std::istringstream restored_stream(restored_bytes);
 check(bytes(load_phase_session(restored_stream))==restored_bytes,"Shot pass restored variant snapshot roundtrips");
 const auto handler_shot_pass=acknowledge_area_pass_result(restored_shot_pass,64,0);
 check(static_cast<unsigned>(handler_shot_pass.phase)==155 && handler_shot_pass.control->facing==64 &&
       handler_shot_pass.control->motion_requested==0 && bytes(restored_shot_pass)==restored_bytes,
       "Shot pass result acknowledgement saves captured presentation");
 const auto handler_bytes=bytes(handler_shot_pass);std::istringstream handler_stream(handler_bytes);
 check(bytes(load_phase_session(handler_stream))==handler_bytes,"Shot pass result handler snapshot roundtrips");
 ContactPreludeContent shot_pass_clock{};shot_pass_clock.field_elapsed[5]=1;shot_pass_clock.defender_status[2]=6;
 ExperienceContent shot_pass_experience{};shot_pass_experience.stage_bases[0]=4;shot_pass_experience.reward_factors[1]=64;
 const auto advanced_shot_pass=advance_area_pass_nonterminal(handler_shot_pass,shot_pass_clock,shot_pass_experience);
 check(static_cast<unsigned>(advanced_shot_pass.phase)==156 && advanced_shot_pass.current.field.elapsed==1 &&
       advanced_shot_pass.current.clock.remaining==447 && bytes(handler_shot_pass)==handler_bytes,
       "Shot pass nonterminal handler saves clock and status without mutating input");
 const auto advanced_bytes=bytes(advanced_shot_pass);std::istringstream advanced_stream(advanced_bytes);
 check(bytes(load_phase_session(advanced_stream))==advanced_bytes,"Shot pass nonterminal snapshot roundtrips");
 rejects([&]{advance_area_pass_nonterminal(advanced_shot_pass,shot_pass_clock,shot_pass_experience);});
 const auto next_shot_pass=advance_area_pass_defender(advanced_shot_pass);
 check(static_cast<unsigned>(next_shot_pass.phase)==150 && next_shot_pass.area_defender_cursor==1 &&
       next_shot_pass.current.field.elapsed==1 && next_shot_pass.current.field.selected_defenders==5 &&
       !next_shot_pass.area_contact && !next_shot_pass.area_pass_contact,
       "Shot pass advances to the next queued defender with accumulated time");
 const auto second_shot_pass=prepare_area_pass_defender(acknowledge_area_pass_defender_announcement(next_shot_pass),50);
 check(second_shot_pass.recovery->opposing_player==14 && second_shot_pass.area_defender_cursor==1,
       "Shot pass prepares the second defender in saved order");
 auto final_defender=advanced_shot_pass;final_defender.current.field.selected_defenders=1;
 final_defender.scheduling->selected_ids={13,0,0,0,0};
 const auto finished_shot_pass=advance_area_pass_defender(final_defender);
 check(static_cast<unsigned>(finished_shot_pass.phase)==157 && finished_shot_pass.area_defender_cursor==1 &&
       !finished_shot_pass.current.field.selected_defenders && !finished_shot_pass.current.field.elapsed,
       "Shot pass clears the completed defender prefix and elapsed time");
 const auto finished_bytes=bytes(finished_shot_pass);std::istringstream finished_stream(finished_bytes);
 check(bytes(load_phase_session(finished_stream))==finished_bytes,"Completed shot pass defender snapshot roundtrips");
 check(continue_session_unopposed_arrival(finished_shot_pass).phase==FieldSessionPhase::shot_loose_recovery,
       "Completed shot pass sequence returns to loose recovery");
 auto added_shot_pass=handler_shot_pass;added_shot_pass.current.clock.remaining=1;added_shot_pass.current.clock.extra_time=5;
 const auto added_shot_wait=advance_area_pass_nonterminal(added_shot_pass,shot_pass_clock,shot_pass_experience);
 check(static_cast<unsigned>(added_shot_wait.phase)==158 && !added_shot_wait.current.clock.remaining,
       "Shot pass added-time entry is saved before presentation");
 const auto added_shot_bytes=bytes(added_shot_wait);std::istringstream added_shot_stream(added_shot_bytes);
 check(bytes(load_phase_session(added_shot_stream))==added_shot_bytes,"Shot pass added-time snapshot roundtrips");
 const auto resumed_shot_pass=resume_area_pass_added_time(added_shot_wait);
 check(resumed_shot_pass.phase==FieldSessionPhase::shot_pass_nonterminal_done && bytes(added_shot_wait)==added_shot_bytes,
       "Shot pass added-time acknowledgement resumes without repeating rewards");
 auto active_added=added_shot_wait;active_added.current.clock.script_flags|=128;
 rejects([&]{resume_area_pass_added_time(active_added);});
 auto ended_shot_pass=handler_shot_pass;ended_shot_pass.current.clock.remaining=0;ended_shot_pass.current.clock.extra_time=0;
 check(static_cast<unsigned>(advance_area_pass_nonterminal(ended_shot_pass,shot_pass_clock,shot_pass_experience).phase)==159,
       "Shot pass period-end entry is saved before presentation");
 rejects([&]{restore_area_pass_ball_variant(restored_shot_pass);});
 rejects([&]{acknowledge_area_pass_result(handler_shot_pass,64,0);});
 auto unsupported_random=prepared_shot_pass;unsupported_random.current.field.random_first=2;
 const auto unsupported_bytes=bytes(unsupported_random);
 rejects([&]{compare_area_pass_defender(unsupported_random,32,0,zero_range);});
 check(bytes(unsupported_random)==unsupported_bytes,"Nonreturning zero-range sample rejects atomically");
 rejects([&]{compare_area_pass_defender(prepared_shot_pass,32,1,zero_range);});
 auto unsupported_table=zero_range;unsupported_table.range=1;
 rejects([&]{compare_area_pass_defender(prepared_shot_pass,32,0,unsupported_table);});
 unsupported_table=zero_range;unsupported_table.offset=1;
 rejects([&]{compare_area_pass_defender(prepared_shot_pass,32,0,unsupported_table);});
 unsupported_table=zero_range;unsupported_table.eligibility=128;
 rejects([&]{compare_area_pass_defender(prepared_shot_pass,32,0,unsupported_table);});
 auto malformed_comparison=compared_shot_pass;malformed_comparison.area_contact->loss=1;
 rejects([&]{bytes(malformed_comparison);});
 rejects([&]{continue_session_unopposed_arrival(defended_entry);});
 auto bad_defended=defended_entry;bad_defended.area_defender_cursor=5;
 rejects([&]{bytes(bad_defended);});
 bad_defended=defended_entry;bad_defended.saved_ball_flight_flag=0;
 rejects([&]{bytes(bad_defended);});
 auto old_defended=defended_bytes;old_defended[10]=static_cast<char>(132);old_defended.pop_back();
 rejects([&]{std::istringstream old(old_defended);load_phase_session(old);});
 auto kickoff=s;kickoff.kickoff_formation=KickoffFormationContext{{124,132}};
 auto kickoff_flight=begin_prepared_session_ball_flight(kickoff,64,192,g,v);
 check(kickoff_flight.kickoff_formation && kickoff_flight.kickoff_formation->keeper_y[1]==132 &&
       !bytes(kickoff_flight).empty(),"Kickoff keeper positions persist into numeric flight");
 auto flight_bytes=bytes(kickoff_flight);std::istringstream flight_stream(flight_bytes);
 check(bytes(load_phase_session(flight_stream))==flight_bytes,"Kickoff numeric flight roundtrip");
 auto old_flight=flight_bytes;old_flight[10]='a';old_flight.resize(370);
 rejects([&]{std::istringstream i(old_flight);load_phase_session(i);});
 auto legacy_kickoff=bytes(kickoff);legacy_kickoff[10]='a';legacy_kickoff.resize(370);std::istringstream old_kickoff(legacy_kickoff);
 check(bytes(load_phase_session(old_kickoff))==bytes(kickoff),"Legacy a kickoff destination still loads");
 auto kickoff_cell=advance_prepared_session_ball_cell(kickoff_flight).state;
 check(kickoff_cell.phase==FieldSessionPhase::ball_cell_callback && kickoff_cell.kickoff_formation &&
       !bytes(resolve_session_ball_cell(kickoff_cell)).empty(),"Kickoff flight survives cell and defender callback");
 kickoff.shot_flight->destination=122;
 kickoff.shot_flight->loose_ball=false;
 auto kickoff_arrival=advance_prepared_session_ball_cell(begin_prepared_session_ball_flight(kickoff,64,192,g,v)).state;
 check(kickoff_arrival.phase==FieldSessionPhase::ball_arrival_callback && kickoff_arrival.kickoff_formation &&
       !bytes(kickoff_arrival).empty(),"Kickoff flight saves arrival before encounter resolution");
 auto kickoff_keeper=continue_session_unopposed_arrival(kickoff_arrival);
 check(kickoff_keeper.phase==FieldSessionPhase::shot_keeper_entry && kickoff_keeper.kickoff_formation,
       "Kickoff arrival routes to goalkeeper with retained coordinates");
 auto kickoff_loose=kickoff;kickoff_loose.shot_flight->loose_ball=true;
 auto kickoff_loose_arrival=advance_prepared_session_ball_cell(begin_prepared_session_ball_flight(kickoff_loose,64,192,g,v)).state;
 check(continue_session_unopposed_arrival(kickoff_loose_arrival).phase==FieldSessionPhase::shot_loose_recovery,
       "Kickoff empty loose arrival retains formation data");
 kickoff_loose.shot_flight->destination=kickoff_loose.current.field.ball_cell;
 check(begin_prepared_session_ball_flight(kickoff_loose,64,192,g,v).phase==FieldSessionPhase::ball_already_arrived,
       "Kickoff already-arrived flight retains formation data");
 auto kickoff_setup=prepare_session_keeper_entry(kickoff_keeper);
 check(kickoff_setup.phase==FieldSessionPhase::keeper_presentation_pending &&
       kickoff_setup.current.field.selected_defenders==1 && kickoff_setup.scheduling->selected_ids[0]==11 &&
       !bytes(kickoff_setup).empty(),"Kickoff arrival prepares the normal keeper before presentation");
 auto keeper_bytes=bytes(kickoff_setup);std::istringstream keeper_stream(keeper_bytes);
 check(bytes(load_phase_session(keeper_stream))==keeper_bytes,"Kickoff keeper setup roundtrip");
 auto prior_keeper=keeper_bytes;prior_keeper[10]='b';prior_keeper.resize(370);
 rejects([&]{std::istringstream i(prior_keeper);load_phase_session(i);});
 auto old_arrival=bytes(kickoff_arrival);old_arrival[10]='b';old_arrival.resize(370);std::istringstream old_arrival_stream(old_arrival);
 check(bytes(load_phase_session(old_arrival_stream))==bytes(kickoff_arrival),"Legacy b kickoff arrival loads");
 rejects([&]{prepare_session_keeper_entry(kickoff_setup);});
 const auto source=bytes(s);auto ready=begin_prepared_session_ball_flight(s,64,192,g,v);
 check(static_cast<unsigned>(ready.phase)==47 && !ready.shot_progress && ready.current.field.selected_defenders==0 &&
  ready.scheduling->selected_ids[0]==0 && ready.current.field.elapsed==0 && ready.ball_trajectory->vx==256 &&
  ready.ball_trajectory->vy==0 && ready.ball_trajectory->x==((120<<8)|64) && ready.current.clock.remaining==448 &&
  bytes(s)==source,"Saved numeric flight setup preserves fractions and clears original prefix/elapsed");
 auto encoded=bytes(ready);check(encoded.size()==414 && static_cast<unsigned char>(encoded[10])==130,"Session64 layout");
 std::istringstream saved(encoded);ready=load_phase_session(saved);
 auto r=advance_prepared_session_ball_cell(ready);
 check(static_cast<unsigned>(r.state.phase)==48 && r.step.additions==8 &&
  r.state.current.field.ball_x==128 && r.state.current.field.ball_cell==122 &&
  r.state.ball_trajectory->x==((128<<8)|64),"Saved flight advances to cell callback with fractional position");
 auto cell=r.state;cell.current.field.random_first=100;
 auto& defender=cell.current.field.motion[field_motion_index(12)];defender.x=128<<8;defender.y=100<<8;
 const auto before_cell=bytes(cell);auto continued=resolve_session_ball_cell(cell);
 check(static_cast<unsigned>(continued.phase)==47 && continued.current.field.selected_defenders==1 &&
  continued.scheduling->selected_ids[0]==12 && bytes(cell)==before_cell && continued.current.clock.remaining==448,
  "Cell callback accumulates matching field defender and resumes numeric flight without time or input writes");
 rejects([&]{resolve_session_ball_cell(continued);});
 auto next=advance_prepared_session_ball_cell(continued);
 check(next.state.current.field.selected_defenders==1 && next.state.scheduling->selected_ids[0]==12,
  "Selected flight prefix survives next numeric cell step");
 rejects([&]{advance_prepared_session_ball_cell(r.state);});
 rejects([&]{begin_prepared_session_ball_flight(ready,64,192,g,v);});
 s.shot_flight->destination=122;ready=begin_prepared_session_ball_flight(s,64,192,g,v);r=advance_prepared_session_ball_cell(ready);
 check(static_cast<unsigned>(r.state.phase)==49 && r.state.current.field.ball_x==132 &&
  r.state.ball_trajectory->x==((132<<8)|64),"Saved arrival snaps high byte only");
 s.shot_flight->destination=110;ready=begin_prepared_session_ball_flight(s,64,192,g,v);
 check(static_cast<unsigned>(ready.phase)==50 && ready.ball_trajectory->x==((120<<8)|64),"Already arrived does not snap or project");
 s.shot_flight->destination=221;s.current.field.selected_defenders=0;s.scheduling->selected_ids.fill(0);s.shot_progress=ShotContactProgress{};
 ready=begin_prepared_session_ball_flight(s,64,192,g,v);check(static_cast<unsigned>(ready.phase)==47,"Unopposed flight setup");
 auto legacy=source;legacy[10]='J';legacy.resize(344);std::istringstream old(legacy);
 check(bytes(load_phase_session(old))==source,"LegacyJ remains readable");
 for(unsigned length=344;length<355;++length){auto bad=encoded;bad.resize(length);rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 for(auto offset:{344u,353u,354u}){auto bad=encoded;bad[offset]=2;rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 for(unsigned offset=345;offset<355;++offset){auto bad=source;bad[offset]=1;rejects([&]{std::istringstream i(bad);load_phase_session(i);});}
 encoded[10]='J';rejects([&]{std::istringstream bad(encoded);load_phase_session(bad);});
 auto corrupt=ready;corrupt.ball_trajectory.reset();rejects([&]{bytes(corrupt);});
 corrupt=ready;corrupt.ball_trajectory->x^=256;rejects([&]{bytes(corrupt);});
 std::cout<<"Saved ball flight session checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

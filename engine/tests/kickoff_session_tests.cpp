#include "captain/kickoff_session.hpp"
#include "captain/control_session.hpp"
#include "captain/control_input.hpp"
#include "captain/action_menu.hpp"
#include "captain/staged_shot.hpp"
#include "captain/ball_flight_session.hpp"
#include "captain/recovery_session.hpp"
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
 auto old_control=control_bytes;old_control[10]='U';old_control.resize(370);
 rejects([&]{std::istringstream i(old_control);load_phase_session(i);});
 auto past_control=control_ready;past_control.phase=FieldSessionPhase::field_round_pending;
 past_control.current.field.elapsed=1;
 check(!bytes(past_control).empty(),"Saved kickoff keeper positions persist at the field-round boundary");
 auto round_bytes=bytes(past_control);std::istringstream round_stream(round_bytes);
 check(bytes(load_phase_session(round_stream))==round_bytes,"Kickoff field-round snapshot roundtrips");
 auto old_round=round_bytes;old_round[10]='W';old_round.resize(370);
 rejects([&]{std::istringstream i(old_round);load_phase_session(i);});
 // Original sampled stage0 opponent cell decision captures action1/subtype0.
 auto ai=control_ready;ai.phase=FieldSessionPhase::ai_cell_decision;ai.current.field.side=11;
 ai.current.field.ball_owner=16;ai.current.field.ball_x=141;ai.current.field.ball_y=148;ai.current.field.ball_cell=140;
 ai.current.field.random_first=0;ai.current.field.random_second=0;ai.current.field.elapsed=1;
 ai.current.clock.remaining=443;ai.current.clock.extra_time=5;ai.current.clock.script_flags=128;ai.current.clock.ball_mode=128;ai.control->controlled_player=6;ai.control->last_carrier_cell=140;
 ai.current.field.roster[16].type=23;ai.experience->opponent_stage=0;ai.score=MatchScoreContext{1,0,4};
 ai.recovery->attack_subtype=128;ai.recovery->special_owner_flags=0;ai.control->facing=0;ai.control->display_parameter=2;ai.control->motion_requested=1;ai.action->action=0;ai.action->power=22;
 const auto ai_before=bytes(ai);auto chosen=capture_sampled_field_ai_choice(ai,1,0);
 check(static_cast<unsigned>(chosen.phase)==194 && chosen.action->action==1 && chosen.recovery->attack_subtype==0 &&
       chosen.action->power==22 && bytes(ai)==ai_before,"Sampled opponent AI captures observed action and subtype atomically");
 auto chosen_bytes=bytes(chosen);std::istringstream chosen_stream(chosen_bytes);
 check(bytes(load_phase_session(chosen_stream))==chosen_bytes,"Sampled AI choice roundtrips");
 rejects([&]{capture_sampled_field_ai_choice(ai,2,0);});rejects([&]{capture_sampled_field_ai_choice(chosen,1,0);});
 auto other_ai=ai;other_ai.current.field.random_first=1;rejects([&]{capture_sampled_field_ai_choice(other_ai,1,0);});
 auto prepared_ai=capture_sampled_field_ai_preparation(chosen,128,21);
 check(static_cast<unsigned>(prepared_ai.phase)==195 && prepared_ai.action->action==1 && prepared_ai.action->power==21 &&
       prepared_ai.recovery->attack_subtype==128 && bytes(chosen)==chosen_bytes,
       "Sampled opponent preparation captures observed subtype and power");
 auto ai_prepared_bytes=bytes(prepared_ai);std::istringstream ai_prepared_stream(ai_prepared_bytes);
 check(bytes(load_phase_session(ai_prepared_stream))==ai_prepared_bytes,"Sampled AI preparation roundtrips");
 rejects([&]{capture_sampled_field_ai_preparation(chosen,128,22);});rejects([&]{capture_sampled_field_ai_preparation(prepared_ai,128,21);});
 auto acknowledged_ai=acknowledge_sampled_field_ai_presentation(prepared_ai);
 check(static_cast<unsigned>(acknowledged_ai.phase)==196 && acknowledged_ai.current.clock.script_flags==0 &&
       acknowledged_ai.current.clock.ball_mode==0 && bytes(prepared_ai)==ai_prepared_bytes,
       "Sampled opponent presentation acknowledges observed flags without changing input");
 auto acknowledged_bytes=bytes(acknowledged_ai);std::istringstream acknowledged_stream(acknowledged_bytes);
 check(bytes(load_phase_session(acknowledged_stream))==acknowledged_bytes,"Sampled AI presentation roundtrips");
 rejects([&]{acknowledge_sampled_field_ai_presentation(acknowledged_ai);});
 auto dispatched_ai=advance_sampled_field_ai_dispatch(acknowledged_ai);
 check(static_cast<unsigned>(dispatched_ai.phase)==197 && dispatched_ai.control->last_carrier_cell==0 &&
       bytes(acknowledged_ai)==acknowledged_bytes,"Sampled opponent dispatch clears the original carrier-cell scratch");
 auto dispatched_bytes=bytes(dispatched_ai);std::istringstream dispatched_stream(dispatched_bytes);
 check(bytes(load_phase_session(dispatched_stream))==dispatched_bytes,"Sampled AI dispatch roundtrips");
 rejects([&]{advance_sampled_field_ai_dispatch(dispatched_ai);});
 auto captured_field=dispatched_ai.current.field;captured_field.elapsed=0;captured_field.movement_region=140;
 captured_field.motion[0].x=0x5f15;captured_field.motion[0].status=1;
 captured_field.motion[field_motion_index(19)].x=0x7700;captured_field.motion[field_motion_index(19)].y=0x8c00;
 auto pass_presented=acknowledge_sampled_field_ai_pass_presentation(dispatched_ai,captured_field);
 check(static_cast<unsigned>(pass_presented.phase)==198 && pass_presented.current.field.elapsed==0 &&
       pass_presented.current.field.movement_region==140 && pass_presented.current.field.motion[0].x==0x5f15 &&
       pass_presented.current.field.motion[0].status==1 && bytes(dispatched_ai)==dispatched_bytes,
       "Sampled pass presentation records explicitly captured field effects");
 auto pass_presented_bytes=bytes(pass_presented);std::istringstream pass_presented_stream(pass_presented_bytes);
 check(bytes(load_phase_session(pass_presented_stream))==pass_presented_bytes,"Sampled pass presentation roundtrips");
 auto altered_field=captured_field;altered_field.ball_owner=17;
 rejects([&]{acknowledge_sampled_field_ai_pass_presentation(dispatched_ai,altered_field);});
 rejects([&]{acknowledge_sampled_field_ai_pass_presentation(pass_presented,captured_field);});
 auto pass_handoff=acknowledge_sampled_field_ai_pass_handoff(pass_presented);
 check(static_cast<unsigned>(pass_handoff.phase)==199 && pass_handoff.recovery->ball_variant==1 && pass_handoff.recovery->special_owner_flags==0 &&
       pass_handoff.control->facing==64 && pass_handoff.control->last_carrier_cell==128 &&
       pass_handoff.control->display_parameter==0 && pass_handoff.control->motion_requested==0 &&
       bytes(pass_presented)==pass_presented_bytes,"Sampled pass handoff records original control effects");
 auto pass_handoff_bytes=bytes(pass_handoff);std::istringstream pass_handoff_stream(pass_handoff_bytes);
 check(bytes(load_phase_session(pass_handoff_stream))==pass_handoff_bytes,"Sampled pass handoff roundtrips");
 rejects([&]{acknowledge_sampled_field_ai_pass_handoff(pass_handoff);});
 auto pass_target=capture_sampled_field_pass_target(pass_handoff,19,103,255,3);
 check(static_cast<unsigned>(pass_target.phase)==200 && pass_target.sampled_field_pass && pass_target.sampled_field_pass->recipient==19 &&
       pass_target.sampled_field_pass->destination==103 && pass_target.sampled_field_pass->counter==255 && pass_target.sampled_field_pass->distance==3 && bytes(pass_handoff)==pass_handoff_bytes,
       "Sampled pass target advances without changing input");
 auto pass_target_bytes=bytes(pass_target);std::istringstream pass_target_stream(pass_target_bytes);
 check(bytes(load_phase_session(pass_target_stream))==pass_target_bytes,"Sampled pass target roundtrips");
 auto old_pass_target=pass_target_bytes;old_pass_target[10]=static_cast<char>(131);rejects([&]{std::istringstream old(old_pass_target);load_phase_session(old);});
 check(static_cast<unsigned char>(pass_target_bytes[10])==138,"Sampled pass target uses version72 marker");
 auto truncated_pass_target=pass_target_bytes;truncated_pass_target.pop_back();rejects([&]{std::istringstream bad(truncated_pass_target);load_phase_session(bad);});
 auto corrupt_pass_target=pass_target_bytes;corrupt_pass_target[corrupt_pass_target.size()-4]=18;rejects([&]{std::istringstream bad(corrupt_pass_target);load_phase_session(bad);});
 auto misplaced_pass_receiver=pass_handoff;misplaced_pass_receiver.current.field.motion[field_motion_index(19)].x=0x5000;
 rejects([&]{capture_sampled_field_pass_target(misplaced_pass_receiver,19,103,255,3);});
 auto invalid_saved_target=pass_target;invalid_saved_target.current.field.motion[field_motion_index(19)].x=0x5000;
 rejects([&]{bytes(invalid_saved_target);});

 rejects([&]{capture_sampled_field_pass_target(pass_handoff,18,103,255,3);});
 rejects([&]{capture_sampled_field_pass_target(pass_target,19,103,255,3);});
 MovementGeometryContent pass_geometry{};pass_geometry.direction_thresholds.fill(65535);
 BallVelocityContent pass_velocity{};pass_velocity.quarter_wave.fill(256);
 auto pass_flight=begin_sampled_field_pass_flight(pass_target,127,0,pass_geometry,pass_velocity);
 check(pass_flight.phase==FieldSessionPhase::area_ball_advance_ready && !pass_flight.sampled_field_pass &&
       pass_flight.area_ai_target && pass_flight.area_ai_target->recipient==19 && pass_flight.area_ai_target->cell==103 &&
       pass_flight.shot_flight && pass_flight.shot_flight->counter==255 && pass_flight.shot_flight->distance==3 &&
       pass_flight.ball_trajectory && pass_flight.ball_trajectory->flags==1 && bytes(pass_target)==pass_target_bytes,
       "Sampled pass enters shared numeric flight with captured recipient and flag");
 auto pass_flight_bytes=bytes(pass_flight);std::istringstream pass_flight_stream(pass_flight_bytes);
 check(bytes(load_phase_session(pass_flight_stream))==pass_flight_bytes,"Sampled pass flight roundtrips");
 rejects([&]{begin_sampled_field_pass_flight(pass_target,126,0,pass_geometry,pass_velocity);});
 rejects([&]{begin_sampled_field_pass_flight(pass_flight,127,0,pass_geometry,pass_velocity);});
 auto received_pass=pass_flight;received_pass.phase=FieldSessionPhase::area_pass_keeper_entry;
 received_pass.current.field.ball_owner=19;received_pass.current.field.ball_x=119;received_pass.current.field.ball_y=140;received_pass.current.field.ball_cell=103;
 received_pass.ball_trajectory->x=0x7700;received_pass.ball_trajectory->y=0x8c00;
 const auto received_bytes=bytes(received_pass);auto passed_control=continue_sampled_field_pass_control(received_pass);
 check(passed_control.phase==FieldSessionPhase::player_control && passed_control.current.field.ball_owner==19 &&
       passed_control.control->facing==0 && passed_control.saved_ball_flight_flag==1 && !passed_control.area_ai_target &&
       !passed_control.shot_flight && !passed_control.ball_trajectory && bytes(received_pass)==received_bytes,
       "Sampled received pass reaches control while retaining original flight flag");
 auto passed_control_bytes=bytes(passed_control);std::istringstream passed_control_stream(passed_control_bytes);
 check(bytes(load_phase_session(passed_control_stream))==passed_control_bytes,"Sampled pass control roundtrips");
 rejects([&]{continue_sampled_field_pass_control(passed_control);});
 auto received_ready=prepare_session_control(passed_control).state;
 auto received_contact=received_ready;received_contact.phase=FieldSessionPhase::field_contact_pending;
 received_contact.current.field.elapsed=1;received_contact.current.field.selected_defenders=3;
 received_contact.scheduling->selected_ids={3,7,8,0,0};
 auto received_contact_bytes=bytes(received_contact);std::istringstream received_contact_stream(received_contact_bytes);
 check(bytes(load_phase_session(received_contact_stream))==received_contact_bytes && received_contact.kickoff_formation &&
       received_contact.saved_ball_flight_flag==1,"Field contact retains kickoff keeper positions and prior pass flag");
 auto sampled_contact=received_contact;sampled_contact.current.field.ball_x=112;sampled_contact.current.clock.remaining=440;sampled_contact.current.clock.script_flags=0;sampled_contact.current.clock.ball_mode=0;
 sampled_contact.control->display_parameter=2;sampled_contact.control->motion_requested=1;
 auto sampled_contact_bytes=bytes(sampled_contact);auto contact_presented=acknowledge_sampled_opponent_contact_presentation(sampled_contact);
 check(static_cast<unsigned>(contact_presented.phase)==201 && contact_presented.control->display_parameter==0 &&
       contact_presented.control->motion_requested==0 && contact_presented.current.field.selected_defenders==3 &&
       contact_presented.saved_ball_flight_flag==1 && bytes(sampled_contact)==sampled_contact_bytes,
       "Sampled opponent contact acknowledges original presentation control effects");
 auto contact_presented_bytes=bytes(contact_presented);std::istringstream contact_presented_stream(contact_presented_bytes);
 check(bytes(load_phase_session(contact_presented_stream))==contact_presented_bytes,"Sampled contact presentation roundtrips");
 rejects([&]{acknowledge_sampled_opponent_contact_presentation(contact_presented);});
 auto altered_contact=sampled_contact;altered_contact.scheduling->selected_ids[0]=4;
 rejects([&]{acknowledge_sampled_opponent_contact_presentation(altered_contact);});










 auto menu=apply_control_buttons(control_ready,64,0).state;
 auto opened=open_human_action_menu(menu,ActionMenuContent{}).state;
 check(opened.phase==FieldSessionPhase::human_action_menu && opened.kickoff_formation &&
       opened.kickoff_formation->keeper_y==menu.kickoff_formation->keeper_y && !bytes(opened).empty(),
       "Kickoff action menu preserves keeper coordinates");
 auto opened_bytes=bytes(opened);std::istringstream opened_stream(opened_bytes);
 check(bytes(load_phase_session(opened_stream))==opened_bytes,"Kickoff action menu roundtrip");
 auto confirmed=poll_human_action_menu(opened,129,ActionMenuContent{}).state;
 check(confirmed.phase==FieldSessionPhase::attack_choice_pending && confirmed.action->action==0 &&
       confirmed.kickoff_formation && !bytes(confirmed).empty(),"Kickoff menu confirmation keeps keeper positions");
 auto confirmed_bytes=bytes(confirmed);std::istringstream confirmed_stream(confirmed_bytes);
 check(bytes(load_phase_session(confirmed_stream))==confirmed_bytes,"Kickoff confirmation roundtrip");
 auto old_confirmed=confirmed_bytes;old_confirmed[10]='Y';old_confirmed.resize(370);
 rejects([&]{std::istringstream i(old_confirmed);load_phase_session(i);});
 auto legacy_menu=opened_bytes;legacy_menu[10]='Y';legacy_menu.resize(370);std::istringstream legacy_y(legacy_menu);
 check(bytes(load_phase_session(legacy_y))==opened_bytes,"Legacy Y kickoff menu still loads");
 auto prepared=confirmed;prepared.phase=FieldSessionPhase::attack_prepared;
 auto prepared_bytes=bytes(prepared);std::istringstream prepared_stream(prepared_bytes);
 check(bytes(load_phase_session(prepared_stream))==prepared_bytes,"Kickoff prepared-shot snapshot roundtrip");
 auto old_prepared=prepared_bytes;old_prepared[10]='Y';old_prepared.resize(370);
 rejects([&]{std::istringstream i(old_prepared);load_phase_session(i);});
 prepared.current.clock.remaining=500;
 auto timed=dispatch_session_shot(prepared,1).state;
 check(timed.phase==FieldSessionPhase::shot_dispatch_ready && timed.current.clock.remaining==499 &&
       timed.kickoff_formation && !bytes(timed).empty(),"Kickoff unopposed shot keeps goalkeeper positions after clock");
 check(timed.control->facing==0 && timed.control->motion_requested==2,
       "Kickoff unopposed shot captures completed initial presentation effects");
 auto timed_bytes=bytes(timed);std::istringstream timed_stream(timed_bytes);
 check(bytes(load_phase_session(timed_stream))==timed_bytes,"Kickoff timed shot roundtrip");
 auto old_timed=timed_bytes;old_timed[10]='Z';old_timed.resize(370);
 rejects([&]{std::istringstream i(old_timed);load_phase_session(i);});
 auto legacy_prepared=bytes(prepared);legacy_prepared[10]='Z';legacy_prepared.resize(370);std::istringstream legacy_z(legacy_prepared);
 check(bytes(load_phase_session(legacy_z))==bytes(prepared),"Legacy Z kickoff prepared shot loads");
 rejects([&]{dispatch_session_shot(timed,1);});
 auto old_opened=opened_bytes;old_opened[10]='X';old_opened.resize(370);
 rejects([&]{std::istringstream i(old_opened);load_phase_session(i);});
 check(menu.phase==FieldSessionPhase::action_menu_pending && menu.kickoff_formation &&
       !bytes(menu).empty(),"Kickoff control can save an action-menu request with keeper positions");
 auto menu_bytes=bytes(menu);std::istringstream menu_stream(menu_bytes);
 check(bytes(load_phase_session(menu_stream))==menu_bytes,"Kickoff action-menu request roundtrips");
 auto v_menu=menu_bytes;v_menu[10]='V';v_menu.resize(370);rejects([&]{std::istringstream i(v_menu);load_phase_session(i);});
 auto legacy_control=control_bytes;legacy_control[10]='V';legacy_control.resize(370);std::istringstream legacy_v(legacy_control);
 check(bytes(load_phase_session(legacy_v))==control_bytes,"Version V kickoff control remains readable");
 auto t_legacy=ball_bytes;t_legacy[10]='T';t_legacy.resize(370);std::istringstream legacy_t(t_legacy);
 check(bytes(load_phase_session(legacy_t))==ball_bytes,"Legacy T ball state loads");
 auto t_new=handoff_bytes;t_new[10]='T';t_new.resize(370);rejects([&]{std::istringstream i(t_new);load_phase_session(i);});
 auto invalid_handoff=handoff;invalid_handoff.control->controlled_player=2;
 rejects([&]{validate_phase_session(invalid_handoff);});
 invalid_handoff=handoff;invalid_handoff.current.field.ball_owner=8;
 rejects([&]{validate_phase_session(invalid_handoff);});
 rejects([&]{continue_prepared_session_kickoff(handoff);});
 auto s_legacy=receiver_bytes;s_legacy[10]='S';s_legacy.resize(370);std::istringstream legacy_s(s_legacy);
 check(bytes(load_phase_session(legacy_s))==receiver_bytes,"Legacy S receiver loads");
 auto s_new=ball_bytes;s_new[10]='S';s_new.resize(370);rejects([&]{std::istringstream i(s_new);load_phase_session(i);});
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

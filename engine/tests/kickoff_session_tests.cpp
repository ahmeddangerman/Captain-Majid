#include "captain/kickoff_session.hpp"
#include "captain/keeper_session.hpp"
#include "captain/goal_session.hpp"
#include "captain/control_session.hpp"
#include "captain/control_input.hpp"
#include "captain/control_travel.hpp"
#include "captain/control_tick.hpp"
#include "captain/clearance_race.hpp"
#include "captain/action_menu.hpp"
#include "captain/staged_shot.hpp"
#include "captain/approach_session.hpp"
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
 // Initial kickoffs use flag0 and prepared post-goal kickoffs use flag4.
 // Neither flag is an input to the traced formation/selection/ball/handoff instructions.
 for (const auto flag : {0u, 4u}) {
  auto captured=placed;captured.score->kickoff=static_cast<std::uint8_t>(flag);
  auto selected=prepare_session_kickoff_receiver(captured,7,candidates);
  auto positioned=prepare_session_kickoff_ball(selected);
  auto transferred=continue_prepared_session_kickoff(positioned);
  check(transferred.score->kickoff==flag && transferred.current.field.ball_owner==7 &&
        transferred.control->controlled_player==1 && !bytes(transferred).empty(),
        "Initial and post-goal kickoff flags survive the same saved transition");
 }
 auto invalid_kickoff_flag=placed;invalid_kickoff_flag.score->kickoff=2;
 rejects([&]{validate_phase_session(invalid_kickoff_flag);});
 auto before_ball_placement=placed;before_ball_placement.score->kickoff=0;
 before_ball_placement.current.field.ball_x=0;before_ball_placement.current.field.ball_y=0;
 before_ball_placement.current.field.ball_cell=0;
 auto before_ball_receiver=prepare_session_kickoff_receiver(before_ball_placement,7,candidates);
 auto after_ball_placement=prepare_session_kickoff_ball(before_ball_receiver);
 check(before_ball_receiver.current.field.ball_x==0 && before_ball_receiver.current.field.ball_y==0 &&
       after_ball_placement.current.field.ball_x==52 && after_ball_placement.current.field.ball_y==84 &&
       after_ball_placement.current.field.ball_cell==0,
       "Unplaced kickoff ball is accepted only until placement supplies field coordinates");
 auto invalid_placed_ball=after_ball_placement;invalid_placed_ball.current.field.ball_x=0;
 rejects([&]{validate_phase_session(invalid_placed_ball);});
 auto control_ready=prepare_session_control(handoff).state;
 check(control_ready.phase==FieldSessionPhase::control_ready && control_ready.current.field.ball_owner==7 &&
       control_ready.kickoff_formation && !control_ready.kickoff_receiver &&
       control_ready.kickoff_formation->keeper_y==handoff.kickoff_formation->keeper_y,
       "Kickoff control setup retains goalkeeper positions and consumes receiver context");
 auto travelled=advance_session_control_travel(control_ready,928,1);
 check(travelled.control->travel_accumulator==232 && control_ready.control->travel_accumulator==0 &&
       travelled.phase==control_ready.phase && travelled.current.field.ball_x==control_ready.current.field.ball_x,
       "Explicit control-travel step updates only the represented accumulator");
 auto idle_travel=advance_session_control_travel(control_ready,928,0);
 check(bytes(idle_travel)==bytes(control_ready),"Neutral home input leaves travel unchanged");
 rejects([&]{advance_session_control_travel(handoff,928,1);});
 auto profile_input=control_ready;profile_input.current.field.roster[7].type=1;
 PlayerProfileMappings profile_mappings{};FieldPhaseContent travel_movement{};
 travel_movement.profiles.ordinary[1].speed_profile=0;
 travel_movement.profiles.speeds.push_back({0,0});
 ControlMovementContent travel_content{};travel_content.carrier_speeds[0]=928;
 auto profiled=advance_profiled_session_control_travel(profile_input,1,profile_mappings,travel_movement,travel_content);
 check(profiled.loaded_speed && *profiled.loaded_speed==928 && profiled.state.control->travel_accumulator==232 &&
       profile_input.control->travel_accumulator==0,"Profiled control travel loads the carrier speed and preserves its input");
 auto neutral_profile=profile_input;neutral_profile.current.field.roster[7].type=0;
 auto neutral_result=advance_profiled_session_control_travel(neutral_profile,0,profile_mappings,travel_movement,travel_content);
 check(!neutral_result.loaded_speed && bytes(neutral_result.state)==bytes(neutral_profile),
       "Neutral home input skips profile loading just like the original gate");
 rejects([&]{advance_profiled_session_control_travel(neutral_profile,1,profile_mappings,travel_movement,travel_content);});
 auto opposing=profile_input;opposing.current.field.side=11;opposing.current.field.ball_owner=12;
 opposing.current.field.roster[12].type=1;opposing.control->controlled_player=1;opposing.control->facing=0;
 auto opposing_travel=advance_profiled_session_control_travel(opposing,0,profile_mappings,travel_movement,travel_content);
 check(opposing_travel.loaded_speed && *opposing_travel.loaded_speed==928 &&
       opposing_travel.state.control->travel_accumulator==65303,
       "Opposing carrier speed is loaded even without held directional input");
 ObservedNmiFieldPoll nmi_poll{};nmi_poll.travel_observed=true;nmi_poll.travel_held=1;nmi_poll.travel_speed=928;
 auto poll_frame=advance_observed_nmi_control_poll(control_ready,profile_mappings,travel_movement,travel_content,255,nmi_poll);
 check(poll_frame.random_index==0 && poll_frame.poll.state.current.field.random_first==255 &&
       poll_frame.poll.state.control->travel_accumulator==232 && poll_frame.poll.state.control->decision_timer==9 &&
       control_ready.control->travel_accumulator==0,"Observed NMI, travel and control poll compose without mutating input");
 auto invalid_poll=nmi_poll;invalid_poll.travel_observed=false;
 rejects([&]{advance_observed_nmi_control_poll(control_ready,profile_mappings,travel_movement,travel_content,255,invalid_poll);});
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
 ai.recovery->attack_subtype=128;ai.recovery->special_owner_flags=0;ai.control->facing=0;ai.control->display_parameter=2;ai.control->motion_requested=1;ai.action->action=0;ai.action->power=22;ai.action->flash_counter=128;
 const auto ai_before=bytes(ai);auto chosen=capture_sampled_field_ai_choice(ai,1,0);
 check(static_cast<unsigned>(chosen.phase)==194 && chosen.action->action==1 && chosen.recovery->attack_subtype==0 &&
       chosen.action->power==22 && bytes(ai)==ai_before,"Sampled opponent AI captures observed action and subtype atomically");
 auto chosen_bytes=bytes(chosen);std::istringstream chosen_stream(chosen_bytes);
 check(bytes(load_phase_session(chosen_stream))==chosen_bytes,"Sampled AI choice roundtrips");
 auto later_ai=ai;later_ai.current.clock.remaining=406;later_ai.current.clock.extra_time=9;later_ai.area_sequence_result=0;
 later_ai.score=MatchScoreContext{2,1,4};
 auto later_choice=capture_sampled_field_ai_choice(later_ai,1,0);
 auto later_prepared=capture_sampled_field_ai_preparation(later_choice,128,21);
 check(later_choice.phase==FieldSessionPhase::sampled_field_ai_choice && later_choice.action->action==1 &&
       later_prepared.phase==FieldSessionPhase::sampled_field_ai_prepared && later_prepared.action->power==21 &&
       later_prepared.recovery->attack_subtype==128 && later_prepared.current.clock.remaining==406,
       "Post-goal player16 decision captures the observed pass and preparation");
 auto later_presented=acknowledge_sampled_field_ai_presentation(later_prepared);
 auto later_dispatch=advance_sampled_field_ai_dispatch(later_presented);
 auto later_motion=later_dispatch.current.field;later_motion.elapsed=0;later_motion.movement_region=140;
 later_motion.motion[field_motion_index(19)].x=0x7700;later_motion.motion[field_motion_index(19)].y=0x8c00;
 auto later_pass_presented=acknowledge_sampled_field_ai_pass_presentation(later_dispatch,later_motion);
 auto later_pass_handoff=acknowledge_sampled_field_ai_pass_handoff(later_pass_presented);
 check(later_pass_handoff.phase==FieldSessionPhase::sampled_field_ai_pass_handoff &&
       later_pass_handoff.current.clock.remaining==406 && later_pass_handoff.recovery->ball_variant==1 &&
       later_pass_handoff.control->last_carrier_cell==128 && later_pass_handoff.control->facing==64,
       "Post-goal sampled pass reaches the observed presentation handoff");
 auto later_target=capture_sampled_field_pass_target(later_pass_handoff,19,103,255,3);
 check(later_target.phase==FieldSessionPhase::sampled_field_ai_pass_target && later_target.sampled_field_pass &&
       later_target.sampled_field_pass->recipient==19 && later_target.sampled_field_pass->destination==103,
       "Post-goal pass targets the observed recipient and cell");
 auto later_choice_bytes=bytes(later_choice);std::istringstream later_choice_stream(later_choice_bytes);
 auto later_prepared_bytes=bytes(later_prepared);std::istringstream later_prepared_stream(later_prepared_bytes);
 check(bytes(load_phase_session(later_choice_stream))==later_choice_bytes &&
       bytes(load_phase_session(later_prepared_stream))==later_prepared_bytes,
       "Post-goal sampled AI states roundtrip without losing their branch context");
 auto invalid_later_ai=later_ai;invalid_later_ai.current.clock.remaining=405;
 rejects([&]{capture_sampled_field_ai_choice(invalid_later_ai,1,0);});
 rejects([&]{capture_sampled_field_ai_choice(ai,2,0);});rejects([&]{capture_sampled_field_ai_choice(chosen,1,0);});
 auto score35_ai=ai;score35_ai.current.clock.remaining=296;score35_ai.current.clock.extra_time=13;score35_ai.score=MatchScoreContext{3,5,4};
 auto score35_choice=capture_sampled_field_ai_choice(score35_ai,1,0);
 check(score35_choice.phase==FieldSessionPhase::sampled_field_ai_choice && score35_choice.action->action==1,"The 3-5 player16 decision captures the observed pass");
 auto score35_prepared=capture_sampled_field_ai_preparation(score35_choice,128,21);
 check(score35_prepared.phase==FieldSessionPhase::sampled_field_ai_prepared && score35_prepared.action->power==21,"The 3-5 player16 pass captures its observed preparation");
 auto score35_presented=acknowledge_sampled_field_ai_presentation(score35_prepared);
 auto score35_dispatch=advance_sampled_field_ai_dispatch(score35_presented);
 auto postgoal19=ai;postgoal19.current.field.ball_owner=19;postgoal19.current.field.ball_x=111;
 postgoal19.current.field.ball_y=141;postgoal19.current.field.ball_cell=91;postgoal19.current.field.roster[19].type=23;
 postgoal19.current.clock.remaining=404;postgoal19.current.clock.extra_time=9;postgoal19.score=MatchScoreContext{2,1,4};
 postgoal19.control->controlled_player=7;postgoal19.control->last_carrier_cell=91;
 postgoal19.action->action=1;postgoal19.action->power=21;postgoal19.saved_ball_flight_flag=1;
 auto postgoal19_choice=capture_sampled_field_ai_choice(postgoal19,2,0);
 check(postgoal19_choice.phase==FieldSessionPhase::control_clock_pending && postgoal19_choice.action->action==2 &&
       postgoal19_choice.recovery->attack_subtype==0 && postgoal19_choice.current.clock.remaining==404,
       "Post-goal player19 resumes the clock after the observed choice");
 auto postgoal_contact=postgoal19_choice;postgoal_contact.phase=FieldSessionPhase::field_contact_pending;
 postgoal_contact.current.clock.remaining=403;postgoal_contact.current.clock.script_flags=0;postgoal_contact.current.clock.ball_mode=0;
 postgoal_contact.current.field.selected_defenders=3;postgoal_contact.scheduling->selected_ids={3,8,9,0,0};
 postgoal_contact.action->flash_counter=13;
 auto postgoal_contact_presented=acknowledge_sampled_opponent_contact_presentation(postgoal_contact);
 auto postgoal_prompt=acknowledge_sampled_field_defense_prompt(postgoal_contact_presented,13);
 check(postgoal_prompt.phase==FieldSessionPhase::sampled_field_defense_prompt && postgoal_prompt.area_defense &&
       postgoal_prompt.area_defense->blink==13 && postgoal_prompt.action->cursor==0 &&
       postgoal_prompt.scheduling->selected_ids==std::array<std::uint8_t,5>{3,8,9,0,0},
       "Post-goal encounter opens the observed three-defender prompt");
 rejects([&]{acknowledge_sampled_field_defense_prompt(postgoal_contact_presented,7);});
 auto wrong_postgoal19=postgoal19;wrong_postgoal19.current.clock.remaining=403;
 rejects([&]{capture_sampled_field_ai_choice(wrong_postgoal19,2,0);});
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
 auto later_flight=begin_sampled_field_pass_flight(later_target,143,27,pass_geometry,pass_velocity);
 check(later_flight.phase==FieldSessionPhase::area_ball_advance_ready && later_flight.ball_trajectory &&
       later_flight.ball_trajectory->x==0x8d8f && later_flight.ball_trajectory->y==0x941b &&
       later_flight.area_ai_target && later_flight.area_ai_target->recipient==19,
       "Post-goal pass enters numeric flight with observed fractional position");
 rejects([&]{begin_sampled_field_pass_flight(later_target,127,0,pass_geometry,pass_velocity);});
 auto later_received=later_flight;later_received.phase=FieldSessionPhase::area_pass_keeper_entry;
 later_received.current.field.ball_owner=19;later_received.current.field.ball_x=118;
 later_received.current.field.ball_y=141;later_received.current.field.ball_cell=103;
 later_received.ball_trajectory->x=30440;later_received.ball_trajectory->y=36351;
 auto later_control=continue_sampled_field_pass_control(later_received);
 check(later_control.phase==FieldSessionPhase::player_control && later_control.current.field.ball_owner==19 &&
       later_control.current.field.ball_cell==103 && later_control.control->facing==64 &&
       !later_control.ball_trajectory && !later_control.area_ai_target && later_control.saved_ball_flight_flag==1,
       "Post-goal pass hands player19 the observed control state");
 auto invalid_later_received=later_received;invalid_later_received.score->right_goals=0;
 rejects([&]{continue_sampled_field_pass_control(invalid_later_received);});
 auto later_control_bytes=bytes(later_control);std::istringstream later_control_stream(later_control_bytes);
 check(bytes(load_phase_session(later_control_stream))==later_control_bytes,
       "Post-goal player19 control handoff roundtrips");
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
 auto field_prompt=acknowledge_sampled_field_defense_prompt(contact_presented,7);
 check(static_cast<unsigned>(field_prompt.phase)==202 && field_prompt.area_defense && field_prompt.area_defense->blink==7 &&
       field_prompt.area_defense->actions==std::array<std::uint8_t,4>{255,255,255,255} && field_prompt.action->cursor==0 &&
       field_prompt.saved_ball_flight_flag==1 && bytes(contact_presented)==contact_presented_bytes,
       "Sampled field defense prompt retains ordered defenders and captured blink");
 auto field_prompt_bytes=bytes(field_prompt);std::istringstream field_prompt_stream(field_prompt_bytes);
 check(bytes(load_phase_session(field_prompt_stream))==field_prompt_bytes,"Sampled field defense prompt roundtrips");
 rejects([&]{acknowledge_sampled_field_defense_prompt(contact_presented,8);});
 rejects([&]{acknowledge_sampled_field_defense_prompt(field_prompt,7);});
 AreaDefenseMenuContent field_menu{};field_menu.directions[0]={0,2,6,1};field_menu.field_icons=std::array<std::uint8_t,7>{10,11,12,13,14,15,16};
 auto postgoal_poll=poll_keeper_area_defense(postgoal_prompt,1,field_menu);
 check(postgoal_poll.state.phase==FieldSessionPhase::sampled_field_defense_poll && postgoal_poll.state.area_defense &&
       postgoal_poll.state.area_defense->actions[0]==0 && postgoal_poll.state.action->cursor==0,
       "Post-goal prompt accepts the first observed defender choice input");
 auto postgoal_confirm=poll_keeper_area_defense(postgoal_poll.state,128,field_menu);
 check(postgoal_confirm.state.phase==FieldSessionPhase::sampled_field_defense_preparation && postgoal_confirm.confirmed &&
       postgoal_confirm.state.recovery->opposing_player==3 && postgoal_confirm.state.area_defense->pressed==128,
       "Post-goal first defender confirmation selects defender3");
 auto observed_postgoal=postgoal_prompt;
 for(unsigned id=1;id<=10;++id) {
  observed_postgoal.current.field.roster[id].type=23;
  observed_postgoal.current.field.roster[id].stamina=id==3 ? 30 : id==5 ? 25 : 140;
 }
 auto observed_choice=poll_keeper_area_defense(observed_postgoal,1,field_menu);
 auto observed_confirm=poll_keeper_area_defense(observed_choice.state,128,field_menu);
 auto observed_registered=prepare_keeper_area_defense(observed_confirm.state,field_menu);
 check(observed_registered.state.phase==FieldSessionPhase::sampled_field_defense_poll &&
       observed_registered.state.action->cursor==1 && observed_registered.state.area_defense->actions[0]==0,
       "Post-goal original roster registers defender3 ordinary choice");
 auto altered_observed=observed_confirm.state;altered_observed.current.field.roster[5].stamina=26;
 rejects([&]{prepare_keeper_area_defense(altered_observed,field_menu);});
 auto observed_second=poll_keeper_area_defense(observed_registered.state,4,field_menu);
 auto observed_second_confirm=poll_keeper_area_defense(observed_second.state,128,field_menu);
 auto observed_second_done=prepare_keeper_area_defense(observed_second_confirm.state,field_menu);
 auto observed_third=poll_keeper_area_defense(observed_second_done.state,4,field_menu);
 auto observed_third_confirm=poll_keeper_area_defense(observed_third.state,128,field_menu);
 auto observed_third_done=prepare_keeper_area_defense(observed_third_confirm.state,field_menu);
 auto observed_complete=poll_keeper_area_defense(observed_third_done.state,128,field_menu);
 check(observed_complete.state.phase==FieldSessionPhase::sampled_field_defense_complete &&
       observed_complete.state.area_defense->actions==std::array<std::uint8_t,4>{0,6,6,255},
       "Post-goal mixed defender choices reach final confirmation");
 auto observed_ai=capture_sampled_field_ai_choice(observed_complete.state,2,0);
 check(observed_ai.phase==FieldSessionPhase::sampled_mixed_defense_ai_choice &&
       observed_ai.action->action==2 && observed_ai.recovery->attack_subtype==0,
       "Post-goal mixed contact captures the observed attacker action");
 auto postgoal_skip=poll_keeper_area_defense(postgoal_prompt,4,field_menu);
 auto postgoal_skip_confirm=poll_keeper_area_defense(postgoal_skip.state,128,field_menu);
 auto postgoal_skip_registered=prepare_keeper_area_defense(postgoal_skip_confirm.state,field_menu);
 check(postgoal_skip_registered.state.phase==FieldSessionPhase::sampled_field_defense_poll &&
       postgoal_skip_registered.state.action->cursor==1 && postgoal_skip_registered.state.area_defense->actions[0]==6,
       "Post-goal first defender skip registers and advances to the next defender");
 auto postgoal_skip2=poll_keeper_area_defense(postgoal_skip_registered.state,4,field_menu);
 auto postgoal_confirm2=poll_keeper_area_defense(postgoal_skip2.state,128,field_menu);
 auto postgoal_registered2=prepare_keeper_area_defense(postgoal_confirm2.state,field_menu);
 auto postgoal_skip3=poll_keeper_area_defense(postgoal_registered2.state,4,field_menu);
 auto postgoal_confirm3=poll_keeper_area_defense(postgoal_skip3.state,128,field_menu);
 auto postgoal_registered3=prepare_keeper_area_defense(postgoal_confirm3.state,field_menu);
 auto postgoal_complete=poll_keeper_area_defense(postgoal_registered3.state,128,field_menu);
 check(postgoal_complete.state.phase==FieldSessionPhase::sampled_field_defense_complete &&
       postgoal_complete.state.area_defense->actions==std::array<std::uint8_t,4>{6,6,6,255},
       "Post-goal three defender skips reach final confirmation");
 auto postgoal_contact_choice=capture_sampled_field_ai_choice(postgoal_complete.state,2,0);
 auto postgoal_contact_prepared=capture_sampled_field_ai_preparation(postgoal_contact_choice,128,24);
 check(postgoal_contact_choice.phase==FieldSessionPhase::sampled_field_contact_ai_choice &&
       postgoal_contact_prepared.phase==FieldSessionPhase::sampled_field_contact_ai_prepared &&
       postgoal_contact_prepared.action->power==24 && postgoal_contact_prepared.recovery->attack_subtype==128,
       "Post-goal skipped encounter captures the observed attacking action preparation");
 auto field_polled=poll_keeper_area_defense(field_prompt,1,field_menu);
 check(static_cast<unsigned>(field_polled.state.phase)==203 && field_polled.state.area_defense->actions[0]==0 &&
       field_polled.state.area_defense->blink==13 && field_polled.state.action->flash_counter==192 && bytes(field_prompt)==field_prompt_bytes,
       "Field defense direction selects the original action without mutating input");
 auto field_confirmed=poll_keeper_area_defense(field_polled.state,128,field_menu);
 check(static_cast<unsigned>(field_confirmed.state.phase)==204 && field_confirmed.confirmed &&
       field_confirmed.state.recovery->opposing_player==3 && field_confirmed.state.area_defense->pressed==128,
       "Field defense confirm captures selected defender and action");
 auto field_confirmed_bytes=bytes(field_confirmed.state);std::istringstream field_confirmed_stream(field_confirmed_bytes);
 check(bytes(load_phase_session(field_confirmed_stream))==field_confirmed_bytes,"Field defense confirmation roundtrips");
 auto field_six=poll_keeper_area_defense(field_prompt,4,field_menu);
 auto field_six_confirmed=poll_keeper_area_defense(field_six.state,128,field_menu);
 auto field_registered=prepare_keeper_area_defense(field_six_confirmed.state,field_menu);
 check(static_cast<unsigned>(field_registered.state.phase)==203 && field_registered.state.action->cursor==1 &&
       field_registered.state.action->flash_counter==192 && field_registered.state.area_defense->blink==0 &&
       field_registered.state.area_defense->actions[0]==6 && field_registered.state.area_defense->pressed==0,
       "Registering a field action advances to the next defender");
 auto field_registered_bytes=bytes(field_registered.state);std::istringstream field_registered_stream(field_registered_bytes);
 check(bytes(load_phase_session(field_registered_stream))==field_registered_bytes,"Registered field defense roundtrips");
 rejects([&]{prepare_keeper_area_defense(field_confirmed.state,field_menu);});
 auto sampled_zero=field_confirmed.state;
 for(unsigned id=1;id<=10;++id){sampled_zero.current.field.roster[id].type=23;sampled_zero.current.field.roster[id].stamina=110;}
 sampled_zero.current.field.roster[3].stamina=30;
 auto zero_registered=prepare_keeper_area_defense(sampled_zero,field_menu);
 check(zero_registered.state.action->cursor==1 && zero_registered.state.area_defense->actions[0]==0 &&
       zero_registered.state.area_defense->pressed==0,"Sampled ordinary defense registers without a special option");
 auto mixed_second=poll_keeper_area_defense(zero_registered.state,4,field_menu);
 auto mixed_second_confirm=poll_keeper_area_defense(mixed_second.state,128,field_menu);
 auto mixed_second_done=prepare_keeper_area_defense(mixed_second_confirm.state,field_menu);
 auto mixed_third=poll_keeper_area_defense(mixed_second_done.state,4,field_menu);
 auto mixed_third_confirm=poll_keeper_area_defense(mixed_third.state,128,field_menu);
 auto mixed_third_done=prepare_keeper_area_defense(mixed_third_confirm.state,field_menu);
 auto mixed_complete=poll_keeper_area_defense(mixed_third_done.state,128,field_menu);
 check(static_cast<unsigned>(mixed_complete.state.phase)==205 &&
       mixed_complete.state.area_defense->actions==std::array<std::uint8_t,4>{0,6,6,255},
       "Ordinary first defender and two skips reach final confirmation");
 auto mixed_ai_choice=capture_sampled_field_ai_choice(mixed_complete.state,2,0);
 check(static_cast<unsigned>(mixed_ai_choice.phase)==215 && mixed_ai_choice.action->action==2 &&
       mixed_ai_choice.area_defense->actions==std::array<std::uint8_t,4>{0,6,6,255},
       "Mixed defense retains the ordinary defender when the opponent choice is captured");



 field_menu.titles={22,0,23};
 auto field_back=poll_keeper_area_defense(field_registered.state,64,field_menu);
 check(field_back.state.action->cursor==0 && field_back.presentation.size()==2 &&
       field_back.presentation.back().kind==ControlPresentationKind::display && field_back.presentation.back().id==22,
       "Returning to the prior field defender requests the field menu title");
 auto field_slot1=poll_keeper_area_defense(field_registered.state,4,field_menu);
 auto field_slot1_confirmed=poll_keeper_area_defense(field_slot1.state,128,field_menu);
 auto field_slot1_registered=prepare_keeper_area_defense(field_slot1_confirmed.state,field_menu);
 check(field_slot1_registered.state.action->cursor==2 && field_slot1_registered.state.area_defense->actions[1]==6,
       "Registering the second defender advances to the third defender");
 auto field_slot2=poll_keeper_area_defense(field_slot1_registered.state,4,field_menu);
 auto field_slot2_confirmed=poll_keeper_area_defense(field_slot2.state,128,field_menu);
 auto field_slot2_registered=prepare_keeper_area_defense(field_slot2_confirmed.state,field_menu);
 check(field_slot2_registered.state.action->cursor==3 && field_slot2_registered.state.area_defense->actions[2]==6,
       "Registering the third defender advances to final confirmation");
 auto field_complete=poll_keeper_area_defense(field_slot2_registered.state,128,field_menu);
 check(static_cast<unsigned>(field_complete.state.phase)==205 && field_complete.confirmed,
       "Final confirmation completes the sampled field choices");
 auto contact_ai_choice=capture_sampled_field_ai_choice(field_complete.state,2,0);
 check(static_cast<unsigned>(contact_ai_choice.phase)==206 && contact_ai_choice.action->action==2 &&
       contact_ai_choice.recovery->attack_subtype==0 && contact_ai_choice.action->power==21 &&
       contact_ai_choice.area_defense->actions==std::array<std::uint8_t,4>{6,6,6,255},
       "Captured opponent choice preserves the defenders and prior power");
 auto contact_ai_bytes=bytes(contact_ai_choice);std::istringstream contact_ai_stream(contact_ai_bytes);
 check(bytes(load_phase_session(contact_ai_stream))==contact_ai_bytes,"Captured opponent choice roundtrips");
 rejects([&]{capture_sampled_field_ai_choice(field_complete.state,1,0);});
 auto contact_ai_prepared=capture_sampled_field_ai_preparation(contact_ai_choice,128,24);
 check(static_cast<unsigned>(contact_ai_prepared.phase)==207 && contact_ai_prepared.action->power==24 &&
       contact_ai_prepared.recovery->attack_subtype==128 && contact_ai_prepared.action->action==2,
       "Captured field contact preparation retains observed power and subtype");
 auto contact_ai_prepared_bytes=bytes(contact_ai_prepared);std::istringstream contact_ai_prepared_stream(contact_ai_prepared_bytes);
 check(bytes(load_phase_session(contact_ai_prepared_stream))==contact_ai_prepared_bytes,"Captured opponent preparation roundtrips");
 rejects([&]{capture_sampled_field_ai_preparation(contact_ai_choice,128,21);});
 FieldPlayerProfiles sampled_attack_profiles{};sampled_attack_profiles.skills.resize(1);sampled_attack_profiles.skills[0].fill(20);
 FieldDefenseContent sampled_attack_content{};sampled_attack_content.skill_selectors[2][0]=1;
 sampled_attack_content.moves[2]={{0,0,40,0}};sampled_attack_content.strength_curve.fill(24);
 auto native_attack_input=contact_ai_choice;native_attack_input.current.field.roster[19].type=23;
 auto native_attack=prepare_sampled_field_contact_attack(native_attack_input,sampled_attack_profiles,sampled_attack_content);
 check(native_attack.action->power==24 && native_attack.recovery->attack_subtype==128 &&
       static_cast<unsigned>(native_attack.phase)==207 && native_attack.current.field.roster[19].stamina==native_attack_input.current.field.roster[19].stamina,
       "Native opponent attack derives captured power without spending opponent stamina");
 auto mixed_attack_input=mixed_ai_choice;mixed_attack_input.current.field.roster[19].type=23;
 auto mixed_attack=prepare_sampled_field_contact_attack(mixed_attack_input,sampled_attack_profiles,sampled_attack_content);
 check(static_cast<unsigned>(mixed_attack.phase)==216 && mixed_attack.action->power==24 &&
       mixed_attack.area_defense->actions==std::array<std::uint8_t,4>{0,6,6,255},
       "Mixed defense opponent preparation retains its active defender");
 auto observed_attack=prepare_sampled_field_contact_attack(observed_ai,sampled_attack_profiles,sampled_attack_content);
 check(observed_attack.phase==FieldSessionPhase::sampled_mixed_defense_ai_prepared &&
       observed_attack.action->power==24 && observed_attack.recovery->attack_subtype==128,
       "Post-goal mixed defense prepares the observed opponent action");
 FieldDefenseContent mixed_defense_content{};mixed_defense_content.skill_selectors[0][0]=1;
 mixed_defense_content.moves[0]={{0,0,70,0}};mixed_defense_content.strength_curve.fill(19);
 auto mixed_defender=prepare_sampled_mixed_defender(mixed_attack,sampled_attack_profiles,mixed_defense_content);
 check(static_cast<unsigned>(mixed_defender.phase)==217 && mixed_defender.recovery->opposing_player==3 &&
       mixed_defender.control->last_carrier_cell==0 && mixed_defender.current.field.roster[3].stamina==30,
       "Mixed defender preparation retains stamina and selects the observed defender");
 auto observed_defender=prepare_sampled_mixed_defender(observed_attack,sampled_attack_profiles,mixed_defense_content);
 check(observed_defender.phase==FieldSessionPhase::sampled_mixed_defender_prepared &&
       observed_defender.recovery->opposing_player==3 && observed_defender.current.field.roster[3].stamina==30,
       "Post-goal mixed contact prepares defender3");
 FieldComparisonContent mixed_compare_content{};mixed_compare_content.variation={{{128,128}}};mixed_compare_content.eligibility={0};
 mixed_compare_content.thresholds={192,128,64,0};
 auto mixed_result=compare_sampled_mixed_defender(mixed_defender,mixed_compare_content);
 check(static_cast<unsigned>(mixed_result.phase)==218 && mixed_result.current.field.roster[3].stamina==0 &&
       mixed_result.action->power==24 && mixed_defender.current.field.roster[3].stamina==30,
       "Weakened mixed defender spends remaining stamina without reducing attack power");
 auto observed_result=compare_sampled_mixed_defender(observed_defender,mixed_compare_content);
 check(observed_result.phase==FieldSessionPhase::sampled_mixed_defender_compared &&
       observed_result.current.field.roster[3].stamina==0 && observed_result.action->power==24,
       "Post-goal defender3 comparison spends the observed remaining stamina");
 auto mixed_result_bytes=bytes(mixed_result);std::istringstream mixed_result_stream(mixed_result_bytes);
 check(bytes(load_phase_session(mixed_result_stream))==mixed_result_bytes,"Mixed defender comparison roundtrips");
 rejects([&]{compare_sampled_mixed_defender(mixed_result,mixed_compare_content);});
 auto mixed_result4=advance_sampled_mixed_result4(mixed_result);
 check(static_cast<unsigned>(mixed_result4.phase)==227 && mixed_result4.area_sequence_result==4 &&
       mixed_result.area_sequence_result==255 && mixed_result4.current.field.roster[3].stamina==0,
       "Observed mixed defender result changes to result4 without changing the field");
 auto observed_result4=advance_sampled_mixed_result4(observed_result);
 check(observed_result4.phase==FieldSessionPhase::sampled_mixed_defender_result4 &&
       observed_result4.area_sequence_result==4 && observed_result4.current.field.roster[3].stamina==0,
       "Post-goal mixed defender follows the observed result4 branch");
 auto mixed_result4_bytes=bytes(mixed_result4);std::istringstream mixed_result4_stream(mixed_result4_bytes);
 check(bytes(load_phase_session(mixed_result4_stream))==mixed_result4_bytes,"Mixed defender result4 roundtrips");
 rejects([&]{advance_sampled_mixed_result4(mixed_result4);});
 rejects([&]{advance_sampled_mixed_result4(mixed_defender);});
 auto mixed_pass_handoff=acknowledge_sampled_mixed_result4_pass(mixed_result4);
 check(static_cast<unsigned>(mixed_pass_handoff.phase)==228 && mixed_pass_handoff.action->action==1 &&
       mixed_pass_handoff.recovery->attack_subtype==0 && mixed_pass_handoff.control->facing==64 &&
       mixed_pass_handoff.current.field.elapsed==19 && mixed_pass_handoff.current.clock.remaining==422 &&
       mixed_pass_handoff.current.clock.extra_time==9 && mixed_result4.current.clock.remaining==440,
       "Acknowledged mixed result4 pass handoff retains observed clock and action");
 auto observed_pass_handoff=acknowledge_sampled_mixed_result4_pass(observed_result4);
 check(observed_pass_handoff.phase==FieldSessionPhase::sampled_mixed_result4_pass &&
       observed_pass_handoff.current.field.elapsed==19 && observed_pass_handoff.current.clock.remaining==385 &&
       observed_pass_handoff.current.clock.extra_time==13 && observed_pass_handoff.control->facing==0 &&
       observed_pass_handoff.action->action==1 && observed_pass_handoff.recovery->attack_subtype==0,
       "Post-goal result4 handoff follows the observed pass clock and facing");
 auto observed_pass_bytes=bytes(observed_pass_handoff);std::istringstream observed_pass_stream(observed_pass_bytes);
 check(bytes(load_phase_session(observed_pass_stream))==observed_pass_bytes,
       "Post-goal result4 handoff roundtrips");
 auto mixed_pass_bytes=bytes(mixed_pass_handoff);std::istringstream mixed_pass_stream(mixed_pass_bytes);
 check(bytes(load_phase_session(mixed_pass_stream))==mixed_pass_bytes,"Mixed result4 pass handoff roundtrips");
 rejects([&]{acknowledge_sampled_mixed_result4_pass(mixed_pass_handoff);});
 auto mixed_target_input=mixed_pass_handoff;
 mixed_target_input.current.field.motion[field_motion_index(12)].x=0x8a4b;
 mixed_target_input.current.field.motion[field_motion_index(12)].y=0xa45b;
 auto mixed_target=capture_sampled_field_pass_target(mixed_target_input,12,142,255,3);
 check(static_cast<unsigned>(mixed_target.phase)==229 && mixed_target.sampled_field_pass &&
       mixed_target.sampled_field_pass->recipient==12 && mixed_target.sampled_field_pass->destination==142 &&
       !mixed_target_input.sampled_field_pass,"Mixed result4 captures the observed receiver and destination");
 auto observed_target_input=observed_pass_handoff;
 observed_target_input.current.field.motion[field_motion_index(12)].x=0x8a4b;
 observed_target_input.current.field.motion[field_motion_index(12)].y=0xa45b;
 auto observed_target=capture_sampled_field_pass_target(observed_target_input,12,142,255,3);
 check(observed_target.phase==FieldSessionPhase::sampled_mixed_result4_target &&
       observed_target.sampled_field_pass->recipient==12 && observed_target.current.clock.remaining==385,
       "Post-goal result4 pass captures the observed player12 target");
 auto mixed_target_bytes=bytes(mixed_target);std::istringstream mixed_target_stream(mixed_target_bytes);
 check(static_cast<unsigned char>(mixed_target_bytes[10])==140 && bytes(load_phase_session(mixed_target_stream))==mixed_target_bytes,
       "Mixed pass target preserves snapshot version and roundtrips");
 auto wrong_mixed_target_version=mixed_target_bytes;wrong_mixed_target_version[10]=static_cast<char>(138);
 rejects([&]{std::istringstream in(wrong_mixed_target_version);load_phase_session(in);});
 auto truncated_mixed_target=mixed_target_bytes;truncated_mixed_target.pop_back();
 rejects([&]{std::istringstream in(truncated_mixed_target);load_phase_session(in);});
 auto corrupt_mixed_target=mixed_target_bytes;corrupt_mixed_target[corrupt_mixed_target.size()-4]=13;
 rejects([&]{std::istringstream in(corrupt_mixed_target);load_phase_session(in);});
 rejects([&]{capture_sampled_field_pass_target(mixed_target_input,19,142,255,3);});
 rejects([&]{capture_sampled_field_pass_target(mixed_target_input,12,141,255,3);});
 rejects([&]{capture_sampled_field_pass_target(mixed_target,12,142,255,3);});
 auto mixed_flight=begin_sampled_field_pass_flight(mixed_target,216,228,pass_geometry,pass_velocity);
 check(mixed_flight.phase==FieldSessionPhase::area_ball_advance_ready && mixed_flight.area_ai_target &&
       mixed_flight.area_ai_target->recipient==12 && mixed_flight.area_ai_target->cell==142 &&
       mixed_flight.current.field.selected_defenders==0 && mixed_flight.current.field.elapsed==0 &&
       !mixed_flight.area_defense && mixed_flight.area_sequence_result==255 && mixed_flight.saved_ball_flight_flag==1 &&
       mixed_flight.ball_trajectory && mixed_flight.ball_trajectory->x==0x70d8 && mixed_flight.ball_trajectory->y==0x8ce4,
       "Mixed pass enters numeric flight and clears the prior encounter");
 auto observed_flight=begin_sampled_field_pass_flight(observed_target,232,255,pass_geometry,pass_velocity);
 check(observed_flight.phase==FieldSessionPhase::area_ball_advance_ready && observed_flight.ball_trajectory &&
       observed_flight.ball_trajectory->x==0x6fe8 && observed_flight.ball_trajectory->y==0x8dff &&
       observed_flight.area_ai_target->recipient==12 && observed_flight.current.clock.remaining==385,
       "Post-goal result4 pass enters flight at the observed fractions");
 rejects([&]{begin_sampled_field_pass_flight(observed_target,216,228,pass_geometry,pass_velocity);});
 auto mixed_flight_bytes=bytes(mixed_flight);std::istringstream mixed_flight_stream(mixed_flight_bytes);
 check(bytes(load_phase_session(mixed_flight_stream))==mixed_flight_bytes,"Mixed pass numeric flight roundtrips");
 rejects([&]{begin_sampled_field_pass_flight(mixed_target,215,228,pass_geometry,pass_velocity);});
 auto mixed_received=mixed_flight;mixed_received.phase=FieldSessionPhase::area_pass_keeper_entry;
 mixed_received.current.field.ball_owner=12;mixed_received.current.field.ball_x=138;mixed_received.current.field.ball_y=164;mixed_received.current.field.ball_cell=142;
 mixed_received.ball_trajectory->x=0x8a78;mixed_received.ball_trajectory->y=0xa484;
 auto mixed_control=continue_sampled_field_pass_control(mixed_received);
 check(mixed_control.phase==FieldSessionPhase::player_control && mixed_control.current.field.ball_owner==12 &&
       mixed_control.current.field.ball_x==138 && mixed_control.current.field.ball_y==164 &&
       mixed_control.control->facing==64 && mixed_control.saved_ball_flight_flag==1 &&
       !mixed_control.area_ai_target && !mixed_control.shot_flight && !mixed_control.ball_trajectory,
       "Mixed result4 pass returns player12 to control without losing the flight flag");
 auto observed_received=observed_flight;observed_received.phase=FieldSessionPhase::area_pass_keeper_entry;
 observed_received.current.field.ball_owner=12;observed_received.current.field.ball_x=137;
 observed_received.current.field.ball_y=164;observed_received.current.field.ball_cell=142;
 observed_received.ball_trajectory->x=0x8908;observed_received.ball_trajectory->y=0xa4ff;
 auto observed_control=continue_sampled_field_pass_control(observed_received);
 check(observed_control.phase==FieldSessionPhase::player_control && observed_control.current.field.ball_owner==12 &&
       observed_control.current.field.ball_x==137 && observed_control.control->facing==64 &&
       observed_control.current.clock.remaining==385 && observed_control.saved_ball_flight_flag==1,
       "Post-goal mixed pass returns player12 to observed control state");
 auto observed_control_bytes=bytes(observed_control);std::istringstream observed_control_stream(observed_control_bytes);
 check(bytes(load_phase_session(observed_control_stream))==observed_control_bytes,"Post-goal mixed control roundtrips");
 auto mixed_control_bytes=bytes(mixed_control);std::istringstream mixed_control_stream(mixed_control_bytes);
 check(bytes(load_phase_session(mixed_control_stream))==mixed_control_bytes,"Mixed pass control roundtrips");
 rejects([&]{continue_sampled_field_pass_control(mixed_control);});
 auto player12_decision=mixed_control;player12_decision.phase=FieldSessionPhase::ai_cell_decision;
 player12_decision.current.field.ball_x=134;player12_decision.current.field.ball_y=160;player12_decision.current.field.ball_cell=130;player12_decision.current.field.elapsed=1;
 player12_decision.current.clock.remaining=421;player12_decision.current.clock.script_flags=128;player12_decision.current.clock.ball_mode=128;player12_decision.current.field.roster[12].type=23;player12_decision.control->display_parameter=2;player12_decision.control->controlled_player=6;player12_decision.control->last_carrier_cell=130;
 player12_decision.control->facing=0;player12_decision.control->motion_requested=1;
 auto player12_choice=capture_sampled_field_ai_choice(player12_decision,1,0);
 check(static_cast<unsigned>(player12_choice.phase)==230 && player12_choice.action->action==1 && player12_choice.recovery->attack_subtype==0,
       "Player12 captured pass choice preserves the observed action");
 auto observed_player12_decision=observed_control;observed_player12_decision.phase=FieldSessionPhase::ai_cell_decision;
 observed_player12_decision.current.field.ball_x=133;observed_player12_decision.current.field.ball_y=161;
 observed_player12_decision.current.field.ball_cell=130;observed_player12_decision.current.field.elapsed=1;
 observed_player12_decision.current.clock.remaining=384;observed_player12_decision.current.clock.script_flags=128;
 observed_player12_decision.current.clock.ball_mode=128;observed_player12_decision.current.field.roster[12].type=23;
 observed_player12_decision.control->display_parameter=2;observed_player12_decision.control->controlled_player=6;
 observed_player12_decision.control->last_carrier_cell=130;observed_player12_decision.control->facing=0;
 observed_player12_decision.control->motion_requested=1;
 auto observed_player12_choice=capture_sampled_field_ai_choice(observed_player12_decision,1,0);
 check(observed_player12_choice.phase==FieldSessionPhase::sampled_player12_ai_choice &&
       observed_player12_choice.action->action==1 && observed_player12_choice.current.clock.remaining==384,
       "Post-goal player12 retains the original pass choice");
 auto observed_player12_prepared=capture_sampled_field_ai_preparation(observed_player12_choice,128,21);
 auto observed_player12_presented=acknowledge_sampled_field_ai_presentation(observed_player12_prepared);
 auto observed_player12_dispatch=advance_sampled_field_ai_dispatch(observed_player12_presented);
 check(observed_player12_dispatch.phase==FieldSessionPhase::sampled_player12_ai_dispatch &&
       observed_player12_dispatch.control->last_carrier_cell==0 &&
       observed_player12_dispatch.current.clock.remaining==384 && observed_player12_dispatch.action->power==21,
       "Post-goal player12 pass presentation reaches the observed dispatch");
 auto observed_player12_field=observed_player12_dispatch.current.field;
 observed_player12_field.elapsed=0;observed_player12_field.movement_region=140;
 observed_player12_field.motion[field_motion_index(19)].x=0x6600;
 observed_player12_field.motion[field_motion_index(19)].y=0x8c00;
 auto observed_player12_handoff=acknowledge_sampled_field_ai_pass_presentation(observed_player12_dispatch,observed_player12_field);
 auto observed_player12_target=capture_sampled_field_pass_target(observed_player12_handoff,19,79,255,3);
 auto observed_player12_flight=begin_sampled_field_pass_flight(observed_player12_target,8,255,pass_geometry,pass_velocity);
 check(observed_player12_flight.ball_trajectory && observed_player12_flight.ball_trajectory->x==0x8508 &&
       observed_player12_flight.ball_trajectory->y==0xa1ff && observed_player12_flight.area_ai_target->recipient==19,
       "Post-goal player12 pass uses the observed target and flight fractions");
 auto observed_return=observed_player12_flight;observed_return.phase=FieldSessionPhase::area_pass_keeper_entry;
 observed_return.current.field.ball_owner=19;observed_return.current.field.ball_x=99;observed_return.current.field.ball_y=141;
 observed_return.current.field.ball_cell=79;observed_return.ball_trajectory->x=0x6364;observed_return.ball_trajectory->y=0x8dc9;
 auto observed_return_control=continue_sampled_field_pass_control(observed_return);
 check(observed_return_control.phase==FieldSessionPhase::player_control && observed_return_control.control->facing==0 &&
       observed_return_control.current.field.ball_owner==19 && observed_return_control.current.clock.remaining==384,
       "Post-goal return pass restores observed player19 control");
 auto player12_prepared=capture_sampled_field_ai_preparation(player12_choice,128,21);
 check(static_cast<unsigned>(player12_prepared.phase)==231 && player12_prepared.action->power==21 && player12_prepared.recovery->attack_subtype==128 &&
       player12_choice.action->power==24,"Player12 captured pass preparation preserves the input");
 auto player12_choice_bytes=bytes(player12_choice);std::istringstream player12_choice_stream(player12_choice_bytes);
 auto player12_prepared_bytes=bytes(player12_prepared);std::istringstream player12_prepared_stream(player12_prepared_bytes);
 check(bytes(load_phase_session(player12_choice_stream))==player12_choice_bytes && bytes(load_phase_session(player12_prepared_stream))==player12_prepared_bytes,
       "Player12 pass choice and preparation roundtrip");
 rejects([&]{capture_sampled_field_ai_choice(player12_decision,0,0);});
 rejects([&]{capture_sampled_field_ai_preparation(player12_choice,128,22);});
 rejects([&]{capture_sampled_field_ai_preparation(player12_prepared,128,21);});
 FieldDefenseContent player12_attack_content{};player12_attack_content.skill_selectors[1][0]=1;
 player12_attack_content.moves[1]={{0,0,20,0}};player12_attack_content.strength_curve.fill(21);
 auto calculated_player12=prepare_sampled_field_contact_attack(player12_choice,sampled_attack_profiles,player12_attack_content);
 check(bytes(calculated_player12)==bytes(player12_prepared) && player12_choice.current.field.roster[12].stamina==calculated_player12.current.field.roster[12].stamina,
       "Player12 attack profile calculates the observed pass power without spending opponent stamina");
 player12_attack_content.moves[1][0].cost_low=60;
 rejects([&]{prepare_sampled_field_contact_attack(player12_choice,sampled_attack_profiles,player12_attack_content);});
 auto player12_presented=acknowledge_sampled_field_ai_presentation(player12_prepared);
 check(static_cast<unsigned>(player12_presented.phase)==232 && player12_presented.current.clock.script_flags==0 &&
       player12_presented.current.clock.ball_mode==0 && player12_prepared.current.clock.script_flags==128,
       "Player12 pass presentation acknowledgement clears observed flags");
 auto player12_presented_bytes=bytes(player12_presented);std::istringstream player12_presented_stream(player12_presented_bytes);
 check(bytes(load_phase_session(player12_presented_stream))==player12_presented_bytes,"Player12 pass presentation roundtrips");
 rejects([&]{acknowledge_sampled_field_ai_presentation(player12_presented);});
 auto player12_dispatch=advance_sampled_field_ai_dispatch(player12_presented);
 check(static_cast<unsigned>(player12_dispatch.phase)==233 && player12_dispatch.control->last_carrier_cell==0 &&
       player12_presented.control->last_carrier_cell==130,"Player12 dispatch clears the carrier cell marker");
 auto player12_dispatch_bytes=bytes(player12_dispatch);std::istringstream player12_dispatch_stream(player12_dispatch_bytes);
 check(bytes(load_phase_session(player12_dispatch_stream))==player12_dispatch_bytes,"Player12 dispatch roundtrips");
 rejects([&]{advance_sampled_field_ai_dispatch(player12_dispatch);});
 auto player12_captured_field=player12_dispatch.current.field;player12_captured_field.elapsed=0;player12_captured_field.movement_region=140;
 player12_captured_field.motion[0].x+=1;player12_captured_field.motion[field_motion_index(19)].x=0x6600;player12_captured_field.motion[field_motion_index(19)].y=0x8c00;
 auto player12_pass_handoff=acknowledge_sampled_field_ai_pass_presentation(player12_dispatch,player12_captured_field);
 check(static_cast<unsigned>(player12_pass_handoff.phase)==234 && player12_pass_handoff.current.field.elapsed==0 &&
       player12_pass_handoff.current.field.motion[0].x==player12_captured_field.motion[0].x &&
       player12_pass_handoff.recovery->ball_variant==1 && player12_pass_handoff.control->facing==64 &&
       player12_pass_handoff.control->last_carrier_cell==128 && player12_pass_handoff.control->display_parameter==0 &&
       player12_pass_handoff.control->motion_requested==0,"Player12 captured pass handoff retains presentation state");
 auto player12_handoff_bytes=bytes(player12_pass_handoff);std::istringstream player12_handoff_stream(player12_handoff_bytes);
 check(bytes(load_phase_session(player12_handoff_stream))==player12_handoff_bytes,"Player12 pass handoff roundtrips");
 auto changed_player12_field=player12_captured_field;changed_player12_field.roster[1].stamina++;
 rejects([&]{acknowledge_sampled_field_ai_pass_presentation(player12_dispatch,changed_player12_field);});
 rejects([&]{acknowledge_sampled_field_ai_pass_presentation(player12_pass_handoff,player12_captured_field);});
 auto player12_target=capture_sampled_field_pass_target(player12_pass_handoff,19,79,255,3);
 check(static_cast<unsigned>(player12_target.phase)==235 && player12_target.sampled_field_pass &&
       player12_target.sampled_field_pass->recipient==19 && player12_target.sampled_field_pass->destination==79 &&
       player12_target.saved_ball_flight_flag==1,"Player12 pass captures the observed player19 target");
 auto player12_target_bytes=bytes(player12_target);std::istringstream player12_target_stream(player12_target_bytes);
 check(static_cast<unsigned char>(player12_target_bytes[10])==140 && bytes(load_phase_session(player12_target_stream))==player12_target_bytes,
       "Player12 pass target and flight flag roundtrip");
 rejects([&]{capture_sampled_field_pass_target(player12_pass_handoff,19,80,255,3);});
 rejects([&]{capture_sampled_field_pass_target(player12_target,19,79,255,3);});
 auto player12_flight=begin_sampled_field_pass_flight(player12_target,120,132,pass_geometry,pass_velocity);
 check(player12_flight.phase==FieldSessionPhase::area_ball_advance_ready && player12_flight.area_ai_target &&
       player12_flight.area_ai_target->recipient==19 && player12_flight.area_ai_target->cell==79 &&
       player12_flight.ball_trajectory && player12_flight.ball_trajectory->x==0x8678 && player12_flight.ball_trajectory->y==0xa084 &&
       player12_flight.ball_trajectory->flags==1 && !player12_flight.sampled_field_pass,
       "Player12 pass enters numeric flight toward player19");
 auto player12_flight_bytes=bytes(player12_flight);std::istringstream player12_flight_stream(player12_flight_bytes);
 check(bytes(load_phase_session(player12_flight_stream))==player12_flight_bytes,"Player12 numeric flight roundtrips");
 rejects([&]{begin_sampled_field_pass_flight(player12_target,119,132,pass_geometry,pass_velocity);});
 auto player19_received=player12_flight;player19_received.phase=FieldSessionPhase::area_pass_keeper_entry;
 player19_received.current.field.ball_owner=19;player19_received.current.field.ball_x=100;player19_received.current.field.ball_y=140;player19_received.current.field.ball_cell=79;
 player19_received.ball_trajectory->x=0x6400;player19_received.ball_trajectory->y=0x8c00;
 auto player19_control=continue_sampled_field_pass_control(player19_received);
 check(player19_control.phase==FieldSessionPhase::player_control && player19_control.current.field.ball_owner==19 &&
       player19_control.current.field.ball_x==100 && player19_control.current.field.ball_cell==79 && player19_control.control->facing==64 &&
       player19_control.saved_ball_flight_flag==1 && !player19_control.area_ai_target && !player19_control.ball_trajectory,
       "Player12 pass returns player19 to control");
 auto player19_control_bytes=bytes(player19_control);std::istringstream player19_control_stream(player19_control_bytes);
 check(bytes(load_phase_session(player19_control_stream))==player19_control_bytes,"Player19 pass control roundtrips");
 rejects([&]{continue_sampled_field_pass_control(player19_control);});
 auto player19_decision=player19_control;player19_decision.phase=FieldSessionPhase::ai_cell_decision;
 player19_decision.current.field.ball_x=93;player19_decision.current.field.ball_y=140;player19_decision.current.field.ball_cell=67;player19_decision.current.field.elapsed=1;
 player19_decision.current.field.roster[0].type=1;player19_decision.current.field.roster[0].stamina=16;player19_decision.current.field.roster[19].type=23;player19_decision.current.clock.remaining=419;player19_decision.current.clock.script_flags=128;player19_decision.current.clock.ball_mode=128;
 player19_decision.recovery->ball_variant=0;player19_decision.control->controlled_player=4;player19_decision.control->last_carrier_cell=67;player19_decision.control->facing=0;player19_decision.control->display_parameter=2;player19_decision.control->motion_requested=1;
 auto player19_shot=capture_sampled_field_ai_choice(player19_decision,0,0);
 check(static_cast<unsigned>(player19_shot.phase)==236 && player19_shot.action->action==0 && player19_shot.recovery->attack_subtype==0 && player19_shot.action->power==21,
       "Player19 captures the observed next shot choice");
 auto postgoal_player19_decision=player19_decision;
 postgoal_player19_decision.current.field.ball_x=92;postgoal_player19_decision.current.field.ball_y=141;
 postgoal_player19_decision.current.clock.remaining=382;postgoal_player19_decision.current.clock.extra_time=13;
 postgoal_player19_decision.score->left_goals=2;postgoal_player19_decision.score->right_goals=1;
 postgoal_player19_decision.current.field.roster[0].stamina=30;
 auto postgoal_player19_shot=capture_sampled_field_ai_choice(postgoal_player19_decision,0,0);
 auto postgoal_player19_prepared=capture_sampled_field_ai_preparation(postgoal_player19_shot,128,22);
 check(postgoal_player19_prepared.phase==FieldSessionPhase::sampled_player19_shot_prepared &&
       postgoal_player19_prepared.action->power==22 && postgoal_player19_prepared.current.clock.remaining==382,
       "Post-goal player19 prepares the observed shot");
 FieldDefenseContent player19_shot_content{};player19_shot_content.skill_selectors[0][0]=1;
 player19_shot_content.moves[0]={{0,0,80,0}};player19_shot_content.strength_curve.fill(22);
 auto player19_shot_prepared=prepare_sampled_field_contact_attack(player19_shot,sampled_attack_profiles,player19_shot_content);
 check(static_cast<unsigned>(player19_shot_prepared.phase)==237 && player19_shot_prepared.action->power==22 &&
       player19_shot_prepared.recovery->attack_subtype==128 && player19_shot.action->power==21,"Player19 shot power uses attack content");
 auto player19_shot_bytes=bytes(player19_shot_prepared);std::istringstream player19_shot_stream(player19_shot_bytes);
 check(bytes(load_phase_session(player19_shot_stream))==player19_shot_bytes,"Player19 shot preparation roundtrips");
 rejects([&]{capture_sampled_field_ai_choice(player19_decision,1,0);});
 player19_shot_content.moves[0][0].cost_low=79;
 rejects([&]{prepare_sampled_field_contact_attack(player19_shot,sampled_attack_profiles,player19_shot_content);});


 mixed_defense_content.strength_curve.fill(18);
 rejects([&]{prepare_sampled_mixed_defender(mixed_attack,sampled_attack_profiles,mixed_defense_content);});


 sampled_attack_content.strength_curve.fill(23);
 rejects([&]{prepare_sampled_field_contact_attack(native_attack_input,sampled_attack_profiles,sampled_attack_content);});

 auto after_contact_presentation=contact_ai_prepared.current.field;
 after_contact_presentation.elapsed=0;after_contact_presentation.movement_region=135;
 after_contact_presentation.motion[0].x+=1;
 auto contact_dispatch=acknowledge_sampled_field_ai_pass_presentation(contact_ai_prepared,after_contact_presentation);
 check(static_cast<unsigned>(contact_dispatch.phase)==208 && contact_dispatch.current.clock.remaining==439 &&
       contact_dispatch.control->last_carrier_cell==0 && contact_dispatch.current.field.motion[0].x==after_contact_presentation.motion[0].x &&
       contact_ai_prepared.current.clock.remaining==440,
       "Captured contact presentation retains movement and advances clock");
 auto contact_dispatch_bytes=bytes(contact_dispatch);std::istringstream contact_dispatch_stream(contact_dispatch_bytes);
 check(bytes(load_phase_session(contact_dispatch_stream))==contact_dispatch_bytes,"Captured contact presentation roundtrips");
 auto invalid_contact_field=after_contact_presentation;invalid_contact_field.ball_owner=18;
 rejects([&]{acknowledge_sampled_field_ai_pass_presentation(contact_ai_prepared,invalid_contact_field);});
 auto contact_control=prepare_session_control(contact_dispatch);
 check(contact_control.state.phase==FieldSessionPhase::control_ready &&
       contact_control.state.current.field.selected_defenders==0 && !contact_control.state.area_defense &&
       contact_control.state.current.clock.remaining==439 && contact_control.state.control->decision_timer==10,
       "Captured contact presentation returns to opponent field control");
 auto next_ai=contact_control.state;next_ai.phase=FieldSessionPhase::ai_cell_decision;
 next_ai.current.field.ball_x=102;next_ai.current.field.ball_y=140;next_ai.current.field.ball_cell=79;
 next_ai.current.field.motion[field_motion_index(19)].x=0x6600;next_ai.current.field.motion[field_motion_index(19)].y=0x8c00;
 next_ai.current.field.elapsed=1;next_ai.current.clock.remaining=437;next_ai.control->last_carrier_cell=79;
 next_ai.current.field.roster[19].type=23;next_ai.control->controlled_player=2;
 auto continued_ai=capture_sampled_field_ai_choice(next_ai,2,0);
 check(continued_ai.phase==FieldSessionPhase::control_clock_pending && continued_ai.action->action==2 &&
       continued_ai.recovery->attack_subtype==0 && continued_ai.current.clock.remaining==437,
       "Captured opponent dribble continues to field clock without dispatch");
 rejects([&]{capture_sampled_field_ai_choice(next_ai,1,0);});
 auto next_shot=next_ai;next_shot.current.field.ball_x=95;next_shot.current.field.ball_cell=67;next_shot.current.field.roster[0].type=1;next_shot.current.field.roster[0].stamina=18;
 next_shot.current.field.motion[field_motion_index(19)].x=0x5f00;
 next_shot.current.clock.remaining=435;next_shot.control->last_carrier_cell=67;next_shot.control->motion_requested=1;next_shot.recovery->attack_subtype=0;
 auto next_shot_choice=capture_sampled_field_ai_choice(next_shot,0,0);
 check(static_cast<unsigned>(next_shot_choice.phase)==209 && next_shot_choice.action->action==0 && next_shot_choice.action->power==24,
       "Captured opponent shot choice preserves prior strength");
 rejects([&]{capture_sampled_field_ai_choice(next_shot,1,0);});
 FieldPlayerProfiles next_shot_profiles{};next_shot_profiles.skills.resize(1);next_shot_profiles.skills[0].fill(20);
 FieldDefenseContent next_shot_content{};next_shot_content.skill_selectors[0][0]=1;
 next_shot_content.moves[0]={{0,0,80,0}};next_shot_content.strength_curve.fill(22);
 auto native_shot=prepare_sampled_field_contact_attack(next_shot_choice,next_shot_profiles,next_shot_content);
 check(static_cast<unsigned>(native_shot.phase)==210 && native_shot.action->power==22 &&
       native_shot.recovery->attack_subtype==128 && native_shot.action->action==0,
       "Opponent shot strength is derived from the player profile");
 auto presented_shot=acknowledge_sampled_field_ai_presentation(native_shot);
 check(static_cast<unsigned>(presented_shot.phase)==211 && !presented_shot.current.clock.script_flags &&
       !presented_shot.current.clock.ball_mode && presented_shot.action->power==22,
       "Acknowledged opponent shot presentation clears original script flags");
 auto shot_clock=advance_sampled_field_ai_dispatch(presented_shot);
 check(static_cast<unsigned>(shot_clock.phase)==212 && shot_clock.current.clock.remaining==434 &&
       shot_clock.current.field.elapsed==2 && shot_clock.control->last_carrier_cell==0 &&
       presented_shot.current.clock.remaining==435,
       "Opponent shot dispatch advances the original clock boundary");
 auto shot_field=shot_clock.current.field;shot_field.elapsed=0;shot_field.movement_region=134;shot_field.motion[0].x+=1;
 auto shot_approach=acknowledge_sampled_field_ai_pass_presentation(shot_clock,shot_field);
 check(static_cast<unsigned>(shot_approach.phase)==213 && shot_approach.current.clock.remaining==434 &&
       shot_approach.current.field.motion[0].x==shot_field.motion[0].x && shot_approach.current.field.elapsed==0,
       "Captured opponent shot approach preserves clock and observed motion");
 auto shot_approach_bytes=bytes(shot_approach);std::istringstream shot_approach_stream(shot_approach_bytes);
 check(bytes(load_phase_session(shot_approach_stream))==shot_approach_bytes,"Captured shot approach roundtrips");
 auto shot_handoff=acknowledge_sampled_field_ai_pass_handoff(shot_approach);
 check(static_cast<unsigned>(shot_handoff.phase)==214 && shot_handoff.recovery->ball_variant==1 &&
       shot_handoff.control->last_carrier_cell==128 && shot_handoff.control->display_parameter==0 &&
       shot_handoff.control->motion_requested==2,"Opponent shot handoff retains the observed control state");
 auto keeper_field=shot_handoff.current.field;keeper_field.ball_x=52;keeper_field.ball_y=124;
 keeper_field.ball_cell=5;keeper_field.selected_defenders=1;
 auto keeper_prompt=capture_sampled_shot_keeper_prompt(shot_handoff,keeper_field);
 check(static_cast<unsigned>(keeper_prompt.phase)==219 && keeper_prompt.current.field.ball_cell==5 &&
       keeper_prompt.scheduling->selected_ids[0]==0 && keeper_prompt.action->menu_route==4 &&
       keeper_prompt.control->motion_requested==0 && keeper_prompt.recovery->x_jitter_control==142,
       "Captured shot arrival reaches the home goalkeeper choice");
 auto keeper_prompt_bytes=bytes(keeper_prompt);std::istringstream keeper_prompt_stream(keeper_prompt_bytes);
 check(bytes(load_phase_session(keeper_prompt_stream))==keeper_prompt_bytes,"Sampled goalkeeper prompt roundtrips");
 auto invalid_keeper_field=keeper_field;invalid_keeper_field.ball_owner=18;
 rejects([&]{capture_sampled_shot_keeper_prompt(shot_handoff,invalid_keeper_field);});
 auto goalkeeper_choice=poll_sampled_goalkeeper_prompt(keeper_prompt,1,{23,22});
 check(static_cast<unsigned>(goalkeeper_choice.state.phase)==220 && goalkeeper_choice.state.keeper_menu_action==0 &&
       goalkeeper_choice.state.action->action==0 && goalkeeper_choice.state.action->cursor==224 &&
       goalkeeper_choice.state.action->flash_counter==13 && goalkeeper_choice.presentation.size()==1 &&
       goalkeeper_choice.presentation[0].kind==ControlPresentationKind::sprite && goalkeeper_choice.presentation[0].id==23,
       "Goalkeeper direction selects a defense action without replacing the attacker action");
 auto goalkeeper_confirmed=poll_sampled_goalkeeper_prompt(goalkeeper_choice.state,128,{23,22});
 check(static_cast<unsigned>(goalkeeper_confirmed.state.phase)==221 && goalkeeper_confirmed.confirmed &&
       goalkeeper_confirmed.state.keeper_menu_action==0 && goalkeeper_confirmed.presentation.empty(),
       "Keeper confirmation retains the chosen action before clearing the overlay");
 auto goalkeeper_confirmed_bytes=bytes(goalkeeper_confirmed.state);std::istringstream goalkeeper_confirmed_stream(goalkeeper_confirmed_bytes);
 check(bytes(load_phase_session(goalkeeper_confirmed_stream))==goalkeeper_confirmed_bytes,"Keeper confirmation roundtrips");
 auto keeper_precompare=acknowledge_sampled_goalkeeper_overlay(goalkeeper_confirmed.state);
 check(static_cast<unsigned>(keeper_precompare.state.phase)==222 && keeper_precompare.state.keeper_menu_action==0 &&
       keeper_precompare.presentation.size()==1 && keeper_precompare.presentation[0].kind==ControlPresentationKind::clear_overlay,
       "Keeper confirmation advances to comparison after overlay acknowledgement");
 ShotContent keeper_test_content{};keeper_test_content.goalkeepers.skills.resize(1);
 keeper_test_content.goalkeepers.skills[0].fill(20);keeper_test_content.keeper.skill_selectors[0]=23;
 keeper_test_content.keeper.moves[0]={{0,0,20,0}};keeper_test_content.keeper.strength_curve.fill(10);
 keeper_test_content.keeper_range=128;keeper_test_content.keeper_offset=128;keeper_test_content.keeper_thresholds={192,128,64,0};
 auto new_shot_presented=acknowledge_sampled_field_ai_presentation(player19_shot_prepared);
 auto postgoal_shot_presented=acknowledge_sampled_field_ai_presentation(postgoal_player19_prepared);
 auto postgoal_shot_clock=advance_sampled_field_ai_dispatch(postgoal_shot_presented);
 auto postgoal_shot_field=postgoal_shot_clock.current.field;postgoal_shot_field.elapsed=0;postgoal_shot_field.movement_region=134;
 auto postgoal_shot_approach=acknowledge_sampled_field_ai_pass_presentation(postgoal_shot_clock,postgoal_shot_field);
 auto postgoal_shot_handoff=acknowledge_sampled_field_ai_pass_handoff(postgoal_shot_approach);
 check(postgoal_shot_handoff.phase==FieldSessionPhase::sampled_opponent_shot_handoff && postgoal_shot_handoff.current.clock.remaining==381 &&
       postgoal_shot_handoff.control->motion_requested==2,"Post-goal player19 shot reaches the observed handoff");
 auto postgoal_keeper_field=postgoal_shot_handoff.current.field;postgoal_keeper_field.ball_x=52;postgoal_keeper_field.ball_y=124;
 postgoal_keeper_field.ball_cell=5;postgoal_keeper_field.selected_defenders=1;
 auto postgoal_keeper_prompt=capture_sampled_shot_keeper_prompt(postgoal_shot_handoff,postgoal_keeper_field);
 check(postgoal_keeper_prompt.phase==FieldSessionPhase::sampled_shot_keeper_prompt && postgoal_keeper_prompt.recovery->x_jitter_control==144 &&
       postgoal_keeper_prompt.current.clock.remaining==381 && postgoal_keeper_prompt.saved_ball_flight_flag==0,
       "Post-goal shot reaches the keeper prompt with its original cleared flight flag");
 auto postgoal_keeper_choice=poll_sampled_goalkeeper_prompt(postgoal_keeper_prompt,1,{23,22});
 auto postgoal_keeper_confirm=poll_sampled_goalkeeper_prompt(postgoal_keeper_choice.state,128,{23,22});
 auto postgoal_keeper_precompare=acknowledge_sampled_goalkeeper_overlay(postgoal_keeper_confirm.state);
 check(postgoal_keeper_precompare.state.phase==FieldSessionPhase::sampled_shot_keeper_precompare &&
       postgoal_keeper_precompare.state.keeper_menu_action==0 && postgoal_keeper_precompare.state.saved_ball_flight_flag==0,
       "Post-goal keeper choice reaches the original comparison boundary");
 auto postgoal_keeper_snapshot=bytes(postgoal_keeper_precompare.state);std::istringstream postgoal_keeper_stream(postgoal_keeper_snapshot);
 check(bytes(load_phase_session(postgoal_keeper_stream))==postgoal_keeper_snapshot,"Cleared flight flag survives selected keeper snapshot");
 auto postgoal_keeper_result=compare_sampled_home_goalkeeper(postgoal_keeper_precompare.state,keeper_test_content);
 check(postgoal_keeper_result.phase==FieldSessionPhase::sampled_shot_keeper_compared && postgoal_keeper_result.current.field.roster[0].stamina==10,
       "Post-goal keeper spends the observed 20 stamina");
 auto new_shot_clock=advance_sampled_field_ai_dispatch(new_shot_presented);
 check(new_shot_presented.phase==FieldSessionPhase::sampled_opponent_shot_presented &&
       new_shot_clock.phase==FieldSessionPhase::sampled_opponent_shot_clock && new_shot_clock.current.clock.remaining==418 &&
       new_shot_clock.current.clock.extra_time==9 && new_shot_clock.current.field.elapsed==2,"New sampled shot advances the clock");
 auto new_shot_field=new_shot_clock.current.field;new_shot_field.elapsed=0;new_shot_field.movement_region=134;new_shot_field.motion[0].x+=1;
 auto new_shot_approach=acknowledge_sampled_field_ai_pass_presentation(new_shot_clock,new_shot_field);
 auto new_shot_handoff=acknowledge_sampled_field_ai_pass_handoff(new_shot_approach);
 auto new_keeper_field=new_shot_handoff.current.field;new_keeper_field.ball_x=52;new_keeper_field.ball_y=124;
 new_keeper_field.ball_cell=5;new_keeper_field.selected_defenders=1;
 auto new_keeper_prompt=capture_sampled_shot_keeper_prompt(new_shot_handoff,new_keeper_field);
 auto new_keeper_choice=poll_sampled_goalkeeper_prompt(new_keeper_prompt,1,{23,22});
 auto new_keeper_confirm=poll_sampled_goalkeeper_prompt(new_keeper_choice.state,128,{23,22});
 auto new_keeper_precompare=acknowledge_sampled_goalkeeper_overlay(new_keeper_confirm.state);
 auto new_keeper_result=compare_sampled_home_goalkeeper(new_keeper_precompare.state,keeper_test_content);
 check(new_keeper_result.phase==FieldSessionPhase::sampled_shot_keeper_compared && new_keeper_result.current.field.roster[0].stamina==0 &&
       new_keeper_precompare.state.current.field.roster[0].stamina==16 && new_keeper_result.current.clock.remaining==418,
       "New sampled shot reaches and resolves the home keeper selection");
 auto new_keeper_bytes=bytes(new_keeper_result);std::istringstream new_keeper_stream(new_keeper_bytes);
 check(bytes(load_phase_session(new_keeper_stream))==new_keeper_bytes,"New sampled keeper result roundtrips");
 auto alternate_choice=poll_sampled_goalkeeper_prompt(new_keeper_prompt,8,{23,22});
 auto alternate_confirm=poll_sampled_goalkeeper_prompt(alternate_choice.state,128,{23,22});
 auto unverified_alternate=alternate_confirm.state;unverified_alternate.current.field.roster[0].stamina=17;
 rejects([&]{acknowledge_sampled_goalkeeper_overlay(unverified_alternate);});
 auto alternate_precompare=acknowledge_sampled_goalkeeper_overlay(alternate_confirm.state);
 auto alternate_content=keeper_test_content;alternate_content.keeper.skill_selectors[1]=23;
 alternate_content.keeper.moves[1]={{0,0,40,0}};alternate_content.keeper.strength_curve.fill(12);
 auto alternate_result=compare_sampled_home_goalkeeper(alternate_precompare.state,alternate_content);
 check(alternate_result.phase==FieldSessionPhase::sampled_shot_keeper_compared && alternate_result.keeper_menu_action==1 &&
       alternate_result.current.field.roster[0].stamina==0 && alternate_precompare.state.current.field.roster[0].stamina==16,
       "Alternate sampled goalkeeper action uses cost40 and exhausts stamina");
 auto alternate_bytes=bytes(alternate_result);std::istringstream alternate_stream(alternate_bytes);
 check(bytes(load_phase_session(alternate_stream))==alternate_bytes,"Alternate goalkeeper result roundtrips");
 auto keeper_result=compare_sampled_home_goalkeeper(keeper_precompare.state,keeper_test_content);
 check(static_cast<unsigned>(keeper_result.phase)==223 && keeper_result.current.field.roster[0].stamina==0 &&
       keeper_result.keeper_menu_action==0 && keeper_precompare.state.current.field.roster[0].stamina==18,
       "Sampled home keeper comparison consumes insufficient stamina");
 auto keeper_result_bytes=bytes(keeper_result);std::istringstream keeper_result_stream(keeper_result_bytes);
 check(bytes(load_phase_session(keeper_result_stream))==keeper_result_bytes,"Home keeper result roundtrips");

 auto opponent_goal_input=keeper_result;opponent_goal_input.experience->totals[0]=20;
 keeper_test_content.experience=ExperienceContent{};keeper_test_content.experience->stage_bases[0]=60;
 keeper_test_content.experience->reward_factors[0]=64;
 auto postgoal_equalizer=prepare_sampled_opponent_goal(postgoal_keeper_result,keeper_test_content);
 auto postgoal_equalizer_presented=acknowledge_sampled_opponent_goal_presentation(postgoal_equalizer);
 auto postgoal_equalizer_restart=begin_sampled_opponent_goal_restart(postgoal_equalizer_presented);
 check(postgoal_equalizer_restart.score->left_goals==2 && postgoal_equalizer_restart.score->right_goals==2 &&
       postgoal_equalizer_restart.current.field.roster[0].stamina==10,"Post-goal shot reaches the observed 2-2 restart");
 auto equalizer_formation=prepare_session_kickoff_formation(postgoal_equalizer_restart,0,c);
 auto equalizer_receiver=prepare_session_kickoff_receiver(equalizer_formation,0,{5,6,7,9,10});
 auto equalizer_ball=prepare_session_kickoff_ball(equalizer_receiver);
 auto equalizer_control=continue_prepared_session_kickoff(equalizer_ball);
 check(equalizer_control.current.field.ball_owner==5 && equalizer_control.control->facing==0 &&
       equalizer_control.current.clock.remaining==381 && equalizer_control.saved_ball_flight_flag==0,
       "The 2-2 restart reaches the observed home kickoff handoff with the original flight flag");
 alternate_content.experience=keeper_test_content.experience;
 auto alternate_goal=prepare_sampled_opponent_goal(alternate_result,alternate_content);
 check(alternate_goal.phase==FieldSessionPhase::sampled_opponent_goal_scored && alternate_goal.keeper_menu_action==1 &&
       alternate_goal.score->right_goals==1,"Alternate keeper action leads to the observed goal");
 auto new_keeper_goal=prepare_sampled_opponent_goal(new_keeper_result,keeper_test_content);
 check(new_keeper_goal.phase==FieldSessionPhase::sampled_opponent_goal_scored && new_keeper_goal.score->left_goals==1 &&
       new_keeper_goal.score->right_goals==1 && new_keeper_goal.current.clock.remaining==418 && new_keeper_goal.current.clock.extra_time==9,
       "New sampled goalkeeper result records the observed equalizing goal");
 auto new_keeper_goal_presented=acknowledge_sampled_opponent_goal_presentation(new_keeper_goal);
 auto new_keeper_restart=begin_sampled_opponent_goal_restart(new_keeper_goal_presented);
 check(new_keeper_restart.phase==FieldSessionPhase::sampled_opponent_goal_restart && new_keeper_restart.current.field.side==0 &&
       new_keeper_restart.score->right_goals==1,"New sampled goal reaches the home restart");
 auto new_keeper_restart_bytes=bytes(new_keeper_restart);std::istringstream new_keeper_restart_stream(new_keeper_restart_bytes);
 check(bytes(load_phase_session(new_keeper_restart_stream))==new_keeper_restart_bytes,"New sampled goal restart roundtrips");
 auto later_kickoff=prepare_session_kickoff_formation(new_keeper_restart,0,c);
 auto later_receiver=prepare_session_kickoff_receiver(later_kickoff,0,{5,6,7,9,10});
 auto later_ball=prepare_session_kickoff_ball(later_receiver);
 auto later_handoff=continue_prepared_session_kickoff(later_ball);
 check(later_handoff.phase==FieldSessionPhase::kickoff_control_pending && later_handoff.current.field.ball_owner==5 &&
       later_handoff.control->facing==0 && later_handoff.current.clock.remaining==418 && later_handoff.current.clock.extra_time==9,
       "Later sampled equalizer returns the home side to kickoff control");
 auto later_handoff_bytes=bytes(later_handoff);std::istringstream later_handoff_stream(later_handoff_bytes);
 check(bytes(load_phase_session(later_handoff_stream))==later_handoff_bytes,"Later kickoff handoff roundtrips");
 auto home_shot_input=later_handoff;home_shot_input.kickoff_receiver.reset();home_shot_input.phase=FieldSessionPhase::attack_prepared;
 home_shot_input.current.field.ball_owner=5;home_shot_input.current.field.ball_x=108;home_shot_input.current.field.ball_y=108;home_shot_input.current.field.ball_cell=87;home_shot_input.current.field.elapsed=1;
 home_shot_input.current.field.selected_defenders=1;home_shot_input.scheduling->selected_ids={17,0,0,0,0};
 home_shot_input.current.field.roster[5].type=23;home_shot_input.current.field.roster[5].stamina=15;home_shot_input.current.field.roster[17].type=23;home_shot_input.current.field.roster[17].stamina=100;
 home_shot_input.current.clock.remaining=411;home_shot_input.action->action=0;home_shot_input.action->menu_route=0;home_shot_input.action->cursor=224;home_shot_input.action->flash_counter=13;home_shot_input.action->power=22;
 home_shot_input.recovery->attack_subtype=128;home_shot_input.recovery->ball_variant=0;home_shot_input.control->last_carrier_cell=128;home_shot_input.control->facing=0;home_shot_input.control->display_parameter=0;home_shot_input.control->motion_requested=0;
 auto home_shot_defender=capture_sampled_home_shot_defender(home_shot_input,1,0);
 check(static_cast<unsigned>(home_shot_defender.phase)==238 && home_shot_defender.recovery->opposing_player==17 && home_shot_defender.area_defense &&
       home_shot_defender.area_defense->actions==std::array<std::uint8_t,4>{1,6,6,255} && !home_shot_input.area_defense,
       "Captured opponent17 action preserves the prepared home shot");
 auto home_defender_bytes=bytes(home_shot_defender);std::istringstream home_defender_stream(home_defender_bytes);
 check(bytes(load_phase_session(home_defender_stream))==home_defender_bytes,"Captured home shot defender roundtrips");
 rejects([&]{capture_sampled_home_shot_defender(home_shot_input,0,0);});
 rejects([&]{capture_sampled_home_shot_defender(home_shot_defender,1,0);});
 FieldDefenseContent home_defense{};home_defense.skill_selectors[1][0]=1;home_defense.moves[1]={{0,0,60,0}};home_defense.strength_curve.fill(18);
 FieldComparisonContent home_comparison{};home_comparison.variation={{{153,0}}};home_comparison.eligibility={0};home_comparison.thresholds={192,128,64,0};
 auto home_defender_result=resolve_sampled_home_shot_defender(home_shot_defender,sampled_attack_profiles,home_defense,home_comparison);
 check(static_cast<unsigned>(home_defender_result.phase)==239 && home_defender_result.area_sequence_result==0 &&
       home_defender_result.current.field.roster[17].stamina==100 && home_defender_result.action->power==22 &&
       home_defender_result.recovery->ball_variant==1 && home_defender_result.control->motion_requested==2,
       "Opponent17 defender comparison leaves the home shot unchanged");
 auto home_result_bytes=bytes(home_defender_result);std::istringstream home_result_stream(home_result_bytes);
 check(bytes(load_phase_session(home_result_stream))==home_result_bytes,"Home defender comparison roundtrips");
 home_defense.moves[1][0].cost_low=59;
 rejects([&]{resolve_sampled_home_shot_defender(home_shot_defender,sampled_attack_profiles,home_defense,home_comparison);});
 rejects([&]{resolve_sampled_home_shot_defender(home_defender_result,sampled_attack_profiles,home_defense,home_comparison);});
 ContactPreludeContent home_prelude{};home_prelude.field_elapsed[3]=1;home_prelude.defender_status[1]=6;
 ExperienceContent home_experience{};home_experience.stage_bases[0]=60;home_experience.reward_factors[1]=13;
 auto home_contact_clock=advance_sampled_home_shot_contact(home_defender_result,home_prelude,home_experience);
 check(static_cast<unsigned>(home_contact_clock.phase)==240 && home_contact_clock.current.clock.remaining==410 &&
       home_contact_clock.current.field.elapsed==2 && home_contact_clock.current.field.motion[field_motion_index(17)].status==6 &&
       home_contact_clock.experience->totals[0]==home_defender_result.experience->totals[0]+3 && home_contact_clock.control->facing==64 &&
       home_contact_clock.control->motion_requested==0 && home_defender_result.current.clock.remaining==411,
       "Home contact applies defender status, experience and the first clock tick");
 auto home_contact_clock_bytes=bytes(home_contact_clock);std::istringstream home_contact_clock_stream(home_contact_clock_bytes);
 check(bytes(load_phase_session(home_contact_clock_stream))==home_contact_clock_bytes,"Home contact clock roundtrips");
 home_prelude.field_elapsed[3]=2;
 rejects([&]{advance_sampled_home_shot_contact(home_defender_result,home_prelude,home_experience);});
 rejects([&]{advance_sampled_home_shot_contact(home_contact_clock,home_prelude,home_experience);});
 auto home_dispatch_clock=advance_sampled_home_shot_dispatch(home_contact_clock);
 check(static_cast<unsigned>(home_dispatch_clock.phase)==241 && home_dispatch_clock.current.clock.remaining==408 &&
       home_dispatch_clock.current.field.elapsed==4 && home_dispatch_clock.area_defender_cursor==1 && home_contact_clock.current.clock.remaining==410,
       "Home shot dispatch advances the original two-tick clock");
 auto captured_home_shot=home_dispatch_clock.current.field;captured_home_shot.elapsed=0;captured_home_shot.movement_region=129;captured_home_shot.motion[0].x+=1;
 auto home_shot_handoff=capture_sampled_home_shot_handoff(home_dispatch_clock,captured_home_shot);
 check(static_cast<unsigned>(home_shot_handoff.phase)==242 && home_shot_handoff.current.field.elapsed==0 &&
       home_shot_handoff.current.field.motion[0].x==captured_home_shot.motion[0].x && home_shot_handoff.current.clock.remaining==408,
       "Home shot handoff retains captured presentation movement");
 auto home_handoff_bytes=bytes(home_shot_handoff);std::istringstream home_handoff_stream(home_handoff_bytes);
 check(bytes(load_phase_session(home_handoff_stream))==home_handoff_bytes,"Home shot handoff roundtrips");
 auto changed_home_shot=captured_home_shot;changed_home_shot.roster[2].stamina++;
 rejects([&]{capture_sampled_home_shot_handoff(home_dispatch_clock,changed_home_shot);});
 rejects([&]{advance_sampled_home_shot_dispatch(home_dispatch_clock);});
 ShotApproachContent home_shot_approach{};home_shot_approach.outer_distance.fill(12);home_shot_approach.inner_distance.fill(12);
 auto home_shot_target=prepare_sampled_home_shot_target(home_shot_handoff,home_shot_approach);
 check(static_cast<unsigned>(home_shot_target.phase)==243 && home_shot_target.shot_flight &&
       home_shot_target.shot_flight->distance==108 && home_shot_target.shot_flight->destination==233 &&
       home_shot_target.shot_flight->counter==255 && !home_shot_target.shot_flight->loose_ball,
       "Home shot target is calculated from the observed field state");
 auto home_target_bytes=bytes(home_shot_target);std::istringstream home_target_stream(home_target_bytes);
 check(bytes(load_phase_session(home_target_stream))==home_target_bytes,"Home shot target roundtrips");
 auto home_numeric=begin_prepared_session_ball_flight(home_shot_target,116,92,pass_geometry,pass_velocity);
 check(home_numeric.phase==FieldSessionPhase::ball_advance_ready && home_numeric.ball_trajectory &&
       home_numeric.ball_trajectory->x==0x6c74 && home_numeric.ball_trajectory->y==0x6c5c &&
       home_numeric.saved_ball_flight_flag==0 && home_numeric.current.field.selected_defenders==0 &&
       !home_numeric.area_defense && home_numeric.area_defender_cursor==0 && home_numeric.area_sequence_result==0,
       "Home shot enters numeric flight with the observed encounter cleanup");
 auto home_numeric_bytes=bytes(home_numeric);std::istringstream home_numeric_stream(home_numeric_bytes);
 check(bytes(load_phase_session(home_numeric_stream))==home_numeric_bytes,"Home numeric flight roundtrips");
 rejects([&]{begin_prepared_session_ball_flight(home_shot_target,115,92,pass_geometry,pass_velocity);});
 // The constructed original trace reaches the opposing keeper after this flight.
 auto home_arrival=home_numeric;home_arrival.phase=FieldSessionPhase::ball_arrival_callback;
 home_arrival.current.field.ball_x=204;home_arrival.current.field.ball_y=124;home_arrival.current.field.ball_cell=233;
 home_arrival.ball_trajectory->x=52367;home_arrival.ball_trajectory->y=31771;
 auto home_keeper_entry=prepare_session_keeper_entry(continue_session_unopposed_arrival(home_arrival));
 auto home_keeper_content=alternate_content;home_keeper_content.goalkeepers.team_types={0};
 auto home_keeper_result=resolve_prepared_session_keeper(home_keeper_entry,1,0,3,home_keeper_content);
 check(home_keeper_result.keeper_decision && home_keeper_result.keeper_decision->result_code==0 &&
       home_keeper_result.current.field.roster[11].stamina==home_keeper_entry.current.field.roster[11].stamina,
       "Home shot flight reaches the sampled opposing keeper result");
 auto home_goal=prepare_session_scored_goal(home_keeper_result,{1,1,4},alternate_content);
 check(home_goal.phase==FieldSessionPhase::goal_celebration_pending && home_goal.score->left_goals==2 &&
       home_goal.score->right_goals==1 && home_goal.experience->totals[0]==home_keeper_result.experience->totals[0]+15,
       "Sampled home shot goal increments the score and awards experience");
 auto home_restart=continue_prepared_goal_restart(home_goal);
 check(home_restart.phase==FieldSessionPhase::goal_kickoff_pending && home_restart.current.field.side==11 &&
       home_restart.score->left_goals==2 && home_restart.score->right_goals==1 && home_restart.score->kickoff==4 &&
       !home_restart.shot_flight && !home_restart.ball_trajectory && !home_restart.keeper_decision,
       "Sampled home goal hands the restart to the opponent");
 auto home_restart_formation=prepare_session_kickoff_formation(home_restart,11,c);
 check(home_restart.current.field.motion[field_motion_index(17)].status==6 &&
       home_restart_formation.current.field.motion[field_motion_index(17)].status==0 &&
       home_restart_formation.current.field.ball_owner==19,
       "Sampled goal restart clears the defender status before kickoff formation");
 auto varied_restart=home_restart;
 for(auto& player:varied_restart.current.field.motion)player.status=3;
 varied_restart.current.field.motion[field_motion_index(17)].status=6;
 varied_restart.current.field.keepers[0].stun=4;
 auto varied_formation=prepare_session_kickoff_formation(varied_restart,11,c);
 for(const auto& player:varied_formation.current.field.motion)
  check(player.status==0,"Observed restart resets all field-player statuses");
 check(varied_formation.current.field.keepers[0].stun==0,"Observed restart resets the home keeper status");
 auto other_restart=home_restart;other_restart.score->left_goals=3;
 auto other_formation=prepare_session_kickoff_formation(other_restart,11,c);
 check(other_formation.current.field.motion[field_motion_index(17)].status==6,
       "Unobserved score does not use the sampled status reset");
 home_shot_approach.outer_distance.fill(11);
 rejects([&]{prepare_sampled_home_shot_target(home_shot_handoff,home_shot_approach);});
 rejects([&]{prepare_sampled_home_shot_target(home_shot_target,home_shot_approach);});
 auto opponent_goal=prepare_sampled_opponent_goal(opponent_goal_input,keeper_test_content);
 check(static_cast<unsigned>(opponent_goal.phase)==224 && opponent_goal.score->left_goals==1 &&
       opponent_goal.score->right_goals==1 && opponent_goal.experience->totals[0]==35 &&
       opponent_goal.control->facing==64 && opponent_goal.control->motion_requested==3 &&
       opponent_goal.recovery->opposing_player==1 && opponent_goal_input.score->right_goals==0,
       "Opponent goal awards the scorer and increments the correct side");
 auto opponent_goal_presented=acknowledge_sampled_opponent_goal_presentation(opponent_goal);
 check(static_cast<unsigned>(opponent_goal_presented.phase)==225 && opponent_goal_presented.current.field.side==0 &&
       opponent_goal_presented.current.field.ball_owner==19 && opponent_goal_presented.control->motion_requested==0 &&
       opponent_goal.current.field.side==11,"Opponent goal presentation hands the restart to the home side");
 auto opponent_restart=begin_sampled_opponent_goal_restart(opponent_goal_presented);
 check(static_cast<unsigned>(opponent_restart.phase)==226 && opponent_restart.current.field.side==0 &&
       opponent_restart.current.field.keepers[0].timer==0 && opponent_restart.current.field.keepers[0].penalty==0 &&
       opponent_restart.current.field.keepers[0].stun==0 && opponent_restart.score->right_goals==1,
       "Opponent goal restart clears the home keeper status before kickoff");
 auto opponent_restart_bytes=bytes(opponent_restart);std::istringstream opponent_restart_stream(opponent_restart_bytes);
 check(bytes(load_phase_session(opponent_restart_stream))==opponent_restart_bytes,"Opponent goal restart roundtrips");

 auto drawn_kickoff=prepare_session_kickoff_formation(opponent_restart,0,c);
 check(drawn_kickoff.phase==FieldSessionPhase::kickoff_formation_placed && drawn_kickoff.current.field.ball_owner==8 &&
       drawn_kickoff.current.field.side==0 && drawn_kickoff.current.field.selected_defenders==1 &&
       !drawn_kickoff.keeper_menu_action && drawn_kickoff.score->right_goals==1,
       "Drawn match restart places the home kickoff formation");
 auto drawn_kickoff_bytes=bytes(drawn_kickoff);std::istringstream drawn_kickoff_stream(drawn_kickoff_bytes);
 check(bytes(load_phase_session(drawn_kickoff_stream))==drawn_kickoff_bytes,"Drawn kickoff formation roundtrips");
 auto drawn_receiver=prepare_session_kickoff_receiver(drawn_kickoff,0,{5,6,7,9,10});
 auto drawn_ball=prepare_session_kickoff_ball(drawn_receiver);
 check(drawn_receiver.kickoff_receiver==5 && drawn_ball.phase==FieldSessionPhase::kickoff_ball_placed &&
       drawn_ball.current.field.ball_owner==8,"Drawn kickoff selects the observed receiver and places the ball");
 auto drawn_handoff=continue_prepared_session_kickoff(drawn_ball);
 check(drawn_handoff.phase==FieldSessionPhase::kickoff_control_pending && drawn_handoff.current.field.ball_owner==5 &&
       drawn_handoff.control->controlled_player==1 && drawn_handoff.control->facing==0,
       "Drawn kickoff presentation hands possession to the selected receiver");









 auto goalkeeper_choice_bytes=bytes(goalkeeper_choice.state);std::istringstream goalkeeper_choice_stream(goalkeeper_choice_bytes);
 check(static_cast<unsigned char>(goalkeeper_choice_bytes[10])==139 && bytes(load_phase_session(goalkeeper_choice_stream))==goalkeeper_choice_bytes,
       "Goalkeeper selection uses a dedicated snapshot and roundtrips");
 auto truncated_goalkeeper=goalkeeper_choice_bytes;truncated_goalkeeper.pop_back();
 rejects([&]{std::istringstream bad(truncated_goalkeeper);load_phase_session(bad);});
 auto corrupt_goalkeeper=goalkeeper_choice_bytes;corrupt_goalkeeper.back()=2;
 rejects([&]{std::istringstream bad(corrupt_goalkeeper);load_phase_session(bad);});
 auto old_goalkeeper=goalkeeper_choice_bytes;old_goalkeeper[10]=static_cast<char>(133);
 rejects([&]{std::istringstream bad(old_goalkeeper);load_phase_session(bad);});

 auto goalkeeper_wait=poll_sampled_goalkeeper_prompt(keeper_prompt,128,{23,22});
 check(bytes(goalkeeper_wait.state)==keeper_prompt_bytes && goalkeeper_wait.presentation.empty(),
       "Confirm without a goalkeeper selection keeps the prompt waiting");
 auto goalkeeper_second=poll_sampled_goalkeeper_prompt(keeper_prompt,2,{23,22});
 auto goalkeeper_other=poll_sampled_goalkeeper_prompt(keeper_prompt,8,{23,22});
 check(goalkeeper_second.state.keeper_menu_action==0 && goalkeeper_second.state.action->cursor==225 &&
       goalkeeper_other.state.keeper_menu_action==1 && goalkeeper_other.state.action->cursor==227 &&
       goalkeeper_other.presentation[0].id==22,"Keeper choices retain distinct action and cursor values");
 auto goalkeeper_inactive=poll_sampled_goalkeeper_prompt(keeper_prompt,4,{23,22});
 check(bytes(goalkeeper_inactive.state)==keeper_prompt_bytes,"Unavailable keeper direction leaves the prompt unchanged");

 rejects([&]{poll_sampled_goalkeeper_prompt(keeper_prompt,129,{23,22});});





























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
 auto rejected_shot=confirmed;rejected_shot.score=MatchScoreContext{2,2,4};
 rejected_shot.saved_ball_flight_flag=0;
 rejected_shot.current.clock.remaining=381;rejected_shot.current.clock.extra_time=13;
 rejected_shot.current.field.ball_owner=5;rejected_shot.current.field.roster[5].type=23;
 rejected_shot.current.field.ball_x=108;rejected_shot.current.field.ball_y=108;rejected_shot.current.field.ball_cell=87;
 rejected_shot.current.field.selected_defenders=0;rejected_shot.current.field.random_first=0;rejected_shot.current.field.random_second=0;
 rejected_shot.action->cursor=224;rejected_shot.action->menu_route=0;
 rejected_shot.current.field.roster[5].stamina=31;rejected_shot.recovery->attack_subtype=128;
 FieldDefenseContent rejected_content{};rejected_content.skill_selectors[0][0]=1;
 rejected_content.moves[0]={{0,0,80,0}};rejected_content.strength_curve.fill(22);
 auto returned_menu=acknowledge_sampled_unaffordable_shot(rejected_shot,sampled_attack_profiles,rejected_content);
 check(returned_menu.phase==FieldSessionPhase::human_action_menu && returned_menu.action->cursor==0 &&
       returned_menu.recovery->attack_subtype==0 && returned_menu.current.field.roster[5].stamina==31,
       "Unaffordable 2-2 kickoff shot returns to the menu without charging stamina");
 auto returned_bytes=bytes(returned_menu);std::istringstream returned_stream(returned_bytes);
 check(bytes(load_phase_session(returned_stream))==returned_bytes,"Unaffordable shot menu return roundtrips");
 auto different_stamina=rejected_shot;different_stamina.current.field.roster[5].stamina=80;
 rejects([&]{acknowledge_sampled_unaffordable_shot(different_stamina,sampled_attack_profiles,rejected_content);});
 auto different_cost=rejected_content;different_cost.moves[0][0].cost_low=79;
 rejects([&]{acknowledge_sampled_unaffordable_shot(rejected_shot,sampled_attack_profiles,different_cost);});
 ActionMenuContent pass_menu_content{};pass_menu_content.directions[0][1]=1;
 auto pass_selected=poll_human_action_menu(returned_menu,2,pass_menu_content).state;
 auto pass_confirmed=poll_human_action_menu(pass_selected,128,pass_menu_content).state;
 auto general_pass_input=pass_confirmed;
 general_pass_input.current.field.motion[field_motion_index(5)].x=0x7f32;
 general_pass_input.current.field.motion[field_motion_index(5)].y=0x8467;
 general_pass_input.current.field.motion[field_motion_index(2)].x=0x8400;
 general_pass_input.current.field.motion[field_motion_index(2)].y=0x8400;
 const auto general_pass=begin_ordinary_human_pass_target(general_pass_input);
 check(general_pass.phase==FieldSessionPhase::ordinary_human_pass_target && general_pass.human_pass_cursor==114 &&
       general_pass.current.field.ball_x==127 && general_pass.current.field.ball_y==132 && general_pass.current.field.ball_cell==114 &&
       general_pass.current.clock.ball_mode==129 && !general_pass.recovery->attack_subtype &&
       general_pass_input.current.field.ball_x==108,"Ordinary pass target refreshes from carrier and preserves input");
 check(general_pass.ordinary_pass_candidates && !general_pass.ordinary_pass_candidates->scanned,
       "Initial ordinary target marks candidate scan as not performed");
 const auto general_bytes=bytes(general_pass);std::istringstream general_stream(general_bytes);
 check(bytes(load_phase_session(general_stream))==general_bytes,"Ordinary pass target snapshot roundtrip");
 auto legacy_target=general_pass;legacy_target.ordinary_pass_candidates.reset();
 const auto legacy_target_bytes=bytes(legacy_target);std::istringstream legacy_target_stream(legacy_target_bytes);
 check(!load_phase_session(legacy_target_stream).ordinary_pass_candidates,
       "Earlier target snapshots retain unknown candidate context");
 const auto legacy_move=poll_ordinary_human_pass_cursor(legacy_target,1,0);
 check(legacy_move.state.ordinary_pass_candidates && legacy_move.state.ordinary_pass_candidates->scanned,
       "A subsequent move resolves unknown legacy candidate context");
 const auto general_move=poll_ordinary_human_pass_cursor(general_pass,1,0);
 check(general_move.cursor_changed && general_move.state.human_pass_cursor==126 && general_pass.human_pass_cursor==114 &&
       general_move.state.current.field.ball_cell==114,"Ordinary cursor move preserves ball and source");
 check(general_move.state.ordinary_pass_candidates && general_move.state.ordinary_pass_candidates->scanned &&
       general_move.state.ordinary_pass_candidates->candidates.count==1 &&
       general_move.state.ordinary_pass_candidates->candidates.allied_count==1 &&
       general_move.state.ordinary_pass_candidates->candidates.ids[0]==2,
       "A changed cursor records a candidate scan");
 const auto moved_bytes=bytes(general_move.state);std::istringstream moved_stream(moved_bytes);
 check(bytes(load_phase_session(moved_stream))==moved_bytes,"Scanned pass candidates survive snapshot roundtrip");
 auto malformed_candidates=general_move.state;
 malformed_candidates.ordinary_pass_candidates->candidates.count=5;
 rejects([&]{validate_phase_session(malformed_candidates);});
 auto malformed_snapshot=general_bytes;malformed_snapshot[421]=2;
 rejects([&]{std::istringstream bad(malformed_snapshot);load_phase_session(bad);});
 const auto general_idle=poll_ordinary_human_pass_cursor(general_move.state,0,0);
 check(!general_idle.cursor_changed && bytes(general_idle.state)==bytes(general_move.state),"Neutral target input preserves cursor");
 auto changed_positions=general_move.state;
 changed_positions.current.field.motion[field_motion_index(2)].x=0x3400;
 const auto still_saved=poll_ordinary_human_pass_cursor(changed_positions,0,0);
 check(still_saved.state.ordinary_pass_candidates->candidates.ids[0]==2,
       "Candidate prefix remains from the last scan when player positions later change");
 const auto free_pass=confirm_ordinary_human_pass_target(general_pass);
 check(free_pass.phase==FieldSessionPhase::ordinary_human_pass_confirmed && free_pass.ordinary_pass_recipient==255 &&
       free_pass.human_pass_cursor==114 && !general_pass.ordinary_pass_recipient,
       "An unscanned initial target confirms an explicitly free pass");
 const auto teammate_pass=confirm_ordinary_human_pass_target(general_move.state);
 check(teammate_pass.ordinary_pass_recipient==2 && teammate_pass.human_pass_cursor==126,
       "One scanned teammate is retained as pass recipient");
 const auto teammate_bytes=bytes(teammate_pass);std::istringstream teammate_stream(teammate_bytes);
 check(bytes(load_phase_session(teammate_stream))==teammate_bytes,"Confirmed pass recipient survives snapshot roundtrip");
 auto wrong_recipient=teammate_pass;wrong_recipient.ordinary_pass_recipient=3;
 rejects([&]{validate_phase_session(wrong_recipient);});
 auto defended_pass=free_pass;
 defended_pass.current.field.selected_defenders=2;
 defended_pass.scheduling->selected_ids={19,20,0,0,0};
 defended_pass.current.field.roster[19].type=1;
 defended_pass.current.field.roster[20].type=1;
 defended_pass.current.field.motion[field_motion_index(20)].status=0;
 defended_pass.recovery->ball_variant=0;
 const auto selection=begin_ordinary_pass_defense_selection(defended_pass);
 check(selection.phase==FieldSessionPhase::ordinary_pass_defense_selection && selection.ordinary_pass_defenses &&
       !selection.ordinary_pass_defenses->count && !selection.current.clock.ball_mode,
       "Acknowledged defense selection begins with no fabricated choices");
 const auto selection_nmi=advance_observed_nmi_events(selection,255,{{ObservedClockEventKind::random,{0,0,0,false}}});
 check(selection_nmi.random_index==0 && !selection_nmi.state.ordinary_pass_defenses->count,
       "Captured random event preserves pending defender choices");
 const auto first_choice=record_ordinary_pass_defense(selection,19,1,0);
 check(first_choice.phase==FieldSessionPhase::ordinary_pass_defense_selection && first_choice.ordinary_pass_defenses->count==1 &&
       first_choice.recovery->opposing_player==19 && !selection.ordinary_pass_defenses->count,
       "First observed defender choice is retained without changing input");
 const auto second_choice=record_ordinary_pass_defense(first_choice,20,1,0);
 check(second_choice.phase==FieldSessionPhase::ordinary_pass_defenses_recorded && second_choice.ordinary_pass_defenses->count==2 &&
       second_choice.recovery->opposing_player==20,"Complete observed defender prefix reaches preparation boundary");
 const auto defense_bytes=bytes(second_choice);std::istringstream defense_stream(defense_bytes);
 check(bytes(load_phase_session(defense_stream))==defense_bytes,"Observed defender choices survive snapshot roundtrip");
 FieldDefenseContent ordinary_pass_content{};ordinary_pass_content.skill_selectors[1][0]=1;
 ordinary_pass_content.moves[1]={{0,0,20,0}};ordinary_pass_content.strength_curve.fill(22);
 const auto ordinary_prepared=prepare_ordinary_pass(second_choice,sampled_attack_profiles,ordinary_pass_content);
 check(ordinary_prepared.phase==FieldSessionPhase::ordinary_pass_prepared &&
       ordinary_prepared.current.field.roster[5].stamina==11 && second_choice.current.field.roster[5].stamina==31 &&
       ordinary_prepared.ordinary_pass_defenses->count==2,"Ordinary pass preparation charges stamina and preserves captured choices");
 const auto ordinary_prepared_bytes=bytes(ordinary_prepared);std::istringstream ordinary_prepared_stream(ordinary_prepared_bytes);
 check(bytes(load_phase_session(ordinary_prepared_stream))==ordinary_prepared_bytes,"Prepared ordinary pass snapshot roundtrip");
 const auto presentation_samples=advance_observed_nmi_events(ordinary_prepared,0,
     {{ObservedClockEventKind::motion_sample,{},0}, {ObservedClockEventKind::facing_sample,{},64}});
 check(presentation_samples.state.control->facing==64 && !presentation_samples.state.control->motion_requested,
       "Explicit defender presentation samples update only captured control fields");
 const auto general_defender=prepare_first_ordinary_pass_defender(presentation_samples.state,2,
     sampled_attack_profiles,ordinary_pass_content);
 check(general_defender.phase==FieldSessionPhase::ordinary_pass_defender_prepared && general_defender.area_contact &&
       general_defender.recovery->opposing_player==19 && general_defender.recovery->ball_variant==2 &&
       general_defender.control->last_carrier_cell==128 && general_defender.area_contact->defense_action==1,
       "First ordinary pass defender is prepared from explicit variant and captured choice");
 const auto defender_bytes=bytes(general_defender);std::istringstream defender_stream(defender_bytes);
 check(bytes(load_phase_session(defender_stream))==defender_bytes,"Ordinary pass defender preparation snapshot roundtrip");
 rejects([&]{prepare_first_ordinary_pass_defender(presentation_samples.state,0,sampled_attack_profiles,ordinary_pass_content);});
 auto comparable=general_defender;
 comparable.action->power=12;comparable.area_contact->defender_power=8;
 comparable.current.field.motion[field_motion_index(19)].status=0;
 comparable.current.field.random_first=87;comparable.current.field.random_second=219;
 FieldComparisonSelection ordinary_comparison{5,153,0,false,{154,96,68,0}};
 const auto ordinary_compared=compare_first_ordinary_pass_defender(comparable,ordinary_comparison);
 auto wrong_pass_table=ordinary_comparison;wrong_pass_table.table_index=4;
 rejects([&]{compare_first_ordinary_pass_defender(comparable,wrong_pass_table);});
 auto terminal_pass_table=ordinary_comparison;terminal_pass_table.thresholds={255,255,255,255};
 rejects([&]{compare_first_ordinary_pass_defender(comparable,terminal_pass_table);});
 check(ordinary_compared.phase==FieldSessionPhase::ordinary_pass_defender_compared && ordinary_compared.area_contact->strength==4 &&
       ordinary_compared.area_contact->loss==1 && ordinary_compared.area_contact->score==192 && ordinary_compared.area_sequence_result==0 &&
       comparable.area_sequence_result==255,"First defender comparison preserves the incoming pass power and records the original result");
 const auto compared_bytes=bytes(ordinary_compared);std::istringstream compared_stream(compared_bytes);
 check(bytes(load_phase_session(compared_stream))==compared_bytes,"Ordinary defender comparison snapshot roundtrip");
 auto ordinary_prelude=home_prelude;ordinary_prelude.field_elapsed[3]=1;
 const auto ordinary_contact=advance_first_ordinary_pass_contact(ordinary_compared,ordinary_prelude,home_experience);
 check(ordinary_contact.phase==FieldSessionPhase::ordinary_pass_first_contact_clock &&
       ordinary_contact.current.clock.remaining+1==ordinary_compared.current.clock.remaining &&
       ordinary_contact.current.field.elapsed==ordinary_compared.current.field.elapsed+1 &&
       ordinary_contact.current.field.motion[field_motion_index(19)].status==6 &&
       ordinary_compared.current.field.motion[field_motion_index(19)].status==0,
       "Nonterminal ordinary pass contact applies the original clock and defender status atomically");
 const auto contact_bytes=bytes(ordinary_contact);std::istringstream contact_stream(contact_bytes);
 check(bytes(load_phase_session(contact_stream))==contact_bytes,"Ordinary contact snapshot roundtrip");
 rejects([&]{advance_first_ordinary_pass_contact(ordinary_contact,ordinary_prelude,home_experience);});
 auto second_pass_content=ordinary_pass_content;second_pass_content.strength_curve.fill(8);
 const auto ordinary_second=prepare_next_ordinary_pass_defender(ordinary_contact,sampled_attack_profiles,second_pass_content);
 check(ordinary_second.phase==FieldSessionPhase::ordinary_pass_next_defender_prepared && ordinary_second.area_defender_cursor==1 &&
       ordinary_second.recovery->opposing_player==20 && ordinary_second.area_contact->cursor==1,
       "The next recorded defender is prepared in order");
 const auto ordinary_second_compared=compare_next_ordinary_pass_defender(ordinary_second,ordinary_comparison);
 const auto second_compared_bytes=bytes(ordinary_second_compared);std::istringstream second_compared_stream(second_compared_bytes);
 check(bytes(load_phase_session(second_compared_stream))==second_compared_bytes,"Next defender comparison snapshot roundtrip");
 const auto ordinary_second_contact=advance_next_ordinary_pass_contact(ordinary_second_compared,ordinary_prelude,home_experience);
 check(ordinary_second_contact.phase==FieldSessionPhase::ordinary_pass_next_contact_clock &&
       ordinary_second_contact.current.field.motion[field_motion_index(20)].status==6,
       "The next defender uses the same bounded comparison and contact clock");
 const auto second_contact_bytes=bytes(ordinary_second_contact);std::istringstream second_contact_stream(second_contact_bytes);
 check(bytes(load_phase_session(second_contact_stream))==second_contact_bytes,"Next defender contact snapshot roundtrip");
 const auto next_wait=advance_observed_nmi_events(ordinary_second_contact,255,{{ObservedClockEventKind::random,{0,0,0,false}}});
 check(next_wait.random_index==0 && next_wait.state.area_defender_cursor==1,"Captured random events preserve the next defender cursor");
 rejects([&]{prepare_next_ordinary_pass_defender(ordinary_second_contact,sampled_attack_profiles,ordinary_pass_content);});
 const auto ordinary_dispatch=advance_ordinary_pass_dispatch_clock(ordinary_second_contact);
 check(ordinary_dispatch.phase==FieldSessionPhase::ordinary_pass_dispatch_clock &&
       ordinary_dispatch.current.clock.remaining+1==ordinary_second_contact.current.clock.remaining &&
       ordinary_dispatch.current.field.elapsed==ordinary_second_contact.current.field.elapsed+1,
       "Pass dispatch advances the final original clock step");
 const auto ordinary_dispatch_bytes=bytes(ordinary_dispatch);std::istringstream ordinary_dispatch_stream(ordinary_dispatch_bytes);
 check(bytes(load_phase_session(ordinary_dispatch_stream))==ordinary_dispatch_bytes,"Ordinary pass dispatch clock snapshot roundtrip");
 rejects([&]{advance_ordinary_pass_dispatch_clock(ordinary_contact);});
 auto movement_dispatch=ordinary_dispatch;
 for (auto& player : movement_dispatch.current.field.roster) player.type=1;
 auto pass_movement_content=travel_movement;pass_movement_content.geometry.direction_thresholds.fill(65535);
 std::vector<ObservedClockEvent> pass_movement_events;
 for(unsigned slot=0;slot<22;++slot) pass_movement_events.push_back({ObservedClockEventKind::field_player,{},static_cast<std::uint8_t>(slot)});
 const auto movement_expected=advance_field_phase(movement_dispatch.current.field,profile_mappings,pass_movement_content);
 const auto pass_movement=advance_observed_pass_field_movement(movement_dispatch,profile_mappings,pass_movement_content,0,pass_movement_events);
 check(pass_movement.state.phase==FieldSessionPhase::ordinary_pass_field_moved && !pass_movement.state.current.field.elapsed &&
       pass_movement.state.current.field.random_first==movement_expected.state.random_first &&
       pass_movement.state.current.field.motion[0].x==movement_expected.state.motion[0].x,
       "Evidence-fed player order composes the existing field movement rules");
 const auto pass_movement_bytes=bytes(pass_movement.state);std::istringstream pass_movement_stream(pass_movement_bytes);
 check(bytes(load_phase_session(pass_movement_stream))==pass_movement_bytes,"Observed pass field movement snapshot roundtrip");
 auto missing_movement=pass_movement_events;missing_movement.pop_back();
 rejects([&]{advance_observed_pass_field_movement(movement_dispatch,profile_mappings,pass_movement_content,0,missing_movement);});
 auto unordered_movement=pass_movement_events;std::swap(unordered_movement[1],unordered_movement[2]);
 rejects([&]{advance_observed_pass_field_movement(movement_dispatch,profile_mappings,pass_movement_content,0,unordered_movement);});
 PassDistanceContent ordinary_distance{};ordinary_distance.values.fill(2);
 auto ordinary_target_input=pass_movement.state;
 ordinary_target_input.human_pass_cursor=static_cast<std::uint8_t>((ordinary_target_input.current.field.ball_cell+1)%240);
 const auto resolved_ordinary_target=resolve_ordinary_pass_flight_target(ordinary_target_input,ordinary_distance);
 check(resolved_ordinary_target.phase==FieldSessionPhase::ordinary_pass_flight_target && resolved_ordinary_target.sampled_field_pass &&
       resolved_ordinary_target.sampled_field_pass->recipient==*pass_movement.state.ordinary_pass_recipient &&
       resolved_ordinary_target.sampled_field_pass->counter==255 && resolved_ordinary_target.sampled_field_pass->distance==2,
       "Ordinary pass resolves the saved recipient and distance at handoff");
 const auto resolved_target_bytes=bytes(resolved_ordinary_target);std::istringstream resolved_target_stream(resolved_target_bytes);
 check(bytes(load_phase_session(resolved_target_stream))==resolved_target_bytes,"Ordinary resolved pass target snapshot roundtrip");
 auto forged_ordinary_target=resolved_ordinary_target;forged_ordinary_target.sampled_field_pass->destination^=1;
 rejects([&]{validate_phase_session(forged_ordinary_target);});
 const auto ordinary_flight=begin_ordinary_pass_flight(resolved_ordinary_target,0,0,pass_geometry,pass_velocity);
 check(ordinary_flight.phase==FieldSessionPhase::ordinary_pass_flight_ready && ordinary_flight.ball_trajectory &&
       ordinary_flight.area_ai_target->recipient==resolved_ordinary_target.sampled_field_pass->recipient &&
       !ordinary_flight.current.field.selected_defenders && !ordinary_flight.ordinary_pass_defenses,
       "Ordinary flight setup transfers the resolved target and clears defender state");
 const auto ordinary_flight_bytes=bytes(ordinary_flight);std::istringstream ordinary_flight_stream(ordinary_flight_bytes);
 check(bytes(load_phase_session(ordinary_flight_stream))==ordinary_flight_bytes,"Ordinary flight setup snapshot roundtrip");
 auto home_flight_for_step=ordinary_flight;
 home_flight_for_step.area_ai_target->recipient=7;
 const auto ordinary_ball_step=advance_ordinary_pass_ball_cell(home_flight_for_step);
 if(ordinary_ball_step.state.phase==FieldSessionPhase::ordinary_pass_flight_arrival)
  rejects([&]{advance_ordinary_pass_ball_cell(ordinary_flight);});
 ClearanceRaceContent free_race_content{};
 rejects([&]{resolve_ordinary_free_pass_contest(home_flight_for_step,profile_mappings,pass_movement_content,free_race_content);});
 check((ordinary_ball_step.state.phase==FieldSessionPhase::ordinary_pass_flight_callback ||
        ordinary_ball_step.state.phase==FieldSessionPhase::ordinary_pass_flight_arrival) && ordinary_ball_step.state.ball_trajectory,
       "Ordinary pass flight advances to an explicit cell or arrival callback");
 const auto ordinary_step_bytes=bytes(ordinary_ball_step.state);std::istringstream ordinary_step_stream(ordinary_step_bytes);
 check(bytes(load_phase_session(ordinary_step_stream))==ordinary_step_bytes,"Ordinary ball step snapshot roundtrip");
 auto ordinary_arrival=ordinary_ball_step.state;
 ordinary_arrival.phase=FieldSessionPhase::ordinary_pass_flight_arrival;
 const auto arrival_cell=ordinary_arrival.shot_flight->destination;
 ordinary_arrival.current.field.ball_x=static_cast<std::uint8_t>(48+(arrival_cell/12)*8+4);
 ordinary_arrival.current.field.ball_y=static_cast<std::uint8_t>(80+(arrival_cell%12)*8+4);
 ordinary_arrival.current.field.ball_cell=arrival_cell;
 ordinary_arrival.ball_trajectory->x=static_cast<std::uint16_t>(ordinary_arrival.current.field.ball_x<<8);
 ordinary_arrival.ball_trajectory->y=static_cast<std::uint16_t>(ordinary_arrival.current.field.ball_y<<8);
 const auto ordinary_owner=continue_ordinary_pass_reception(ordinary_arrival);
 check(ordinary_owner.phase==FieldSessionPhase::ordinary_pass_received && ordinary_owner.current.field.ball_owner==7,
       "Ordinary pass arrival transfers possession to its saved teammate");
 const auto ordinary_position=position_ordinary_pass_recipient(ordinary_owner);
 check(ordinary_position.phase==FieldSessionPhase::ordinary_pass_recipient_positioned &&
       ordinary_position.current.field.ball_x==(ordinary_position.current.field.motion[field_motion_index(7)].x>>8),
       "Ordinary reception places the ball at the recipient");
 const auto ordinary_received_bytes=bytes(ordinary_owner);std::istringstream ordinary_received_stream(ordinary_received_bytes);
 check(bytes(load_phase_session(ordinary_received_stream))==ordinary_received_bytes,"Ordinary reception snapshot roundtrip");
 const auto ordinary_position_bytes=bytes(ordinary_position);std::istringstream ordinary_position_stream(ordinary_position_bytes);
 check(bytes(load_phase_session(ordinary_position_stream))==ordinary_position_bytes,"Ordinary recipient position snapshot roundtrip");
 const auto ordinary_control=continue_ordinary_pass_control(ordinary_position,0);
 check(ordinary_control.phase==FieldSessionPhase::ordinary_pass_control_entry && ordinary_control.control->facing==0 &&
       !ordinary_control.ball_trajectory && !ordinary_control.area_ai_target,
       "Acknowledged teammate reception returns to control without the flight context");
 const auto ordinary_ready=prepare_session_control(ordinary_control);
 check(ordinary_ready.state.phase==FieldSessionPhase::control_ready && ordinary_ready.state.area_defender_cursor==2 &&
       ordinary_ready.state.current.field.ball_owner==7,"Ordinary reception uses the existing control setup and preserves the original cursor residue");
 auto forged_ready=ordinary_ready.state;forged_ready.area_defender_cursor=6;
 rejects([&]{validate_phase_session(forged_ready);});
 auto ordinary_poll_ready=ordinary_ready.state;ordinary_poll_ready.control->decision_timer=0;
 ObservedNmiFieldPoll ordinary_poll_sample{};
 const auto ordinary_return_poll=advance_observed_ordinary_return_poll(ordinary_poll_ready,profile_mappings,
     pass_movement_content,travel_content,0,ordinary_poll_sample);
 check(ordinary_return_poll.poll.state.phase==FieldSessionPhase::ordinary_return_clock_pending &&
       ordinary_return_poll.poll.state.area_defender_cursor==2,"Returned control preserves cursor into its evidence-fed clock boundary");
 const auto ordinary_return_bytes=bytes(ordinary_return_poll.poll.state);std::istringstream ordinary_return_stream(ordinary_return_bytes);
 check(bytes(load_phase_session(ordinary_return_stream))==ordinary_return_bytes,"Returned control clock snapshot roundtrip");
 auto ordinary_return_encounter=ordinary_return_poll.poll.state;
 ordinary_return_encounter.phase=FieldSessionPhase::ordinary_return_contact_pending;
 ordinary_return_encounter.current.clock.ball_mode=0;ordinary_return_encounter.current.clock.script_flags=0;
 ordinary_return_encounter.current.field.selected_defenders=1;
 ordinary_return_encounter.scheduling->selected_ids={19,0,0,0,0};
 const auto ordinary_encounter_bytes=bytes(ordinary_return_encounter);std::istringstream ordinary_encounter_stream(ordinary_encounter_bytes);
 check(bytes(load_phase_session(ordinary_encounter_stream))==ordinary_encounter_bytes,"Returned encounter preserves its original cursor");
 auto empty_return_encounter=ordinary_return_encounter;empty_return_encounter.current.field.selected_defenders=0;
 rejects([&]{validate_phase_session(empty_return_encounter);});
 const auto returned_menu_wait=advance_observed_nmi_events(ordinary_return_encounter,0,
     {{ObservedClockEventKind::motion_sample,{},0},{ObservedClockEventKind::facing_sample,{},0},
      {ObservedClockEventKind::display_sample,{},0}});
 const auto returned_action_menu=open_ordinary_return_action_menu(returned_menu_wait.state,pass_menu_content);
 check(returned_action_menu.state.phase==FieldSessionPhase::ordinary_return_action_menu &&
       returned_action_menu.state.area_defender_cursor==2 && returned_action_menu.state.action->cursor==0,
       "Returned encounter opens the shared action menu while preserving cursor residue");
 const auto returned_menu_bytes=bytes(returned_action_menu.state);std::istringstream returned_menu_stream(returned_menu_bytes);
 check(bytes(load_phase_session(returned_menu_stream))==returned_menu_bytes,"Returned encounter menu snapshot roundtrip");
 rejects([&]{prepare_ordinary_pass(first_choice,sampled_attack_profiles,ordinary_pass_content);});
 rejects([&]{record_ordinary_pass_defense(selection,20,1,0);});
 rejects([&]{record_ordinary_pass_defense(selection,19,2,0);});
 rejects([&]{confirm_ordinary_human_pass_target(legacy_target);});
 auto multiple=general_move.state;multiple.ordinary_pass_candidates->candidates.count=2;
 multiple.ordinary_pass_candidates->candidates.allied_count=2;multiple.ordinary_pass_candidates->candidates.ids[1]=3;
 rejects([&]{confirm_ordinary_human_pass_target(multiple);});
 const auto general_edge=poll_ordinary_human_pass_cursor(general_pass,1,1);
 check(general_edge.state.human_pass_cursor==126,"Directional press edge does not suppress held movement");
 const auto target_nmi=advance_observed_nmi_events(general_pass,255,{{ObservedClockEventKind::random,{0,0,0,false}}});
 check(target_nmi.random_index==0 && target_nmi.state.phase==FieldSessionPhase::ordinary_human_pass_target,
       "Captured NMI sequence retains ordinary target phase");
 const auto sampled_motion=advance_observed_nmi_events(general_pass,0,{{ObservedClockEventKind::motion_sample,{},1}});
 check(sampled_motion.state.control->motion_requested==1 && !general_pass.control->motion_requested,
       "Explicit captured target-wait motion sample preserves its input");
 rejects([&]{poll_ordinary_human_pass_cursor(general_pass,1,128);});
 rejects([&]{poll_ordinary_human_pass_cursor(general_pass,1,64);});
 rejects([&]{poll_ordinary_human_pass_cursor(general_pass,1,192);});
 auto wrong_action=general_pass_input;wrong_action.action->action=3;
 rejects([&]{begin_ordinary_human_pass_target(wrong_action);});
 auto stale_flight_pass=pass_confirmed;stale_flight_pass.saved_ball_flight_flag=1;
 rejects([&]{begin_sampled_human_pass_target(stale_flight_pass);});
 auto pass_target_cursor=begin_sampled_human_pass_target(pass_confirmed);
 auto pass_cursor_right=poll_sampled_human_pass_target(pass_target_cursor,1,0);
 auto pass_cursor_twice=poll_sampled_human_pass_target(pass_cursor_right,1,0);
 check(pass_target_cursor.human_pass_cursor && *pass_target_cursor.human_pass_cursor==87 &&
       pass_cursor_right.human_pass_cursor && *pass_cursor_right.human_pass_cursor==99 &&
       pass_cursor_twice.human_pass_cursor && *pass_cursor_twice.human_pass_cursor==111 &&
       pass_target_cursor.current.clock.ball_mode==129 && pass_target_cursor.saved_ball_flight_flag==0,
       "Observed human pass cursor opens with the original cleared flight flag");
 auto pass_cursor_bytes=bytes(pass_cursor_right);std::istringstream pass_cursor_stream(pass_cursor_bytes);
 check(static_cast<unsigned char>(pass_cursor_bytes[10])==141 && bytes(load_phase_session(pass_cursor_stream))==pass_cursor_bytes,
       "Human pass cursor snapshot roundtrips");
 auto old_pass_cursor_version=pass_cursor_bytes;old_pass_cursor_version[10]=static_cast<char>(133);
 rejects([&]{std::istringstream in(old_pass_cursor_version);load_phase_session(in);});
 auto truncated_pass_cursor=pass_cursor_bytes;truncated_pass_cursor.pop_back();
 rejects([&]{std::istringstream in(truncated_pass_cursor);load_phase_session(in);});
 auto corrupt_pass_cursor=pass_cursor_bytes;corrupt_pass_cursor.back()=static_cast<char>(255);
 rejects([&]{std::istringstream in(corrupt_pass_cursor);load_phase_session(in);});
 auto pass_empty_confirm=poll_sampled_human_pass_target(pass_target_cursor,0,128);
 auto pass_cancel_pending=poll_sampled_human_pass_target(pass_target_cursor,0,64);
 check(static_cast<unsigned>(pass_empty_confirm.phase)==245 && static_cast<unsigned>(pass_cancel_pending.phase)==246 &&
       *pass_empty_confirm.human_pass_cursor==87,"Observed empty-target confirmation and cancellation remain pending");
 FieldDefenseContent human_pass_content{};human_pass_content.skill_selectors[1][0]=1;
 human_pass_content.moves[1]={{0,0,20,0}};human_pass_content.strength_curve.fill(21);
 auto human_pass_prepared=prepare_sampled_human_empty_pass(pass_empty_confirm,sampled_attack_profiles,human_pass_content);
 check(static_cast<unsigned>(human_pass_prepared.phase)==247 && human_pass_prepared.current.field.roster[5].stamina==11 &&
       human_pass_prepared.action->power==21 && human_pass_prepared.recovery->attack_subtype==128 &&
       human_pass_prepared.current.clock.ball_mode==0,"Observed human pass preparation spends20 stamina and produces power21");
 auto human_pass_handoff=acknowledge_sampled_human_empty_pass_handoff(human_pass_prepared);
 check(static_cast<unsigned>(human_pass_handoff.phase)==248 && human_pass_handoff.recovery->ball_variant==1 &&
       human_pass_handoff.current.clock.remaining==381,"Human empty-target pass reaches the observed handoff");
 auto human_pass_field=human_pass_handoff.current.field;human_pass_field.ball_owner=3;human_pass_field.movement_region=129;
 human_pass_field.motion[0].x+=1;
 auto human_pass_recovered=capture_sampled_human_empty_pass_recovery(human_pass_handoff,human_pass_field);
 check(human_pass_recovered.phase==FieldSessionPhase::recovered_owner && human_pass_recovered.current.field.ball_owner==3 &&
       human_pass_recovered.saved_ball_flight_flag==1 && human_pass_recovered.recovery->attack_subtype==0,
       "Observed same-cell pass reaches player3 recovery with the original flight flag");
 auto human_recovery_bytes=bytes(human_pass_recovered);std::istringstream human_recovery_stream(human_recovery_bytes);
 check(bytes(load_phase_session(human_recovery_stream))==human_recovery_bytes,"Human pass recovery snapshot roundtrips");
 auto altered_human_pass_field=human_pass_field;altered_human_pass_field.roster[5].stamina++;
 rejects([&]{capture_sampled_human_empty_pass_recovery(human_pass_handoff,altered_human_pass_field);});
 auto player3_pass_choice=pass_confirmed;player3_pass_choice.current.field.ball_owner=3;
 player3_pass_choice.current.field.roster[3].type=23;player3_pass_choice.current.field.roster[3].stamina=0;
 player3_pass_choice.current.field.roster[17].type=23;player3_pass_choice.current.field.roster[17].stamina=100;
 player3_pass_choice.current.field.motion[field_motion_index(17)].status=0;
 player3_pass_choice.current.field.selected_defenders=1;player3_pass_choice.scheduling->selected_ids={17,0,0,0,0};
 player3_pass_choice.current.field.elapsed=1;
 player3_pass_choice.current.clock.remaining=374;player3_pass_choice.recovery->opposing_player=1;
 player3_pass_choice.action->power=21;player3_pass_choice.saved_ball_flight_flag=1;
 auto player3_target=begin_sampled_human_pass_target(player3_pass_choice);
 auto player3_empty=poll_sampled_human_pass_target(player3_target,0,128);
 auto player3_pass_prepared=prepare_sampled_human_empty_pass(player3_empty,sampled_attack_profiles,human_pass_content);
 check(static_cast<unsigned>(player3_pass_prepared.phase)==251 && player3_pass_prepared.current.field.roster[3].stamina==0 &&
       player3_pass_prepared.action->power==0 && player3_pass_prepared.recovery->opposing_player==17 &&
       player3_pass_prepared.saved_ball_flight_flag==1,"Exhausted player3 prepares the observed weakened contact pass");
 auto player3_prepared_bytes=bytes(player3_pass_prepared);std::istringstream player3_prepared_stream(player3_prepared_bytes);
 check(bytes(load_phase_session(player3_prepared_stream))==player3_prepared_bytes,"Player3 weakened pass snapshot roundtrips");
 rejects([&]{poll_sampled_human_pass_target(player3_target,1,0);});
 auto altered_player3_content=human_pass_content;altered_player3_content.moves[1][0].cost_low=19;
 rejects([&]{prepare_sampled_human_empty_pass(player3_empty,sampled_attack_profiles,altered_player3_content);});
 auto player3_defense_content=home_defense;player3_defense_content.moves[1][0].cost_low=60;
 auto player3_defender=prepare_sampled_player3_pass_defender(player3_pass_prepared,sampled_attack_profiles,player3_defense_content);
 check(static_cast<unsigned>(player3_defender.phase)==252 && player3_defender.area_contact &&
       player3_defender.area_contact->defense_action==1 && player3_defender.area_contact->defender_power==18 &&
       player3_defender.area_contact->cost==60 && player3_defender.control->facing==64,
       "Player3 pass retains the observed defender17 choice and preparation");
 FieldComparisonSelection player3_direct_comparison{5,128,0,false,{192,128,64,0}};
 auto player3_compared=compare_sampled_player3_pass_defender(player3_defender,player3_direct_comparison);
 check(static_cast<unsigned>(player3_compared.phase)==253 && player3_compared.area_contact->score==255 &&
       player3_compared.area_contact->result==0 && player3_compared.current.field.roster[17].stamina==100,
       "Player3 zero-power pass matches the observed defender comparison");
 auto player3_defender_bytes=bytes(player3_defender);std::istringstream player3_defender_stream(player3_defender_bytes);
 auto player3_compared_bytes=bytes(player3_compared);std::istringstream player3_compared_stream(player3_compared_bytes);
 check(bytes(load_phase_session(player3_defender_stream))==player3_defender_bytes &&
       bytes(load_phase_session(player3_compared_stream))==player3_compared_bytes,
       "Player3 defender preparation and comparison snapshots retain the choice and cursor");
 auto wrong_player3_table=player3_direct_comparison;wrong_player3_table.table_index=4;
 rejects([&]{compare_sampled_player3_pass_defender(player3_defender,wrong_player3_table);});
 auto wrong_player3_variation=player3_direct_comparison;wrong_player3_variation.offset=128;
 rejects([&]{compare_sampled_player3_pass_defender(player3_defender,wrong_player3_variation);});
 auto wrong_player3_defender_content=player3_defense_content;wrong_player3_defender_content.moves[1][0].cost_low=59;
 rejects([&]{prepare_sampled_player3_pass_defender(player3_pass_prepared,sampled_attack_profiles,wrong_player3_defender_content);});
 auto player3_prelude=home_prelude;player3_prelude.field_elapsed[3]=1;
 auto player3_contact=advance_sampled_player3_pass_contact(player3_compared,player3_prelude,home_experience);
 check(static_cast<unsigned>(player3_contact.phase)==254 && player3_contact.current.clock.remaining==373 &&
       player3_contact.current.field.elapsed==2 && player3_contact.current.field.motion[field_motion_index(17)].status==6 &&
       player3_contact.experience->totals[0]==player3_compared.experience->totals[0]+3,
       "Player3 pass contact applies the observed defender status, clock and experience");
 auto player3_dispatch=advance_sampled_player3_pass_dispatch(player3_contact);
 check(static_cast<unsigned>(player3_dispatch.phase)==255 && player3_dispatch.current.clock.remaining==372 &&
       player3_dispatch.current.field.elapsed==3 && player3_dispatch.area_defender_cursor==1,
       "Player3 pass dispatch advances the observed final clock tick");
 auto player3_dispatch_bytes=bytes(player3_dispatch);std::istringstream player3_dispatch_stream(player3_dispatch_bytes);
 check(bytes(load_phase_session(player3_dispatch_stream))==player3_dispatch_bytes,
       "Phase255 player3 dispatch snapshot roundtrips");
 rejects([&]{advance_sampled_player3_pass_dispatch(player3_dispatch);});
 auto player3_pass_handoff_field=player3_dispatch.current.field;player3_pass_handoff_field.elapsed=0;
 player3_pass_handoff_field.movement_region=129;player3_pass_handoff_field.motion[0].x++;
 auto player3_pass_handoff=capture_sampled_player3_pass_handoff(player3_dispatch,player3_pass_handoff_field);
 check(static_cast<unsigned>(player3_pass_handoff.phase)==111 && player3_pass_handoff.current.field.elapsed==0 &&
       player3_pass_handoff.current.field.motion[0].x==player3_pass_handoff_field.motion[0].x &&
       player3_pass_handoff.current.clock.remaining==372,"Player3 contact pass reaches captured handoff");
 auto player3_handoff_bytes=bytes(player3_pass_handoff);std::istringstream player3_handoff_stream(player3_handoff_bytes);
 check(bytes(load_phase_session(player3_handoff_stream))==player3_handoff_bytes,"Player3 pass handoff snapshot roundtrips");
 auto wrong_handoff_field=player3_pass_handoff_field;wrong_handoff_field.roster[3].stamina++;
 rejects([&]{capture_sampled_player3_pass_handoff(player3_dispatch,wrong_handoff_field);});
 auto player3_pass_target=capture_sampled_player3_pass_target(player3_pass_handoff,255,87,255,2);
 check(static_cast<unsigned>(player3_pass_target.phase)==112 && player3_pass_target.sampled_field_pass &&
       player3_pass_target.sampled_field_pass->recipient==255 && player3_pass_target.sampled_field_pass->destination==87 &&
       player3_pass_target.sampled_field_pass->distance==2,"Player3 pass retains the observed empty same-cell target");
 auto player3_target_bytes=bytes(player3_pass_target);std::istringstream player3_target_stream(player3_target_bytes);
 check(bytes(load_phase_session(player3_target_stream))==player3_target_bytes,"Player3 empty pass target snapshot roundtrips");
 auto wrong_player3_target_version=player3_target_bytes;wrong_player3_target_version[10]=static_cast<char>(140);
 rejects([&]{std::istringstream in(wrong_player3_target_version);load_phase_session(in);});
 auto truncated_player3_target=player3_target_bytes;truncated_player3_target.pop_back();
 rejects([&]{std::istringstream in(truncated_player3_target);load_phase_session(in);});
 rejects([&]{capture_sampled_player3_pass_target(player3_pass_handoff,255,88,255,2);});
 auto player3_race_input=player3_pass_target;
 for(auto& m:player3_race_input.current.field.motion){m.x=160<<8;m.y=140<<8;}
 for(auto id:{5,19}){auto& m=player3_race_input.current.field.motion[field_motion_index(id)];m.x=108<<8;m.y=108<<8;}
 player3_race_input.current.field.roster[5]={1,0,123};player3_race_input.current.field.roster[19]={1,0,123};
 FieldPhaseContent player3_race_movement{};player3_race_movement.profiles.speeds.resize(1);
 player3_race_movement.geometry.direction_thresholds.fill(65535);
 PlayerProfileMappings player3_race_mappings{};ClearanceRaceContent player3_race_content{};
 const auto race_input_bytes=bytes(player3_race_input);
 auto player3_race=resolve_sampled_player3_pass_race(player3_race_input,player3_race_mappings,player3_race_movement,player3_race_content);
 check(player3_race.clearance_race && player3_race.clearance_race->remaining==2 &&
       player3_race.clearance_race->active_candidate==5 && player3_race.clearance_race->opposing_candidate==19 &&
       player3_race.current.field.ball_owner==3 && player3_race.current.field.selected_defenders==0 &&
       player3_race.current.field.elapsed==1 && bytes(player3_race_input)==race_input_bytes,
       "Player3 empty pass calculates the observed contest before ownership changes");
 auto player3_race_bytes=bytes(player3_race);std::istringstream player3_race_stream(player3_race_bytes);
 check(bytes(load_phase_session(player3_race_stream))==player3_race_bytes,"Player3 contest snapshot roundtrips");
 rejects([&]{resolve_sampled_player3_pass_race(player3_race,player3_race_mappings,player3_race_movement,player3_race_content);});
 auto wrong_race_input=player3_race_input;wrong_race_input.current.field.motion[field_motion_index(5)].x=160<<8;
 const auto wrong_race_bytes=bytes(wrong_race_input);
 rejects([&]{resolve_sampled_player3_pass_race(wrong_race_input,player3_race_mappings,player3_race_movement,player3_race_content);});
 check(bytes(wrong_race_input)==wrong_race_bytes,"Unobserved pass-race outcome leaves its input unchanged");
 auto player3_winner=position_sampled_player3_pass_winner(player3_race);
 const auto& race_winner=player3_winner.current.field.motion[field_motion_index(5)];
 check(player3_winner.current.field.ball_owner==5 && (race_winner.x>>8)==108 && (race_winner.y>>8)==108 &&
       (race_winner.x&255)==(player3_race.current.field.motion[field_motion_index(5)].x&255) &&
       (race_winner.y&255)==(player3_race.current.field.motion[field_motion_index(5)].y&255),
       "Pass contest winner takes ownership with its coordinate fractions preserved");
 auto player3_winner_bytes=bytes(player3_winner);std::istringstream player3_winner_stream(player3_winner_bytes);
 check(bytes(load_phase_session(player3_winner_stream))==player3_winner_bytes,"Positioned pass winner snapshot roundtrips");
 rejects([&]{position_sampled_player3_pass_winner(player3_winner);});
 auto player3_control_entry=continue_sampled_player3_pass_winner(player3_winner);
 check(player3_control_entry.phase==FieldSessionPhase::player_control && player3_control_entry.current.field.ball_owner==5 &&
       !player3_control_entry.clearance_race && !player3_control_entry.sampled_field_pass && !player3_control_entry.human_pass_cursor &&
       !player3_control_entry.area_contact && player3_control_entry.area_defender_cursor==1 && player3_control_entry.saved_ball_flight_flag==1,
       "Acknowledged pass winner enters player control without stale pass contexts");
 rejects([&]{continue_sampled_player3_pass_winner(player3_race);});
 auto player3_entry_bytes=bytes(player3_control_entry);std::istringstream player3_entry_stream(player3_entry_bytes);
 check(bytes(load_phase_session(player3_entry_stream))==player3_entry_bytes,"Pass winner control handoff snapshot roundtrips");
 auto player3_ready=prepare_session_control(player3_control_entry).state;
 check(player3_ready.area_defender_cursor==1,"Control setup preserves the observed defender cursor residue");
 player3_ready.control->decision_timer=0;ControlMovementContent player3_control_movement{};
 auto player3_next_poll=poll_session_control(player3_ready,0,0,player3_race_mappings,player3_race_movement,player3_control_movement);
 check(player3_next_poll.state.phase==FieldSessionPhase::control_clock_pending && player3_next_poll.state.area_defender_cursor==1,
       "Pass-winner control poll preserves the cursor until the next clock");
 auto player5_contact=player3_next_poll.state;player5_contact.phase=FieldSessionPhase::field_contact_pending;
 player5_contact.current.field.elapsed=1;player5_contact.current.field.selected_defenders=5;
 player5_contact.scheduling->selected_ids={14,16,18,19,20};
 auto player5_menu=open_human_action_menu(player5_contact,ActionMenuContent{}).state;
 check(player5_menu.phase==FieldSessionPhase::human_action_menu && player5_menu.area_defender_cursor==1,
       "The next five-defender menu retains the original cursor residue");
 ActionMenuContent player5_dribble_menu{};player5_dribble_menu.directions[0][3]=2;
 auto player5_dribble_choice=poll_human_action_menu(player5_menu,8,player5_dribble_menu).state;
 auto player5_dribble_confirm=poll_human_action_menu(player5_dribble_choice,128,player5_dribble_menu).state;
 check(player5_dribble_confirm.phase==FieldSessionPhase::attack_choice_pending && player5_dribble_confirm.action->action==2 &&
       player5_dribble_confirm.area_defender_cursor==1 && !bytes(player5_dribble_confirm).empty(),"Confirmed dribble retains the observed defender cursor");
 auto player5_observed_choice=player5_dribble_confirm;
 player5_observed_choice.current.clock.remaining=367;player5_observed_choice.current.field.roster[5]={23,0,10};
 player5_observed_choice.current.field.roster[14]={23,15,100};
 const std::array<std::uint8_t,5> player5_defense_actions{1,1,1,1,1},player5_defense_subtypes{};
 auto player5_captured_defense=capture_sampled_player5_defense_choices(player5_observed_choice,player5_defense_actions,player5_defense_subtypes);
 check(player5_captured_defense.human_defense_choices && player5_captured_defense.human_defense_choices->actions==player5_defense_actions &&
       player5_captured_defense.recovery->opposing_player==20 && player5_captured_defense.area_defender_cursor==1,
       "Five observed defending actions and their final selected player are retained");
 auto player5_choices_bytes=bytes(player5_captured_defense);std::istringstream player5_choices_stream(player5_choices_bytes);
 check(static_cast<unsigned char>(player5_choices_bytes[10])==142 && bytes(load_phase_session(player5_choices_stream))==player5_choices_bytes,"Five defender choices roundtrip");
 auto truncated_player5_choices=player5_choices_bytes;truncated_player5_choices.pop_back();
 rejects([&]{std::istringstream in(truncated_player5_choices);load_phase_session(in);});
 auto altered_player5_choices=player5_choices_bytes;altered_player5_choices.back()=1;
 rejects([&]{std::istringstream in(altered_player5_choices);load_phase_session(in);});
 auto wrong_player5_version=player5_choices_bytes;wrong_player5_version[10]=static_cast<char>(133);
 rejects([&]{std::istringstream in(wrong_player5_version);load_phase_session(in);});
 auto player5_wrong_actions=player5_defense_actions;player5_wrong_actions[4]=0;
 rejects([&]{capture_sampled_player5_defense_choices(player5_observed_choice,player5_wrong_actions,player5_defense_subtypes);});
 auto player5_dribble_content=sampled_attack_content;player5_dribble_content.strength_curve.fill(24);
 auto player5_dribble_prepared=prepare_sampled_player5_dribble(player5_captured_defense,sampled_attack_profiles,player5_dribble_content);
 check(player5_dribble_prepared.phase==FieldSessionPhase::attack_prepared && player5_dribble_prepared.current.field.roster[5].stamina==0 &&
       player5_dribble_prepared.action->power==0 && player5_dribble_prepared.human_defense_choices &&
       player5_dribble_prepared.area_defender_cursor==1,"Exhausted dribble preparation retains all five defender choices");
 auto player5_prepared_bytes=bytes(player5_dribble_prepared);std::istringstream player5_prepared_stream(player5_prepared_bytes);
 check(bytes(load_phase_session(player5_prepared_stream))==player5_prepared_bytes,"Prepared dribble snapshot roundtrips");
 auto player5_prefix=begin_sampled_player5_dribble_defenders(player5_dribble_prepared);
 check(player5_prefix.area_defender_cursor==0 && player5_prefix.control->last_carrier_cell==0 &&
       player5_prefix.recovery->opposing_player==20 && player5_prefix.human_defense_choices,
       "Dribble defender prefix resets the old cursor and selects the observed first index");
 auto player5_prefix_bytes=bytes(player5_prefix);std::istringstream player5_prefix_stream(player5_prefix_bytes);
 check(bytes(load_phase_session(player5_prefix_stream))==player5_prefix_bytes,"Dribble defender prefix roundtrips");
 rejects([&]{begin_sampled_player5_dribble_defenders(player5_prefix);});
 auto player5_defender=prepare_sampled_player5_dribble_defender(player5_prefix,sampled_attack_profiles,player3_defense_content);
 check(player5_defender.recovery->opposing_player==14 && player5_defender.area_contact && player5_defender.area_contact->defender_power==18 &&
       player5_defender.area_contact->cost==60 && player5_defender.current.field.roster[14].stamina==100,
       "First opposing dribble defender retains the observed strength and cost without spending stamina");
 FieldComparisonSelection player5_comparison{9,4,64,false,{10,5,1,0}};
 auto player5_compared=compare_sampled_player5_dribble_defender(player5_defender,player5_comparison);
 check(player5_compared.area_contact->strength==4 && player5_compared.area_contact->loss==1 &&
       player5_compared.area_contact->score==0 && player5_compared.area_contact->result==3 && player5_compared.area_sequence_result==3,
       "First dribble defender wins result3 with a positive rolled strength");
 auto player5_compared_bytes=bytes(player5_compared);std::istringstream player5_compared_stream(player5_compared_bytes);
 check(bytes(load_phase_session(player5_compared_stream))==player5_compared_bytes,"Compared five-defender snapshot roundtrips");
 auto player5_terminal_prelude=home_prelude;player5_terminal_prelude.field_elapsed[2]=1;
 auto player5_terminal_experience=home_experience;player5_terminal_experience.reward_factors[2]=5;
 auto player5_terminal_clock=advance_sampled_player5_dribble_terminal(player5_compared,player5_terminal_prelude,player5_terminal_experience);
 check(player5_terminal_clock.current.clock.remaining==366 && player5_terminal_clock.current.field.elapsed==2 &&
       player5_terminal_clock.current.field.motion[field_motion_index(5)].status==5 && player5_terminal_clock.control->facing==64 &&
       player5_terminal_clock.experience->totals[0]==player5_compared.experience->totals[0]+1,
       "Captured result3 presentation is followed by the observed reward, attacker status and clock");
 auto player5_terminal_bytes=bytes(player5_terminal_clock);std::istringstream player5_terminal_stream(player5_terminal_bytes);
 check(bytes(load_phase_session(player5_terminal_stream))==player5_terminal_bytes,"Dribble terminal clock roundtrips");
 rejects([&]{advance_sampled_player5_dribble_terminal(player5_terminal_clock,player5_terminal_prelude,player5_terminal_experience);});
 auto player5_terminal_field_input=player5_terminal_clock;
 for(unsigned id=1;id<=21;++id)if(id!=11 && id!=5 && id!=14)player5_terminal_field_input.current.field.roster[id].type=1;
 const auto terminal_field_before=bytes(player5_terminal_field_input);
 auto player5_terminal_field=advance_sampled_player5_terminal_field(player5_terminal_field_input,player3_race_mappings,player3_race_movement);
 check(player5_terminal_field.current.field.elapsed==0 && player5_terminal_field.current.field.movement_region==129 &&
       player5_terminal_field.current.clock.remaining==366 && player5_terminal_field.current.field.ball_owner==5 &&
       bytes(player5_terminal_field_input)==terminal_field_before,"Terminal field movement advances before possession changes");
 auto player5_terminal_field_bytes=bytes(player5_terminal_field);std::istringstream player5_terminal_field_stream(player5_terminal_field_bytes);
 check(bytes(load_phase_session(player5_terminal_field_stream))==player5_terminal_field_bytes,"Terminal field state roundtrips");
 rejects([&]{advance_sampled_player5_terminal_field(player5_terminal_field,player3_race_mappings,player3_race_movement);});
 auto player14_possession=transfer_sampled_player5_dribble_possession(player5_terminal_field);
 check(player14_possession.request_side_music && player14_possession.state.current.field.ball_owner==14 &&
       player14_possession.state.current.field.side==11 && player14_possession.state.current.field.selected_defenders==5 &&
       player14_possession.state.human_defense_choices,"Result3 transfers possession while preserving the original selected prefix");
 auto player14_bytes=bytes(player14_possession.state);std::istringstream player14_stream(player14_bytes);
 check(bytes(load_phase_session(player14_stream))==player14_bytes,"Player14 possession snapshot roundtrips");
 auto player14_ready=prepare_sampled_player5_result3_control(player14_possession.state).state;
 check(player14_ready.phase==FieldSessionPhase::control_ready && player14_ready.current.field.ball_owner==14 &&
       player14_ready.current.field.side==11 && !player14_ready.current.field.selected_defenders &&
       !player14_ready.human_defense_choices && !player14_ready.area_contact,
       "Opponent control setup clears the selected prefix at the original later boundary");
 auto player14_decision=player14_ready;player14_decision.phase=FieldSessionPhase::ai_cell_decision;
 player14_decision.current.field.ball_x=100;player14_decision.current.field.ball_y=107;player14_decision.current.field.ball_cell=75;
 player14_decision.current.field.elapsed=1;player14_decision.current.clock.remaining=363;
 player14_decision.control->controlled_player=9;player14_decision.control->last_carrier_cell=75;player14_decision.control->motion_requested=1;
 auto player14_choice=capture_sampled_field_ai_choice(player14_decision,1,0);
 check(player14_choice.phase==FieldSessionPhase::sampled_player14_ai_choice && player14_choice.action->action==1 &&
       player14_choice.recovery->attack_subtype==0,"Player14's observed pass choice has a distinct extended phase");
 auto player14_choice_bytes=bytes(player14_choice);std::istringstream player14_choice_stream(player14_choice_bytes);
 check(static_cast<unsigned char>(player14_choice_bytes[10])==143 && static_cast<unsigned char>(player14_choice_bytes[258])==0 &&
       static_cast<unsigned char>(player14_choice_bytes[259])==1 && bytes(load_phase_session(player14_choice_stream))==player14_choice_bytes,
       "Extended phase uses a version143 two-byte phase and roundtrips");
 auto bad_extended=player14_choice_bytes;bad_extended[259]=2;
 rejects([&]{std::istringstream in(bad_extended);load_phase_session(in);});
 auto zero_extended=player14_choice_bytes;zero_extended[259]=0;
 rejects([&]{std::istringstream in(zero_extended);load_phase_session(in);});
 auto truncated_extended=player14_choice_bytes;truncated_extended.pop_back();
 rejects([&]{std::istringstream in(truncated_extended);load_phase_session(in);});
 auto player14_prepared=prepare_sampled_field_contact_attack(player14_choice,sampled_attack_profiles,human_pass_content);
 check(player14_prepared.phase==FieldSessionPhase::sampled_player14_ai_prepared && player14_prepared.action->power==21 &&
       player14_prepared.recovery->attack_subtype==128 && player14_prepared.current.field.roster[14].stamina==100,
       "Player14's opponent pass preparation is profile calculated without spending stamina");
 auto player14_prepared_bytes=bytes(player14_prepared);std::istringstream player14_prepared_stream(player14_prepared_bytes);
 check(bytes(load_phase_session(player14_prepared_stream))==player14_prepared_bytes,"Extended prepared phase roundtrips");
 auto player14_presented=acknowledge_sampled_field_ai_presentation(player14_prepared);
 check(player14_presented.phase==FieldSessionPhase::sampled_player14_ai_presented && !player14_presented.current.clock.ball_mode &&
       !player14_presented.current.clock.script_flags,"Player14 pass presentation clears original control flags");
 auto player14_dispatch=advance_sampled_field_ai_dispatch(player14_presented);
 check(player14_dispatch.phase==FieldSessionPhase::sampled_player14_ai_dispatch && !player14_dispatch.control->last_carrier_cell,
       "Player14 pass dispatch clears the carrier-cell cache");
 auto player14_pass_handoff=advance_sampled_player14_pass_handoff(player14_dispatch,player3_race_mappings,player3_race_movement);
 check(player14_pass_handoff.phase==FieldSessionPhase::sampled_player14_pass_handoff &&
       !player14_pass_handoff.current.field.elapsed && player14_pass_handoff.current.field.movement_region==129 &&
       player14_pass_handoff.recovery->ball_variant==1 && player14_pass_handoff.control->facing==0 &&
       player14_pass_handoff.control->last_carrier_cell==128 && !player14_pass_handoff.control->display_parameter &&
       !player14_pass_handoff.control->motion_requested,"Player14 pass computes field movement and retains observed facing");
 auto player14_handoff_bytes=bytes(player14_pass_handoff);std::istringstream player14_handoff_stream(player14_handoff_bytes);
 check(bytes(load_phase_session(player14_handoff_stream))==player14_handoff_bytes,"Extended pass handoff snapshot roundtrips");
 auto player14_target_input=player14_pass_handoff;
 player14_target_input.current.field.motion[field_motion_index(19)].x=92<<8;
 player14_target_input.current.field.motion[field_motion_index(19)].y=108<<8;
 auto player14_target=capture_sampled_field_pass_target(player14_target_input,19,63,255,3);
 check(player14_target.phase==FieldSessionPhase::sampled_player14_pass_target && player14_target.sampled_field_pass &&
       player14_target.sampled_field_pass->recipient==19 && player14_target.sampled_field_pass->destination==63,
       "Player14's observed target retains player19 and cell63");
 auto player14_target_bytes=bytes(player14_target);std::istringstream player14_target_stream(player14_target_bytes);
 check(static_cast<unsigned char>(player14_target_bytes[10])==144 && bytes(load_phase_session(player14_target_stream))==player14_target_bytes,
       "Extended pass target and recipient roundtrip");
 auto truncated_player14_target=player14_target_bytes;truncated_player14_target.pop_back();
 rejects([&]{std::istringstream in(truncated_player14_target);load_phase_session(in);});
 auto wrong_extended_target=player14_target_bytes;wrong_extended_target[10]=static_cast<char>(143);
 rejects([&]{std::istringstream in(wrong_extended_target);load_phase_session(in);});
 rejects([&]{capture_sampled_field_pass_target(player14_target_input,19,64,255,3);});
 auto player14_flight=begin_sampled_field_pass_flight(player14_target,101,13,pass_geometry,pass_velocity);
 check(player14_flight.phase==FieldSessionPhase::area_ball_advance_ready && !player14_flight.sampled_field_pass &&
       player14_flight.area_ai_target->recipient==19 && player14_flight.shot_flight->destination==63 &&
       player14_flight.ball_trajectory->flags==1 && (player14_flight.ball_trajectory->x&255)==101 &&
       (player14_flight.ball_trajectory->y&255)==13,"Player14 pass enters numeric flight with the observed fractions");
 rejects([&]{begin_sampled_field_pass_flight(player14_target,100,13,pass_geometry,pass_velocity);});
 auto player19_returned=player14_flight;player19_returned.phase=FieldSessionPhase::area_pass_keeper_entry;
 player19_returned.current.field.ball_owner=19;player19_returned.current.field.ball_x=91;player19_returned.current.field.ball_y=109;player19_returned.current.field.ball_cell=63;
 player19_returned.ball_trajectory->x=91<<8;player19_returned.ball_trajectory->y=109<<8;
 auto player19_new_control=continue_sampled_field_pass_control(player19_returned);
 check(player19_new_control.phase==FieldSessionPhase::player_control && player19_new_control.current.field.ball_owner==19 &&
       player19_new_control.control->facing==0 && player19_new_control.saved_ball_flight_flag==1 &&
       !player19_new_control.area_ai_target && !player19_new_control.ball_trajectory,
       "Player14's pass returns player19 to control with the observed facing");
 auto earlier_race_input=human_pass_handoff;
 for(auto& m:earlier_race_input.current.field.motion){m.x=160<<8;m.y=140<<8;m.status=0;}
 for(unsigned id=1;id<=21;++id)if(id!=11 && id!=5)earlier_race_input.current.field.roster[id].type=1;
 earlier_race_input.current.field.motion[field_motion_index(3)].x=96<<8;
 earlier_race_input.current.field.motion[field_motion_index(3)].y=108<<8;
 earlier_race_input.current.field.motion[field_motion_index(17)].x=120<<8;
 earlier_race_input.current.field.motion[field_motion_index(17)].y=108<<8;
 earlier_race_input.recovery->opposing_player=1;
 const auto earlier_before=bytes(earlier_race_input);
 auto earlier_recovery=resolve_sampled_human_empty_pass_recovery(earlier_race_input,player3_race_mappings,player3_race_movement,player3_race_content);
 check(earlier_recovery.state.phase==FieldSessionPhase::recovered_owner && earlier_recovery.state.current.field.ball_owner==3 &&
       earlier_recovery.state.current.field.motion[field_motion_index(3)].x>>8==108 && earlier_recovery.state.saved_ball_flight_flag==1 &&
       earlier_recovery.recipient && earlier_recovery.recipient->ball_owner==3 && bytes(earlier_race_input)==earlier_before,
       "Earlier empty pass calculates the timeout and stationary recovery to player3");
 auto earlier_wrong=earlier_race_input;earlier_wrong.current.field.motion[field_motion_index(3)].x=108<<8;
 const auto earlier_wrong_before=bytes(earlier_wrong);
 rejects([&]{resolve_sampled_human_empty_pass_recovery(earlier_wrong,player3_race_mappings,player3_race_movement,player3_race_content);});
 check(bytes(earlier_wrong)==earlier_wrong_before,"An unobserved early contest winner is rejected atomically");
 auto player19_new_decision=prepare_session_control(player19_new_control).state;
 player19_new_decision.phase=FieldSessionPhase::ai_cell_decision;player19_new_decision.current.field.ball_x=85;
 player19_new_decision.current.field.ball_y=109;player19_new_decision.current.field.ball_cell=51;
 player19_new_decision.current.field.elapsed=1;player19_new_decision.current.field.roster[19]={23,15,100};
 player19_new_decision.current.field.roster[0]={1,0,44};
 player19_new_decision.current.clock.remaining=361;player19_new_decision.control->controlled_player=7;
 player19_new_decision.control->last_carrier_cell=51;player19_new_decision.control->motion_requested=1;
 auto player19_new_shot=capture_sampled_field_ai_choice(player19_new_decision,0,0);
 check(player19_new_shot.phase==FieldSessionPhase::sampled_player19_return_shot_choice && player19_new_shot.action->action==0 &&
       player19_new_shot.action->power==21,"Player19's next observed shot choice is saved in its own phase");
 auto player19_new_prepared=prepare_sampled_field_contact_attack(player19_new_shot,sampled_attack_profiles,rejected_content);
 check(player19_new_prepared.phase==FieldSessionPhase::sampled_player19_return_shot_prepared && player19_new_prepared.action->power==22 &&
       player19_new_prepared.recovery->attack_subtype==128 && player19_new_prepared.current.field.roster[19].stamina==100,
       "Player19's next shot uses calculated opponent preparation");
 auto player19_new_bytes=bytes(player19_new_prepared);std::istringstream player19_new_stream(player19_new_bytes);
 check(bytes(load_phase_session(player19_new_stream))==player19_new_bytes,"Player19 extended shot phase roundtrips");
 auto player19_new_presented=acknowledge_sampled_field_ai_presentation(player19_new_prepared);
 check(player19_new_presented.phase==FieldSessionPhase::sampled_player19_return_shot_presented &&
       !player19_new_presented.current.clock.script_flags && !player19_new_presented.current.clock.ball_mode,
       "Player19's return shot presentation is explicitly acknowledged");
 auto player19_new_clock=advance_sampled_field_ai_dispatch(player19_new_presented);
 check(player19_new_clock.phase==FieldSessionPhase::sampled_player19_return_shot_clock && player19_new_clock.current.clock.remaining==360 &&
       player19_new_clock.current.field.elapsed==2 && !player19_new_clock.control->last_carrier_cell,
       "Player19's shot dispatch applies its observed clock");
 auto player19_new_approach=advance_sampled_player19_return_shot_approach(player19_new_clock,player3_race_mappings,player3_race_movement);
 check(player19_new_approach.phase==FieldSessionPhase::sampled_player19_return_shot_approach &&
       !player19_new_approach.current.field.elapsed && player19_new_approach.current.field.movement_region==129,
       "Player19's shot computes the intervening field movement");
 auto player19_new_handoff=acknowledge_sampled_field_ai_pass_handoff(player19_new_approach);
 check(player19_new_handoff.phase==FieldSessionPhase::sampled_player19_return_shot_handoff && player19_new_handoff.recovery->ball_variant==1 &&
       player19_new_handoff.control->last_carrier_cell==128 && !player19_new_handoff.control->display_parameter &&
       player19_new_handoff.control->motion_requested==2,"Player19's shot reaches the observed flight handoff");
 auto player19_handoff_bytes=bytes(player19_new_handoff);std::istringstream player19_handoff_stream(player19_handoff_bytes);
 check(bytes(load_phase_session(player19_handoff_stream))==player19_handoff_bytes,"Player19 return shot handoff roundtrips");
 auto player19_prompt_field=player19_new_handoff.current.field;player19_prompt_field.ball_x=52;
 player19_prompt_field.ball_y=124;player19_prompt_field.ball_cell=5;player19_prompt_field.selected_defenders=1;
 auto player19_new_prompt=capture_sampled_shot_keeper_prompt(player19_new_handoff,player19_prompt_field);
 check(player19_new_prompt.phase==FieldSessionPhase::sampled_player19_return_keeper_prompt &&
       player19_new_prompt.current.field.roster[0].stamina==44 && player19_new_prompt.recovery->x_jitter_control==110 &&
       !player19_new_prompt.saved_ball_flight_flag && player19_new_prompt.current.field.selected_defenders==1,
       "Player19's captured shot reaches the observed keeper prompt with cleared flight flag");
 auto player19_prompt_bytes=bytes(player19_new_prompt);std::istringstream player19_prompt_stream(player19_prompt_bytes);
 check(bytes(load_phase_session(player19_prompt_stream))==player19_prompt_bytes,"Player19 return keeper prompt roundtrips");
 auto player19_new_keeper_choice=poll_sampled_goalkeeper_prompt(player19_new_prompt,1,{23,22});
 auto player19_new_keeper_confirm=poll_sampled_goalkeeper_prompt(player19_new_keeper_choice.state,128,{23,22});
 auto player19_new_keeper_precompare=acknowledge_sampled_goalkeeper_overlay(player19_new_keeper_confirm.state);
 check(player19_new_keeper_precompare.state.phase==FieldSessionPhase::sampled_player19_return_keeper_precompare &&
       player19_new_keeper_precompare.state.keeper_menu_action==0 && !player19_new_keeper_precompare.state.saved_ball_flight_flag,
       "Player19 return keeper selection and overlay acknowledgement are saved");
 auto player19_keeper_select_bytes=bytes(player19_new_keeper_precompare.state);std::istringstream player19_keeper_select_stream(player19_keeper_select_bytes);
 check(static_cast<unsigned char>(player19_keeper_select_bytes[10])==145 && bytes(load_phase_session(player19_keeper_select_stream))==player19_keeper_select_bytes,
       "Extended keeper selection with zero flight flag roundtrips");
 auto truncated_extended_keeper=player19_keeper_select_bytes;truncated_extended_keeper.pop_back();
 rejects([&]{std::istringstream in(truncated_extended_keeper);load_phase_session(in);});
 auto invalid_extended_keeper=player19_keeper_select_bytes;invalid_extended_keeper.back()=1;
 rejects([&]{std::istringstream in(invalid_extended_keeper);load_phase_session(in);});
 auto player19_new_keeper_result=compare_sampled_home_goalkeeper(player19_new_keeper_precompare.state,keeper_test_content);
 check(player19_new_keeper_result.phase==FieldSessionPhase::sampled_player19_return_keeper_compared &&
       player19_new_keeper_result.current.field.roster[0].stamina==24 && player19_new_keeper_result.keeper_menu_action==0,
       "Player19 return keeper comparison spends the observed stamina");
 auto player19_new_goal=prepare_sampled_opponent_goal(player19_new_keeper_result,keeper_test_content);
 check(player19_new_goal.phase==FieldSessionPhase::sampled_player19_return_goal_scored &&
       player19_new_goal.score->left_goals==2 && player19_new_goal.score->right_goals==3 &&
       player19_new_goal.recovery->opposing_player==5 && player19_new_goal.control->facing==0 && player19_new_goal.control->motion_requested==3,
       "Player19's observed keeper result records the 2-3 goal");
 auto player19_goal_bytes=bytes(player19_new_goal);std::istringstream player19_goal_stream(player19_goal_bytes);
 check(bytes(load_phase_session(player19_goal_stream))==player19_goal_bytes,"Player19 return goal snapshot roundtrips");
 auto player19_goal_presented=acknowledge_sampled_opponent_goal_presentation(player19_new_goal);
 auto player19_goal_restart=begin_sampled_opponent_goal_restart(player19_goal_presented);
 check(player19_goal_presented.phase==FieldSessionPhase::sampled_player19_return_goal_presented &&
       player19_goal_restart.phase==FieldSessionPhase::sampled_player19_return_goal_restart &&
       player19_goal_restart.current.field.side==0 && player19_goal_restart.score->right_goals==3 &&
       player19_goal_restart.recovery->opposing_player==5 && player19_goal_restart.control->facing==0,
       "The 2-3 goal presentation reaches the observed home kickoff restart");
 auto player19_restart_bytes=bytes(player19_goal_restart);std::istringstream player19_restart_stream(player19_restart_bytes);
 check(bytes(load_phase_session(player19_restart_stream))==player19_restart_bytes,"Player19 return restart snapshot roundtrips");
 auto player23_formation=prepare_session_kickoff_formation(player19_goal_restart,0,c);
 auto player23_receiver=prepare_session_kickoff_receiver(player23_formation,0,{5,6,7,9,10});
 auto player23_ball=prepare_session_kickoff_ball(player23_receiver);
 auto player23_handoff=continue_prepared_session_kickoff(player23_ball);
 check(player23_handoff.phase==FieldSessionPhase::kickoff_control_pending && player23_handoff.current.field.ball_owner==5 &&
       player23_handoff.score->left_goals==2 && player23_handoff.score->right_goals==3 &&
       player23_handoff.current.clock.remaining==360 && player23_handoff.control->facing==0 &&
       !player23_handoff.saved_ball_flight_flag,"The 2-3 restart returns the observed home receiver to control");
 auto player23_bytes=bytes(player23_handoff);std::istringstream player23_stream(player23_bytes);
 check(bytes(load_phase_session(player23_stream))==player23_bytes,"The 2-3 kickoff handoff roundtrips");
 auto player23_rejected_input=player23_handoff;player23_rejected_input.kickoff_receiver.reset();
 player23_rejected_input.phase=FieldSessionPhase::attack_choice_pending;player23_rejected_input.current.clock.remaining=355;
 player23_rejected_input.current.field.ball_owner=5;player23_rejected_input.current.field.ball_x=108;player23_rejected_input.current.field.ball_y=108;
 player23_rejected_input.current.field.ball_cell=87;player23_rejected_input.current.field.elapsed=1;
 player23_rejected_input.current.field.roster[5].type=23;player23_rejected_input.current.field.roster[5].stamina=0;
 player23_rejected_input.current.field.roster[17].type=23;player23_rejected_input.current.field.roster[17].stamina=100;
 player23_rejected_input.current.field.motion[field_motion_index(17)].status=0;
 player23_rejected_input.current.field.selected_defenders=1;player23_rejected_input.scheduling->selected_ids={17,0,0,0,0};
 player23_rejected_input.action->action=0;player23_rejected_input.action->cursor=224;player23_rejected_input.action->menu_route=0;
 player23_rejected_input.recovery->attack_subtype=128;
 auto player23_returned=acknowledge_sampled_unaffordable_shot(player23_rejected_input,sampled_attack_profiles,rejected_content);
 check(player23_returned.phase==FieldSessionPhase::human_action_menu && !player23_returned.action->cursor &&
       !player23_returned.recovery->attack_subtype && player23_returned.current.field.selected_defenders==1 &&
       !player23_returned.current.field.roster[5].stamina,"The exhausted 2-3 contact shot returns to the same menu");
 auto player23_pass_choice=poll_human_action_menu(player23_returned,2,pass_menu_content).state;
 auto player23_pass_confirm=poll_human_action_menu(player23_pass_choice,128,pass_menu_content).state;
 auto player23_target=begin_sampled_human_pass_target(player23_pass_confirm);
 check(player23_target.phase==FieldSessionPhase::sampled_score23_pass_target && *player23_target.human_pass_cursor==87 &&
       player23_target.current.clock.ball_mode==129,"The exhausted 2-3 pass opens its observed target cursor");
 auto player23_target_right=poll_sampled_human_pass_target(player23_target,1,0);
 check(*player23_target_right.human_pass_cursor==99 && player23_target_right.human_pass_steps==1,
       "The 2-3 target cursor moves through the observed first cell");
 auto player23_target_bytes=bytes(player23_target_right);std::istringstream player23_target_stream(player23_target_bytes);
 check(static_cast<unsigned char>(player23_target_bytes[10])==146 && bytes(load_phase_session(player23_target_stream))==player23_target_bytes,
       "Extended human pass cursor roundtrips");
 auto truncated_score23=player23_target_bytes;truncated_score23.pop_back();
 rejects([&]{std::istringstream i(truncated_score23);load_phase_session(i);});
 auto wrong_score23_version=player23_target_bytes;wrong_score23_version[10]=static_cast<char>(143);
 rejects([&]{std::istringstream i(wrong_score23_version);load_phase_session(i);});
 auto wrong_score23_cursor=player23_target_bytes;wrong_score23_cursor[wrong_score23_cursor.size()-2]=77;
 rejects([&]{std::istringstream i(wrong_score23_cursor);load_phase_session(i);});
 auto player23_empty=poll_sampled_human_pass_target(player23_target,0,128);
 auto player23_cancel=poll_sampled_human_pass_target(player23_target,0,64);
 check(player23_empty.phase==FieldSessionPhase::sampled_score23_empty_confirmed && player23_cancel.phase==FieldSessionPhase::sampled_score23_pass_cancel,
       "The 2-3 same-cell confirmation and cancellation remain distinct");
 auto player23_prepared=prepare_sampled_human_empty_pass(player23_empty,sampled_attack_profiles,human_pass_content);
 check(player23_prepared.phase==FieldSessionPhase::sampled_score23_pass_prepared && !player23_prepared.action->power &&
       !player23_prepared.current.field.roster[5].stamina && player23_prepared.recovery->opposing_player==17 &&
       !player23_prepared.current.clock.ball_mode,"The 2-3 contact pass prepares at weakened power zero");
 auto player23_defender=prepare_sampled_player3_pass_defender(player23_prepared,sampled_attack_profiles,player3_defense_content);
 check(player23_defender.phase==FieldSessionPhase::sampled_score23_defender_prepared && player23_defender.area_contact &&
       player23_defender.area_contact->defender_power==18 && player23_defender.area_contact->cost==60 &&
       player23_defender.control->facing==64,"The 2-3 pass prepares the observed defender17");
 auto player23_compared=compare_sampled_player3_pass_defender(player23_defender,player3_direct_comparison);
 check(player23_compared.phase==FieldSessionPhase::sampled_score23_defender_compared &&
       player23_compared.area_contact->score==255 && !player23_compared.area_sequence_result &&
       !player23_compared.saved_ball_flight_flag,"The exhausted 2-3 pass retains its comparison and cleared flight flag");
 auto player23_compared_bytes=bytes(player23_compared);std::istringstream player23_compared_stream(player23_compared_bytes);
 check(bytes(load_phase_session(player23_compared_stream))==player23_compared_bytes,
       "The 2-3 defender comparison snapshot roundtrips");
 auto player23_contact=advance_sampled_player3_pass_contact(player23_compared,player3_prelude,home_experience);
 auto player23_dispatch=advance_sampled_player3_pass_dispatch(player23_contact);
 check(player23_contact.phase==FieldSessionPhase::sampled_score23_contact_clock && player23_contact.current.clock.remaining==354 &&
       player23_contact.current.field.elapsed==2 && player23_contact.current.field.motion[field_motion_index(17)].status==6 &&
       player23_contact.experience->totals[0]==player23_compared.experience->totals[0]+3 &&
       player23_dispatch.phase==FieldSessionPhase::sampled_score23_dispatch_clock && player23_dispatch.current.clock.remaining==353 &&
       player23_dispatch.current.field.elapsed==3 && player23_dispatch.area_defender_cursor==1,
       "The 2-3 pass advances the observed contact and dispatch clocks");
 auto player23_motion_input=player23_dispatch;
 for(unsigned id=1;id<=21;++id)if(id!=11 && id!=5 && id!=17)player23_motion_input.current.field.roster[id].type=1;
 auto player23_motion=advance_sampled_score23_pass_handoff(player23_motion_input,player3_race_mappings,player3_race_movement);
 check(player23_motion.phase==FieldSessionPhase::sampled_score23_pass_handoff && !player23_motion.current.field.elapsed &&
       player23_motion.current.field.movement_region==129 && !player23_motion.saved_ball_flight_flag,
       "The 2-3 pass handoff advances the field and keeps the original flight flag");
 auto player23_race_input=player23_motion;
 for(auto& actor:player23_race_input.current.field.motion){actor.x=180<<8;actor.y=180<<8;}
 player23_race_input.current.field.motion[field_motion_index(1)].x=90<<8;
 player23_race_input.current.field.motion[field_motion_index(1)].y=108<<8;
 player23_race_input.current.field.motion[field_motion_index(19)].x=120<<8;
 player23_race_input.current.field.motion[field_motion_index(19)].y=108<<8;
 auto player23_recovered=resolve_sampled_human_empty_pass_recovery(player23_race_input,player3_race_mappings,player3_race_movement,player3_race_content);
 check(player23_recovered.recipient && player23_recovered.recipient->ball_owner==19 &&
       player23_recovered.state.current.field.ball_owner==19 && player23_recovered.state.current.field.side==11 &&
       player23_recovered.state.saved_ball_flight_flag==1 && player23_recovered.state.area_defender_cursor==1,
       "The 2-3 pass timeout recovers possession for the observed opponent");
 auto player23_control=continue_session_recovery(player23_recovered.state);
 auto player23_ready=prepare_session_control(player23_control).state;
 check(player23_control.phase==FieldSessionPhase::player_control && player23_ready.phase==FieldSessionPhase::control_ready &&
       player23_ready.current.field.ball_owner==19 && player23_ready.area_defender_cursor==1 && player23_control.control->facing==0,
       "The 2-3 recovered opponent reaches field control with its original cursor residue");
 auto player23_ai_input=player23_ready;player23_ai_input.phase=FieldSessionPhase::ai_cell_decision;
 player23_ai_input.current.clock.remaining=351;player23_ai_input.current.clock.extra_time=13;
 player23_ai_input.current.field.ball_x=101;player23_ai_input.current.field.ball_y=108;player23_ai_input.current.field.ball_cell=75;
 player23_ai_input.current.field.elapsed=1;player23_ai_input.current.field.roster[19].type=23;
 player23_ai_input.control->controlled_player=5;player23_ai_input.control->last_carrier_cell=75;
 player23_ai_input.action->action=1;player23_ai_input.action->power=0;player23_ai_input.recovery->attack_subtype=0;
 auto player23_ai_choice=capture_sampled_field_ai_choice(player23_ai_input,2,0);
 check(player23_ai_choice.phase==FieldSessionPhase::control_clock_pending && player23_ai_choice.action->action==2 &&
       player23_ai_choice.area_defender_cursor==1,"The 2-3 opponent's observed dribble choice returns to the control clock");
 auto player23_next_ai=player23_ai_choice;player23_next_ai.phase=FieldSessionPhase::ai_cell_decision;
 player23_next_ai.current.clock.remaining=349;player23_next_ai.current.field.ball_x=94;player23_next_ai.current.field.ball_cell=63;
 auto player23_next_ai_bytes=bytes(player23_next_ai);std::istringstream player23_next_ai_stream(player23_next_ai_bytes);
 check(bytes(load_phase_session(player23_next_ai_stream))==player23_next_ai_bytes,
       "The next opponent AI decision retains its original defender cursor");
 auto player23_outside_trace=player23_next_ai;player23_outside_trace.current.clock.remaining=348;
 rejects([&]{validate_phase_session(player23_outside_trace);});
 player23_next_ai.control->last_carrier_cell=63;
 player23_next_ai.recovery->ball_variant=0;
 auto score35_next_ai=player23_next_ai;score35_next_ai.score=MatchScoreContext{3,5,4};score35_next_ai.current.clock.remaining=294;
 score35_next_ai.current.field.ball_x=85;score35_next_ai.current.field.ball_y=157;score35_next_ai.current.field.ball_cell=57;
 score35_next_ai.control->controlled_player=2;score35_next_ai.control->last_carrier_cell=57;score35_next_ai.area_defender_cursor=0;
 score35_next_ai.action->action=1;score35_next_ai.action->power=21;score35_next_ai.recovery->attack_subtype=128;
 auto score35_next_shot=capture_sampled_field_ai_choice(score35_next_ai,0,0);
 check(score35_next_shot.phase==FieldSessionPhase::sampled_score23_shot_choice && score35_next_shot.action->power==21,"The 3-5 player19 shot choice retains its prior power");
 auto score35_next_prepared=prepare_sampled_field_contact_attack(score35_next_shot,sampled_attack_profiles,rejected_content);
 check(score35_next_prepared.action->power==22 && score35_next_prepared.recovery->attack_subtype==128,"The 3-5 opponent shot calculates its preparation");
 auto score35_next_presented=acknowledge_sampled_field_ai_presentation(score35_next_prepared);
 auto score35_next_clock=advance_sampled_field_ai_dispatch(score35_next_presented);
 auto score35_next_approach=advance_sampled_player19_return_shot_approach(score35_next_clock,player3_race_mappings,player3_race_movement);
 auto score35_next_handoff=acknowledge_sampled_field_ai_pass_handoff(score35_next_approach);
 ShotApproachContent score35_approach_content{};score35_approach_content.outer_distance.fill(4);score35_approach_content.inner_distance.fill(4);
 auto score35_next_target=prepare_session_shot_approach(score35_next_handoff,score35_approach_content).state;
 check(score35_next_target.shot_flight->distance==36 && score35_next_target.shot_flight->destination==5,"The 3-5 opponent shot calculates its target");
 auto score35_next_flight=begin_prepared_session_ball_flight(score35_next_target,66,124,pass_geometry,pass_velocity);
 check(score35_next_flight.phase==FieldSessionPhase::sampled_score23_ball_advance_ready && !score35_next_flight.saved_ball_flight_flag,"The 3-5 opponent shot starts numeric flight");
 rejects([&]{begin_prepared_session_ball_flight(score35_next_target,65,124,pass_geometry,pass_velocity);});
 auto score35_arrival=score35_next_flight;score35_arrival.phase=FieldSessionPhase::sampled_score23_ball_arrival_callback;
 score35_arrival.current.field.ball_x=52;score35_arrival.current.field.ball_y=124;score35_arrival.current.field.ball_cell=5;
 score35_arrival.ball_trajectory->x=52<<8;score35_arrival.ball_trajectory->y=124<<8;score35_arrival.ball_trajectory->direction=159;score35_arrival.recovery->x_jitter_control=159;score35_arrival.current.field.roster[0].stamina=84;
 auto score35_keeper_prompt=continue_session_unopposed_arrival(score35_arrival);
 check(score35_keeper_prompt.phase==FieldSessionPhase::sampled_score23_keeper_prompt && score35_keeper_prompt.current.field.roster[0].stamina==84,"The 3-5 shot enters the home goalkeeper prompt");
 auto score35_keeper_selected=poll_sampled_goalkeeper_prompt(score35_keeper_prompt,1,{23,22});
 auto score35_keeper_confirmed=poll_sampled_goalkeeper_prompt(score35_keeper_selected.state,128,{23,22});
 auto score35_keeper_precompare=acknowledge_sampled_goalkeeper_overlay(score35_keeper_confirmed.state);
 auto score35_keeper_result=compare_sampled_home_goalkeeper(score35_keeper_precompare.state,keeper_test_content);
 check(score35_keeper_result.phase==FieldSessionPhase::sampled_score23_keeper_compared && score35_keeper_result.current.field.roster[0].stamina==64,
       "The 3-5 goalkeeper comparison spends the observed stamina");
 auto invalid_score35_keeper=score35_keeper_result;invalid_score35_keeper.current.field.roster[0].stamina=63;
 rejects([&]{bytes(invalid_score35_keeper);});
 auto score36_goal=prepare_sampled_opponent_goal(score35_keeper_result,keeper_test_content);
 check(score36_goal.score->left_goals==3 && score36_goal.score->right_goals==6 && score36_goal.recovery->opposing_player==4,
       "The observed opponent shot records the 3-6 goal");
 auto score36_presented=acknowledge_sampled_opponent_goal_presentation(score36_goal);
 auto score36_restart=begin_sampled_opponent_goal_restart(score36_presented);
 check(score36_presented.current.field.side==0 && score36_restart.phase==FieldSessionPhase::sampled_score24_goal_restart && score36_restart.control->facing==64,
       "The 3-6 goal advances to the observed home restart");
 auto invalid_score36_goal=score36_goal;invalid_score36_goal.score->right_goals=5;
 rejects([&]{bytes(invalid_score36_goal);});
 auto invalid_score36_restart=score36_restart;invalid_score36_restart.control->facing=0;
 rejects([&]{bytes(invalid_score36_restart);});
 auto score36_formation=prepare_session_kickoff_formation(score36_restart,0,c);
 auto score36_receiver=prepare_session_kickoff_receiver(score36_formation,0,{5,6,7,9,10});
 auto score36_ball=prepare_session_kickoff_ball(score36_receiver);
 auto score36_handoff=continue_prepared_session_kickoff(score36_ball);
 check(score36_handoff.current.field.ball_owner==5 && score36_handoff.control->controlled_player==1 && !score36_handoff.control->facing,
       "The 3-6 restart reaches home player5 control");
 auto player23_next_shot=capture_sampled_field_ai_choice(player23_next_ai,0,0);
 check(player23_next_shot.phase==FieldSessionPhase::sampled_score23_shot_choice && !player23_next_shot.action->action &&
       player23_next_shot.area_defender_cursor==1,"The 2-3 opponent shot preserves its observed cursor");
 auto player23_next_prepared=prepare_sampled_field_contact_attack(player23_next_shot,sampled_attack_profiles,rejected_content);
 check(player23_next_prepared.phase==FieldSessionPhase::sampled_score23_shot_prepared && player23_next_prepared.action->power==22 &&
       player23_next_prepared.recovery->attack_subtype==128,"The 2-3 shot uses the original profile preparation");
 auto player23_next_presented=acknowledge_sampled_field_ai_presentation(player23_next_prepared);
 auto player23_next_clock=advance_sampled_field_ai_dispatch(player23_next_presented);
 check(player23_next_clock.phase==FieldSessionPhase::sampled_score23_shot_clock && player23_next_clock.current.clock.remaining==348 &&
       player23_next_clock.current.field.elapsed==2 && player23_next_clock.area_defender_cursor==1,
       "The 2-3 opponent shot acknowledges presentation and advances the clock");
 auto player23_next_shot_bytes=bytes(player23_next_clock);std::istringstream player23_next_shot_stream(player23_next_shot_bytes);
 check(bytes(load_phase_session(player23_next_shot_stream))==player23_next_shot_bytes,"The 2-3 shot clock snapshot roundtrips");
 auto player23_next_approach=advance_sampled_player19_return_shot_approach(player23_next_clock,player3_race_mappings,player3_race_movement);
 auto player23_next_handoff=acknowledge_sampled_field_ai_pass_handoff(player23_next_approach);
 check(player23_next_approach.phase==FieldSessionPhase::sampled_score23_shot_approach && !player23_next_approach.current.field.elapsed &&
       !player23_next_approach.control->last_carrier_cell && player23_next_approach.current.field.movement_region==129 &&
       player23_next_handoff.phase==FieldSessionPhase::sampled_score23_shot_handoff && player23_next_handoff.recovery->ball_variant==1 &&
       player23_next_handoff.control->last_carrier_cell==128 && player23_next_handoff.control->motion_requested==2,
       "The 2-3 shot calculates field movement before the flight handoff");
 ShotApproachContent player23_shot_content{};player23_shot_content.outer_distance.fill(5);player23_shot_content.inner_distance.fill(5);
 auto player23_target_shot=prepare_session_shot_approach(player23_next_handoff,player23_shot_content).state;
 check(player23_target_shot.phase==FieldSessionPhase::sampled_score23_shot_target && player23_target_shot.shot_flight &&
       player23_target_shot.shot_flight->distance==45 && player23_target_shot.shot_flight->destination==5,
       "The 2-3 shot calculates the observed goalkeeper destination");
 auto player23_flight=begin_prepared_session_ball_flight(player23_target_shot,44,73,pass_geometry,pass_velocity);
 check(player23_flight.phase==FieldSessionPhase::sampled_score23_ball_advance_ready && player23_flight.ball_trajectory &&
       !player23_flight.saved_ball_flight_flag && player23_flight.area_defender_cursor==1 &&
       (player23_flight.ball_trajectory->x&255)==44 && (player23_flight.ball_trajectory->y&255)==73,
       "The opponent shot starts numeric flight with the observed fractions and flag");
 auto player23_flight_bytes=bytes(player23_flight);std::istringstream player23_flight_stream(player23_flight_bytes);
 check(bytes(load_phase_session(player23_flight_stream))==player23_flight_bytes,"The 2-3 shot trajectory roundtrips");
 rejects([&]{begin_prepared_session_ball_flight(player23_target_shot,43,73,pass_geometry,pass_velocity);});
 auto player23_arrival=player23_flight;player23_arrival.phase=FieldSessionPhase::sampled_score23_ball_arrival_callback;
 player23_arrival.current.field.ball_x=52;player23_arrival.current.field.ball_y=124;player23_arrival.current.field.ball_cell=5;
 player23_arrival.ball_trajectory->x=52<<8;player23_arrival.ball_trajectory->y=124<<8;
 player23_arrival.ball_trajectory->direction=113;player23_arrival.recovery->x_jitter_control=113;
 player23_arrival.current.field.roster[0].stamina=42;
 auto player23_keeper_prompt=continue_session_unopposed_arrival(player23_arrival);
 check(player23_keeper_prompt.phase==FieldSessionPhase::sampled_score23_keeper_prompt &&
       player23_keeper_prompt.current.field.selected_defenders==1 && player23_keeper_prompt.scheduling->selected_ids[0]==0 &&
       player23_keeper_prompt.action->menu_route==4 && !player23_keeper_prompt.action->cursor &&
       !player23_keeper_prompt.control->motion_requested && !player23_keeper_prompt.shot_flight && !player23_keeper_prompt.ball_trajectory &&
       player23_keeper_prompt.area_defender_cursor==1 && !player23_keeper_prompt.saved_ball_flight_flag,
       "The calculated 2-3 shot reaches the observed goalkeeper prompt");
 auto player23_keeper_selected=poll_sampled_goalkeeper_prompt(player23_keeper_prompt,1,{23,22});
 auto player23_keeper_confirmed=poll_sampled_goalkeeper_prompt(player23_keeper_selected.state,128,{23,22});
 auto player23_keeper_overlay=acknowledge_sampled_goalkeeper_overlay(player23_keeper_confirmed.state);
 auto player23_keeper_compared=compare_sampled_home_goalkeeper(player23_keeper_overlay.state,keeper_test_content);
 check(player23_keeper_selected.state.phase==FieldSessionPhase::sampled_score23_keeper_selected &&
       player23_keeper_confirmed.state.phase==FieldSessionPhase::sampled_score23_keeper_confirmed &&
       player23_keeper_overlay.state.phase==FieldSessionPhase::sampled_score23_keeper_precompare &&
       player23_keeper_compared.phase==FieldSessionPhase::sampled_score23_keeper_compared &&
       player23_keeper_compared.current.field.roster[0].stamina==22 && player23_keeper_compared.area_defender_cursor==1,
       "The 2-3 goalkeeper spends twenty stamina for the observed comparison");
 auto player23_keeper_bytes=bytes(player23_keeper_compared);std::istringstream player23_keeper_stream(player23_keeper_bytes);
 check(bytes(load_phase_session(player23_keeper_stream))==player23_keeper_bytes,"The 2-3 goalkeeper choice roundtrips");
 auto player24_goal=prepare_sampled_opponent_goal(player23_keeper_compared,keeper_test_content);
 check(player24_goal.phase==FieldSessionPhase::sampled_score24_goal_scored && player24_goal.score->left_goals==2 &&
       player24_goal.score->right_goals==4 && player24_goal.recovery->opposing_player==1 && !player24_goal.control->facing &&
       player24_goal.control->motion_requested==3 && !player24_goal.area_defender_cursor,
       "The observed opponent goal reaches 2-4 with its original presentation state");
 auto player24_presented=acknowledge_sampled_opponent_goal_presentation(player24_goal);
 auto player24_restart=begin_sampled_opponent_goal_restart(player24_presented);
 check(player24_presented.phase==FieldSessionPhase::sampled_score24_goal_presented && player24_presented.current.field.side==0 &&
       !player24_presented.control->motion_requested && player24_restart.phase==FieldSessionPhase::sampled_score24_goal_restart &&
       player24_restart.score->right_goals==4,"The 2-4 opponent goal returns to the home kickoff restart");
 auto player24_formation_input=player24_restart;player24_formation_input.current.field.motion[field_motion_index(17)].status=2;
 auto player24_formation=prepare_session_kickoff_formation(player24_formation_input,0,c);
 check(player24_formation.phase==FieldSessionPhase::kickoff_formation_placed &&
       !player24_formation.current.field.motion[field_motion_index(17)].status && !player24_formation.keeper_menu_action,
       "The 2-4 kickoff resets the observed player status and consumed goalkeeper choice");
 auto player24_receiver=prepare_session_kickoff_receiver(player24_formation,0,{5,6,7,9,10});
 auto player24_ball=prepare_session_kickoff_ball(player24_receiver);
 auto player24_handoff=continue_prepared_session_kickoff(player24_ball);
 check(player24_handoff.current.field.ball_owner==5 && player24_handoff.score->right_goals==4 &&
       !player24_handoff.control->facing,"The 2-4 kickoff returns the observed home receiver");
 auto player24_dribble_input=player24_handoff;player24_dribble_input.kickoff_receiver.reset();
 player24_dribble_input.phase=FieldSessionPhase::attack_choice_pending;player24_dribble_input.current.clock.remaining=342;
 player24_dribble_input.current.field.elapsed=1;player24_dribble_input.current.field.ball_x=108;player24_dribble_input.current.field.ball_y=108;
 player24_dribble_input.current.field.ball_cell=87;player24_dribble_input.current.field.roster[5].type=23;player24_dribble_input.current.field.roster[5].stamina=0;
 player24_dribble_input.current.field.roster[17].type=23;player24_dribble_input.current.field.roster[17].stamina=100;
 player24_dribble_input.current.field.selected_defenders=1;player24_dribble_input.scheduling->selected_ids={17,0,0,0,0};
 player24_dribble_input.action->action=2;player24_dribble_input.action->cursor=227;player24_dribble_input.action->menu_route=0;
 player24_dribble_input.action->power=22;player24_dribble_input.recovery->opposing_player=1;player24_dribble_input.recovery->attack_subtype=128;
 auto player24_dribble_choice=capture_sampled_score24_dribble_defense(player24_dribble_input,1,0);
 auto player24_dribble_prepared=prepare_sampled_player5_dribble(player24_dribble_choice,sampled_attack_profiles,player5_dribble_content);
 auto player24_dribble_defender=prepare_sampled_player5_dribble_defender(player24_dribble_prepared,sampled_attack_profiles,player3_defense_content);
 FieldComparisonSelection player24_comparison{9,192,64,true,{154,96,68,0}};
 auto player24_dribble_result=compare_sampled_player5_dribble_defender(player24_dribble_defender,player24_comparison);
 check(player24_dribble_choice.phase==FieldSessionPhase::sampled_score24_dribble_choice &&
       player24_dribble_prepared.phase==FieldSessionPhase::sampled_score24_dribble_prepared && !player24_dribble_prepared.action->power &&
       player24_dribble_defender.phase==FieldSessionPhase::sampled_score24_dribble_defender &&
       player24_dribble_result.phase==FieldSessionPhase::sampled_score24_dribble_compared &&
       player24_dribble_result.area_contact->strength==4 && player24_dribble_result.area_contact->result==3,
       "The exhausted 2-4 dribble matches defender17's observed result");
 auto player24_dribble_clock=advance_sampled_player5_dribble_terminal(player24_dribble_result,player5_terminal_prelude,player5_terminal_experience);
 auto player24_dribble_field=advance_sampled_player5_terminal_field(player24_dribble_clock,player3_race_mappings,player3_race_movement);
 auto player24_dribble_possession=transfer_sampled_player5_dribble_possession(player24_dribble_field);
 check(player24_dribble_clock.phase==FieldSessionPhase::sampled_score24_dribble_clock && player24_dribble_clock.current.clock.remaining==341 &&
       player24_dribble_clock.current.field.elapsed==2 && player24_dribble_clock.current.field.motion[field_motion_index(5)].status==5 &&
       player24_dribble_field.phase==FieldSessionPhase::sampled_score24_dribble_field && !player24_dribble_field.current.field.elapsed &&
       player24_dribble_possession.state.phase==FieldSessionPhase::sampled_score24_dribble_possession &&
       player24_dribble_possession.state.current.field.ball_owner==17 && player24_dribble_possession.state.current.field.side==11 &&
       player24_dribble_possession.request_side_music,"The 2-4 dribble result transfers possession to defender17");
 auto player24_dribble_ready=prepare_sampled_player5_result3_control(player24_dribble_possession.state);
 check(player24_dribble_ready.state.phase==FieldSessionPhase::control_ready && player24_dribble_ready.state.current.field.ball_owner==17,
       "The 2-4 dribble result reaches opponent control");
 auto player24_opponent_ai=player24_dribble_ready.state;player24_opponent_ai.phase=FieldSessionPhase::ai_cell_decision;
 player24_opponent_ai.current.clock.remaining=338;player24_opponent_ai.current.field.ball_x=101;player24_opponent_ai.current.field.ball_y=106;
 player24_opponent_ai.current.field.ball_cell=75;player24_opponent_ai.current.field.elapsed=1;
 player24_opponent_ai.control->controlled_player=9;player24_opponent_ai.control->last_carrier_cell=75;
 auto player24_opponent_choice=capture_sampled_field_ai_choice(player24_opponent_ai,2,0);
 check(player24_opponent_choice.phase==FieldSessionPhase::control_clock_pending && player24_opponent_choice.action->action==2 &&
       player24_opponent_choice.current.field.ball_owner==17,"Player17's observed dribble decision returns to the control clock");
 auto player24_shot_input=player24_opponent_ai;player24_shot_input.current.clock.remaining=336;
 player24_shot_input.current.field.ball_x=94;player24_shot_input.current.field.ball_cell=63;player24_shot_input.control->last_carrier_cell=63;
 auto player24_shot_choice=capture_sampled_field_ai_choice(player24_shot_input,0,0);
 auto player24_shot_prepared=prepare_sampled_field_contact_attack(player24_shot_choice,sampled_attack_profiles,rejected_content);
 auto player24_shot_presented=acknowledge_sampled_field_ai_presentation(player24_shot_prepared);
 auto player24_shot_clock=advance_sampled_field_ai_dispatch(player24_shot_presented);
 auto player24_shot_approach=advance_sampled_player19_return_shot_approach(player24_shot_clock,player3_race_mappings,player3_race_movement);
 auto player24_shot_handoff=acknowledge_sampled_field_ai_pass_handoff(player24_shot_approach);
 auto player24_shot_target=prepare_session_shot_approach(player24_shot_handoff,player23_shot_content).state;
 auto player24_shot_flight=begin_prepared_session_ball_flight(player24_shot_target,116,105,pass_geometry,pass_velocity);
 check(player24_shot_flight.phase==FieldSessionPhase::sampled_score23_ball_advance_ready &&
       player24_shot_flight.current.field.ball_owner==17 && !player24_shot_flight.saved_ball_flight_flag &&
       (player24_shot_flight.ball_trajectory->x&255)==116 && (player24_shot_flight.ball_trajectory->y&255)==105,
       "Player17's next shot uses the shared numeric flight with its observed fractions");
 rejects([&]{begin_prepared_session_ball_flight(player24_shot_target,44,73,pass_geometry,pass_velocity);});
 auto player24_shot_arrival=player24_shot_flight;player24_shot_arrival.phase=FieldSessionPhase::sampled_score23_ball_arrival_callback;
 player24_shot_arrival.current.field.ball_x=52;player24_shot_arrival.current.field.ball_y=124;player24_shot_arrival.current.field.ball_cell=5;
 player24_shot_arrival.ball_trajectory->x=52<<8;player24_shot_arrival.ball_trajectory->y=124<<8;
 player24_shot_arrival.ball_trajectory->direction=111;player24_shot_arrival.recovery->x_jitter_control=111;
 player24_shot_arrival.current.field.roster[0].stamina=44;
 auto player24_keeper_prompt=continue_session_unopposed_arrival(player24_shot_arrival);
 auto player24_keeper_selected=poll_sampled_goalkeeper_prompt(player24_keeper_prompt,1,{23,22});
 auto player24_keeper_confirmed=poll_sampled_goalkeeper_prompt(player24_keeper_selected.state,128,{23,22});
 auto player24_keeper_overlay=acknowledge_sampled_goalkeeper_overlay(player24_keeper_confirmed.state);
 auto player24_keeper_compared=compare_sampled_home_goalkeeper(player24_keeper_overlay.state,keeper_test_content);
 auto player25_goal=prepare_sampled_opponent_goal(player24_keeper_compared,keeper_test_content);
 check(player24_keeper_compared.current.field.roster[0].stamina==24 && player25_goal.score->right_goals==5 &&
       player25_goal.current.field.ball_owner==17 && player25_goal.recovery->opposing_player==4 &&
       !player25_goal.control->facing && player25_goal.control->motion_requested==3,
       "Player17's shot reaches the observed 2-5 goal through the shared goalkeeper path");
 auto player25_presented=acknowledge_sampled_opponent_goal_presentation(player25_goal);
 auto player25_restart=begin_sampled_opponent_goal_restart(player25_presented);
 player25_restart.current.field.motion[field_motion_index(5)].status=2;
 auto player25_formation=prepare_session_kickoff_formation(player25_restart,0,c);
 auto player25_receiver=prepare_session_kickoff_receiver(player25_formation,0,{5,6,7,9,10});
 auto player25_ball=prepare_session_kickoff_ball(player25_receiver);
 auto player25_handoff=continue_prepared_session_kickoff(player25_ball);
 check(player25_handoff.current.field.ball_owner==5 && player25_handoff.score->right_goals==5 &&
       !player25_handoff.current.field.motion[field_motion_index(5)].status && !player25_handoff.control->facing,
       "The 2-5 goal clears the observed status and returns to home kickoff control");
 auto player25_pass_input=player25_handoff;player25_pass_input.kickoff_receiver.reset();
 player25_pass_input.phase=FieldSessionPhase::attack_choice_pending;player25_pass_input.current.clock.remaining=325;
 player25_pass_input.current.field.elapsed=1;player25_pass_input.current.field.ball_x=108;player25_pass_input.current.field.ball_y=108;
 player25_pass_input.current.field.ball_cell=87;player25_pass_input.current.field.roster[5].type=23;player25_pass_input.current.field.roster[5].stamina=0;
 player25_pass_input.current.field.selected_defenders=3;player25_pass_input.scheduling->selected_ids={17,19,20,0,0};
 player25_pass_input.current.field.motion[field_motion_index(7)].x=116<<8;
 player25_pass_input.current.field.motion[field_motion_index(7)].y=140<<8;
 player25_pass_input.action->action=1;player25_pass_input.action->cursor=225;player25_pass_input.action->menu_route=0;
 player25_pass_input.action->power=22;player25_pass_input.recovery->opposing_player=4;player25_pass_input.recovery->attack_subtype=128;
 auto score36_pass_input=player25_pass_input;score36_pass_input.score=MatchScoreContext{3,6,4};score36_pass_input.current.clock.remaining=283;score36_pass_input.current.field.roster[5].stamina=10;
 auto score36_pass_target=begin_sampled_human_pass_target(score36_pass_input);
 for(auto held:{1,4,4,4,4})score36_pass_target=poll_sampled_human_pass_target(score36_pass_target,held,0);
 auto score36_pass_confirmed=poll_sampled_human_pass_target(score36_pass_target,0,128);
 check(score36_pass_confirmed.phase==FieldSessionPhase::sampled_score25_player7_confirmed && *score36_pass_confirmed.human_pass_cursor==103,
       "The observed 3-6 pass selects player7");
 auto score36_defenses=capture_sampled_score25_pass_defenses(score36_pass_confirmed,{1,1,1},{0,0,0});
 auto score36_pass_prepared=prepare_sampled_score25_player7_pass(score36_defenses,sampled_attack_profiles,human_pass_content);
 check(score36_pass_prepared.current.field.roster[5].stamina==0 && score36_pass_prepared.action->power==0,
       "The 3-6 pass spends the remaining ten stamina");
 auto score36_contact=score36_pass_prepared;
 for(unsigned defender=0;defender<3;++defender) {
  score36_contact=prepare_sampled_player3_pass_defender(score36_contact,sampled_attack_profiles,player3_defense_content);
  score36_contact=compare_sampled_player3_pass_defender(score36_contact,{5,153,0,false,{154,96,68,0}});
  score36_contact=advance_sampled_player3_pass_contact(score36_contact,player3_prelude,home_experience);
 }
 auto score36_dispatch=advance_sampled_player3_pass_dispatch(score36_contact);
 check(score36_dispatch.current.clock.remaining==279 && score36_dispatch.area_defender_cursor==3,
       "The observed 3-6 pass reaches dispatch after three defenders");
 auto score36_handoff_pass=advance_sampled_score23_pass_handoff(score36_dispatch,player3_race_mappings,player3_race_movement);
 check(score36_handoff_pass.phase==FieldSessionPhase::sampled_score25_pass_handoff && score36_handoff_pass.current.clock.remaining==279 && !score36_handoff_pass.current.field.elapsed,
       "The 3-6 pass calculates its field handoff");
 auto score36_pass_captured=capture_sampled_player3_pass_target(score36_handoff_pass,7,103,255,3);
 check(score36_pass_captured.sampled_field_pass && score36_pass_captured.sampled_field_pass->distance==3,
       "The 3-6 pass retains the observed player7 target");
 auto score36_pass_flight=begin_sampled_field_pass_flight(score36_pass_captured,82,109,pass_geometry,pass_velocity);
 check(score36_pass_flight.phase==FieldSessionPhase::area_ball_advance_ready && score36_pass_flight.saved_ball_flight_flag==1,
       "The 3-6 pass begins numeric player7 flight");
 rejects([&]{begin_sampled_field_pass_flight(score36_pass_captured,81,109,pass_geometry,pass_velocity);});
 auto score36_pass_arrival=score36_pass_flight;score36_pass_arrival.phase=FieldSessionPhase::area_ball_arrival_callback;
 score36_pass_arrival.current.field.ball_x=113;score36_pass_arrival.current.field.ball_y=143;score36_pass_arrival.current.field.ball_cell=103;
 score36_pass_arrival.ball_trajectory->x=113<<8;score36_pass_arrival.ball_trajectory->y=143<<8;
 auto score36_pass_received=continue_area_already_arrived(score36_pass_arrival);
 check(score36_pass_received.current.field.ball_owner==7 && score36_pass_received.area_defender_cursor==3,
       "The 3-6 pass hands ownership to player7 and preserves the defender cursor");
 score36_pass_received.current.field.motion[field_motion_index(7)].x=113<<8;score36_pass_received.current.field.motion[field_motion_index(7)].y=143<<8;
 auto score36_pass_positioned=continue_area_pass_received(score36_pass_received);
 auto score36_player7_control=continue_sampled_field_pass_control(score36_pass_positioned);
 check(score36_player7_control.phase==FieldSessionPhase::player_control && score36_player7_control.current.field.ball_x==113 && score36_player7_control.current.field.ball_y==143,
       "The 3-6 pass returns player7 to control at the observed position");
 auto score36_player7_tick=score36_player7_control;score36_player7_tick.phase=FieldSessionPhase::field_round_pending;score36_player7_tick.current.clock.remaining=278;score36_player7_tick.current.field.elapsed=1;
 check(!bytes(score36_player7_tick).empty(),"The observed 3-6 player7 clock retains its cursor");
 auto invalid_score36_player7_tick=score36_player7_tick;invalid_score36_player7_tick.current.clock.remaining=268;
 rejects([&]{bytes(invalid_score36_player7_tick);});
 auto player25_target=begin_sampled_human_pass_target(player25_pass_input);
 for(auto held:{1,4,4,4,4})player25_target=poll_sampled_human_pass_target(player25_target,held,0);
 check(player25_target.phase==FieldSessionPhase::sampled_score25_pass_target && *player25_target.human_pass_cursor==103 &&
       player25_target.human_pass_steps==5,"The observed 2-5 pass cursor reaches player7");
 auto player25_target_bytes=bytes(player25_target);std::istringstream player25_target_stream(player25_target_bytes);
 check(bytes(load_phase_session(player25_target_stream))==player25_target_bytes,"The teammate target cursor roundtrips");
 auto player25_pass_confirmed=poll_sampled_human_pass_target(player25_target,0,128);
 check(player25_pass_confirmed.phase==FieldSessionPhase::sampled_score25_player7_confirmed &&
       *player25_pass_confirmed.human_pass_cursor==103,"The observed pass confirms player7 as recipient");
 auto player25_defenses=capture_sampled_score25_pass_defenses(player25_pass_confirmed,{1,1,1},{0,0,0});
 check(player25_defenses.phase==FieldSessionPhase::sampled_score25_pass_defenses &&
       player25_defenses.recovery->opposing_player==20 && player25_defenses.action->power==22 &&
       player25_defenses.current.clock.ball_mode==0,"The captured three defender choices preserve pass preparation inputs");
 rejects([&]{capture_sampled_score25_pass_defenses(player25_pass_confirmed,{1,2,1},{0,0,0});});
 auto player25_prepared=prepare_sampled_score25_player7_pass(player25_defenses,sampled_attack_profiles,human_pass_content);
 check(player25_prepared.phase==FieldSessionPhase::sampled_score25_pass_prepared && player25_prepared.action->power==0 &&
       player25_prepared.current.field.roster[5].stamina==0,"The exhausted teammate pass uses numerical attack preparation");
 auto player25_prepared_bytes=bytes(player25_prepared);std::istringstream player25_prepared_stream(player25_prepared_bytes);
 check(bytes(load_phase_session(player25_prepared_stream))==player25_prepared_bytes,"The prepared teammate pass roundtrips");
 auto player25_first_defender=prepare_sampled_player3_pass_defender(player25_prepared,sampled_attack_profiles,player3_defense_content);
 check(player25_first_defender.phase==FieldSessionPhase::sampled_score25_first_defender_prepared &&
       player25_first_defender.recovery->opposing_player==17 && player25_first_defender.area_contact->cost==60,
       "The first defender is numerically prepared for the player7 pass");
 auto player25_first_compared=compare_sampled_player3_pass_defender(player25_first_defender,{5,153,0,false,{154,96,68,0}});
 check(player25_first_compared.phase==FieldSessionPhase::sampled_score25_first_defender_compared &&
       player25_first_compared.area_contact->score==255 && !player25_first_compared.area_contact->result,
       "The first defender comparison matches the observed pass");
 auto player25_first_clock=advance_sampled_player3_pass_contact(player25_first_compared,player3_prelude,home_experience);
 check(player25_first_clock.phase==FieldSessionPhase::sampled_score25_first_contact_clock &&
       player25_first_clock.current.clock.remaining==324 && player25_first_clock.current.field.elapsed==2 &&
       player25_first_clock.current.field.motion[field_motion_index(17)].status==6,
       "The first defender contact applies its clock and status");
 auto player25_second_defender=prepare_sampled_player3_pass_defender(player25_first_clock,sampled_attack_profiles,player3_defense_content);
 check(player25_second_defender.phase==FieldSessionPhase::sampled_score25_second_defender_prepared &&
       player25_second_defender.recovery->opposing_player==19 && player25_second_defender.area_defender_cursor==1 &&
       player25_second_defender.current.clock.remaining==324,
       "The second defender is prepared after the first contact");
 check(player25_second_defender.area_contact->defender_power==player25_first_defender.area_contact->defender_power &&
       player25_second_defender.area_contact->cost==player25_first_defender.area_contact->cost &&
       player25_second_defender.area_contact->effect==player25_first_defender.area_contact->effect &&
       player25_first_clock.recovery->opposing_player==17,
       "Earlier and later sampled defenders share preparation without mutating the input");
 auto second_bytes=bytes(player25_second_defender);std::istringstream second_stream(second_bytes);
 check(bytes(load_phase_session(second_stream))==second_bytes,"The second defender preparation roundtrips");
 rejects([&]{prepare_sampled_player3_pass_defender(player25_second_defender,sampled_attack_profiles,player3_defense_content);});
 auto player25_second_compared=compare_sampled_player3_pass_defender(player25_second_defender,{5,153,0,false,{154,96,68,0}});
 check(player25_second_compared.phase==FieldSessionPhase::sampled_score25_second_defender_compared &&
       player25_second_compared.area_contact->score==255 && !player25_second_compared.area_contact->result,
       "The second defender comparison is calculated");
 auto player25_second_clock=advance_sampled_player3_pass_contact(player25_second_compared,player3_prelude,home_experience);
 check(player25_second_clock.phase==FieldSessionPhase::sampled_score25_second_contact_clock &&
       player25_second_clock.current.clock.remaining==323 && player25_second_clock.current.field.elapsed==3 &&
       player25_second_clock.current.field.motion[field_motion_index(19)].status==6,
       "The second defender contact advances clock and status");
 auto player25_third_defender=prepare_sampled_player3_pass_defender(player25_second_clock,sampled_attack_profiles,player3_defense_content);
 check(player25_third_defender.phase==FieldSessionPhase::sampled_score25_third_defender_prepared && player25_third_defender.recovery->opposing_player==20 && player25_third_defender.area_defender_cursor==2,"The third defender is prepared");
 auto player25_third_compared=compare_sampled_player3_pass_defender(player25_third_defender,{5,153,0,false,{154,96,68,0}});
 check(player25_third_compared.phase==FieldSessionPhase::sampled_score25_third_defender_compared && player25_third_compared.area_contact->score==255,"The third defender comparison is calculated");
 auto player25_third_clock=advance_sampled_player3_pass_contact(player25_third_compared,player3_prelude,home_experience);
 check(player25_third_clock.phase==FieldSessionPhase::sampled_score25_third_contact_clock && player25_third_clock.current.clock.remaining==322 && player25_third_clock.current.field.motion[field_motion_index(20)].status==6,"The third defender contact advances clock and status");
 auto player25_dispatch=advance_sampled_player3_pass_dispatch(player25_third_clock);
 check(player25_dispatch.phase==FieldSessionPhase::sampled_score25_dispatch_clock && player25_dispatch.current.clock.remaining==321 && player25_dispatch.area_defender_cursor==3,"The pass dispatch advances after three defenders");
 auto dispatch_bytes=bytes(player25_dispatch);std::istringstream dispatch_stream(dispatch_bytes);
 check(bytes(load_phase_session(dispatch_stream))==dispatch_bytes,"The pass dispatch snapshot roundtrips");
 rejects([&]{advance_sampled_player3_pass_dispatch(player25_dispatch);});
 auto invalid_dispatch=dispatch_bytes;invalid_dispatch[10]=static_cast<char>(143);
 rejects([&]{std::istringstream in(invalid_dispatch);load_phase_session(in);});
 invalid_dispatch=dispatch_bytes;invalid_dispatch.pop_back();
 rejects([&]{std::istringstream in(invalid_dispatch);load_phase_session(in);});
 auto changed_second=player25_second_defender;changed_second.area_defender_cursor=2;
 rejects([&]{validate_phase_session(changed_second);});
 auto player25_field_handoff=advance_sampled_score23_pass_handoff(player25_dispatch,player3_race_mappings,player3_race_movement);
 check(player25_field_handoff.phase==FieldSessionPhase::sampled_score25_pass_handoff && !player25_field_handoff.current.field.elapsed && player25_field_handoff.current.clock.remaining==321,"The player7 pass advances its field movement");
 auto player25_handoff_bytes=bytes(player25_field_handoff);std::istringstream player25_handoff_stream(player25_handoff_bytes);
 check(bytes(load_phase_session(player25_handoff_stream))==player25_handoff_bytes,"The player7 pass handoff snapshot roundtrips");
 rejects([&]{advance_sampled_score23_pass_handoff(player25_field_handoff,player3_race_mappings,player3_race_movement);});
 auto player25_target_captured=capture_sampled_player3_pass_target(player25_field_handoff,7,103,255,3);
 check(player25_target_captured.phase==FieldSessionPhase::sampled_score25_pass_target_captured && player25_target_captured.sampled_field_pass->recipient==7,"The player7 pass target is saved");
 auto player25_target_saved=bytes(player25_target_captured);std::istringstream player25_target_read(player25_target_saved);
 check(bytes(load_phase_session(player25_target_read))==player25_target_saved,"Player7 target and cursor survive reload");
 rejects([&]{capture_sampled_player3_pass_target(player25_field_handoff,8,103,255,3);});
 auto wrong_player25_target=player25_target_saved;wrong_player25_target[10]=static_cast<char>(146);
 rejects([&]{std::istringstream in(wrong_player25_target);load_phase_session(in);});
 wrong_player25_target=player25_target_saved;wrong_player25_target.pop_back();
 rejects([&]{std::istringstream in(wrong_player25_target);load_phase_session(in);});
 rejects([&]{capture_sampled_player3_pass_target(player25_target_captured,7,103,255,3);});
 auto player25_flight=begin_sampled_field_pass_flight(player25_target_captured,167,221,pass_geometry,pass_velocity);
 check(player25_flight.phase==FieldSessionPhase::area_ball_advance_ready && player25_flight.area_ai_target->recipient==7 && player25_flight.current.field.selected_defenders==0 && player25_flight.area_defender_cursor==3 && player25_flight.saved_ball_flight_flag==1,"The player7 pass begins numeric flight");
 rejects([&]{begin_sampled_field_pass_flight(player25_target_captured,166,221,pass_geometry,pass_velocity);});
 auto player25_flight_bytes=bytes(player25_flight);std::istringstream player25_flight_stream(player25_flight_bytes);
 check(bytes(load_phase_session(player25_flight_stream))==player25_flight_bytes,"Player7 flight snapshot roundtrips");
 auto invalid_player25_flight=player25_flight;invalid_player25_flight.area_defender_cursor=2;
 rejects([&]{validate_phase_session(invalid_player25_flight);});
 auto player25_arrival=player25_flight;player25_arrival.phase=FieldSessionPhase::area_ball_arrival_callback;
 player25_arrival.current.field.ball_x=116;player25_arrival.current.field.ball_y=140;player25_arrival.current.field.ball_cell=103;
 player25_arrival.ball_trajectory->x=(116<<8)|167;player25_arrival.ball_trajectory->y=(140<<8)|221;
 auto player25_received=continue_area_already_arrived(player25_arrival);
 check(player25_received.current.field.ball_owner==7 && player25_received.area_defender_cursor==3,"Player7 receives the pass and the defender cursor is retained");
 auto player25_positioned=continue_area_pass_received(player25_received);
 check(player25_positioned.phase==FieldSessionPhase::area_pass_keeper_entry && player25_positioned.current.field.ball_owner==7,"Player7 reception positions the ball");
 auto player25_control_input=player25_positioned;
 player25_control_input.current.field.ball_x=117;player25_control_input.current.field.ball_y=139;
 player25_control_input.ball_trajectory->x=(117<<8)|167;player25_control_input.ball_trajectory->y=(139<<8)|221;
 auto player25_control_return=continue_sampled_field_pass_control(player25_control_input);
 check(player25_control_return.phase==FieldSessionPhase::player_control && player25_control_return.current.field.ball_owner==7 && player25_control_return.area_defender_cursor==3 && player25_control_return.control->facing==0,"Player7 resumes control after the pass");
 auto player25_next_round=player25_control_return;player25_next_round.phase=FieldSessionPhase::field_round_pending;
 player25_next_round.current.clock.remaining=320;player25_next_round.current.field.elapsed=1;validate_phase_session(player25_next_round);
 auto player25_round_bytes=bytes(player25_next_round);std::istringstream player25_round_stream(player25_round_bytes);
 check(bytes(load_phase_session(player25_round_stream))==player25_round_bytes,"Player7 retained cursor survives the next field round");
 auto player25_p7_contact=player25_next_round;player25_p7_contact.phase=FieldSessionPhase::field_contact_pending;
 player25_p7_contact.current.clock.remaining=311;player25_p7_contact.current.field.elapsed=1;
 player25_p7_contact.current.field.selected_defenders=5;player25_p7_contact.scheduling->selected_ids={12,14,15,16,18};
 auto player25_p7_menu=open_human_action_menu(player25_p7_contact,ActionMenuContent{}).state;
 check(player25_p7_menu.phase==FieldSessionPhase::human_action_menu && player25_p7_menu.area_defender_cursor==3,"Player7 contact opens a menu retaining the original cursor");
 auto player25_p7_shot_choice=poll_human_action_menu(player25_p7_menu,1,ActionMenuContent{}).state;
 auto player25_p7_shot_confirm=poll_human_action_menu(player25_p7_shot_choice,128,ActionMenuContent{}).state;
 check(player25_p7_shot_confirm.phase==FieldSessionPhase::attack_choice_pending && player25_p7_shot_confirm.area_defender_cursor==3 && player25_p7_shot_confirm.action->action==0 && !bytes(player25_p7_shot_confirm).empty(),"Player7 shot choice retains the original cursor");
 auto player25_p7_shot_observed=player25_p7_shot_confirm;
 player25_p7_shot_observed.current.field.roster[7].type=23;player25_p7_shot_observed.current.field.roster[7].stamina=210;
 auto score36_p7_shot_observed=player25_p7_shot_observed;
 score36_p7_shot_observed.score=MatchScoreContext{3,6,4};score36_p7_shot_observed.current.clock.remaining=269;
 score36_p7_shot_observed.current.field.ball_x=113;score36_p7_shot_observed.current.field.ball_y=143;
 score36_p7_shot_observed.current.field.roster[7].stamina=140;score36_p7_shot_observed.current.field.selected_defenders=4;
 score36_p7_shot_observed.scheduling->selected_ids={12,15,16,21,0};
 auto score36_p7_defenses=capture_sampled_player5_defense_choices(score36_p7_shot_observed,{1,1,1,1,1},{0,0,0,0,0});
 auto score36_p7_prepared=prepare_session_basic_shot(score36_p7_defenses,sampled_attack_profiles,next_shot_content).state;
 check(score36_p7_prepared.current.field.roster[7].stamina==60 && score36_p7_prepared.action->power==22 && score36_p7_prepared.recovery->opposing_player==21,
       "The 3-6 player7 shot prepares against four observed defenders");
 for(auto id:{12,15,16,21}) {score36_p7_prepared.current.field.roster[id].type=23;score36_p7_prepared.current.field.roster[id].stamina=100;score36_p7_prepared.current.field.motion[field_motion_index(id)].status=0;}
 auto score36_p7_prefix=begin_sampled_player5_dribble_defenders(score36_p7_prepared);
 for(unsigned defender=0;defender<4;++defender) {
  score36_p7_prefix=prepare_sampled_player5_dribble_defender(score36_p7_prefix,sampled_attack_profiles,player3_defense_content);
  score36_p7_prefix=compare_sampled_player5_dribble_defender(score36_p7_prefix,{1,153,0,false,{154,96,68,0}});
  score36_p7_prefix=advance_sampled_player3_pass_contact(score36_p7_prefix,player3_prelude,home_experience);
  score36_p7_prefix=begin_sampled_player5_dribble_defenders(score36_p7_prefix);
 }
 auto score36_p7_dispatch=advance_sampled_player3_pass_dispatch(score36_p7_prefix);
 check(score36_p7_dispatch.area_defender_cursor==4 && score36_p7_dispatch.current.clock.remaining==263,
       "The 3-6 player7 shot crosses four defender contacts and dispatch");
 auto score36_p7_field=advance_sampled_score23_pass_handoff(score36_p7_dispatch,player3_race_mappings,player3_race_movement);
 check(score36_p7_field.current.field.elapsed==0 && score36_p7_field.current.clock.remaining==263 && score36_p7_field.area_defender_cursor==4,
       "The 3-6 player7 shot calculates its field handoff");
 auto player25_p7_defenses=capture_sampled_player5_defense_choices(player25_p7_shot_observed,{1,1,1,1,1},{0,0,0,0,0});
 check(player25_p7_defenses.human_defense_choices && player25_p7_defenses.recovery->opposing_player==18,"Player7 shot preserves five observed defense choices");
 auto player25_p7_defenses_bytes=bytes(player25_p7_defenses);std::istringstream player25_p7_defenses_stream(player25_p7_defenses_bytes);
 check(bytes(load_phase_session(player25_p7_defenses_stream))==player25_p7_defenses_bytes,"Player7 defense choices roundtrip");
 auto player25_p7_shot_prepared=prepare_session_basic_shot(player25_p7_defenses,sampled_attack_profiles,next_shot_content).state;
 check(player25_p7_shot_prepared.phase==FieldSessionPhase::attack_prepared && player25_p7_shot_prepared.current.field.roster[7].stamina==130 && player25_p7_shot_prepared.action->power==22,"Player7 shot preparation calculates cost and power");
 auto player25_p7_prepared_bytes=bytes(player25_p7_shot_prepared);std::istringstream player25_p7_prepared_stream(player25_p7_prepared_bytes);
 check(bytes(load_phase_session(player25_p7_prepared_stream))==player25_p7_prepared_bytes,"Player7 prepared shot roundtrips");
 rejects([&]{capture_sampled_player5_defense_choices(player25_p7_shot_observed,{1,1,1,1,0},{0,0,0,0,0});});
 auto player25_p7_prefix=begin_sampled_player5_dribble_defenders(player25_p7_shot_prepared);
 check(player25_p7_prefix.area_defender_cursor==0 && player25_p7_prefix.control->last_carrier_cell==0 && player25_p7_prefix.current.field.roster[7].stamina==130,"Player7 shot starts its observed defender prefix");
 auto player25_p7_prefix_bytes=bytes(player25_p7_prefix);std::istringstream player25_p7_prefix_stream(player25_p7_prefix_bytes);
 check(bytes(load_phase_session(player25_p7_prefix_stream))==player25_p7_prefix_bytes,"Player7 defender prefix roundtrips");
 auto player25_p7_defender_input=player25_p7_prefix;player25_p7_defender_input.current.field.roster[12].type=23;player25_p7_defender_input.current.field.roster[12].stamina=100;
 auto player25_p7_first_defender=prepare_sampled_player5_dribble_defender(player25_p7_defender_input,sampled_attack_profiles,player3_defense_content);
 check(player25_p7_first_defender.area_contact && player25_p7_first_defender.area_contact->cost==60 && player25_p7_first_defender.recovery->opposing_player==12,"Player7 shot prepares the first defender");
 auto player25_p7_compared=compare_sampled_player5_dribble_defender(player25_p7_first_defender,{1,153,0,false,{154,96,68,0}});
 check(player25_p7_compared.area_contact->score==255 && player25_p7_compared.area_sequence_result==0,"Player7 shot compares the first defender using the direct shot row");
 rejects([&]{compare_sampled_player5_dribble_defender(player25_p7_first_defender,{9,153,0,false,{154,96,68,0}});});
 auto player25_p7_contact_clock=advance_sampled_player3_pass_contact(player25_p7_compared,player3_prelude,home_experience);
 check(player25_p7_contact_clock.current.clock.remaining==310 && player25_p7_contact_clock.current.field.motion[field_motion_index(12)].status==6 && player25_p7_contact_clock.control->motion_requested==0,"Player7 first defender contact advances clock and status");
 rejects([&]{advance_sampled_player3_pass_contact(player25_p7_contact_clock,player3_prelude,home_experience);});
 auto player25_p7_next_defender=begin_sampled_player5_dribble_defenders(player25_p7_contact_clock);
 check(player25_p7_next_defender.area_defender_cursor==1 && !player25_p7_next_defender.area_contact,"Player7 defender sequence advances after contact");
 player25_p7_next_defender.current.field.roster[14].type=23;player25_p7_next_defender.current.field.roster[14].stamina=100;
 auto player25_p7_second=prepare_sampled_player5_dribble_defender(player25_p7_next_defender,sampled_attack_profiles,player3_defense_content);
 check(player25_p7_second.recovery->opposing_player==14 && player25_p7_second.area_contact->cursor==1,"Player7 shot prepares the second defender");
 auto player25_p7_second_compared=compare_sampled_player5_dribble_defender(player25_p7_second,{1,153,0,false,{154,96,68,0}});
 auto player25_p7_second_clock=advance_sampled_player3_pass_contact(player25_p7_second_compared,player3_prelude,home_experience);
 check(player25_p7_second_clock.current.clock.remaining==309 && player25_p7_second_clock.current.field.motion[field_motion_index(14)].status==6,"Player7 shot advances second defender contact");
 auto player25_p7_all_defenders=player25_p7_second_clock;
 for(const auto id:std::array<std::uint8_t,3>{15,16,18}) {
  player25_p7_all_defenders=begin_sampled_player5_dribble_defenders(player25_p7_all_defenders);
  player25_p7_all_defenders.current.field.roster[id].type=23;player25_p7_all_defenders.current.field.roster[id].stamina=100;
  player25_p7_all_defenders=prepare_sampled_player5_dribble_defender(player25_p7_all_defenders,sampled_attack_profiles,player3_defense_content);
  player25_p7_all_defenders=compare_sampled_player5_dribble_defender(player25_p7_all_defenders,{1,153,0,false,{154,96,68,0}});
  player25_p7_all_defenders=advance_sampled_player3_pass_contact(player25_p7_all_defenders,player3_prelude,home_experience);
 }
 auto player25_p7_defenders_done=begin_sampled_player5_dribble_defenders(player25_p7_all_defenders);
 check(player25_p7_defenders_done.area_defender_cursor==5 && player25_p7_defenders_done.current.clock.remaining==306,"Player7 completes all five defenders");
 auto player25_p7_shot_dispatch=advance_sampled_player3_pass_dispatch(player25_p7_defenders_done);
 check(player25_p7_shot_dispatch.current.clock.remaining==304 && player25_p7_shot_dispatch.current.field.elapsed==8,"Player7 shot dispatch advances two clock units");
 rejects([&]{advance_sampled_player3_pass_dispatch(player25_p7_shot_dispatch);});
 auto player25_p7_shot_field=advance_sampled_score23_pass_handoff(player25_p7_shot_dispatch,player3_race_mappings,player3_race_movement);
 check(player25_p7_shot_field.phase==FieldSessionPhase::attack_prepared && player25_p7_shot_field.current.clock.remaining==304 && !player25_p7_shot_field.current.field.elapsed,"Player7 shot advances field movement after dispatch");
 rejects([&]{advance_sampled_score23_pass_handoff(player25_p7_shot_field,player3_race_mappings,player3_race_movement);});
 ShotApproachContent player25_shot_content{};player25_shot_content.outer_distance.fill(11);player25_shot_content.inner_distance.fill(11);
 auto score36_p7_shot_target=prepare_session_shot_approach(score36_p7_field,player25_shot_content).state;
 auto score36_p7_shot_flight=begin_prepared_session_ball_flight(score36_p7_shot_target,223,75,pass_geometry,pass_velocity);
 check(score36_p7_shot_target.shot_flight->distance==99 && score36_p7_shot_target.shot_flight->destination==233 && score36_p7_shot_flight.phase==FieldSessionPhase::sampled_score25_shot_advance,
       "The 3-6 player7 shot calculates its target and numeric flight");
 rejects([&]{begin_prepared_session_ball_flight(score36_p7_shot_target,222,75,pass_geometry,pass_velocity);});
 auto score36_p7_shot_arrival=score36_p7_shot_flight;score36_p7_shot_arrival.phase=FieldSessionPhase::sampled_score25_shot_arrival;
 score36_p7_shot_arrival.current.field.ball_x=204;score36_p7_shot_arrival.current.field.ball_y=124;score36_p7_shot_arrival.current.field.ball_cell=233;
 score36_p7_shot_arrival.ball_trajectory->x=204<<8;score36_p7_shot_arrival.ball_trajectory->y=124<<8;score36_p7_shot_arrival.current.field.keepers[1].stun=0;
 auto score36_p7_keeper_entry=continue_session_unopposed_arrival(score36_p7_shot_arrival);
 auto score36_p7_keeper_prepared=prepare_session_keeper_entry(score36_p7_keeper_entry);
 check(score36_p7_keeper_prepared.recovery->opposing_player==11 && score36_p7_keeper_prepared.area_defender_cursor==4,
       "The 3-6 player7 shot prepares the opposing keeper");
 auto score36_p7_keeper_decision=resolve_prepared_session_keeper(score36_p7_keeper_prepared,0,0,3,home_keeper_content);
 check(score36_p7_keeper_decision.area_defender_cursor==0,"The 3-6 keeper comparison clears the observed defender cursor");
 auto player25_p7_shot_target=prepare_session_shot_approach(player25_p7_shot_field,player25_shot_content).state;
 check(player25_p7_shot_target.phase==FieldSessionPhase::sampled_score25_shot_target && player25_p7_shot_target.shot_flight->distance==99 && player25_p7_shot_target.shot_flight->destination==233,"Player7 calculates the shot target");
 auto player25_p7_shot_flight=begin_prepared_session_ball_flight(player25_p7_shot_target,52,187,pass_geometry,pass_velocity);
 check(player25_p7_shot_flight.phase==FieldSessionPhase::sampled_score25_shot_advance && player25_p7_shot_flight.ball_trajectory && !player25_p7_shot_flight.saved_ball_flight_flag && player25_p7_shot_flight.area_defender_cursor==5,"Player7 shot starts numeric flight");
 rejects([&]{begin_prepared_session_ball_flight(player25_p7_shot_target,51,187,pass_geometry,pass_velocity);});
 auto player25_p7_shot_flight_bytes=bytes(player25_p7_shot_flight);std::istringstream player25_p7_shot_flight_stream(player25_p7_shot_flight_bytes);
 check(bytes(load_phase_session(player25_p7_shot_flight_stream))==player25_p7_shot_flight_bytes,"Player7 shot trajectory roundtrips");
 auto player25_p7_shot_step=advance_prepared_session_ball_cell(player25_p7_shot_flight).state;
 check(player25_p7_shot_step.phase==FieldSessionPhase::sampled_score25_shot_callback || player25_p7_shot_step.phase==FieldSessionPhase::sampled_score25_shot_arrival,"Player7 shot advances to a cell boundary");
 auto player25_p7_shot_target_bytes=bytes(player25_p7_shot_target);std::istringstream player25_p7_shot_target_stream(player25_p7_shot_target_bytes);
 check(bytes(load_phase_session(player25_p7_shot_target_stream))==player25_p7_shot_target_bytes,"Player7 shot target roundtrips");
 auto wrong_player25_shot_version=player25_p7_shot_target_bytes;wrong_player25_shot_version[10]=static_cast<char>(142);
 rejects([&]{std::istringstream in(wrong_player25_shot_version);load_phase_session(in);});
 auto bad_player25_shot=player25_p7_shot_target;bad_player25_shot.shot_flight->destination=232;
 rejects([&]{validate_phase_session(bad_player25_shot);});
 auto player25_p7_shot_arrival=player25_p7_shot_flight;player25_p7_shot_arrival.phase=FieldSessionPhase::sampled_score25_shot_arrival;
 player25_p7_shot_arrival.current.field.ball_x=204;player25_p7_shot_arrival.current.field.ball_y=124;player25_p7_shot_arrival.current.field.ball_cell=233;
 player25_p7_shot_arrival.ball_trajectory->x=204<<8;player25_p7_shot_arrival.ball_trajectory->y=124<<8;player25_p7_shot_arrival.current.field.keepers[1].stun=0;
 auto player25_p7_keeper_entry=continue_session_unopposed_arrival(player25_p7_shot_arrival);
 check(player25_p7_keeper_entry.phase==FieldSessionPhase::shot_keeper_entry && player25_p7_keeper_entry.area_defender_cursor==5,"Player7 shot enters the goalkeeper with retained cursor");
 auto player25_p7_keeper_prepared=prepare_session_keeper_entry(player25_p7_keeper_entry);
 check(player25_p7_keeper_prepared.phase==FieldSessionPhase::keeper_presentation_pending && player25_p7_keeper_prepared.recovery->opposing_player==11,"Player7 shot prepares the opposing goalkeeper");
 auto player25_p7_keeper_decision=resolve_prepared_session_keeper(player25_p7_keeper_prepared,0,0,3,home_keeper_content);
 check(player25_p7_keeper_decision.area_defender_cursor==0,"Player7 goalkeeper comparison resets the original defender cursor");
 for(unsigned id=1;id<=21;++id)if(id!=11 && id!=16)score35_dispatch.current.field.roster[id].type=1;
 auto score35_field=advance_sampled_player14_pass_handoff(score35_dispatch,player3_race_mappings,player3_race_movement);
 check(score35_field.phase==FieldSessionPhase::sampled_field_ai_pass_presented && !score35_field.current.field.elapsed,"The 3-5 opponent pass calculates field movement");
 auto score35_handoff=acknowledge_sampled_field_ai_pass_handoff(score35_field);
 check(score35_handoff.control->facing==0 && score35_handoff.recovery->ball_variant==1,"The 3-5 player16 pass preserves the observed facing");
 auto score35_target_input=score35_handoff;score35_target_input.current.field.motion[field_motion_index(19)].x=92<<8;score35_target_input.current.field.motion[field_motion_index(19)].y=164<<8;
 auto score35_target=capture_sampled_field_pass_target(score35_target_input,19,70,255,4);
 check(score35_target.sampled_field_pass && score35_target.sampled_field_pass->destination==70 && score35_target.sampled_field_pass->distance==4,"The 3-5 opponent pass saves the observed target");
 auto score35_target_bytes=bytes(score35_target);std::istringstream score35_target_stream(score35_target_bytes);
 check(bytes(load_phase_session(score35_target_stream))==score35_target_bytes,"The 3-5 opponent target roundtrips");
 auto score35_flight=begin_sampled_field_pass_flight(score35_target,99,120,pass_geometry,pass_velocity);
 check(score35_flight.phase==FieldSessionPhase::area_ball_advance_ready && score35_flight.area_ai_target->cell==70,"The 3-5 opponent pass starts numeric flight");
 rejects([&]{begin_sampled_field_pass_flight(score35_target,98,120,pass_geometry,pass_velocity);});
 auto score35_receipt=score35_flight;score35_receipt.phase=FieldSessionPhase::area_pass_keeper_entry;
 score35_receipt.current.field.ball_owner=19;score35_receipt.current.field.ball_x=92;score35_receipt.current.field.ball_y=163;score35_receipt.current.field.ball_cell=70;
 score35_receipt.ball_trajectory->x=92<<8;score35_receipt.ball_trajectory->y=163<<8;
 auto score35_control=continue_sampled_field_pass_control(score35_receipt);
 check(score35_control.phase==FieldSessionPhase::player_control && score35_control.control->facing==64 && score35_control.saved_ball_flight_flag==1,"The 3-5 pass returns possession to player19");
 auto confirmed_empty_bytes=bytes(pass_empty_confirm);std::istringstream confirmed_empty_stream(confirmed_empty_bytes);
 auto cancel_pending_bytes=bytes(pass_cancel_pending);std::istringstream cancel_pending_stream(cancel_pending_bytes);
 check(bytes(load_phase_session(confirmed_empty_stream))==confirmed_empty_bytes && bytes(load_phase_session(cancel_pending_stream))==cancel_pending_bytes,
       "Pass confirmation and cancellation snapshots roundtrip");
 rejects([&]{poll_sampled_human_pass_target(pass_cursor_right,0,128);});
 rejects([&]{poll_sampled_human_pass_target(pass_cursor_twice,1,0);});
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

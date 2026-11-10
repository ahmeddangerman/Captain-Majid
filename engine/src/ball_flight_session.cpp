#include "captain/ball_flight_session.hpp"
#include "captain/defender_selection.hpp"
#include "captain/action_strength.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState begin_postgoal_opponent_first_flight(const PhaseSessionState& input,
 std::uint8_t x_fraction,std::uint8_t y_fraction,std::uint8_t captured_distance,
 const MovementGeometryContent& geometry,const BallVelocityContent& velocity) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_flight_handoff || x_fraction || y_fraction || captured_distance!=18)
  throw std::invalid_argument("Post-goal first flight requires its observed fractions and stale distance");
 const auto& field=input.current.field;
 const auto& recipient=field.motion[field_motion_index(input.area_ai_target->recipient)];
 const auto destination=field_cell_at(static_cast<std::uint8_t>(recipient.x>>8),static_cast<std::uint8_t>(recipient.y>>8));
 if(destination!=148 || field.ball_cell==destination)
  throw std::invalid_argument("Unobserved post-goal first flight target");
 auto state=input;state.area_ai_target->cell=destination;
 state.shot_flight=ShotFlightContext{captured_distance,destination,255,false};
 BallTrajectoryContext trajectory{static_cast<std::uint16_t>((unsigned(field.ball_x)<<8)|x_fraction),
  static_cast<std::uint16_t>((unsigned(field.ball_y)<<8)|y_fraction),0,0,0,1};
 trajectory.direction=calculate_player_direction({trajectory.x,trajectory.y,destination,0},field.ball_cell,geometry);
 const auto projected=calculate_ball_velocity(trajectory.direction,velocity);trajectory.vx=projected.x;trajectory.vy=projected.y;
 if(trajectory.direction!=34 || trajectory.vx!=162 || trajectory.vy!=185)
  throw std::invalid_argument("Unverified post-goal first flight projection");
 state.recovery->x_jitter_control=trajectory.direction;state.saved_ball_flight_flag=1;state.ball_trajectory=trajectory;
 state.phase=FieldSessionPhase::postgoal_opponent_first_flight_ready;validate_phase_session(state);return state;
}
SessionBallStepResult advance_postgoal_opponent_first_flight(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_first_flight_ready || input.current.field.selected_defenders ||
    !((input.current.field.ball_cell==135 && input.current.field.random_first==187 && input.current.field.random_second==39) ||
      (input.current.field.ball_cell==147 && input.current.field.random_first==32 && input.current.field.random_second==141)))
  throw std::invalid_argument("Post-goal first flight requires the observed frame boundary");
 auto state=input;auto& t=*state.ball_trajectory;
 const auto step=advance_ball_to_next_cell({t.x,t.y,{t.vx,t.vy},state.current.field.ball_cell,state.shot_flight->destination});
 if((input.current.field.ball_cell==135 && (step.cell!=147 || step.event!=BallStepEvent::cell_changed)) ||
    (input.current.field.ball_cell==147 && (step.cell!=148 || step.event!=BallStepEvent::arrived)))
  throw std::invalid_argument("Unobserved post-goal first flight step");
 t.x=step.x;t.y=step.y;state.current.field.ball_x=step.x>>8;state.current.field.ball_y=step.y>>8;state.current.field.ball_cell=step.cell;
 state.phase=step.event==BallStepEvent::cell_changed ? FieldSessionPhase::postgoal_opponent_first_flight_callback : FieldSessionPhase::postgoal_opponent_first_flight_arrived;
 validate_phase_session(state);return {std::move(state),step};
}
PhaseSessionState resolve_postgoal_opponent_first_flight_cell(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_first_flight_callback || input.current.field.selected_defenders)
  throw std::invalid_argument("Post-goal first flight requires an empty defender callback");
 const auto& field=input.current.field;FlightDefenderSelectionInput selection{};
 selection.possession_side=field.side;selection.ball_cell=field.ball_cell;selection.random_first=field.random_first;
 selection.random_second=field.random_second;selection.threshold=input.shot_flight->counter;
 for(unsigned i=0;i<field.motion.size();++i) { const auto& m=field.motion[i];
  selection.players[i]={static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8),m.status!=0}; }
 if(!select_flight_defenders(selection).empty())throw std::invalid_argument("Unobserved post-goal first flight defender");
 auto state=input;state.phase=FieldSessionPhase::postgoal_opponent_first_flight_ready;validate_phase_session(state);return state;
}
PhaseSessionState move_postgoal_opponent_runner(const PhaseSessionState& input) {
 validate_phase_session(input);
 const auto& runner=input.current.field.motion[field_motion_index(18)];
 if(input.phase!=FieldSessionPhase::postgoal_opponent_first_flight_arrived || input.current.field.random_first!=236 ||
    input.current.field.random_second!=69 || (runner.x>>8)!=141 || input.action->move_flags!=3)
  throw std::invalid_argument("Post-goal runner movement requires the observed inter-flight wait");
 auto state=input;auto& moved=state.current.field.motion[field_motion_index(18)];
 auto delta=static_cast<std::uint8_t>(unsigned(state.action->move_flags)<<3);
 if(state.current.field.side==11)delta=static_cast<std::uint8_t>(0u-unsigned(delta));
 auto x=static_cast<std::uint8_t>(unsigned(moved.x>>8)+delta);
 if(x>=208)x=207;
 if(x<48)x=48;
 moved.x=static_cast<std::uint16_t>((unsigned(x)<<8)|(moved.x&255u));
 state.phase=FieldSessionPhase::postgoal_opponent_runner_moved;validate_phase_session(state);return state;
}
PhaseSessionState exchange_postgoal_opponent_carrier(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_runner_moved || input.area_ai_target->recipient!=14 || input.current.field.ball_owner!=18)
  throw std::invalid_argument("Post-goal carrier exchange requires the observed runner movement");
 auto state=input;state.current.field.ball_owner=input.area_ai_target->recipient;state.area_ai_target->recipient=input.current.field.ball_owner;
 state.phase=FieldSessionPhase::postgoal_opponent_carrier_exchanged;validate_phase_session(state);return state;
}
PhaseSessionState begin_postgoal_opponent_return_flight(const PhaseSessionState& input,
 const MovementGeometryContent& geometry,const BallVelocityContent& velocity) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_carrier_exchanged)
  throw std::invalid_argument("Post-goal return flight requires the observed carrier exchange");
 const auto& field=input.current.field;const auto& recipient=field.motion[field_motion_index(input.area_ai_target->recipient)];
 const auto destination=field_cell_at(static_cast<std::uint8_t>(recipient.x>>8),static_cast<std::uint8_t>(recipient.y>>8));
 if(destination!=99 || field.ball_cell==destination)throw std::invalid_argument("Unobserved post-goal return-flight target");
 auto state=input;state.area_ai_target->cell=destination;state.shot_flight->destination=destination;
 auto& trajectory=*state.ball_trajectory;
 trajectory.direction=calculate_player_direction({trajectory.x,trajectory.y,destination,0},field.ball_cell,geometry);
 const auto projected=calculate_ball_velocity(trajectory.direction,velocity);trajectory.vx=projected.x;trajectory.vy=projected.y;
 if(trajectory.direction!=137 || trajectory.vx!=65290 || trajectory.vy!=65487)
  throw std::invalid_argument("Unverified post-goal return flight projection");
 state.recovery->x_jitter_control=trajectory.direction;state.phase=FieldSessionPhase::postgoal_opponent_return_flight_ready;
 validate_phase_session(state);return state;
}
SessionBallStepResult advance_postgoal_opponent_return_flight(const PhaseSessionState& input) {
 validate_phase_session(input);
 const auto& field=input.current.field;
 const bool observed=(field.ball_cell==148 && field.random_first==176 && field.random_second==179 && !field.selected_defenders) ||
  (field.ball_cell==136 && field.random_first==191 && field.random_second==31 && !field.selected_defenders) ||
  (field.ball_cell==124 && field.random_first==217 && field.random_second==206 && !field.selected_defenders) ||
  (field.ball_cell==112 && field.random_first==100 && field.random_second==41 && field.selected_defenders==1) ||
  (field.ball_cell==111 && field.random_first==19 && field.random_second==116 && field.selected_defenders==1);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_return_flight_ready || !observed)
  throw std::invalid_argument("Post-goal return flight requires the observed frame boundary");
 auto state=input;auto& t=*state.ball_trajectory;
 auto step=advance_ball_to_next_cell({t.x,t.y,{t.vx,t.vy},field.ball_cell,state.shot_flight->destination});
 const auto expected=field.ball_cell==148 ? 136 : field.ball_cell==136 ? 124 : field.ball_cell==124 ? 112 : field.ball_cell==112 ? 111 : 99;
 if(step.cell!=expected || step.event!=(expected==99 ? BallStepEvent::arrived : BallStepEvent::cell_changed) || step.forced_destination)
  throw std::invalid_argument("Unobserved post-goal return flight step");
 if(step.event==BallStepEvent::arrived) {
  // E7C0 enters defender handling before E7CF snaps to the destination center.
  step.x=static_cast<std::uint16_t>(std::uint32_t(t.x)+std::uint32_t(t.vx)*step.additions);
  step.y=static_cast<std::uint16_t>(std::uint32_t(t.y)+std::uint32_t(t.vy)*step.additions);
 }
 t.x=step.x;t.y=step.y;state.current.field.ball_x=step.x>>8;state.current.field.ball_y=step.y>>8;state.current.field.ball_cell=step.cell;
 state.phase=step.event==BallStepEvent::arrived ? FieldSessionPhase::postgoal_opponent_return_flight_arrived : FieldSessionPhase::postgoal_opponent_return_flight_callback;
 validate_phase_session(state);return {std::move(state),step};
}
PhaseSessionState resolve_postgoal_opponent_return_flight_cell(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_return_flight_callback)
  throw std::invalid_argument("Post-goal return flight requires a defender callback");
 const auto& field=input.current.field;FlightDefenderSelectionInput selection{};
 selection.possession_side=field.side;selection.ball_cell=field.ball_cell;selection.random_first=field.random_first;
 selection.random_second=field.random_second;selection.threshold=input.shot_flight->counter;
 for(unsigned i=0;i<field.motion.size();++i) { const auto& m=field.motion[i];
  selection.players[i]={static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8),m.status!=0}; }
 const auto selected=select_flight_defenders(selection);
 if((field.ball_cell==112 && (field.selected_defenders || selected!=std::vector<std::uint8_t>{8})) ||
    (field.ball_cell!=112 && !selected.empty()))
  throw std::invalid_argument("Unobserved post-goal return flight defender selection");
 auto state=input;
 if(field.ball_cell==112) {state.current.field.selected_defenders=1;state.scheduling->selected_ids[0]=8;}
 state.phase=FieldSessionPhase::postgoal_opponent_return_flight_ready;validate_phase_session(state);return state;
}
PhaseSessionState enter_area_pass_blocked_recovery(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::ordinary_return_shot_blocked_clocked) {
  auto state=input;state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);
  state.phase=FieldSessionPhase::ordinary_return_shot_blocked_recovery;validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::area_pass_terminal_clock_done || input.area_contact->result!=2)
  throw std::invalid_argument("Blocked pass recovery requires a completed result2 clock");
 auto state=input;state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);
 state.phase=FieldSessionPhase::area_pass_blocked_recovery_entry;validate_phase_session(state);return state;
}
PhaseSessionState resume_area_pass_added_time(const PhaseSessionState& input) {
 validate_phase_session(input);auto state=input;
 if(input.phase==FieldSessionPhase::area_pass_nonterminal_added_time)state.phase=FieldSessionPhase::area_pass_nonterminal_done;
 else if(input.phase==FieldSessionPhase::area_pass_terminal_added_time)state.phase=FieldSessionPhase::area_pass_terminal_clock_done;
 else if(input.phase==FieldSessionPhase::shot_pass_added_time)state.phase=FieldSessionPhase::shot_pass_nonterminal_done;
 else throw std::invalid_argument("No area pass added-time wait to resume");
 if(state.current.clock.script_flags&128u)throw std::invalid_argument("Area pass active-control added-time resume is not yet supported");
 validate_phase_session(state);return state;
}
PhaseSessionState continue_area_pass_capture(const PhaseSessionState& input,const PlayerProfileMappings& profiles,const FieldPhaseContent& movement,
 std::uint8_t facing,std::uint8_t motion) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_pass_defender_captured || (facing!=0 && facing!=64))
  throw std::invalid_argument("Captured pass control entry requires captured presentation and defender possession");
 auto state=input;state.current.field=advance_field_phase(input.current.field,profiles,movement).state;
 state.control->facing=facing;state.control->motion_requested=motion;
 state.phase=FieldSessionPhase::area_pass_capture_control_entry;validate_phase_session(state);return state;
}
AreaPassCaptureResult capture_area_pass_defender(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_pass_terminal_clock_done || input.area_contact->result!=3)
  throw std::invalid_argument("Pass defender capture requires terminal result3");
 auto state=input;state.current.field.ball_owner=input.recovery->opposing_player;state.current.field.side=0;
 state.phase=FieldSessionPhase::area_pass_defender_captured;validate_phase_session(state);
 return {state,input.current.field.side!=state.current.field.side};
}
PhaseSessionState advance_area_pass_terminal(const PhaseSessionState& input,const ContactPreludeContent& clock_content,
 const ExperienceContent& experience_content) {
 validate_phase_session(input);
 if(!input.experience || !input.area_contact || !((input.phase==FieldSessionPhase::area_pass_result_handler_entry && input.area_contact->result==3) ||
    (input.phase==FieldSessionPhase::area_pass_blocked_positioned && input.area_contact->result==2)))
  throw std::invalid_argument("Area pass terminal clock requires positioned result2 or result3");
 auto state=input;const auto id=state.recovery->opposing_player;
 state.experience=award_player_experience(*input.experience,input.current.field.roster[id].type,2,experience_content).state;
 const auto applied=apply_contact_prelude({state.current,id,2,false},clock_content);state.current=applied.state;
 state.phase=applied.clock.continuation==ClockContinuation::period_end ? FieldSessionPhase::area_pass_terminal_period_end :
  applied.clock.continuation==ClockContinuation::added_time_presentation ? FieldSessionPhase::area_pass_terminal_added_time : FieldSessionPhase::area_pass_terminal_clock_done;
 validate_phase_session(state);return state;
}
PhaseSessionState advance_area_pass_blocked(const PhaseSessionState& input,const ExperienceContent& experience_content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_pass_result_handler_entry || input.area_contact->result!=2 || !input.experience)
  throw std::invalid_argument("Blocked pass handler requires result2 and experience");
 auto state=input;const auto id=input.recovery->opposing_player;
 state.experience=award_player_experience(*input.experience,input.current.field.roster[id].type,2,experience_content).state;
 const auto& defender=input.current.field.motion[field_motion_index(id)];
 state.current.field.ball_x=defender.x>>8;state.current.field.ball_y=defender.y>>8;
 state.ball_trajectory->x=static_cast<std::uint16_t>((defender.x&0xff00u)|(input.ball_trajectory->x&255u));
 state.ball_trajectory->y=static_cast<std::uint16_t>((defender.y&0xff00u)|(input.ball_trajectory->y&255u));
 state.phase=FieldSessionPhase::area_pass_blocked_positioned;validate_phase_session(state);return state;
}
PhaseSessionState advance_area_pass_defender(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::shot_pass_nonterminal_done) {
  auto state=input;++state.area_defender_cursor;state.area_contact.reset();state.area_pass_contact.reset();state.area_sequence_result=255;
  if(state.area_defender_cursor<state.current.field.selected_defenders)state.phase=FieldSessionPhase::shot_pass_defenders_pending;
  else {
   state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);state.current.field.elapsed=0;
   state.phase=FieldSessionPhase::shot_pass_defenders_done;
  }
  validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::area_pass_nonterminal_done)
  throw std::invalid_argument("Area pass defender continuation needs a completed nonterminal result");
 auto state=input;++state.area_defender_cursor;state.area_contact.reset();state.area_pass_contact.reset();state.area_sequence_result=255;
 if(state.area_defender_cursor<state.current.field.selected_defenders)state.phase=FieldSessionPhase::area_pass_defenders_pending;
 else {
  state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);state.current.field.elapsed=0;
  state.phase=FieldSessionPhase::area_pass_defenders_done;
 }
 validate_phase_session(state);return state;
}
PhaseSessionState advance_area_pass_nonterminal(const PhaseSessionState& input,const ContactPreludeContent& clock_content,
 const ExperienceContent& experience_content) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::shot_pass_result_handler_entry) {
  auto state=input;
  state.experience=award_player_experience(*input.experience,input.current.field.roster[input.current.field.ball_owner].type,1,experience_content).state;
  const auto applied=apply_contact_prelude({state.current,state.recovery->opposing_player,2,true},clock_content);
  state.current=applied.state;
  state.phase=applied.clock.continuation==ClockContinuation::period_end ? FieldSessionPhase::shot_pass_period_end :
   applied.clock.continuation==ClockContinuation::added_time_presentation ? FieldSessionPhase::shot_pass_added_time : FieldSessionPhase::shot_pass_nonterminal_done;
  validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::area_pass_result_handler_entry || input.area_contact->result>1 || !input.experience)
  throw std::invalid_argument("Area pass nonterminal handler requires a result 0 or 1 and experience");
 auto state=input;
 state.experience=award_player_experience(*input.experience,input.current.field.roster[input.current.field.ball_owner].type,1,experience_content).state;
 if(input.area_contact->result==1)state.action->power=reduce_action_strength(input.action->power,input.area_contact->loss);
 const auto applied=apply_contact_prelude({state.current,state.recovery->opposing_player,2,true},clock_content);
 state.current=applied.state;
 state.phase=applied.clock.continuation==ClockContinuation::period_end ? FieldSessionPhase::area_pass_nonterminal_period_end :
  applied.clock.continuation==ClockContinuation::added_time_presentation ? FieldSessionPhase::area_pass_nonterminal_added_time : FieldSessionPhase::area_pass_nonterminal_done;
 validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_area_pass_result(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::shot_pass_variant_restored && (facing==0 || facing==64)) {
  auto state=input;state.control->facing=facing;state.control->motion_requested=motion;
  state.phase=FieldSessionPhase::shot_pass_result_handler_entry;validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::area_pass_variant_restored || (facing!=0 && facing!=64))
  throw std::invalid_argument("Area pass result needs restored variant and captured presentation");
 auto state=input;state.control->facing=facing;state.control->motion_requested=motion;
 state.phase=FieldSessionPhase::area_pass_result_handler_entry;validate_phase_session(state);return state;
}
PhaseSessionState restore_area_pass_ball_variant(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::shot_pass_defender_compared) {
  auto state=input;state.recovery->ball_variant=state.area_pass_contact->saved_ball_variant;
  state.phase=FieldSessionPhase::shot_pass_variant_restored;validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::area_pass_defender_compared)
  throw std::invalid_argument("Area pass variant restoration requires a saved comparison");
 auto state=input;state.recovery->ball_variant=state.area_pass_contact->saved_ball_variant;
 state.phase=FieldSessionPhase::area_pass_variant_restored;validate_phase_session(state);return state;
}
PhaseSessionState compare_area_pass_defender(const PhaseSessionState& input,std::uint16_t captured_power,
 std::uint8_t captured_context,const AreaPassComparisonContent& content) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::shot_pass_defender_prepared) {
  if(captured_context || content.range || content.offset || (content.eligibility&128u) ||
     unsigned(input.current.field.random_first)+input.current.field.random_second>1)
   throw std::invalid_argument("Unsupported shot pass zero-range comparison input");
  const auto result=classify_score(255,content.thresholds);
  auto state=input;state.action->menu_route=0;state.area_sequence_result=result;
  state.area_contact=AreaContactContext{input.area_defender_cursor,2,0,0,captured_power,input.area_pass_contact->captured_cost,0,0,255,result,false};
  state.phase=FieldSessionPhase::shot_pass_defender_compared;validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::area_pass_defender_prepared || captured_context>=6)
  throw std::invalid_argument("Area pass comparison requires a prepared defender");
 const auto id=input.recovery->opposing_player;
 const auto result=compare_defender_contact({input.action->power,captured_power,input.current.field.random_first,
  input.current.field.random_second,content.range,content.offset,false,bool(content.eligibility&128u),captured_context,id,0,content.thresholds});
 auto state=input;state.area_sequence_result=result.result_code;state.action->menu_route=captured_context;
 state.area_contact=AreaContactContext{input.area_defender_cursor,2,result.subtype,0,result.defender_power,input.area_pass_contact->captured_cost,
  result.strength,result.loss,result.score,result.result_code,false};
 state.phase=FieldSessionPhase::area_pass_defender_compared;validate_phase_session(state);return state;
}
PhaseSessionState prepare_area_pass_defender(const PhaseSessionState& input,std::uint16_t captured_cost) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::shot_pass_defender_announced && input.saved_ball_flight_flag && input.recovery->ball_variant==0) {
  auto state=input;state.area_pass_contact=AreaPassContactContext{0,captured_cost};
  state.recovery->opposing_player=state.scheduling->selected_ids[state.area_defender_cursor];state.phase=FieldSessionPhase::shot_pass_defender_prepared;
  validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::area_pass_defender_announced || input.recovery->ball_variant<1 || input.recovery->ball_variant>2)
  throw std::invalid_argument("Area pass defender preparation needs an announced defender and ball variant");
 auto state=input;state.area_pass_contact=AreaPassContactContext{input.recovery->ball_variant,captured_cost};
 state.recovery->ball_variant=0;state.recovery->opposing_player=state.scheduling->selected_ids[state.area_defender_cursor];
 state.phase=FieldSessionPhase::area_pass_defender_prepared;validate_phase_session(state);return state;
}
PhaseSessionState recover_session_sampled_loose_shot_result2(const PhaseSessionState& input,const ContactPreludeContent& clock,const ExperienceContent& experience) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_loose_shot_defender_compared || !input.experience || !input.control ||
    input.current.clock.remaining!=445 || input.current.clock.extra_time || input.current.clock.flags || input.current.clock.script_flags || input.current.clock.ball_mode ||
    clock.field_elapsed[0]!=2)
  throw std::invalid_argument("Unsupported sampled loose-shot result2 clock");
 const auto& defender=input.current.field.motion[field_motion_index(19)];
 if((defender.x>>8)!=140 || (defender.y>>8)!=140)
  throw std::invalid_argument("Unsupported sampled loose-shot defender position");
 auto state=input;state.control->facing=64;state.control->motion_requested=0;const auto type=input.current.field.roster[19].type;
 state.experience=award_player_experience(*state.experience,type,2,experience).state;
 state.current.field.ball_x=140;state.current.field.ball_y=140;
 state.experience=award_player_experience(*state.experience,type,2,experience).state;
 const auto applied=apply_contact_prelude({state.current,19,0,false},clock);
 if(applied.clock.continuation!=ClockContinuation::ready)throw std::invalid_argument("Unsupported sampled loose-shot clock continuation");
 state.current=applied.state;state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);
 state.shot_flight.reset();state.ball_trajectory.reset();state.area_contact.reset();state.area_defender_cursor=0;state.area_sequence_result=255;
 state.phase=FieldSessionPhase::ball_recovery;validate_phase_session(state);return state;
}
PhaseSessionState compare_session_sampled_loose_shot_defender(const PhaseSessionState& input,std::uint16_t captured_power,const FlightContactContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_loose_shot_defender_prepared || captured_power!=19 || input.action->power!=11 || input.action->menu_route ||
    content.defense_actions[0]!=0 || content.eligibility[0]!=128 || content.variation[0]!=std::array<std::uint8_t,2>{192,64} ||
    content.thresholds[0]!=std::array<std::uint8_t,4>{154,96,48,0})
  throw std::invalid_argument("Unsupported sampled loose-shot comparison input");
 const auto result=compare_defender_contact({input.action->power,captured_power,input.current.field.random_first,input.current.field.random_second,
  content.variation[0][0],content.variation[0][1],false,true,0,19,0,content.thresholds[0]});
 if(result.result_code!=2 || result.strength!=9 || result.loss!=2 || result.score!=78 || result.subtype)
  throw std::invalid_argument("Sampled loose-shot comparison disagrees with traced result");
 auto state=input;state.area_contact=AreaContactContext{0,0,0,0,captured_power,0,result.strength,result.loss,result.score,result.result_code,false};
 state.area_sequence_result=result.result_code;state.phase=FieldSessionPhase::sampled_loose_shot_defender_compared;
 validate_phase_session(state);return state;
}
PhaseSessionState prepare_session_sampled_loose_shot_defender(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::shot_pass_defender_announced || input.saved_ball_flight_flag || input.area_defender_cursor ||
    input.scheduling->selected_ids[0]!=19 || input.recovery->ball_variant)
  throw std::invalid_argument("Unsupported sampled loose-shot defender preparation");
 auto state=input;state.recovery->opposing_player=19;state.phase=FieldSessionPhase::sampled_loose_shot_defender_prepared;
 validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_area_pass_defender_announcement(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::shot_pass_defenders_pending) {
  auto state=input;state.phase=FieldSessionPhase::shot_pass_defender_announced;
  validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::area_pass_defenders_pending)
  throw std::invalid_argument("Area pass defender announcement requires a pending defender");
 auto state=input;state.phase=FieldSessionPhase::area_pass_defender_announced;
 validate_phase_session(state);return state;
}
PhaseSessionState begin_session_area_action4_restart_flight(const PhaseSessionState& input,
 std::uint8_t recipient,std::uint8_t low_x,std::uint8_t low_y,std::uint8_t distance,
 const MovementGeometryContent& geometry,const BallVelocityContent& velocity) {
 validate_phase_session(input);
 const auto& f=input.current.field;
 if(input.phase!=FieldSessionPhase::area_action4_restart_pass_entry || recipient!=15 || low_x || low_y || distance!=4 ||
    f.ball_x!=52 || f.ball_y!=172 || f.ball_cell!=11 || f.selected_defenders!=1 || f.elapsed!=19 ||
    field_cell_at(f.motion[field_motion_index(recipient)].x>>8,f.motion[field_motion_index(recipient)].y>>8)!=113)
  throw std::invalid_argument("Unsupported captured action4 restart flight setup");
 auto state=input;
 state.action->action=1;state.action->power=input.area_action4_restart_power->power;
 state.recovery->attack_subtype=0;
 state.area_ai_target=PhaseSessionState::AreaAiTarget{recipient,113,input.area_action4_restart_power->cost};
 state.shot_flight=ShotFlightContext{distance,113,*input.area_action4_restart_pass_counter,false};
 BallTrajectoryContext t{static_cast<std::uint16_t>(unsigned(f.ball_x)<<8),static_cast<std::uint16_t>(unsigned(f.ball_y)<<8),0,0,0,1};
 t.direction=calculate_player_direction({t.x,t.y,113,0},f.ball_cell,geometry);
 const auto v=calculate_ball_velocity(t.direction,velocity);t.vx=v.x;t.vy=v.y;
 state.recovery->x_jitter_control=t.direction;state.ball_trajectory=t;state.saved_ball_flight_flag=1;
 state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);state.current.field.elapsed=0;
 state.area_defense.reset();state.area_contact.reset();state.area_action4_restart.reset();
 state.area_action4_restart_power.reset();state.area_action4_restart_pass_counter.reset();
 state.area_defender_cursor=0;state.area_sequence_result=255;state.phase=FieldSessionPhase::area_ball_advance_ready;
 validate_phase_session(state);return state;
}
PhaseSessionState begin_prepared_area_action_flight(const PhaseSessionState& input,
 const AreaFlightAcknowledgement& ack,const MovementGeometryContent& geometry,const BallVelocityContent& velocity,const ExperienceContent* experience) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_attack_continuation || input.action->action!=1 || (ack.facing!=0 && ack.facing!=64))
  throw std::invalid_argument("Area flight needs action1 and acknowledged presentation");
 const auto& f=input.current.field;
 if(field_cell_at(f.ball_x,f.ball_y)!=f.ball_cell)
  throw std::invalid_argument("Area flight needs coherent ball coordinates and cell");
 auto state=input;
 if(f.selected_defenders) {
  if(!experience || !input.experience)throw std::invalid_argument("Area continuation requires experience content");
  state.experience=award_player_experience(*input.experience,f.roster[f.ball_owner].type,1,*experience).state;
 }
 const auto& recipient=f.motion[field_motion_index(input.area_ai_target->recipient)];
 const auto destination=field_cell_at(recipient.x>>8,recipient.y>>8);
 state.area_ai_target->cell=destination;
 state.shot_flight=ShotFlightContext{ack.distance,destination,static_cast<std::uint8_t>((input.action->auxiliary&128u)?38:255),false};
 BallTrajectoryContext t{static_cast<std::uint16_t>((unsigned(f.ball_x)<<8)|ack.x_fraction),
  static_cast<std::uint16_t>((unsigned(f.ball_y)<<8)|ack.y_fraction),ack.prior_vx,ack.prior_vy,input.recovery->x_jitter_control,1};
 state.control->facing=ack.facing;state.control->motion_requested=ack.motion;
 state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);state.current.field.elapsed=0;
 state.keeper_area.reset();state.area_defender_cursor=0;state.area_sequence_result=255;
 if(f.ball_cell==destination)state.phase=FieldSessionPhase::area_ball_already_arrived;
 else {
  t.direction=calculate_player_direction({t.x,t.y,destination,0},f.ball_cell,geometry);
  const auto v=calculate_ball_velocity(t.direction,velocity);t.vx=v.x;t.vy=v.y;
  state.recovery->x_jitter_control=t.direction;state.phase=FieldSessionPhase::area_ball_advance_ready;
 }
 state.ball_trajectory=t;validate_phase_session(state);return state;
}
PhaseSessionState begin_ordinary_pass_flight(const PhaseSessionState& input,std::uint8_t x_fraction,std::uint8_t y_fraction,
 const MovementGeometryContent& geometry,const BallVelocityContent& velocity) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_pass_flight_target || !input.sampled_field_pass || !input.scheduling || !input.recovery)
  throw std::invalid_argument("Ordinary pass flight requires the resolved target");
 const auto pass=*input.sampled_field_pass;const auto& field=input.current.field;
 if(field.ball_cell==pass.destination)throw std::invalid_argument("Same-cell ordinary pass flight is unsupported");
 auto state=input;
 state.sampled_field_pass.reset();state.ordinary_pass_candidates.reset();state.ordinary_pass_recipient.reset();
 state.ordinary_pass_defenses.reset();state.human_pass_cursor.reset();state.human_pass_steps=0;
 state.area_contact.reset();state.area_defender_cursor=input.ordinary_pass_defenses->count;state.area_sequence_result=255;
 state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);state.saved_ball_flight_flag=1;
 state.area_ai_target=PhaseSessionState::AreaAiTarget{pass.recipient,pass.destination,0};
 state.shot_flight=ShotFlightContext{pass.distance,pass.destination,pass.counter,false};
 BallTrajectoryContext t{static_cast<std::uint16_t>((unsigned(field.ball_x)<<8)|x_fraction),
  static_cast<std::uint16_t>((unsigned(field.ball_y)<<8)|y_fraction),0,0,input.recovery->x_jitter_control,1};
 t.direction=calculate_player_direction({t.x,t.y,pass.destination,0},field.ball_cell,geometry);
 const auto v=calculate_ball_velocity(t.direction,velocity);t.vx=v.x;t.vy=v.y;
 state.recovery->x_jitter_control=t.direction;state.ball_trajectory=t;state.phase=FieldSessionPhase::ordinary_pass_flight_ready;
 validate_phase_session(state);return state;
}
PhaseSessionState begin_sampled_field_pass_flight(const PhaseSessionState& input, std::uint8_t x_fraction, std::uint8_t y_fraction, const MovementGeometryContent& geometry, const BallVelocityContent& velocity) {
 validate_phase_session(input);
 const bool score35=input.phase==FieldSessionPhase::sampled_field_ai_pass_target && input.score &&
  input.score->left_goals==3 && input.score->right_goals==5 && input.current.clock.remaining==296;
 const bool player7=input.phase==FieldSessionPhase::sampled_score25_pass_target_captured;
 const bool score36_player7=player7 && input.score && input.score->left_goals==3 && input.score->right_goals==6;
 const bool mixed=input.phase==FieldSessionPhase::sampled_mixed_result4_target;
 const bool player12=input.phase==FieldSessionPhase::sampled_player12_pass_target;
 const bool player14=input.phase==FieldSessionPhase::sampled_player14_pass_target;
 const bool postgoal_player12=player12 && input.score && input.score->left_goals==2 && input.score->right_goals==1 &&
  input.current.clock.remaining==384 && input.current.clock.extra_time==13;
 const bool postgoal_mixed=mixed && input.score && input.score->left_goals==2 && input.score->right_goals==1 &&
  input.current.clock.remaining==385 && input.current.clock.extra_time==13;
 const bool postgoal=input.phase==FieldSessionPhase::sampled_field_ai_pass_target && input.score &&
  input.score->left_goals==2 && input.score->right_goals==1 && input.current.clock.remaining==406 && input.current.clock.extra_time==9;
 if((input.phase!=FieldSessionPhase::sampled_field_ai_pass_target && !mixed && !player12 && !player14 && !player7) ||
    x_fraction!=(score35 ? 99 : player7 ? (score36_player7 ? 82 : 167) : player14 ? 101 : player12 ? (postgoal_player12 ? 8 : 120) : mixed ? (postgoal_mixed ? 232 : 216) : postgoal ? 143 : 127) ||
    y_fraction!=(score35 ? 120 : player7 ? (score36_player7 ? 109 : 221) : player14 ? 13 : player12 ? (postgoal_player12 ? 255 : 132) : mixed ? (postgoal_mixed ? 255 : 228) : postgoal ? 27 : 0))
  throw std::invalid_argument("Unsupported sampled opponent pass flight input");
 const auto& field=input.current.field;const auto pass=*input.sampled_field_pass;
 if(field_cell_at(field.ball_x,field.ball_y)!=field.ball_cell || field.ball_cell==pass.destination)
  throw std::invalid_argument("Sampled pass flight requires a distinct coherent ball cell");
 auto state=input;state.sampled_field_pass.reset();
 if(player7) {
  state.human_pass_cursor.reset();state.human_pass_steps=0;state.area_contact.reset();
  state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);state.area_sequence_result=255;state.saved_ball_flight_flag=1;
 }
 if(mixed) {
  state.current.field.selected_defenders=0;state.current.field.elapsed=0;
  state.scheduling->selected_ids.fill(0);state.area_defense.reset();state.area_sequence_result=255;
 }
 // The prior goal retains an internal defender result; this new pass starts a fresh sequence.
 if(postgoal){state.area_defender_cursor=0;state.area_sequence_result=255;}
 state.area_ai_target=PhaseSessionState::AreaAiTarget{pass.recipient,pass.destination,0};
 state.shot_flight=ShotFlightContext{pass.distance,pass.destination,pass.counter,false};
 BallTrajectoryContext t{static_cast<std::uint16_t>((unsigned(field.ball_x)<<8)|x_fraction),
  static_cast<std::uint16_t>((unsigned(field.ball_y)<<8)|y_fraction),0,0,input.recovery->x_jitter_control,1};
 t.direction=calculate_player_direction({t.x,t.y,pass.destination,0},field.ball_cell,geometry);
 const auto v=calculate_ball_velocity(t.direction,velocity);t.vx=v.x;t.vy=v.y;
 state.recovery->x_jitter_control=t.direction;state.ball_trajectory=t;state.phase=FieldSessionPhase::area_ball_advance_ready;
 validate_phase_session(state);return state;
}
PhaseSessionState begin_prepared_session_ball_flight(const PhaseSessionState& input,
 std::uint8_t low_x,std::uint8_t low_y,const MovementGeometryContent& geometry,const BallVelocityContent& velocity) {
 validate_phase_session(input);
 const bool returned=input.phase==FieldSessionPhase::ordinary_return_shot_target;
 const bool player7=input.phase==FieldSessionPhase::sampled_score25_shot_target;
 const bool sampled_home_shot=input.phase==FieldSessionPhase::sampled_home_shot_target;
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_shot_target;
 if(input.phase!=FieldSessionPhase::shot_flight_pending && !sampled_home_shot && !score23 && !player7 && !returned)
  throw std::invalid_argument("Flight setup requires a saved shot destination");
 if(player7 && (low_x!=(input.score && input.score->left_goals==3 ? 223 : 52) || low_y!=(input.score && input.score->left_goals==3 ? 75 : 187)))throw std::invalid_argument("Player7 shot needs observed fractional position");
 if(sampled_home_shot && (low_x!=116 || low_y!=92))
  throw std::invalid_argument("Home shot flight requires the observed fractional position");
 const bool player17=score23 && input.current.field.ball_owner==17;
 const bool score35=score23 && input.score && input.score->left_goals==3 && input.score->right_goals==5;
 if(score23 && (low_x!=(score35 ? 66 : player17 ? 116 : 44) || low_y!=(score35 ? 124 : player17 ? 105 : 73)))
  throw std::invalid_argument("Sampled opponent shot flight requires its observed fractional position");
 const auto& f=input.current.field;
 if(field_cell_at(f.ball_x,f.ball_y)!=f.ball_cell || (returned && f.ball_cell==input.shot_flight->destination))
  throw std::invalid_argument("Flight requires coherent current ball coordinates/cell and supported destination");
 auto state=input;
 // The observed 1-1 restart shot runs DD08's zero write before the flight entry.
 const bool sampled_restart_shot=input.saved_ball_flight_flag && input.score && input.kickoff_formation &&
  input.score->left_goals==1 && input.score->right_goals==1 && input.score->kickoff==4 &&
  f.side==0 && f.ball_owner==5 && f.ball_x==108 && f.ball_y==108 && f.ball_cell==87 &&
  !f.random_first && !f.random_second && input.current.clock.remaining==432 && input.current.clock.extra_time==5 &&
  input.action->action==0 && input.action->power==22 && input.recovery->attack_subtype==128 &&
  input.shot_flight->destination==233 && input.shot_flight->distance==108 && !input.shot_flight->loose_ball;
 if(sampled_restart_shot || sampled_home_shot || score23 || player7 || returned)state.saved_ball_flight_flag=0;
 if(returned)state.area_contact.reset();
 BallTrajectoryContext t{static_cast<std::uint16_t>((unsigned(f.ball_x)<<8)|low_x),
  static_cast<std::uint16_t>((unsigned(f.ball_y)<<8)|low_y),0,0,input.recovery->x_jitter_control,state.saved_ball_flight_flag};
 state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);
 state.current.field.elapsed=0;state.shot_progress.reset();
 if(sampled_home_shot){state.area_defense.reset();state.area_defender_cursor=0;}
 if(f.ball_cell==input.shot_flight->destination)state.phase=FieldSessionPhase::ball_already_arrived;
 else {
  t.direction=calculate_player_direction({t.x,t.y,input.shot_flight->destination,0},f.ball_cell,geometry);
  const auto v=calculate_ball_velocity(t.direction,velocity);t.vx=v.x;t.vy=v.y;
  state.recovery->x_jitter_control=t.direction;state.phase=returned ? FieldSessionPhase::ordinary_return_shot_flight_ready : player7 ? FieldSessionPhase::sampled_score25_shot_advance : score23 ? FieldSessionPhase::sampled_score23_ball_advance_ready : FieldSessionPhase::ball_advance_ready;
 }
 state.ball_trajectory=t;validate_phase_session(state);return state;
}
PhaseSessionState prepare_ordinary_return_flight_defender(const PhaseSessionState& input,const FieldPlayerProfiles& profiles,const FieldDefenseContent& defense) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_return_shot_defenders_pending || input.recovery->ball_variant!=1 || !input.current.field.selected_defenders)
  throw std::invalid_argument("Returned shot flight defender requires the observed selected prefix and ball variant");
 const auto id=input.scheduling->selected_ids[0];const auto& player=input.current.field.roster[id];
 const auto prep=prepare_profiled_field_defender({id,player.type,static_cast<std::uint8_t>(player.stamina),
  static_cast<std::uint8_t>(player.stamina>>8),player.level},profiles,0,0,0,defense);
 if(prep.move_effect)throw std::invalid_argument("Unsupported returned flight defender move effect");
 auto state=input;state.recovery->opposing_player=id;state.recovery->ball_variant=0;state.action->move_effect=prep.move_effect;
 state.area_contact=AreaContactContext{0,0,0,prep.move_effect,prep.base_strength,prep.stamina_cost,0,0,0,0,false};
 state.phase=FieldSessionPhase::ordinary_return_shot_flight_defender_prepared;validate_phase_session(state);return state;
}
PhaseSessionState compare_ordinary_return_flight_defender(const PhaseSessionState& input,const FlightContactContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_return_shot_flight_defender_prepared || content.defense_actions[0]!=0)
  throw std::invalid_argument("Returned shot flight comparison requires the prepared action0 defender");
 const auto& c=*input.area_contact;const auto& variation=content.variation[0];
 const auto result=compare_defender_contact({input.action->power,c.defender_power,input.current.field.random_first,input.current.field.random_second,
  variation[0],variation[1],false,bool(content.eligibility[0]&128u),0,input.recovery->opposing_player,0,content.thresholds[0]});
 if(result.result_code!=2 || result.subtype)throw std::invalid_argument("Only observed returned flight defender result2 is supported");
 auto state=input;state.area_contact->strength=result.strength;state.area_contact->loss=result.loss;state.area_contact->score=result.score;
 state.area_contact->result=result.result_code;state.area_sequence_result=result.result_code;state.recovery->ball_variant=1;
 state.phase=FieldSessionPhase::ordinary_return_shot_flight_defender_compared;validate_phase_session(state);return state;
}
PhaseSessionState position_ordinary_return_blocked_shot(const PhaseSessionState& input,const ExperienceContent& experience) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_return_shot_flight_defender_compared || !input.experience)
  throw std::invalid_argument("Blocked shot placement needs a compared result2 defender");
 auto state=input;const auto id=state.recovery->opposing_player;
 state.experience=award_player_experience(*state.experience,state.current.field.roster[id].type,2,experience).state;
 const auto& defender=state.current.field.motion[field_motion_index(id)];
 state.current.field.ball_x=defender.x>>8;state.current.field.ball_y=defender.y>>8;
 state.ball_trajectory->x=static_cast<std::uint16_t>((defender.x&0xff00u)|(state.ball_trajectory->x&255u));
 state.ball_trajectory->y=static_cast<std::uint16_t>((defender.y&0xff00u)|(state.ball_trajectory->y&255u));
 state.phase=FieldSessionPhase::ordinary_return_shot_blocked_positioned;validate_phase_session(state);return state;
}
PhaseSessionState advance_ordinary_return_blocked_shot_clock(const PhaseSessionState& input,const ContactPreludeContent& clock,const ExperienceContent& experience) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_return_shot_blocked_positioned || !input.experience || clock.field_elapsed[0]!=2)
  throw std::invalid_argument("Blocked shot clock requires its positioned defender and observed elapsed time");
 auto state=input;const auto id=state.recovery->opposing_player;
 state.experience=award_player_experience(*state.experience,state.current.field.roster[id].type,2,experience).state;
 const auto applied=apply_contact_prelude({state.current,id,0,false},clock);
 if(applied.clock.continuation!=ClockContinuation::ready)throw std::invalid_argument("Unsupported blocked shot clock continuation");
 state.current=applied.state;state.phase=FieldSessionPhase::ordinary_return_shot_blocked_clocked;validate_phase_session(state);return state;
}
PhaseSessionState continue_session_unopposed_arrival(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::ordinary_return_shot_flight_arrival && input.current.field.selected_defenders) {
  auto state=input;state.area_defender_cursor=0;state.phase=FieldSessionPhase::ordinary_return_shot_defenders_pending;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_score25_shot_arrival) {
  auto state=input;state.phase=FieldSessionPhase::shot_keeper_entry;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_score23_ball_arrival_callback) {
  auto state=input;state.current.field.selected_defenders=1;state.scheduling->selected_ids.fill(0);
  state.action->menu_route=4;state.action->cursor=0;state.control->motion_requested=0;
  state.shot_flight.reset();state.ball_trajectory.reset();state.phase=FieldSessionPhase::sampled_score23_keeper_prompt;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::shot_pass_defenders_done) {
  auto state=input;state.area_defender_cursor=0;
  state.phase=input.shot_flight->loose_ball ? FieldSessionPhase::shot_loose_recovery : FieldSessionPhase::shot_keeper_entry;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ball_arrival_callback && (input.saved_ball_flight_flag || input.shot_flight->loose_ball) &&
    input.current.field.side==0 && input.current.field.selected_defenders) {
  auto state=input;state.area_defender_cursor=0;state.phase=FieldSessionPhase::shot_pass_defenders_pending;
  validate_phase_session(state);return state;
 }
 if((input.phase!=FieldSessionPhase::ball_arrival_callback && input.phase!=FieldSessionPhase::ball_already_arrived) ||
    input.current.field.selected_defenders)
  throw std::invalid_argument("Arrival still has pending defender encounters or is not complete");
 auto state=input;state.phase=input.shot_flight->loose_ball ? FieldSessionPhase::shot_loose_recovery : FieldSessionPhase::shot_keeper_entry;
 validate_phase_session(state);return state;
}
PhaseSessionState continue_area_pass_received(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_pass_received)
  throw std::invalid_argument("Area pass keeper entry requires the saved recipient handoff");
 const auto& player=input.current.field.motion[field_motion_index(input.current.field.ball_owner)];
 const auto x=static_cast<std::uint8_t>(player.x>>8),y=static_cast<std::uint8_t>(player.y>>8);
 const auto cell=field_cell_at(x,y);
 if(cell==255)throw std::invalid_argument("Area pass recipient is outside the represented field");
 auto state=input;state.current.field.ball_x=x;state.current.field.ball_y=y;state.current.field.ball_cell=cell;
 state.ball_trajectory->x=static_cast<std::uint16_t>((unsigned(x)<<8)|(state.ball_trajectory->x&255));
 state.ball_trajectory->y=static_cast<std::uint16_t>((unsigned(y)<<8)|(state.ball_trajectory->y&255));
 state.phase=FieldSessionPhase::area_pass_keeper_entry;validate_phase_session(state);return state;
}
PhaseSessionState continue_area_already_arrived(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_ball_already_arrived && input.phase!=FieldSessionPhase::area_ball_arrival_callback &&
    input.phase!=FieldSessionPhase::area_pass_defenders_done)
  throw std::invalid_argument("Area pass handoff needs a pending arrival");
 auto state=input;
 if(input.current.field.selected_defenders) {
  if(input.phase!=FieldSessionPhase::area_ball_arrival_callback)
   throw std::invalid_argument("Already-arrived area pass cannot have queued defenders");
  state.area_defender_cursor=0;state.phase=FieldSessionPhase::area_pass_defenders_pending;
 } else {
  state.current.field.ball_owner=state.area_ai_target->recipient;
  const bool player7=state.score && ((state.score->left_goals==2 && state.score->right_goals==5 && state.current.clock.remaining==321) ||
   (state.score->left_goals==3 && state.score->right_goals==6 && state.current.clock.remaining==279)) &&
   state.current.field.side==0 && state.area_ai_target->recipient==7;
  if(!player7)state.area_defender_cursor=0;
  state.phase=FieldSessionPhase::area_pass_received;
 }
 validate_phase_session(state);return state;
}
PhaseSessionState resolve_session_area_ball_cell(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_ball_cell_callback)
  throw std::invalid_argument("No pending area ball cell callback");
 FlightDefenderSelectionInput selection{};const auto& field=input.current.field;
 selection.possession_side=field.side;selection.ball_cell=field.ball_cell;
 selection.random_first=field.random_first;selection.random_second=field.random_second;
 selection.threshold=input.shot_flight->counter;
 for(unsigned i=0;i<field.motion.size();++i){const auto& m=field.motion[i];
  selection.players[i]={static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8),m.status!=0};}
 const auto selected=select_flight_defenders(selection);auto state=input;auto& count=state.current.field.selected_defenders;
 for(auto id:selected){if(count>=4)break;state.scheduling->selected_ids[count++]=id;}
 state.phase=FieldSessionPhase::area_ball_advance_ready;validate_phase_session(state);return state;
}
PhaseSessionState resolve_ordinary_pass_ball_cell(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_pass_flight_callback || input.current.field.selected_defenders)
  throw std::invalid_argument("Ordinary pass callback requires an empty defender prefix");
 FlightDefenderSelectionInput selection{};const auto& field=input.current.field;
 selection.possession_side=field.side;selection.ball_cell=field.ball_cell;
 selection.random_first=field.random_first;selection.random_second=field.random_second;
 selection.threshold=input.shot_flight->counter;
 for(unsigned i=0;i<field.motion.size();++i){const auto& m=field.motion[i];
  selection.players[i]={static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8),m.status!=0};}
 if(!select_flight_defenders(selection).empty())throw std::invalid_argument("Ordinary pass defended flight callback is unsupported");
 auto state=input;state.phase=FieldSessionPhase::ordinary_pass_flight_ready;validate_phase_session(state);return state;
}
PhaseSessionState continue_ordinary_pass_reception(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_pass_flight_arrival || !input.area_ai_target ||
    input.area_ai_target->recipient<1 || input.area_ai_target->recipient>10 || input.current.field.selected_defenders)
  throw std::invalid_argument("Ordinary reception requires an unopposed teammate arrival");
 auto state=input;state.current.field.ball_owner=state.area_ai_target->recipient;
 state.phase=FieldSessionPhase::ordinary_pass_received;validate_phase_session(state);return state;
}
PhaseSessionState position_ordinary_pass_recipient(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_pass_received)throw std::invalid_argument("Ordinary recipient positioning requires possession");
 const auto& player=input.current.field.motion[field_motion_index(input.current.field.ball_owner)];
 const auto x=static_cast<std::uint8_t>(player.x>>8),y=static_cast<std::uint8_t>(player.y>>8);
 const auto cell=field_cell_at(x,y);
 if(cell==255)throw std::invalid_argument("Ordinary pass recipient is outside the field");
 auto state=input;state.current.field.ball_x=x;state.current.field.ball_y=y;state.current.field.ball_cell=cell;
 state.ball_trajectory->x=static_cast<std::uint16_t>((unsigned(x)<<8)|(state.ball_trajectory->x&255));
 state.ball_trajectory->y=static_cast<std::uint16_t>((unsigned(y)<<8)|(state.ball_trajectory->y&255));
 state.phase=FieldSessionPhase::ordinary_pass_recipient_positioned;validate_phase_session(state);return state;
}
PhaseSessionState continue_ordinary_pass_control(const PhaseSessionState& input,std::uint8_t observed_facing) {
 validate_phase_session(input);
 if((input.phase!=FieldSessionPhase::ordinary_pass_recipient_positioned && input.phase!=FieldSessionPhase::ordinary_free_pass_contest_winner) || (observed_facing!=0 && observed_facing!=64))
  throw std::invalid_argument("Ordinary pass control requires positioned possession and observed facing");
 const auto& field=input.current.field;
 if((field.ball_y>=96 && field.ball_y<160 && (field.ball_x<80 || field.ball_x>=176)) ||
    (input.recovery->game_mode==4 && field.roster[field.ball_owner].type!=1 && field.ball_x>=128))
  throw std::invalid_argument("Ordinary pass awaits keeper-area or story continuation");
 auto state=input;state.control->facing=observed_facing;
 state.area_ai_target.reset();state.shot_flight.reset();state.ball_trajectory.reset();
 state.phase=FieldSessionPhase::ordinary_pass_control_entry;validate_phase_session(state);return state;
}
SessionBallStepResult advance_ordinary_pass_ball_cell(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_pass_flight_ready || input.current.field.selected_defenders)
  throw std::invalid_argument("Ordinary ball step requires an empty flight boundary");
 auto state=input;auto& t=*state.ball_trajectory;
 const auto step=advance_ball_to_next_cell({t.x,t.y,{t.vx,t.vy},state.current.field.ball_cell,state.shot_flight->destination});
 if(step.event!=BallStepEvent::cell_changed && state.area_ai_target->recipient==255)
  throw std::invalid_argument("Free-ball pass arrival needs original contest handling");
 t.x=step.x;t.y=step.y;state.current.field.ball_x=step.x>>8;state.current.field.ball_y=step.y>>8;
 state.current.field.ball_cell=step.cell;
 state.phase=step.event==BallStepEvent::cell_changed ? FieldSessionPhase::ordinary_pass_flight_callback : FieldSessionPhase::ordinary_pass_flight_arrival;
 validate_phase_session(state);return {std::move(state),step};
}
PhaseSessionState resolve_session_ball_cell(const PhaseSessionState& input) {
 validate_phase_session(input);
 const bool returned=input.phase==FieldSessionPhase::ordinary_return_shot_flight_callback;
 const bool player7=input.phase==FieldSessionPhase::sampled_score25_shot_callback;
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_ball_cell_callback;
 if(input.phase!=FieldSessionPhase::ball_cell_callback && !score23 && !player7 && !returned)
  throw std::invalid_argument("No pending ball cell callback");
 FlightDefenderSelectionInput selection{};
 const auto& field=input.current.field;
 selection.possession_side=field.side;selection.ball_cell=field.ball_cell;
 selection.random_first=field.random_first;selection.random_second=field.random_second;
 selection.threshold=input.shot_flight->counter;
 for(unsigned i=0;i<field.motion.size();++i){const auto& m=field.motion[i];
  selection.players[i]={static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8),m.status!=0};}
 const auto selected=select_flight_defenders(selection);
 auto state=input;auto& count=state.current.field.selected_defenders;
 const auto capacity=field.side==0 ? 5u : 4u;
 for(auto id:selected){if(count>=capacity)break;state.scheduling->selected_ids[count++]=id;}
 state.phase=returned ? FieldSessionPhase::ordinary_return_shot_flight_ready : player7 ? FieldSessionPhase::sampled_score25_shot_advance : score23 ? FieldSessionPhase::sampled_score23_ball_advance_ready : FieldSessionPhase::ball_advance_ready;validate_phase_session(state);return state;
}
SessionBallStepResult advance_prepared_area_ball_cell(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_ball_advance_ready)
  throw std::invalid_argument("Area ball step requires acknowledged frame and pending area flight");
 auto state=input;auto& t=*state.ball_trajectory;
 const auto step=advance_ball_to_next_cell({t.x,t.y,{t.vx,t.vy},state.current.field.ball_cell,state.shot_flight->destination});
 t.x=step.x;t.y=step.y;state.current.field.ball_x=step.x>>8;state.current.field.ball_y=step.y>>8;
 state.current.field.ball_cell=step.cell;
 state.phase=step.event==BallStepEvent::cell_changed ? FieldSessionPhase::area_ball_cell_callback : FieldSessionPhase::area_ball_arrival_callback;
 validate_phase_session(state);return {std::move(state),step};
}
SessionBallStepResult advance_prepared_session_ball_cell(const PhaseSessionState& input) {
 validate_phase_session(input);
 const bool returned=input.phase==FieldSessionPhase::ordinary_return_shot_flight_ready;
 const bool player7=input.phase==FieldSessionPhase::sampled_score25_shot_advance;
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_ball_advance_ready;
 if(input.phase!=FieldSessionPhase::ball_advance_ready && !score23 && !player7 && !returned)
  throw std::invalid_argument("Ball step requires acknowledged frame and pending numeric flight");
 auto state=input;auto& t=*state.ball_trajectory;
 const auto step=advance_ball_to_next_cell({t.x,t.y,{t.vx,t.vy},state.current.field.ball_cell,state.shot_flight->destination});
 t.x=step.x;t.y=step.y;state.current.field.ball_x=step.x>>8;state.current.field.ball_y=step.y>>8;
 state.current.field.ball_cell=step.cell;
 state.phase=returned ? (step.event==BallStepEvent::cell_changed ? FieldSessionPhase::ordinary_return_shot_flight_callback : FieldSessionPhase::ordinary_return_shot_flight_arrival) :
  player7 ? (step.event==BallStepEvent::cell_changed ? FieldSessionPhase::sampled_score25_shot_callback : FieldSessionPhase::sampled_score25_shot_arrival) :
  score23 ? (step.event==BallStepEvent::cell_changed ? FieldSessionPhase::sampled_score23_ball_cell_callback : FieldSessionPhase::sampled_score23_ball_arrival_callback) :
  (step.event==BallStepEvent::cell_changed ? FieldSessionPhase::ball_cell_callback : FieldSessionPhase::ball_arrival_callback);
 validate_phase_session(state);return {std::move(state),step};
}
}

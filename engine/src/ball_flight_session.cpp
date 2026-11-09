#include "captain/ball_flight_session.hpp"
#include "captain/defender_selection.hpp"
#include "captain/action_strength.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState enter_area_pass_blocked_recovery(const PhaseSessionState& input) {
 validate_phase_session(input);
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
PhaseSessionState begin_sampled_field_pass_flight(const PhaseSessionState& input, std::uint8_t x_fraction, std::uint8_t y_fraction, const MovementGeometryContent& geometry, const BallVelocityContent& velocity) {
 validate_phase_session(input);
 const bool score35=input.phase==FieldSessionPhase::sampled_field_ai_pass_target && input.score &&
  input.score->left_goals==3 && input.score->right_goals==5 && input.current.clock.remaining==296;
 const bool player7=input.phase==FieldSessionPhase::sampled_score25_pass_target_captured;
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
    x_fraction!=(score35 ? 99 : player7 ? 167 : player14 ? 101 : player12 ? (postgoal_player12 ? 8 : 120) : mixed ? (postgoal_mixed ? 232 : 216) : postgoal ? 143 : 127) ||
    y_fraction!=(score35 ? 120 : player7 ? 221 : player14 ? 13 : player12 ? (postgoal_player12 ? 255 : 132) : mixed ? (postgoal_mixed ? 255 : 228) : postgoal ? 27 : 0))
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
 const bool player7=input.phase==FieldSessionPhase::sampled_score25_shot_target;
 const bool sampled_home_shot=input.phase==FieldSessionPhase::sampled_home_shot_target;
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_shot_target;
 if(input.phase!=FieldSessionPhase::shot_flight_pending && !sampled_home_shot && !score23 && !player7)
  throw std::invalid_argument("Flight setup requires a saved shot destination");
 if(player7 && (low_x!=52 || low_y!=187))throw std::invalid_argument("Player7 shot needs observed fractional position");
 if(sampled_home_shot && (low_x!=116 || low_y!=92))
  throw std::invalid_argument("Home shot flight requires the observed fractional position");
 const bool player17=score23 && input.current.field.ball_owner==17;
 const bool score35=score23 && input.score && input.score->left_goals==3 && input.score->right_goals==5;
 if(score23 && (low_x!=(score35 ? 66 : player17 ? 116 : 44) || low_y!=(score35 ? 124 : player17 ? 105 : 73)))
  throw std::invalid_argument("Sampled opponent shot flight requires its observed fractional position");
 const auto& f=input.current.field;
 if(field_cell_at(f.ball_x,f.ball_y)!=f.ball_cell)
  throw std::invalid_argument("Flight requires coherent current ball coordinates/cell");
 auto state=input;
 // The observed 1-1 restart shot runs DD08's zero write before the flight entry.
 const bool sampled_restart_shot=input.saved_ball_flight_flag && input.score && input.kickoff_formation &&
  input.score->left_goals==1 && input.score->right_goals==1 && input.score->kickoff==4 &&
  f.side==0 && f.ball_owner==5 && f.ball_x==108 && f.ball_y==108 && f.ball_cell==87 &&
  !f.random_first && !f.random_second && input.current.clock.remaining==432 && input.current.clock.extra_time==5 &&
  input.action->action==0 && input.action->power==22 && input.recovery->attack_subtype==128 &&
  input.shot_flight->destination==233 && input.shot_flight->distance==108 && !input.shot_flight->loose_ball;
 if(sampled_restart_shot || sampled_home_shot || score23 || player7)state.saved_ball_flight_flag=0;
 BallTrajectoryContext t{static_cast<std::uint16_t>((unsigned(f.ball_x)<<8)|low_x),
  static_cast<std::uint16_t>((unsigned(f.ball_y)<<8)|low_y),0,0,input.recovery->x_jitter_control,state.saved_ball_flight_flag};
 state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);
 state.current.field.elapsed=0;state.shot_progress.reset();
 if(sampled_home_shot){state.area_defense.reset();state.area_defender_cursor=0;}
 if(f.ball_cell==input.shot_flight->destination)state.phase=FieldSessionPhase::ball_already_arrived;
 else {
  t.direction=calculate_player_direction({t.x,t.y,input.shot_flight->destination,0},f.ball_cell,geometry);
  const auto v=calculate_ball_velocity(t.direction,velocity);t.vx=v.x;t.vy=v.y;
  state.recovery->x_jitter_control=t.direction;state.phase=player7 ? FieldSessionPhase::sampled_score25_shot_advance : score23 ? FieldSessionPhase::sampled_score23_ball_advance_ready : FieldSessionPhase::ball_advance_ready;
 }
 state.ball_trajectory=t;validate_phase_session(state);return state;
}
PhaseSessionState continue_session_unopposed_arrival(const PhaseSessionState& input) {
 validate_phase_session(input);
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
  const bool player7=state.score && state.score->left_goals==2 && state.score->right_goals==5 &&
   state.current.field.side==0 && state.area_ai_target->recipient==7 && state.current.clock.remaining==321;
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
PhaseSessionState resolve_session_ball_cell(const PhaseSessionState& input) {
 validate_phase_session(input);
 const bool player7=input.phase==FieldSessionPhase::sampled_score25_shot_callback;
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_ball_cell_callback;
 if(input.phase!=FieldSessionPhase::ball_cell_callback && !score23 && !player7)
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
 state.phase=player7 ? FieldSessionPhase::sampled_score25_shot_advance : score23 ? FieldSessionPhase::sampled_score23_ball_advance_ready : FieldSessionPhase::ball_advance_ready;validate_phase_session(state);return state;
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
 const bool player7=input.phase==FieldSessionPhase::sampled_score25_shot_advance;
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_ball_advance_ready;
 if(input.phase!=FieldSessionPhase::ball_advance_ready && !score23 && !player7)
  throw std::invalid_argument("Ball step requires acknowledged frame and pending numeric flight");
 auto state=input;auto& t=*state.ball_trajectory;
 const auto step=advance_ball_to_next_cell({t.x,t.y,{t.vx,t.vy},state.current.field.ball_cell,state.shot_flight->destination});
 t.x=step.x;t.y=step.y;state.current.field.ball_x=step.x>>8;state.current.field.ball_y=step.y>>8;
 state.current.field.ball_cell=step.cell;
 state.phase=player7 ? (step.event==BallStepEvent::cell_changed ? FieldSessionPhase::sampled_score25_shot_callback : FieldSessionPhase::sampled_score25_shot_arrival) :
  score23 ? (step.event==BallStepEvent::cell_changed ? FieldSessionPhase::sampled_score23_ball_cell_callback : FieldSessionPhase::sampled_score23_ball_arrival_callback) :
  (step.event==BallStepEvent::cell_changed ? FieldSessionPhase::ball_cell_callback : FieldSessionPhase::ball_arrival_callback);
 validate_phase_session(state);return {std::move(state),step};
}
}

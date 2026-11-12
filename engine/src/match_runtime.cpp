#include "captain/match_runtime.hpp"
#include "captain/control_session.hpp"
#include "captain/ball_step.hpp"
#include "captain/defender_selection.hpp"
#include "captain/field_player_profile.hpp"
#include "captain/action_strength.hpp"
#include "captain/ball_recovery.hpp"
#include "captain/staged_shot.hpp"
#include "captain/selected_dispatch.hpp"
#include "captain/attack_clock.hpp"
#include "captain/approach_session.hpp"
#include "captain/area_ai_session.hpp"
#include "captain/ball_flight_session.hpp"
#include "captain/keeper_session.hpp"
#include "captain/goal_session.hpp"
#include "captain/keeper_save_session.hpp"
#include "captain/recovery_session.hpp"
#include "captain/kickoff_session.hpp"
#include "captain/match_clock.hpp"
#include <stdexcept>
namespace captain {
namespace {
void use_trajectory_fraction(MatchRuntimeState& state,const std::optional<BallTrajectoryContext>& trajectory) {
 if(trajectory)state.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(trajectory->x),static_cast<std::uint8_t>(trajectory->y)};
 else state.ball_fraction.reset();
}
void advance_demo_random(MatchRuntimeState& state) {
 auto random=state.demo_rng;random^=random<<13;random^=random>>17;random^=random<<5;
 state.demo_rng=random;state.session.current.field.random_first=static_cast<std::uint8_t>(random);
 state.session.current.field.random_second=static_cast<std::uint8_t>(random>>8);
}
MatchStepResult resolve_opponent_shot_defender(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
 const auto& field=input.session.current.field;const auto& data=*content.data;
 if(!data.flight || !data.contact || !data.experience || !input.session.experience || !input.session.scheduling || !input.session.action || !input.session.recovery || !input.session.ball_trajectory || !input.session.shot_flight){result.unsupported=UnsupportedCapability::content;return result;}
 if(field.side!=11 || !field.selected_defenders || field.selected_defenders>2 || field.elapsed || input.session.saved_ball_flight_flag || input.session.shot_flight->loose_ball || data.flight->defense_actions[0]!=0){result.unsupported=UnsupportedCapability::defender_contact;return result;}
 try {
  auto next=input;auto& session=next.session;const auto& rules=*data.flight;const auto variation=rules.variation[0];
  for(unsigned slot=0;slot<field.selected_defenders;++slot){
   const auto id=input.session.scheduling->selected_ids[slot];if(id<1 || id>10){result.unsupported=UnsupportedCapability::defender_contact;return result;}
   const auto& current=session.current.field;const auto& player=current.roster[id];const FieldPlayerRecord record{id,player.type,static_cast<std::uint8_t>(player.stamina),static_cast<std::uint8_t>(player.stamina>>8),player.level};
   const auto prepared=prepare_profiled_field_defender(record,data.field_players,0,0,0,data.defense);
   const auto contact=compare_defender_contact({session.action->power,prepared.base_strength,current.random_first,current.random_second,variation[0],variation[1],false,bool(rules.eligibility[0]&128u),0,id,0,rules.thresholds[0]});
   if(prepared.move_effect || contact.special || contact.subtype || contact.result_code>=2){result.unsupported=UnsupportedCapability::defender_contact;return result;}
   session.recovery->opposing_player=id;session.action->move_effect=prepared.move_effect;
   if(contact.result_code==1)session.action->power=reduce_action_strength(session.action->power,contact.loss);
   session.experience=award_player_experience(*session.experience,current.roster[current.ball_owner].type,1,*data.experience).state;
   const auto clock=apply_contact_prelude({session.current,id,0,true},*data.contact);
   if(clock.clock.continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
   session.current=clock.state;
  }
  session.current.field.selected_defenders=0;session.current.field.elapsed=0;session.scheduling->selected_ids.fill(0);
  session.phase=FieldSessionPhase::shot_keeper_entry;next.retained_defense_action=0;use_trajectory_fraction(next,session.ball_trajectory);
  next.last_sequence=event.sequence;validate_phase_session(session);project_field_random_memory(session.current.field,next.random_memory);result.state=std::move(next);return result;
 } catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
}
MatchStepResult resolve_pass_flight_contact(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};

    const auto& data=*content.data;const auto& field=input.session.current.field;
    const bool human=field.side==0;
    const auto trajectory=human ? input.pass_trajectory : input.defense_menu->ai_trajectory;
    const auto cell=human ? input.pass_cell : input.defense_menu->ai_ball_cell;
    if(!data.pass_flight_comparison || !data.contact || !data.experience || !input.session.experience || !input.session.action || !input.session.recovery || !trajectory || !cell){result.unsupported=UnsupportedCapability::content;return result;}
    const auto capacity=human ? 5u : 4u;
    if(input.flight_defenders.size()+input.processed_flight_defenders.size()>capacity){result.unsupported=UnsupportedCapability::phase;return result;}
    for(const auto prior:input.processed_flight_defenders)if((human && (prior<12 || prior>21)) || (!human && (prior<1 || prior>10))){result.unsupported=UnsupportedCapability::phase;return result;}
    for(const auto pending:input.flight_defenders)if((human && (pending<12 || pending>21)) || (!human && (pending<1 || pending>10))){result.unsupported=UnsupportedCapability::phase;return result;}
    if(field.elapsed!=input.processed_flight_defenders.size() || ((input.flight_defenders.size()>1 || !input.processed_flight_defenders.empty()) && data.contact->field_elapsed[5]!=1)){result.unsupported=UnsupportedCapability::defender_contact;return result;}
    const auto id=input.flight_defenders.front();if((human && (id<12 || id>21 || field.ball_owner<1 || field.ball_owner>10)) || (!human && (id<1 || id>10 || field.ball_owner<12 || field.ball_owner>21))){result.unsupported=UnsupportedCapability::defender_contact;return result;}
    try {
     const auto& player=field.roster[id];const FieldPlayerRecord record{id,player.type,static_cast<std::uint8_t>(player.stamina),static_cast<std::uint8_t>(player.stamina>>8),player.level};
     const auto prep=prepare_profiled_field_defender(record,data.field_players,2,0,0,data.defense);const auto& rules=*data.pass_flight_comparison;
     const auto contact=compare_defender_contact({input.session.action->power,prep.base_strength,field.random_first,field.random_second,rules.range,rules.offset,false,bool(rules.eligibility&128u),0,id,0,rules.thresholds});
     if(prep.move_effect){result.unsupported=UnsupportedCapability::defender_contact;return result;}
     if(contact.result_code==2){
      if(!data.movement){result.unsupported=UnsupportedCapability::content;return result;}
      if(field.elapsed!=input.processed_flight_defenders.size() || data.contact->field_elapsed[5]!=1 || !input.session.scheduling){result.unsupported=UnsupportedCapability::defender_contact;return result;}
      if((input.session.recovery->special_owner_flags&128u) && field.ball_owner==20){result.unsupported=UnsupportedCapability::defender_contact;return result;}
      auto next=input;auto& session=next.session;const auto flight=*trajectory;
      const auto& defender=field.motion[field_motion_index(id)];session.current.field.ball_x=static_cast<std::uint8_t>(defender.x>>8);session.current.field.ball_y=static_cast<std::uint8_t>(defender.y>>8);
      session.current.field.ball_cell=*cell;session.recovery->opposing_player=id;
      for(unsigned i=0;i<2;++i)session.experience=award_player_experience(*session.experience,player.type,2,*data.experience).state;
      auto clock=apply_contact_prelude({session.current,id,2,false},*data.contact);
      if(clock.clock.continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
      session.current=clock.state;session.current.field.selected_defenders=0;session.scheduling->selected_ids.fill(0);
      const auto ball=jitter_loose_ball(session.current.field.ball_x,session.current.field.ball_y,session.current.field.random_first,session.current.field.random_second);
      if(!ball.inside_field){result.unsupported=UnsupportedCapability::defender_contact;return result;}
      session.current.field.ball_x=ball.x;session.current.field.ball_y=ball.y;session.recovery->x_jitter_control=ball.x_jitter_control;
      session.current.field=advance_field_phase(session.current.field,data.field_players,*data.movement).state;
      const auto& moved=session.current.field;BallRecoveryInput recovery{};
      recovery.ball_x=moved.ball_x;recovery.ball_y=moved.ball_y;recovery.ball_owner=moved.ball_owner;recovery.opposing_player=id;
      recovery.random_e2=moved.random_first;recovery.random_e3=moved.random_second;recovery.ball_update_active=moved.elapsed!=0;
      for(unsigned i=0;i<moved.motion.size();++i){const auto& p=moved.motion[i];recovery.players[i]={static_cast<std::uint8_t>(p.x>>8),static_cast<std::uint8_t>(p.y>>8),p.status!=0};}
      const auto recovered=recover_stationary_ball(recovery);
      if((ball.y>=96 && ball.y<160 && (ball.x<80 || ball.x>=176)) || (session.recovery->game_mode==4 && recovered.possession_side==0 && moved.roster[recovered.ball_owner].type!=1 && ball.x>=128)) {result.unsupported=UnsupportedCapability::defender_contact;return result;}
      session.current.field.side=recovered.possession_side;session.current.field.ball_owner=recovered.ball_owner;session.recovery->attack_subtype=recovered.attack_subtype;
      auto& owner=session.current.field.motion[field_motion_index(recovered.ball_owner)];owner.x=static_cast<std::uint16_t>((owner.x&255u)|(unsigned(ball.x)<<8));owner.y=static_cast<std::uint16_t>((owner.y&255u)|(unsigned(ball.y)<<8));
      session.phase=FieldSessionPhase::player_control;next.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(flight.x),static_cast<std::uint8_t>(flight.y)};
      if(human){session.human_pass_cursor.reset();session.human_pass_steps=0;session.ordinary_pass_candidates.reset();session.ordinary_pass_recipient.reset();session.ordinary_pass_defenses.reset();next.pass_target.reset();next.pass_trajectory.reset();next.pass_cell.reset();next.pass_stage=MatchPassStage::none;}else next.defense_menu.reset();
      next.flight_defenders.clear();next.processed_flight_defenders.clear();next.last_sequence=event.sequence;validate_phase_session(session);
      project_field_random_memory(session.current.field,next.random_memory);result.state=std::move(next);return result;
     }
     if(contact.result_code==3){
      if(!data.movement){result.unsupported=UnsupportedCapability::content;return result;}
      if(field.elapsed!=input.processed_flight_defenders.size() || data.contact->field_elapsed[5]!=1 || !input.session.scheduling){result.unsupported=UnsupportedCapability::defender_contact;return result;}
      auto next=input;auto& session=next.session;const auto flight=*trajectory;
      session.current.field.ball_x=static_cast<std::uint8_t>(flight.x>>8);session.current.field.ball_y=static_cast<std::uint8_t>(flight.y>>8);session.current.field.ball_cell=*cell;
      session.current.field.selected_defenders=static_cast<std::uint8_t>(next.processed_flight_defenders.size()+next.flight_defenders.size());session.scheduling->selected_ids.fill(0);
      unsigned slot=0;for(const auto prior:next.processed_flight_defenders)session.scheduling->selected_ids[slot++]=prior;
      for(const auto pending:next.flight_defenders)session.scheduling->selected_ids[slot++]=pending;
      session.recovery->opposing_player=id;session.experience=award_player_experience(*session.experience,player.type,2,*data.experience).state;
      auto clock=apply_contact_prelude({session.current,id,2,false},*data.contact);
      if(clock.clock.continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
      session.current=clock.state;session.current.field.side=id<11 ? 0 : 11;session.current.field.ball_owner=id;
      session.current.field=advance_field_phase(session.current.field,data.field_players,*data.movement).state;
      session.phase=FieldSessionPhase::player_control;next.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(flight.x),static_cast<std::uint8_t>(flight.y)};
      if(human){session.human_pass_cursor.reset();session.human_pass_steps=0;session.ordinary_pass_candidates.reset();session.ordinary_pass_recipient.reset();session.ordinary_pass_defenses.reset();next.pass_target.reset();next.pass_trajectory.reset();next.pass_cell.reset();next.pass_stage=MatchPassStage::none;}else next.defense_menu.reset();
      next.flight_defenders.clear();next.processed_flight_defenders.clear();next.last_sequence=event.sequence;validate_phase_session(session);
      project_field_random_memory(session.current.field,next.random_memory);result.state=std::move(next);return result;
     }
     auto next=input;auto& session=next.session;const auto& flight=*trajectory;
     if(!human){session.current.field.ball_x=static_cast<std::uint8_t>(flight.x>>8);session.current.field.ball_y=static_cast<std::uint8_t>(flight.y>>8);session.current.field.ball_cell=*cell;}
     session.recovery->opposing_player=id;
     if(contact.result_code==1)session.action->power=reduce_action_strength(session.action->power,contact.loss);
     session.experience=award_player_experience(*session.experience,field.roster[field.ball_owner].type,1,*data.experience).state;
     auto clock=apply_contact_prelude({session.current,id,2,true},*data.contact);
     if(clock.clock.continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
     session.current=clock.state;next.processed_flight_defenders.push_back(id);next.flight_defenders.erase(next.flight_defenders.begin());
     if(next.flight_defenders.empty()){session.current.field.elapsed=0;next.processed_flight_defenders.clear();}
     next.last_sequence=event.sequence;project_field_random_memory(session.current.field,next.random_memory);result.state=std::move(next);return result;
    } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::defender_contact;return result;}

}
MatchStepResult resolve_free_pass_arrival(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};

   if(!content.data->pass_race || !content.data->movement || !input.session.recovery){result.unsupported=UnsupportedCapability::content;return result;}
   try {
    auto next=input;auto& session=next.session;const auto trajectory=*next.pass_trajectory;
    session.current.field.ball_x=static_cast<std::uint8_t>(trajectory.x>>8);session.current.field.ball_y=static_cast<std::uint8_t>(trajectory.y>>8);session.current.field.ball_cell=*next.pass_cell;
    auto race=resolve_field_loose_ball_race(session.current.field,session.recovery->opposing_player,next.pass_target->distance,content.data->field_players,*content.data->movement,*content.data->pass_race);
    if((race.winner && (race.rounds>5 || *race.winner<1 || *race.winner==11 || *race.winner>21)) || (!race.winner && race.rounds>5)){result.unsupported=UnsupportedCapability::action;return result;}
    session.current.field=std::move(race.field);auto& field=session.current.field;std::uint8_t winner=0;
    if(race.winner)winner=*race.winner;
    else {
     field=advance_field_phase(field,content.data->field_players,*content.data->movement).state;
     BallRecoveryInput recovery{};recovery.ball_x=field.ball_x;recovery.ball_y=field.ball_y;recovery.ball_owner=field.ball_owner;
     recovery.opposing_player=session.recovery->opposing_player;recovery.random_e2=field.random_first;recovery.random_e3=field.random_second;recovery.ball_update_active=field.elapsed!=0;
     for(unsigned i=0;i<field.motion.size();++i){const auto& p=field.motion[i];recovery.players[i]={static_cast<std::uint8_t>(p.x>>8),static_cast<std::uint8_t>(p.y>>8),p.status!=0};}
     const auto recovered=recover_stationary_ball(recovery);winner=recovered.ball_owner;session.recovery->attack_subtype=recovered.attack_subtype;
    }
    field.ball_owner=winner;field.side=winner<11 ? 0 : 11;
    if(!race.winner || winner>=11)next.retained_defense_action.reset();
    auto& player=field.motion[field_motion_index(winner)];player.x=static_cast<std::uint16_t>((player.x&255u)|(unsigned(field.ball_x)<<8));player.y=static_cast<std::uint16_t>((player.y&255u)|(unsigned(field.ball_y)<<8));
    if((field.ball_y>=96 && field.ball_y<160 && (field.ball_x<80 || field.ball_x>=176)) || (session.recovery->game_mode==4 && field.side==0 && field.roster[field.ball_owner].type!=1 && field.ball_x>=128)){result.unsupported=UnsupportedCapability::action;return result;}
    session.phase=FieldSessionPhase::player_control;session.human_pass_cursor.reset();session.human_pass_steps=0;
    session.ordinary_pass_candidates.reset();session.ordinary_pass_recipient.reset();session.ordinary_pass_defenses.reset();
    next.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(trajectory.x),static_cast<std::uint8_t>(trajectory.y)};
    next.pass_target.reset();next.pass_trajectory.reset();next.pass_cell.reset();next.pass_stage=MatchPassStage::none;
    next.last_sequence=event.sequence;validate_phase_session(session);project_field_random_memory(field,next.random_memory);result.state=std::move(next);return result;
   }catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::action;return result;}

}
MatchStepResult prepare_human_shot(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(input.session.current.field.selected_defenders){result.unsupported=UnsupportedCapability::defender_contact;return result;}
 if(content.data->field_players.skills.empty() || content.data->attack.moves[0].empty()){result.unsupported=UnsupportedCapability::content;return result;}
 try {
  result.state.session=prepare_session_basic_shot(input.session,content.data->field_players,content.data->attack).state;
  result.state.last_sequence=event.sequence;project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult dispatch_human_shot(const MatchRuntimeState& input,const MatchInput& event) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
 if(!input.session.control){result.unsupported=UnsupportedCapability::phase;return result;}
 try {
  auto next=input;next.session.control->facing=0;next.session.control->motion_requested=2;
  if(next.session.current.field.side==11){next.session.current.clock.ball_mode=0;next.session.current.clock.script_flags=0;}
  const auto dispatch=dispatch_session_shot(next.session,1);
  if(!dispatch.clock || dispatch.clock->continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
  next.session=dispatch.state;next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult move_human_shot_field(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
 if(!content.data->movement || !content.data->experience){result.unsupported=UnsupportedCapability::content;return result;}
 try {
  result.state.session=resolve_session_selected_result(input.session,content.data->field_players,*content.data->movement,*content.data->experience).state;
  result.state.last_sequence=event.sequence;project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult continue_human_shot_attack(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
 if(!input.retained_defense_action || *input.retained_defense_action>6){result.unsupported=UnsupportedCapability::defense_action_state;return result;}
 if(!content.data->contact){result.unsupported=UnsupportedCapability::content;return result;}
 try {
  const auto continuation=continue_prepared_session_attack(input.session,*content.data->contact,*input.retained_defense_action);
  if(!continuation.clock || continuation.clock->continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
  result.state.session=continuation.state;
  if(result.state.session.current.field.side==11){result.state.session.recovery->ball_variant=static_cast<std::uint8_t>((result.state.session.current.field.random_first&1)+1);result.state.session.control->last_carrier_cell=128;result.state.session.control->display_parameter=0;result.state.session.control->motion_requested=2;}
  result.state.retained_defense_action.reset();result.state.last_sequence=event.sequence;
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult approach_human_shot(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
 if(!content.data->approach){result.unsupported=UnsupportedCapability::content;return result;}
 try {
  result.state.session=prepare_session_shot_approach(input.session,*content.data->approach).state;
  result.state.last_sequence=event.sequence;project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult begin_human_shot_flight(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
 if(!input.ball_fraction){result.unsupported=UnsupportedCapability::ball_fraction;return result;}
 if(!content.data->movement || !content.ball_velocity){result.unsupported=UnsupportedCapability::content;return result;}
 try {
  auto session=input.session;if(session.current.field.side==11)session.saved_ball_flight_flag=0;
  result.state.session=begin_prepared_session_ball_flight(session,input.ball_fraction->x,input.ball_fraction->y,content.data->movement->geometry,*content.ball_velocity);
  result.state.last_sequence=event.sequence;project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult advance_human_shot_flight(const MatchRuntimeState& input,const MatchInput& event) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
 try {
  result.state.session=input.session.phase==FieldSessionPhase::ball_cell_callback ? resolve_session_ball_cell(input.session) : advance_prepared_session_ball_cell(input.session).state;
  use_trajectory_fraction(result.state,result.state.session.ball_trajectory);
  result.state.last_sequence=event.sequence;project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult enter_human_shot_keeper(const MatchRuntimeState& input,const MatchInput& event) {
 MatchStepResult result{input};
 if(input.session.phase==FieldSessionPhase::ball_arrival_callback && event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
 if(input.session.phase==FieldSessionPhase::shot_keeper_entry && event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
 if(input.session.current.field.selected_defenders){result.unsupported=UnsupportedCapability::defender_contact;return result;}
 if(!input.session.shot_flight || input.session.shot_flight->loose_ball || input.session.current.field.keepers[input.session.current.field.side==0 ? 1 : 0].stun){result.unsupported=UnsupportedCapability::action;return result;}
 try {
  result.state.session=input.session.phase==FieldSessionPhase::ball_arrival_callback ? continue_session_unopposed_arrival(input.session) : prepare_session_keeper_entry(input.session);
  use_trajectory_fraction(result.state,result.state.session.ball_trajectory);
  result.state.last_sequence=event.sequence;project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult resolve_opponent_keeper_ai(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
 if(!content.data->ai_keeper_choice){result.unsupported=UnsupportedCapability::content;return result;}
 if(!input.session.action || input.session.action->menu_route || input.session.current.field.roster[11].type!=0){result.unsupported=UnsupportedCapability::opponent_action;return result;}
 try {
  const auto& field=input.session.current.field;const auto& player=field.roster[11];
  const FieldPlayerRecord record{11,player.type,static_cast<std::uint8_t>(player.stamina),static_cast<std::uint8_t>(player.stamina>>8),player.level};
  const auto& ai=*content.data->ai_keeper_choice;const auto choice=choose_opponent_keeper_action(record,content.data->goalkeepers,input.session.action->menu_route,field.random_first,ai);
  result.state.session=resolve_prepared_session_keeper(input.session,choice.action,choice.subtype,ai.team_id,*content.data);
  use_trajectory_fraction(result.state,result.state.session.ball_trajectory);
  result.state.retained_defense_action=choice.action;
  result.state.last_sequence=event.sequence;project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::opponent_action;return result;}
}
MatchStepResult score_human_shot(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
 if(!input.session.keeper_decision){result.unsupported=UnsupportedCapability::phase;return result;}
 if(!content.data->experience){result.unsupported=UnsupportedCapability::content;return result;}
 try {
  if(input.session.keeper_decision->result_code==0){
   if(!input.session.score){result.unsupported=UnsupportedCapability::opponent_execution;return result;}
   result.state.session=prepare_session_scored_goal(input.session,*input.session.score,*content.data);
   use_trajectory_fraction(result.state,result.state.session.ball_trajectory);
   result.state.home_keeper_y.reset();
  } else if(input.session.keeper_decision->result_code==2 || input.session.keeper_decision->result_code==3){result.state.session=resolve_session_keeper_save(input.session,*content.data).state;result.state.retained_defense_action.reset();if(result.state.session.phase!=FieldSessionPhase::keeper_possession || result.state.session.current.field.side!=0)result.state.home_keeper_y.reset();}
  else {result.unsupported=UnsupportedCapability::opponent_execution;return result;}
  result.state.last_sequence=event.sequence;project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::opponent_execution;return result;}
}
MatchStepResult restart_after_human_goal(const MatchRuntimeState& input,const MatchInput& event) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
 try {
  result.state.session=continue_prepared_goal_restart(input.session);use_trajectory_fraction(result.state,input.session.ball_trajectory);result.state.last_sequence=event.sequence;
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult recover_after_keeper_save(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
 if(!content.data->movement){result.unsupported=UnsupportedCapability::content;return result;}
 try {
  if(input.session.phase==FieldSessionPhase::keeper_punch_recovery){
   if(!result.state.session.control){result.unsupported=UnsupportedCapability::phase;return result;}
   result.state.session.control->motion_requested=0;
   result.state.session=advance_session_keeper_punch_recovery(result.state.session,content.data->field_players,*content.data->movement).state;
  } else result.state.session=advance_session_recovery(input.session,content.data->field_players,*content.data->movement).state;
  result.state.ball_fraction.reset();result.state.last_sequence=event.sequence;project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.state=input;result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult continue_keeper_save_recovery(const MatchRuntimeState& input,const MatchInput& event) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
 try {
  result.state.session=continue_session_recovery(input.session);result.state.last_sequence=event.sequence;
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult prepare_keeper_save_area(const MatchRuntimeState& input,const MatchInput& event) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
 try {
  result.state.session=prepare_session_keeper_area(input.session);result.state.last_sequence=event.sequence;
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::action;return result;}
}
MatchStepResult advance_goal_kickoff(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};const auto phase=input.session.phase;
 const auto expected=phase==FieldSessionPhase::kickoff_ball_placed ? MatchInputKind::presentation_complete : MatchInputKind::tick;
 if(event.kind!=expected){result.unsupported=expected==MatchInputKind::presentation_complete ? UnsupportedCapability::presentation : UnsupportedCapability::action;return result;}
 if((phase==FieldSessionPhase::goal_kickoff_pending || phase==FieldSessionPhase::kickoff_formation_placed) && !content.data->kickoff){result.unsupported=UnsupportedCapability::content;return result;}
 try {
  if(phase==FieldSessionPhase::goal_kickoff_pending){
   for(auto& player:result.state.session.current.field.motion)player.status=0;
   for(auto& keeper:result.state.session.current.field.keepers){keeper.penalty=0;keeper.stun=0;}
   result.state.session=prepare_session_kickoff_formation(result.state.session,input.session.current.field.side,content.data->kickoff->formation,true);
  } else if(phase==FieldSessionPhase::kickoff_formation_placed)
   result.state.session=prepare_session_kickoff_receiver(input.session,input.session.current.field.random_first,content.data->kickoff->receivers);
  else if(phase==FieldSessionPhase::kickoff_receiver_selected)result.state.session=prepare_session_kickoff_ball(input.session);
  else if(phase==FieldSessionPhase::kickoff_ball_placed)result.state.session=continue_prepared_session_kickoff(input.session);
  else {auto prepared=prepare_session_control(input.session);result.state.session=prepared.state;for(const auto& request:prepared.presentation)result.events.push_back({request});}
  result.state.last_sequence=event.sequence;project_field_random_memory(result.state.session.current.field,result.state.random_memory);return result;
 } catch(const std::invalid_argument&) {result.state=input;result.unsupported=UnsupportedCapability::action;return result;}
}
}
const char* capability_name(UnsupportedCapability c) {
 switch(c) {case UnsupportedCapability::content:return "content";case UnsupportedCapability::initialization:return "match initialization";case UnsupportedCapability::input_sequence:return "input sequence";case UnsupportedCapability::ball_fraction:return "ball fractional position is unknown";case UnsupportedCapability::random_state:return "random index is not present in development snapshot";case UnsupportedCapability::timing:return "live timing and random scheduling";case UnsupportedCapability::action:return "action routing";case UnsupportedCapability::defender_contact:return "pass-flight defender contact";case UnsupportedCapability::defensive_prompt:return "opponent contact defensive prompt";case UnsupportedCapability::opponent_action:return "opponent attack choice";case UnsupportedCapability::opponent_execution:return "opponent attack execution and defensive resolution";case UnsupportedCapability::period_transition:return "period or added-time transition";case UnsupportedCapability::presentation:return "presentation completion";case UnsupportedCapability::phase:return "session phase";case UnsupportedCapability::defense_action_state:return "retained defensive action is unknown";case UnsupportedCapability::keeper_position:return "home goalkeeper position is unknown";case UnsupportedCapability::keeper_outlet:return "home goalkeeper automatic outlet is not implemented";} return "unknown";
}
MatchBallView match_ball_view(const MatchRuntimeState& state) {
 if((state.session.phase==FieldSessionPhase::ball_advance_ready || state.session.phase==FieldSessionPhase::ball_cell_callback || state.session.phase==FieldSessionPhase::ball_arrival_callback || state.session.phase==FieldSessionPhase::ball_already_arrived ||
     state.session.phase==FieldSessionPhase::ordinary_pass_flight_ready || state.session.phase==FieldSessionPhase::ordinary_pass_flight_callback || state.session.phase==FieldSessionPhase::ordinary_pass_flight_arrival) && state.session.ball_trajectory){
  const auto& t=*state.session.ball_trajectory;return {static_cast<std::uint8_t>(t.x>>8),static_cast<std::uint8_t>(t.y>>8),state.session.current.field.ball_cell,true};
 }
 if(state.defense_menu && static_cast<unsigned>(state.defense_menu->stage)>=static_cast<unsigned>(MatchDefenseStage::ai_flight_ready)){
  const auto& m=*state.defense_menu;if(!m.ai_trajectory || !m.ai_ball_cell)throw std::invalid_argument("Missing opponent flight ball context");
  return {static_cast<std::uint8_t>(m.ai_trajectory->x>>8),static_cast<std::uint8_t>(m.ai_trajectory->y>>8),*m.ai_ball_cell,true};
 }
 const bool flight=state.pass_stage==MatchPassStage::flight_ready || state.pass_stage==MatchPassStage::flight_cell || state.pass_stage==MatchPassStage::flight_arrived;
 if(flight){
  if(!state.pass_trajectory || !state.pass_cell)throw std::invalid_argument("Missing runtime flight ball context");
  return {static_cast<std::uint8_t>(state.pass_trajectory->x>>8),static_cast<std::uint8_t>(state.pass_trajectory->y>>8),*state.pass_cell,true};
 }
 const auto& field=state.session.current.field;return {field.ball_x,field.ball_y,field.ball_cell,false};
}
MatchStepResult create_match(const MatchConfig& config,const MatchContent& content) {
 MatchStepResult result{};
 if(!content.data){result.unsupported=UnsupportedCapability::content;return result;}
 if(!config.initial || config.clock || (config.mode!=MatchTimingMode::original_backed && config.mode!=MatchTimingMode::provisional_demo) || (config.mode==MatchTimingMode::original_backed && config.seed) || (config.mode==MatchTimingMode::provisional_demo && !config.seed)){result.unsupported=UnsupportedCapability::initialization;return result;}
 validate_phase_session(*config.initial);
 if(config.home_keeper_y && (config.mode!=MatchTimingMode::provisional_demo || config.initial->phase!=FieldSessionPhase::shot_keeper_entry || config.initial->current.field.side!=11)){result.unsupported=UnsupportedCapability::initialization;return result;}
 if(config.retained_defense_action){const auto& session=*config.initial;
  const bool ready=session.phase==FieldSessionPhase::control_ready && !session.current.field.selected_defenders;
  const bool shot=session.current.field.side==0 && !session.current.field.selected_defenders && session.action && session.action->action==0 &&
     (session.phase==FieldSessionPhase::attack_choice_pending || session.phase==FieldSessionPhase::attack_prepared || session.phase==FieldSessionPhase::shot_dispatch_ready || session.phase==FieldSessionPhase::selected_attack_presentation);
  if(config.mode!=MatchTimingMode::provisional_demo || *config.retained_defense_action>6 || !(ready || shot)){
   result.unsupported=UnsupportedCapability::initialization;return result;
  }
 }
 result.state.session=*config.initial;
 result.state.random_index=config.random_index;
 result.state.mode=config.mode;result.state.demo_rng=config.seed;result.state.ball_fraction=config.ball_fraction;result.state.retained_defense_action=config.retained_defense_action;result.state.home_keeper_y=config.home_keeper_y;
 project_field_random_memory(result.state.session.current.field,result.state.random_memory);
 return result;
}
MatchStepResult create_first_match(std::uint32_t seed,const MatchContent& content) {
 MatchStepResult result{};
 if(!seed){result.unsupported=UnsupportedCapability::initialization;return result;}
 if(!content.data || !content.data->first_match_lineup || !content.data->world_step || !content.data->kickoff ||
    !content.data->ai_keeper_choice || content.data->ai_keeper_choice->team_id!=3){result.unsupported=UnsupportedCapability::content;return result;}
 try {
  auto& state=result.state;auto& s=state.session;auto& field=s.current.field;
  field.ball_owner=0;field.ball_x=0;field.ball_y=0;field.ball_cell=0;
  field=initialize_first_match_roster(field,*content.data->first_match_lineup,content.data->field_players,content.data->goalkeepers,*content.data->world_step);
  s.current.clock.remaining=180;s.phase=FieldSessionPhase::goal_kickoff_pending;
  s.experience=ExperienceState{};
  // Original match display initialization writes $0469=1; this byte is
  // retained in the current snapshot model as the high byte of slot10.
  s.experience->totals[10]=256;
  s.recovery=RecoveryContext{};s.control=ControlContext{};s.scheduling=SchedulingContext{};
  s.action=ActionContext{};s.score=MatchScoreContext{};
  state.mode=MatchTimingMode::provisional_demo;state.demo_rng=seed;state.ball_fraction=MatchBallFraction{0,0};
  advance_demo_random(state);project_field_random_memory(s.current.field,state.random_memory);validate_phase_session(s);
  return result;
 }catch(const std::invalid_argument&){result=MatchStepResult{};result.unsupported=UnsupportedCapability::content;return result;}
}
namespace {
MatchStepResult handle_human_keeper_menu(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(!input.session.action || !input.session.control || !input.session.recovery){result.unsupported=UnsupportedCapability::phase;return result;}
 if(!content.data->action_menu || !content.data->action_menu->keeper_directions || !content.data->action_menu->keeper_icons){result.unsupported=UnsupportedCapability::content;return result;}
 if(input.session.action->menu_route==0){
  if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
  result.state.session.action->menu_route=4;result.state.session.action->cursor=0;result.state.session.action->auxiliary=0;
  result.state.session.control->motion_requested=0;result.state.session.control->display_parameter=0;result.state.last_sequence=event.sequence;return result;
 }
 if(input.session.action->menu_route!=4 || event.kind!=MatchInputKind::tick || !input.demo_rng){result.unsupported=UnsupportedCapability::phase;return result;}
 auto next=input;advance_demo_random(next);auto& menu=*next.session.action;const auto directions=event.pressed&15u;
 if(directions){unsigned direction=0;while(!(directions&(1u<<direction)))++direction;const auto action=(*content.data->action_menu->keeper_directions)[direction];
  if(action==2 && next.session.current.field.roster[0].type!=34){}
  else if(action==0 || action==1){next.retained_defense_action=action;menu.cursor=static_cast<std::uint8_t>(((menu.cursor&3u)==direction ? menu.cursor : direction)|128u);}
  else if(action!=255){result.unsupported=UnsupportedCapability::action;return result;}
 }
 if((event.pressed&128u) && (menu.cursor&128u)){
  if(!next.retained_defense_action || *next.retained_defense_action>1 || !content.data->ai_keeper_choice){result.unsupported=UnsupportedCapability::content;return result;}
  try{next.session=resolve_prepared_session_keeper(next.session,*next.retained_defense_action,0,content.data->ai_keeper_choice->team_id,*content.data);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::opponent_execution;return result;}
 } else if(menu.cursor&128u){
  if(!(menu.cursor&32u)){menu.cursor|=32u;menu.flash_counter=0;}
  if(menu.flash_counter)--menu.flash_counter;else {menu.flash_counter=13;menu.cursor^=64u;}
 }
 next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
}
MatchStepResult handle_contested_human_pass(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
 if(!input.session.current.field.selected_defenders || !input.session.scheduling || !input.session.recovery){result.unsupported=UnsupportedCapability::defender_contact;return result;}
 if(input.session.phase==FieldSessionPhase::ordinary_human_pass_confirmed){
  result.state.session=begin_ordinary_pass_defense_selection(input.session);result.state.last_sequence=event.sequence;return result;
 }
 if(input.session.phase==FieldSessionPhase::ordinary_pass_defense_selection){
  if(!content.data->ai_pass_defense){result.unsupported=UnsupportedCapability::content;return result;}
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  auto next=input;advance_demo_random(next);const auto id=next.session.scheduling->selected_ids[next.session.ordinary_pass_defenses->count];const auto& actor=next.session.current.field.roster[id];
  std::array<std::uint8_t,10> home_types{};for(unsigned i=0;i<10;++i)home_types[i]=next.session.current.field.roster[i+1].type;
  try {const auto choice=actor.type==0 ? choose_generic_ai_pass_defense(id,static_cast<std::uint8_t>(actor.stamina),static_cast<std::uint8_t>(actor.stamina>>8),
    next.session.current.field.ball_x,next.session.current.field.ball_y,next.session.current.field.random_first,next.session.recovery->ball_variant,home_types,content.data->field_players,*content.data->ai_pass_defense) :
    choose_ordinary_ai_pass_defense(actor.type,static_cast<std::uint8_t>(actor.stamina),static_cast<std::uint8_t>(actor.stamina>>8),
    next.session.current.field.ball_x,next.session.current.field.ball_y,next.session.current.field.random_first,next.session.recovery->ball_variant,home_types,*content.data->ai_pass_defense);
   next.session=record_ordinary_pass_defense(next.session,id,choice,0);
  }catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.session.phase==FieldSessionPhase::ordinary_pass_defenses_recorded){
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(content.data->field_players.skills.empty() || content.data->attack.moves[1].empty()){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  try{next.session=prepare_ordinary_pass(next.session,content.data->field_players,content.data->attack);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.pass_stage=MatchPassStage::awaiting_presentation;next.last_sequence=event.sequence;
  project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 result.unsupported=UnsupportedCapability::phase;return result;
}
MatchStepResult handle_contested_pass_progress(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(input.mode==MatchTimingMode::provisional_demo && (input.session.phase==FieldSessionPhase::ordinary_pass_defender_prepared || input.session.phase==FieldSessionPhase::ordinary_pass_next_defender_prepared)) {
  if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
  if(!input.session.area_contact || input.session.area_contact->effect){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  const auto defense_action=input.session.area_contact->defense_action;
  const auto& comparison=defense_action==0 ? content.data->ordinary_pass_block_comparison : defense_action==1 ? content.data->ordinary_pass_defender_comparison : content.data->ordinary_pass_intercept_comparison;
  if(!comparison){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;
  try {next.session=input.session.phase==FieldSessionPhase::ordinary_pass_defender_prepared ? compare_first_ordinary_pass_defender(next.session,*comparison) : compare_next_ordinary_pass_defender(next.session,*comparison);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && (input.session.phase==FieldSessionPhase::ordinary_pass_defender_compared || input.session.phase==FieldSessionPhase::ordinary_pass_next_defender_compared)) {
  if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
  if(!content.data->contact || !content.data->experience){result.unsupported=UnsupportedCapability::content;return result;}
  if(input.session.area_sequence_result==3) {
   if(!content.data->movement || !input.session.recovery || !input.session.experience || !input.session.scheduling){result.unsupported=UnsupportedCapability::content;return result;}
   const auto defender=input.session.recovery->opposing_player;
   if(defender<12 || defender>21 || input.session.current.field.selected_defenders!=1){result.unsupported=UnsupportedCapability::defender_contact;return result;}
   auto next=input;auto& session=next.session;
   try {
    session.experience=award_player_experience(*session.experience,session.current.field.roster[defender].type,2,*content.data->experience).state;
    const auto contact=apply_contact_prelude({session.current,defender,session.area_contact->defense_action,false},*content.data->contact);
    if(contact.clock.continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
    session.current=contact.state;
    session.current.field=advance_field_phase(session.current.field,content.data->field_players,*content.data->movement).state;
    session.current.field.ball_owner=defender;session.current.field.side=11;
    session.human_pass_cursor.reset();session.human_pass_steps=0;session.ordinary_pass_candidates.reset();
    session.ordinary_pass_recipient.reset();session.ordinary_pass_defenses.reset();session.area_contact.reset();
    session.area_defender_cursor=0;session.area_sequence_result=255;session.phase=FieldSessionPhase::player_control;
    validate_phase_session(session);
   }catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
   next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
  }
  auto next=input;
  try {next.session=input.session.phase==FieldSessionPhase::ordinary_pass_defender_compared ? advance_first_ordinary_pass_contact(next.session,*content.data->contact,*content.data->experience) : advance_next_ordinary_pass_contact(next.session,*content.data->contact,*content.data->experience);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && (input.session.phase==FieldSessionPhase::ordinary_pass_first_contact_clock || input.session.phase==FieldSessionPhase::ordinary_pass_next_contact_clock)) {
  if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
  auto next=input;
  try {next.session=unsigned(input.session.area_defender_cursor)+1<input.session.ordinary_pass_defenses->count ? prepare_next_ordinary_pass_defender(next.session,content.data->field_players,content.data->defense) : advance_ordinary_pass_dispatch_clock(next.session);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_dispatch_clock) {
  if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
  if(!content.data->movement){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;
  try {next.session.current.field=advance_field_phase(next.session.current.field,content.data->field_players,*content.data->movement).state;
       next.session.phase=FieldSessionPhase::ordinary_pass_field_moved;validate_phase_session(next.session);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_field_moved) {
  if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
  if(!content.pass_distance){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;
  try {next.session=resolve_ordinary_pass_flight_target(next.session,*content.pass_distance);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_flight_target) {
  if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
  if(!content.data->movement || !content.ball_velocity){result.unsupported=UnsupportedCapability::content;return result;}
  if(!input.ball_fraction){result.unsupported=UnsupportedCapability::ball_fraction;return result;}
  auto next=input;
  try {next.session=begin_ordinary_pass_flight(next.session,input.ball_fraction->x,input.ball_fraction->y,content.data->movement->geometry,*content.ball_velocity);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && (input.session.phase==FieldSessionPhase::ordinary_pass_flight_ready || input.session.phase==FieldSessionPhase::ordinary_pass_flight_callback)) {
  if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
  auto next=input;
  try {
   const auto& session=input.session;
   if(session.phase==FieldSessionPhase::ordinary_pass_flight_ready && session.area_ai_target && session.area_ai_target->recipient==255 && session.ball_trajectory && session.shot_flight) {
    const auto& t=*session.ball_trajectory;
    const auto step=advance_ball_to_next_cell({t.x,t.y,{t.vx,t.vy},session.current.field.ball_cell,session.shot_flight->destination});
    if(step.event!=BallStepEvent::cell_changed && !session.current.field.selected_defenders) {
     if(!content.data->movement || !content.data->pass_race){result.unsupported=UnsupportedCapability::content;return result;}
     next.session=resolve_ordinary_free_pass_contest(session,content.data->field_players,*content.data->movement,*content.data->pass_race);
    } else next.session=advance_ordinary_pass_ball_cell(session).state;
   } else next.session=session.phase==FieldSessionPhase::ordinary_pass_flight_ready ? advance_ordinary_pass_ball_cell(session).state : resolve_ordinary_pass_ball_cell(session);
  }
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && (input.session.phase==FieldSessionPhase::ordinary_pass_flight_arrival || input.session.phase==FieldSessionPhase::ordinary_pass_received)) {
  const auto arrival=input.session.phase==FieldSessionPhase::ordinary_pass_flight_arrival;
  if(event.kind!=(arrival ? MatchInputKind::presentation_complete : MatchInputKind::tick)){result.unsupported=UnsupportedCapability::action;return result;}
  if(arrival && input.session.current.field.selected_defenders) {
   const auto& session=input.session;const auto& field=session.current.field;
   const bool free_target=session.area_ai_target && session.area_ai_target->recipient==255;
   if((field.selected_defenders!=1 && !(free_target && field.selected_defenders==2)) || !session.scheduling || !session.area_ai_target || !session.ball_trajectory || !session.recovery || !session.experience ||
      !session.action || !session.saved_ball_flight_flag){result.unsupported=UnsupportedCapability::defender_contact;return result;}
   if(!content.data->pass_flight_comparison || !content.data->contact || !content.data->experience || !content.data->movement){result.unsupported=UnsupportedCapability::content;return result;}
   const auto cursor=free_target ? session.area_defender_cursor : 0;
   if(cursor>=field.selected_defenders || (free_target ? field.elapsed!=cursor : field.elapsed!=0)){result.unsupported=UnsupportedCapability::defender_contact;return result;}
   const auto id=session.scheduling->selected_ids[cursor];if(id<12 || id>21){result.unsupported=UnsupportedCapability::defender_contact;return result;}
   try {
    const auto& player=field.roster[id];const FieldPlayerRecord record{id,player.type,static_cast<std::uint8_t>(player.stamina),static_cast<std::uint8_t>(player.stamina>>8),player.level};
    const auto prep=prepare_profiled_field_defender(record,content.data->field_players,2,0,0,content.data->defense);
    const auto& rules=*content.data->pass_flight_comparison;
    const auto comparison=compare_defender_contact({session.action->power,prep.base_strength,field.random_first,field.random_second,rules.range,rules.offset,false,bool(rules.eligibility&128u),0,id,0,rules.thresholds});
    if(prep.move_effect || (comparison.special && !(comparison.result_code==3 && comparison.subtype==128)) ||
       (comparison.result_code!=0 && comparison.result_code!=3)){result.unsupported=UnsupportedCapability::defender_contact;return result;}
    if(comparison.result_code==0) {
     if(content.data->contact->field_elapsed[5]!=1 || field.motion[field_motion_index(id)].status){result.unsupported=UnsupportedCapability::defender_contact;return result;}
     auto next=input;auto& s=next.session;s.recovery->opposing_player=id;
     s.experience=award_player_experience(*s.experience,field.roster[field.ball_owner].type,1,*content.data->experience).state;
     auto clock=apply_contact_prelude({s.current,id,2,true},*content.data->contact);
     if(clock.clock.continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
     s.current=clock.state;
     s.area_defender_cursor=static_cast<std::uint8_t>(cursor+1);
     if(s.area_defender_cursor==field.selected_defenders) {s.current.field.selected_defenders=0;s.current.field.elapsed=0;s.scheduling->selected_ids.fill(0);}

     next.last_sequence=event.sequence;validate_phase_session(s);project_field_random_memory(s.current.field,next.random_memory);result.state=std::move(next);return result;
    }
    if(free_target && !(field.selected_defenders==2 && cursor==0 && comparison.special && comparison.subtype==128)) {result.unsupported=UnsupportedCapability::defender_contact;return result;}
    auto next=input;auto& s=next.session;const auto trajectory=*s.ball_trajectory;
    s.recovery->opposing_player=id;s.experience=award_player_experience(*s.experience,player.type,2,*content.data->experience).state;
    auto clock=apply_contact_prelude({s.current,id,2,false},*content.data->contact);
    if(clock.clock.continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
    s.current=clock.state;s.current.field.side=11;s.current.field.ball_owner=id;
    s.current.field=advance_field_phase(s.current.field,content.data->field_players,*content.data->movement).state;
    s.human_pass_cursor.reset();s.human_pass_steps=0;s.ordinary_pass_candidates.reset();s.ordinary_pass_recipient.reset();s.ordinary_pass_defenses.reset();
    s.area_ai_target.reset();s.shot_flight.reset();s.ball_trajectory.reset();s.area_contact.reset();s.area_defender_cursor=0;s.area_sequence_result=255;
    s.phase=FieldSessionPhase::player_control;next.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(trajectory.x),static_cast<std::uint8_t>(trajectory.y)};
    next.last_sequence=event.sequence;validate_phase_session(s);project_field_random_memory(s.current.field,next.random_memory);result.state=std::move(next);return result;
   }catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  }
  auto next=input;
  try {
   if(arrival && input.session.area_ai_target && input.session.area_ai_target->recipient==255) {
    if(!content.data->movement || !content.data->pass_race){result.unsupported=UnsupportedCapability::content;return result;}
    next.session=resolve_ordinary_free_pass_contest(input.session,content.data->field_players,*content.data->movement,*content.data->pass_race);
   } else next.session=arrival ? continue_ordinary_pass_reception(input.session) : position_ordinary_pass_recipient(input.session);
  }
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_free_pass_contest_winner) {
  if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
  const auto& field=input.session.current.field;
  // The original presentation-facing handoff has been observed at this winner and position.
  if(field.ball_owner!=2 || field.ball_x!=132 || field.ball_y!=164 || !input.session.ball_trajectory){result.unsupported=UnsupportedCapability::presentation;return result;}
  auto next=input;next.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(input.session.ball_trajectory->x),static_cast<std::uint8_t>(input.session.ball_trajectory->y)};
  try {next.session=continue_ordinary_pass_control(next.session,64);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_recipient_positioned) {
  if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
  // The recipient-facing presentation sample is only established for this controlled route.
  const auto& field=input.session.current.field;
  if(field.ball_owner!=7 || field.ball_x!=113 || field.ball_y!=143 || !input.session.ball_trajectory){result.unsupported=UnsupportedCapability::presentation;return result;}
  auto next=input;next.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(input.session.ball_trajectory->x),static_cast<std::uint8_t>(input.session.ball_trajectory->y)};
  try {next.session=continue_ordinary_pass_control(next.session,0);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 result.unsupported=UnsupportedCapability::phase;return result;
}
MatchStepResult handle_home_keeper_outlet(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
  if(input.pass_stage==MatchPassStage::flight_arrived){
   if(event.kind!=MatchInputKind::presentation_complete || !input.keeper_outlet_recipient || !input.pass_target || !input.pass_trajectory || !input.pass_cell ||
      !input.flight_defenders.empty() || *input.pass_cell!=input.pass_target->destination || *input.keeper_outlet_recipient!=input.pass_target->recipient){result.unsupported=UnsupportedCapability::keeper_outlet;return result;}
   auto& state=result.state;auto& field=state.session.current.field;const auto recipient=*input.keeper_outlet_recipient;
   const auto& player=field.motion[field_motion_index(recipient)];field.ball_owner=recipient;field.ball_x=static_cast<std::uint8_t>(player.x>>8);
   field.ball_y=static_cast<std::uint8_t>(player.y>>8);field.ball_cell=field_cell_at(field.ball_x,field.ball_y);
   state.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(input.pass_trajectory->x),static_cast<std::uint8_t>(input.pass_trajectory->y)};
   state.session.phase=FieldSessionPhase::player_control;state.keeper_outlet_recipient.reset();state.home_keeper_y.reset();state.pass_target.reset();state.pass_trajectory.reset();state.pass_cell.reset();state.pass_stage=MatchPassStage::none;
   state.last_sequence=event.sequence;validate_phase_session(state.session);return result;
  }
  if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::keeper_outlet;return result;}
  if(input.keeper_outlet_recipient){
   if(input.session.action && input.session.action->action==1 && input.pass_stage==MatchPassStage::flight_cell){
    if(!input.pass_cell || !input.pass_target || !input.pass_trajectory){result.unsupported=UnsupportedCapability::keeper_outlet;return result;}
    const auto& field=input.session.current.field;FlightDefenderSelectionInput selection{};
    selection.possession_side=field.side;selection.ball_cell=*input.pass_cell;selection.random_first=field.random_first;
    selection.random_second=field.random_second;selection.threshold=input.pass_target->counter;
    for(unsigned i=0;i<field.motion.size();++i){const auto& player=field.motion[i];
     selection.players[i]={static_cast<std::uint8_t>(player.x>>8),static_cast<std::uint8_t>(player.y>>8),player.status!=0};}
    if(!accumulate_flight_defenders(selection,input.flight_defenders).empty()){result.unsupported=UnsupportedCapability::keeper_outlet;return result;}
    result.state.pass_stage=MatchPassStage::flight_ready;result.state.last_sequence=event.sequence;return result;
   }
   if(input.session.action && input.session.action->action==1 && input.pass_stage==MatchPassStage::flight_ready){
    if(!input.pass_target || !input.pass_trajectory || !input.pass_cell){result.unsupported=UnsupportedCapability::keeper_outlet;return result;}
    const auto& trajectory=*input.pass_trajectory;
    const auto step=advance_ball_to_next_cell({trajectory.x,trajectory.y,{trajectory.vx,trajectory.vy},*input.pass_cell,input.pass_target->destination});
    result.state.pass_trajectory->x=step.x;result.state.pass_trajectory->y=step.y;result.state.pass_cell=step.cell;
    result.state.pass_stage=step.event==BallStepEvent::cell_changed ? MatchPassStage::flight_cell : MatchPassStage::flight_arrived;
    result.state.last_sequence=event.sequence;return result;
   }
   if(input.session.action && input.session.action->action==1 && input.pass_stage==MatchPassStage::target_ready){
    if(!input.pass_target || !input.session.recovery){result.unsupported=UnsupportedCapability::keeper_outlet;return result;}
    if(!input.ball_fraction){result.unsupported=UnsupportedCapability::ball_fraction;return result;}
    if(!content.data->movement || !content.ball_velocity){result.unsupported=UnsupportedCapability::content;return result;}
    const auto& field=input.session.current.field;
    if(field.ball_cell==input.pass_target->destination){result.unsupported=UnsupportedCapability::keeper_outlet;return result;}
    BallTrajectoryContext trajectory{static_cast<std::uint16_t>((unsigned(field.ball_x)<<8)|input.ball_fraction->x),
     static_cast<std::uint16_t>((unsigned(field.ball_y)<<8)|input.ball_fraction->y),0,0,0,1};
    trajectory.direction=calculate_player_direction({trajectory.x,trajectory.y,input.pass_target->destination,0},field.ball_cell,content.data->movement->geometry);
    const auto velocity=calculate_ball_velocity(trajectory.direction,*content.ball_velocity);trajectory.vx=velocity.x;trajectory.vy=velocity.y;
    result.state.pass_trajectory=trajectory;result.state.pass_cell=field.ball_cell;result.state.session.recovery->x_jitter_control=trajectory.direction;
    result.state.session.current.field.selected_defenders=0;
    result.state.pass_stage=MatchPassStage::flight_ready;result.state.last_sequence=event.sequence;return result;
   }
   if(input.session.action && input.session.action->action==1 && input.pass_stage==MatchPassStage::none){
    if(!content.pass_distance){result.unsupported=UnsupportedCapability::content;return result;}
    try{const auto& field=input.session.current.field;const auto& receiver=field.motion[field_motion_index(*input.keeper_outlet_recipient)];
     const auto destination=field_cell_at(static_cast<std::uint8_t>(receiver.x>>8),static_cast<std::uint8_t>(receiver.y>>8));
     const auto distance=calculate_pass_distance(field.ball_x,field.ball_y,destination,*content.pass_distance);
     result.state.pass_target=MatchPassTarget{*input.keeper_outlet_recipient,destination,26,distance};
     result.state.pass_stage=MatchPassStage::target_ready;result.state.session.saved_ball_flight_flag=1;result.state.last_sequence=event.sequence;return result;
    }catch(const std::invalid_argument&){result.state=input;result.unsupported=UnsupportedCapability::keeper_outlet;return result;}
   }
   if(!input.session.action || !input.session.recovery || input.session.action->action!=0){result.unsupported=UnsupportedCapability::keeper_outlet;return result;}
   if(!input.home_keeper_y){result.unsupported=UnsupportedCapability::keeper_position;return result;}
   result.state.session.action->action=1;result.state.session.action->auxiliary=0;result.state.session.recovery->attack_subtype=0;
   result.state.session.recovery->ball_variant=static_cast<std::uint8_t>((input.session.current.field.random_first&1u)+1u);
   result.state.session.current.field.keepers[0].stun=0;
   result.state.session.current.field.ball_x=input.session.current.field.keepers[0].penalty_duration;
   result.state.session.current.field.ball_y=*input.home_keeper_y;
   result.state.session.current.field.ball_cell=field_cell_at(result.state.session.current.field.ball_x,result.state.session.current.field.ball_y);
   try{validate_phase_session(result.state.session);}catch(const std::invalid_argument&){result.state=input;result.unsupported=UnsupportedCapability::keeper_outlet;return result;}
   result.state.last_sequence=event.sequence;return result;
  }
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  auto next=input;advance_demo_random(next);auto pick=static_cast<std::uint8_t>(next.session.current.field.random_first&7u);
  if(pick>=6)pick=static_cast<std::uint8_t>(pick-6u);
  next.keeper_outlet_recipient=static_cast<std::uint8_t>(pick+5u);next.last_sequence=event.sequence;
  project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
}
}
MatchStepResult step_match(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(!content.data){result.unsupported=UnsupportedCapability::content;return result;}
 if(event.sequence<=input.last_sequence){result.unsupported=UnsupportedCapability::input_sequence;return result;}
 if(input.mode==MatchTimingMode::provisional_demo && input.session.current.field.side==11 &&
    (input.session.phase==FieldSessionPhase::field_contact_pending || (input.session.phase==FieldSessionPhase::field && input.defense_menu &&
     (static_cast<unsigned>(input.defense_menu->stage)>=static_cast<unsigned>(MatchDefenseStage::ai_flight_ready) || (!input.defense_menu->defender_count && static_cast<unsigned>(input.defense_menu->stage)>=static_cast<unsigned>(MatchDefenseStage::ai_chosen)))))) {
  const auto count=input.session.current.field.selected_defenders;
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::dispatch_pending){
   if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
   if(!content.data->movement){result.unsupported=UnsupportedCapability::content;return result;}
   if(!input.defense_menu->defender_count && (!input.session.recovery || !input.session.control)){result.unsupported=UnsupportedCapability::phase;return result;}
   auto next=input;
   try {
    next.session.current.field=advance_field_phase(input.session.current.field,content.data->field_players,*content.data->movement).state;
    if(next.session.action && next.session.action->action==1){
     if(next.defense_menu->defender_count)next.retained_defense_action.reset();
     if(!next.defense_menu->defender_count){next.session.recovery->ball_variant=static_cast<std::uint8_t>((next.session.current.field.random_first&1)+1);next.session.control->last_carrier_cell=128;next.session.control->display_parameter=0;next.session.control->motion_requested=0;}
     if(!next.defense_menu->ai_pass_target)throw std::invalid_argument("Missing opponent pass target");
     const auto recipient=next.defense_menu->ai_pass_target->recipient;
     const auto& player=next.session.current.field.motion[field_motion_index(recipient)];
     const auto cell=field_cell_at(static_cast<std::uint8_t>(player.x>>8),static_cast<std::uint8_t>(player.y>>8));
     if(cell==255)throw std::invalid_argument("Opponent pass recipient outside field");
     next.defense_menu->ai_pass_target->cell=cell;next.defense_menu->stage=MatchDefenseStage::ai_pass_handoff;
    } else if(next.session.action && next.session.action->action==0 && next.defense_menu->defender_count==1 && next.defense_menu->poll.defense.actions[0]==6) {
     next.session.phase=FieldSessionPhase::shot_approach_ready;next.defense_menu.reset();validate_phase_session(next.session);
    } else {next.session.phase=FieldSessionPhase::player_control;next.defense_menu.reset();validate_phase_session(next.session);}
   } catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::opponent_execution;return result;}
   next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
  }
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::attack_prepared){
   if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
   if(input.session.action && input.session.action->action==1){
    if(!input.session.control || !input.session.recovery){result.unsupported=UnsupportedCapability::phase;return result;}
    if(!input.defense_menu->defender_count){result.state.session.current.clock.ball_mode=0;result.state.session.current.clock.script_flags=0;result.state.session.control->last_carrier_cell=0;}
    else {result.state.session.recovery->ball_variant=1;result.state.session.control->last_carrier_cell=128;}
   } else if(input.session.action && input.session.action->action==0 && input.defense_menu->defender_count==1 && input.defense_menu->poll.defense.actions[0]==6) {
    if(!input.session.control || !input.session.recovery){result.unsupported=UnsupportedCapability::phase;return result;}
    result.state.session.recovery->ball_variant=1;result.state.session.control->last_carrier_cell=128;
    result.state.session.control->facing=0;result.state.session.control->motion_requested=2;result.state.session.control->display_parameter=0;
   }
   result.state.defense_menu->stage=MatchDefenseStage::clock_pending;result.state.last_sequence=event.sequence;return result;
  }
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::ai_flight_arrived){
   if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
   if(!input.flight_defenders.empty())return resolve_pass_flight_contact(input,event,content);
   const auto& menu=*input.defense_menu;
   if(!menu.ai_pass_target || !menu.ai_trajectory || !menu.ai_ball_cell || *menu.ai_ball_cell!=menu.ai_pass_target->cell ||
      menu.ai_trajectory->flags!=1 || input.session.current.field.selected_defenders){result.unsupported=UnsupportedCapability::phase;return result;}
   const auto recipient=menu.ai_pass_target->recipient;
   if(recipient<12 || recipient>21){result.unsupported=UnsupportedCapability::action;return result;}
   const auto& player=input.session.current.field.motion[field_motion_index(recipient)];
   const auto x=static_cast<std::uint8_t>(player.x>>8),y=static_cast<std::uint8_t>(player.y>>8);
   if(field_cell_at(x,y)!=menu.ai_pass_target->cell){result.unsupported=UnsupportedCapability::phase;return result;}
   auto next=input;next.session.current.field.ball_owner=recipient;next.session.current.field.ball_x=x;next.session.current.field.ball_y=y;
   next.session.current.field.ball_cell=*menu.ai_ball_cell;next.session.phase=FieldSessionPhase::player_control;
   next.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(menu.ai_trajectory->x),static_cast<std::uint8_t>(menu.ai_trajectory->y)};
   next.defense_menu.reset();next.last_sequence=event.sequence;validate_phase_session(next.session);
   project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
  }
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::ai_flight_cell){
   if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
   const auto& menu=*input.defense_menu;if(!menu.ai_ball_cell || !menu.ai_trajectory){result.unsupported=UnsupportedCapability::phase;return result;}
   const auto& field=input.session.current.field;FlightDefenderSelectionInput selection{};
   selection.possession_side=field.side;selection.ball_cell=*menu.ai_ball_cell;selection.random_first=field.random_first;selection.random_second=field.random_second;selection.threshold=255;
   for(unsigned i=0;i<field.motion.size();++i){const auto& player=field.motion[i];selection.players[i]={static_cast<std::uint8_t>(player.x>>8),static_cast<std::uint8_t>(player.y>>8),player.status!=0};}
   result.state.flight_defenders=accumulate_flight_defenders(selection,input.flight_defenders);
   if(!result.state.flight_defenders.empty())result.state.retained_defense_action.reset();
   result.state.defense_menu->stage=MatchDefenseStage::ai_flight_ready;result.state.last_sequence=event.sequence;return result;
  }
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::ai_flight_ready){
   if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
   const auto& menu=*input.defense_menu;if(!menu.ai_pass_target || !menu.ai_trajectory || !menu.ai_ball_cell){result.unsupported=UnsupportedCapability::phase;return result;}
   const auto& t=*menu.ai_trajectory;const auto step=advance_ball_to_next_cell({t.x,t.y,{t.vx,t.vy},*menu.ai_ball_cell,menu.ai_pass_target->cell});
   result.state.defense_menu->ai_trajectory->x=step.x;result.state.defense_menu->ai_trajectory->y=step.y;result.state.defense_menu->ai_ball_cell=step.cell;
   result.state.defense_menu->stage=step.event==BallStepEvent::cell_changed ? MatchDefenseStage::ai_flight_cell : MatchDefenseStage::ai_flight_arrived;
   result.state.last_sequence=event.sequence;return result;
  }
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::ai_pass_target_ready){
   if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
   if(!content.data->movement || !content.ball_velocity){result.unsupported=UnsupportedCapability::content;return result;}
   if(!input.ball_fraction){result.unsupported=UnsupportedCapability::ball_fraction;return result;}
   if(!input.defense_menu->ai_pass_target || !input.defense_menu->ai_pass_distance || !input.session.scheduling || !input.session.recovery ||
      input.session.current.field.ball_cell==input.defense_menu->ai_pass_target->cell){result.unsupported=UnsupportedCapability::phase;return result;}
   auto next=input;const auto& field=input.session.current.field;
   BallTrajectoryContext trajectory{static_cast<std::uint16_t>((unsigned(field.ball_x)<<8)|input.ball_fraction->x),
     static_cast<std::uint16_t>((unsigned(field.ball_y)<<8)|input.ball_fraction->y),0,0,0,1};
   trajectory.direction=calculate_player_direction({trajectory.x,trajectory.y,input.defense_menu->ai_pass_target->cell,0},field.ball_cell,content.data->movement->geometry);
   const auto velocity=calculate_ball_velocity(trajectory.direction,*content.ball_velocity);trajectory.vx=velocity.x;trajectory.vy=velocity.y;
   next.defense_menu->ai_trajectory=trajectory;next.defense_menu->ai_ball_cell=field.ball_cell;
   next.defense_menu->stage=MatchDefenseStage::ai_flight_ready;next.session.recovery->x_jitter_control=trajectory.direction;
   next.session.saved_ball_flight_flag=1;next.session.current.field.selected_defenders=0;next.session.scheduling->selected_ids.fill(0);next.session.phase=FieldSessionPhase::field;
   next.last_sequence=event.sequence;result.state=std::move(next);return result;
  }
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::ai_pass_handoff){
   if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
   if(!content.pass_distance){result.unsupported=UnsupportedCapability::content;return result;}
   if(!input.defense_menu->ai_pass_target){result.unsupported=UnsupportedCapability::phase;return result;}
   try {const auto& field=input.session.current.field;
    result.state.defense_menu->ai_pass_distance=calculate_pass_distance(field.ball_x,field.ball_y,input.defense_menu->ai_pass_target->cell,*content.pass_distance);
   } catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::opponent_execution;return result;}
   result.state.defense_menu->stage=MatchDefenseStage::ai_pass_target_ready;result.state.last_sequence=event.sequence;return result;
  }
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::clock_pending){
   if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
   if(input.defense_menu->defender_count && !content.data->area_prefix_elapsed){result.unsupported=UnsupportedCapability::content;return result;}
   const bool skipped_shot=input.session.action && input.session.action->action==0 && input.defense_menu->defender_count==1 && input.defense_menu->poll.defense.actions[0]==6;
   if(!input.session.action || (input.session.action->action!=1 && input.session.action->action!=2 && !skipped_shot)){result.unsupported=UnsupportedCapability::phase;return result;}
   auto next=input;if(skipped_shot)next.session.area_defender_cursor=1;auto& current=next.session.current;const auto elapsed=input.defense_menu->defender_count ? (*content.data->area_prefix_elapsed)[input.session.action->action] : 0;
   current.field.elapsed=static_cast<std::uint8_t>(unsigned(current.field.elapsed)+elapsed);
   const auto recovered=advance_keeper_recovery({current.field.keepers,current.shared_recovery_flags,current.shared_recovery_timer},elapsed);
   current.field.keepers=recovered.keepers;current.shared_recovery_flags=recovered.shared_flags;current.shared_recovery_timer=recovered.shared_timer;
   const auto clock=advance_match_clock(current.clock,elapsed,current.field.random_first);
   if(clock.continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
   current.clock=clock.state;next.defense_menu->stage=MatchDefenseStage::dispatch_pending;next.last_sequence=event.sequence;
   project_field_random_memory(current.field,next.random_memory);result.state=std::move(next);return result;
  }
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::ai_chosen){
   if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
   const bool skipped_generic_shot=input.session.action && input.session.action->action==0 && input.defense_menu->defender_count==1 &&
    input.defense_menu->poll.defense.actions[0]==6 && input.session.current.field.ball_owner>=12 && input.session.current.field.ball_owner<=21 &&
    input.session.current.field.roster[input.session.current.field.ball_owner].type==0;
   if(!input.session.action || !input.session.recovery || input.session.recovery->ball_variant || (input.session.action->action!=1 && input.session.action->action!=2 && (input.defense_menu->defender_count || input.session.action->action!=0) && !skipped_generic_shot)){result.unsupported=UnsupportedCapability::phase;return result;}
   const auto action=input.session.action->action;
   if(content.data->field_players.skills.empty() || content.data->attack.moves[action].empty()){result.unsupported=UnsupportedCapability::content;return result;}
   const auto& field=input.session.current.field;const auto owner=field.ball_owner;
   if(owner<12 || owner>21){result.unsupported=UnsupportedCapability::phase;return result;}
   const auto& p=field.roster[owner];
   try {
    const auto prepared=prepare_attacking_action({{owner,p.type,static_cast<std::uint8_t>(p.stamina),static_cast<std::uint8_t>(p.stamina>>8),p.level},action,0,
     input.session.recovery->attack_subtype,11,field.selected_defenders,field.random_first,field.random_second,false},content.data->field_players,content.data->attack);
    result.state.session.action->power=prepared.action_power.power;result.state.session.action->move_effect=prepared.preparation.move_effect;
    result.state.session.action->move_flags=prepared.move_flags;result.state.session.recovery->attack_subtype=prepared.action_power.subtype;
    result.state.defense_menu->attack_cost=prepared.preparation.stamina_cost;
   } catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::opponent_execution;return result;}
   if(!input.defense_menu->defender_count && action==0){result.state.defense_menu.reset();result.state.session.phase=FieldSessionPhase::attack_prepared;}
   else result.state.defense_menu->stage=MatchDefenseStage::attack_prepared;
   result.state.last_sequence=event.sequence;return result;
  }
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::complete){
   if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
   if(!content.data->ai_field_choice || !content.data->ai_area_choice){result.unsupported=UnsupportedCapability::content;return result;}
   if(!input.demo_rng || !input.session.action || !input.session.recovery){result.unsupported=UnsupportedCapability::phase;return result;}
   const auto& field=input.session.current.field;const auto owner=field.ball_owner;
   if(owner<12 || owner>21){result.unsupported=UnsupportedCapability::phase;return result;}
   auto next=input;advance_demo_random(next);const auto& actor=field.roster[owner];
   std::array<std::array<std::uint8_t,2>,10> positions{};
   for(unsigned id=12;id<=21;++id){const auto& player=field.motion[field_motion_index(static_cast<std::uint8_t>(id))];positions[id-12]={{static_cast<std::uint8_t>(player.x>>8),static_cast<std::uint8_t>(player.y>>8)}};}
   try {
    const auto flags=static_cast<std::uint8_t>(actor.stamina),alternate=static_cast<std::uint8_t>(actor.stamina>>8);
    const auto& ai=*content.data->ai_field_choice;
    AiAreaPlan plan{};
    if(actor.type==0) {
     const auto candidate=choose_generic_ai_field_candidate(owner,flags,alternate,field.ball_x,field.ball_y,next.session.current.field.random_first,content.data->field_players,ai);
     if(candidate.candidate==1 && !next.session.recovery->ball_variant && ai.no_variant_zero_shot_options && (*ai.no_variant_zero_shot_options)[0])
      plan={0,0,next.session.current.field.random_first,std::nullopt,std::nullopt};
     else plan=plan_generic_ai_field_pass(owner,flags,alternate,field.ball_x,field.ball_y,next.session.current.field.random_first,next.session.current.field.random_second,positions,content.data->field_players,ai,*content.data->ai_area_choice);
    } else plan=plan_ai_field_pass_candidate(actor.type,flags,alternate,owner,field.ball_x,field.ball_y,next.session.current.field.random_first,next.session.current.field.random_second,positions,ai,*content.data->ai_area_choice);
    next.session.action->action=plan.action;next.session.recovery->attack_subtype=plan.subtype;next.session.current.field.random_first=plan.random_first;
    if(plan.action==1)next.defense_menu->ai_pass_target=MatchAiPassTarget{*plan.recipient,*plan.cell};
   } catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::opponent_action;return result;}
   next.defense_menu->stage=MatchDefenseStage::ai_chosen;next.last_sequence=event.sequence;
   project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
  }
  if(!input.defense_menu) {
   if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::defensive_prompt;return result;}
   if(!content.data->area_defense_menu || !content.data->area_defense_menu->field_icons){result.unsupported=UnsupportedCapability::content;return result;}
   if(!input.demo_rng || !input.session.scheduling || !input.session.action || !input.session.recovery || !count || count>4){result.unsupported=UnsupportedCapability::phase;return result;}
   result.state.defense_menu=MatchDefenseMenu{};result.state.defense_menu->defender_count=count;result.state.defense_menu->poll.flash_counter=128;
   result.state.defense_menu->poll.defense.blink=13;result.state.last_sequence=event.sequence;return result;
  }
  if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
  if(!content.data->area_defense_menu || !content.data->area_defense_menu->field_icons){result.unsupported=UnsupportedCapability::content;return result;}
  if(!input.demo_rng || !input.session.scheduling || !input.session.recovery || !count || count>4){result.unsupported=UnsupportedCapability::phase;return result;}
  const auto& menu=*input.defense_menu;
  if(menu.stage==MatchDefenseStage::choice_confirmed) {
   if(menu.poll.cursor>=count || menu.poll.defense.actions[menu.poll.cursor]!=6 || menu.poll.defense.pressed!=128){result.unsupported=UnsupportedCapability::action;return result;}
   auto& next=*result.state.defense_menu;const auto slot=next.poll.cursor;
   next.poll.defense.subtypes[slot]=0;next.poll.defense.pressed=0;++next.poll.cursor;
   next.stage=MatchDefenseStage::polling;
   result.events.push_back({{ControlPresentationKind::sprite,(*content.data->area_defense_menu->field_icons)[6],slot}});
   if(next.poll.cursor<count){next.poll.flash_counter|=64;next.poll.defense.blink=0;result.events.push_back({{ControlPresentationKind::display,content.data->area_defense_menu->titles[0]}});}
   result.state.last_sequence=event.sequence;return result;
  }
  if(menu.stage!=MatchDefenseStage::polling){result.unsupported=UnsupportedCapability::phase;return result;}
  if(menu.poll.cursor==count && !(event.pressed&192u) && (event.pressed&15u)){result.unsupported=UnsupportedCapability::action;return result;}
  auto next=input;advance_demo_random(next);
  const auto poll=poll_defense_menu(menu.poll,count,input.session.scheduling->selected_ids,true,event.pressed,*content.data->area_defense_menu);
  next.defense_menu->poll=poll.state;
  if(poll.boundary==DefenseMenuBoundary::choice_confirmed){next.defense_menu->stage=MatchDefenseStage::choice_confirmed;
   if(menu.poll.cursor<count && menu.poll.defense.actions[menu.poll.cursor]==6 && poll.opposing_player){next.retained_defense_action=6;next.session.recovery->opposing_player=*poll.opposing_player;}
   else next.retained_defense_action.reset();}
  else if(poll.boundary==DefenseMenuBoundary::complete)next.defense_menu->stage=MatchDefenseStage::complete;
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);
  for(const auto& request:poll.presentation)result.events.push_back({request});
  return result;
 }
 if((input.session.phase==FieldSessionPhase::player_control || (input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_control_entry)) && event.kind==MatchInputKind::tick) {
  auto prepared=prepare_session_control(input.session);
  result.state.session=prepared.state;result.state.last_sequence=event.sequence;
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);
  for(const auto& request:prepared.presentation) result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && event.kind==MatchInputKind::tick && input.session.current.field.side==0 &&
    (input.session.phase==FieldSessionPhase::action_menu_pending || input.session.phase==FieldSessionPhase::field_contact_pending || input.session.phase==FieldSessionPhase::human_action_menu)) {
  if(input.session.current.field.ball_owner==0 && (input.session.phase==FieldSessionPhase::field_contact_pending ||
     (input.session.phase==FieldSessionPhase::human_action_menu && event.pressed!=0 && event.pressed!=2 &&
      !(event.pressed==128 && input.session.action && input.session.action->action==1 && (input.session.action->cursor&128u))))){result.unsupported=UnsupportedCapability::action;return result;}
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->action_menu){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  auto menu=(next.session.phase==FieldSessionPhase::action_menu_pending || next.session.phase==FieldSessionPhase::field_contact_pending)
    ? open_human_action_menu(next.session,*content.data->action_menu)
    : poll_human_action_menu(next.session,event.pressed,*content.data->action_menu);
  next.session=menu.state;next.last_sequence=event.sequence;
  if(input.session.phase==FieldSessionPhase::field_contact_pending || next.session.current.field.selected_defenders)next.retained_defense_action.reset();
  project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);
  for(const auto& request:menu.presentation)result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::control_ready && event.kind==MatchInputKind::tick) {
  if(input.session.current.field.ball_owner==0 && (!input.session.control || !input.session.control->decision_timer || !((event.held==0 && event.pressed==0) || (event.held==64 && event.pressed==64)))){result.unsupported=UnsupportedCapability::action;return result;}
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->movement || !content.data->control_movement){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  auto poll=poll_session_control(next.session,event.held,event.pressed,content.data->field_players,*content.data->movement,*content.data->control_movement);
  next.session=poll.state;next.last_sequence=event.sequence;
  const bool ai_cell=next.session.current.field.side==11 && next.session.phase==FieldSessionPhase::ai_cell_decision;
  if(next.session.current.field.selected_defenders || (!ai_cell && next.session.phase!=FieldSessionPhase::control_ready && next.session.phase!=FieldSessionPhase::control_clock_pending && next.session.phase!=FieldSessionPhase::action_menu_pending))next.retained_defense_action.reset();
  if(!ai_cell && !(next.session.phase==FieldSessionPhase::field_contact_pending) && next.session.phase!=FieldSessionPhase::control_ready && next.session.phase!=FieldSessionPhase::control_clock_pending && next.session.phase!=FieldSessionPhase::action_menu_pending)next.ball_fraction.reset();
  project_field_random_memory(next.session.current.field,next.random_memory);
  result.state=std::move(next);
  for(const auto& request:poll.presentation)result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ai_cell_decision && event.kind==MatchInputKind::tick) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->ai_field_choice){result.unsupported=UnsupportedCapability::content;return result;}
  const auto& field=input.session.current.field;const auto owner=field.ball_owner;
  if(field.side!=11 || field.selected_defenders || owner<12 || owner>21 || !input.session.action || !input.session.recovery){result.unsupported=UnsupportedCapability::phase;return result;}
  auto next=input;advance_demo_random(next);const auto& actor=field.roster[owner];
  std::array<std::array<std::uint8_t,2>,10> positions{};
  for(unsigned id=12;id<=21;++id){const auto& player=field.motion[field_motion_index(static_cast<std::uint8_t>(id))];positions[id-12]={{static_cast<std::uint8_t>(player.x>>8),static_cast<std::uint8_t>(player.y>>8)}};}
  try {
   const auto flags=static_cast<std::uint8_t>(actor.stamina),alternate=static_cast<std::uint8_t>(actor.stamina>>8);
   const auto& ai=*content.data->ai_field_choice;
   const auto candidate=actor.type==0 ? choose_generic_ai_field_candidate(owner,flags,alternate,field.ball_x,field.ball_y,next.session.current.field.random_first,content.data->field_players,ai) :
    choose_ai_field_candidate(actor.type,flags,alternate,field.ball_x,field.ball_y,next.session.current.field.random_first,ai);
   if(actor.type==0 && candidate.candidate!=2 && candidate.candidate!=3){result.unsupported=UnsupportedCapability::opponent_action;return result;}
   AiAreaPlan plan{};
   if(candidate.candidate==1 && !input.session.recovery->ball_variant && ai.no_variant_zero_shot_options && actor.type<ai.no_variant_zero_shot_options->size() && (*ai.no_variant_zero_shot_options)[actor.type]){
    plan={0,0,next.session.current.field.random_first,std::nullopt,std::nullopt};
   } else if(candidate.candidate==3){
    if(!input.session.recovery->ball_variant && (!ai.empty_action2_options || actor.type>=ai.empty_action2_options->size() || !(*ai.empty_action2_options)[actor.type])){result.unsupported=UnsupportedCapability::opponent_action;return result;}
    plan={2,0,next.session.current.field.random_first,std::nullopt,std::nullopt};
   } else {
    if(!content.data->ai_area_choice){result.unsupported=UnsupportedCapability::content;return result;}
    plan=actor.type==0 ? plan_generic_ai_field_pass(owner,flags,alternate,field.ball_x,field.ball_y,next.session.current.field.random_first,next.session.current.field.random_second,positions,content.data->field_players,ai,*content.data->ai_area_choice) :
     plan_ai_field_pass_candidate(actor.type,flags,alternate,owner,field.ball_x,field.ball_y,next.session.current.field.random_first,next.session.current.field.random_second,positions,ai,*content.data->ai_area_choice);
   }
   if(plan.action==0){next.session.phase=FieldSessionPhase::field;next.defense_menu=MatchDefenseMenu{};next.defense_menu->stage=MatchDefenseStage::ai_chosen;}
   else if(plan.action==1){
    if(input.session.recovery->ball_variant || !plan.recipient || !plan.cell){result.unsupported=UnsupportedCapability::opponent_action;return result;}
    next.session.phase=FieldSessionPhase::field;next.defense_menu=MatchDefenseMenu{};next.defense_menu->stage=MatchDefenseStage::ai_chosen;
    next.defense_menu->ai_pass_target=MatchAiPassTarget{*plan.recipient,*plan.cell};
   } else if(plan.action==2)next.session.phase=FieldSessionPhase::control_clock_pending;
   else {result.unsupported=UnsupportedCapability::opponent_action;return result;}
   next.session.action->action=plan.action;next.session.action->menu_route=0;next.session.recovery->attack_subtype=plan.subtype;
   next.session.current.field.random_first=plan.random_first;
   next.last_sequence=event.sequence;validate_phase_session(next.session);project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
  } catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::opponent_action;return result;}
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::attack_choice_pending &&
    input.session.current.field.side==0 && input.session.action && input.session.action->action==1 &&
    input.session.action->menu_route==0 && event.kind==MatchInputKind::action && event.action==MatchAction::pass && !event.target) {
  if(input.session.current.field.ball_owner==0 && !input.home_keeper_y){result.unsupported=UnsupportedCapability::keeper_position;return result;}
  try{result.state.session=begin_ordinary_human_pass_target(input.session,input.home_keeper_y);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::action;return result;}
  if(input.session.current.field.selected_defenders)result.state.retained_defense_action.reset();
  result.state.last_sequence=event.sequence;
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::attack_choice_pending &&
    input.session.current.field.side==0 && input.session.action && input.session.action->action==0 &&
    input.session.action->menu_route==0 && event.kind==MatchInputKind::action && event.action==MatchAction::shot && !event.target) {
  return prepare_human_shot(input,event,content);
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::attack_prepared &&
    input.session.action && input.session.action->action==0 && !input.session.current.field.selected_defenders) {
  return dispatch_human_shot(input,event);
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::shot_dispatch_ready &&
    input.session.action && input.session.action->action==0 && !input.session.current.field.selected_defenders) {
  return move_human_shot_field(input,event,content);
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::selected_attack_presentation &&
    input.session.action && input.session.action->action==0 && !input.session.current.field.selected_defenders) {
  return continue_human_shot_attack(input,event,content);
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::shot_approach_ready &&
    input.session.action && input.session.action->action==0 && (!input.session.current.field.selected_defenders ||
     (input.session.current.field.side==11 && input.session.current.field.ball_owner==21 && input.session.current.field.selected_defenders==1 && input.session.area_defender_cursor==1 && !input.session.shot_progress)))
  return approach_human_shot(input,event,content);
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::shot_flight_pending &&
    input.session.action && input.session.action->action==0 && (!input.session.current.field.selected_defenders ||
     (input.session.current.field.side==11 && input.session.current.field.ball_owner==21 && input.session.current.field.selected_defenders==1 && input.session.area_defender_cursor==1 && !input.session.shot_progress)))
  return begin_human_shot_flight(input,event,content);
 if(input.mode==MatchTimingMode::provisional_demo && (input.session.phase==FieldSessionPhase::ball_advance_ready || input.session.phase==FieldSessionPhase::ball_cell_callback) &&
    input.session.action && input.session.action->action==0)
  return advance_human_shot_flight(input,event);
 if(input.mode==MatchTimingMode::provisional_demo && (input.session.phase==FieldSessionPhase::ball_arrival_callback || input.session.phase==FieldSessionPhase::shot_keeper_entry) &&
    input.session.current.field.side==11 && input.session.phase==FieldSessionPhase::ball_arrival_callback && input.session.current.field.selected_defenders && input.session.action && input.session.action->action==0)
  return resolve_opponent_shot_defender(input,event,content);
 if(input.mode==MatchTimingMode::provisional_demo && (input.session.phase==FieldSessionPhase::ball_arrival_callback || input.session.phase==FieldSessionPhase::shot_keeper_entry) &&
    input.session.action && input.session.action->action==0)
  return enter_human_shot_keeper(input,event);
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::keeper_presentation_pending && input.session.current.field.side==11)
  return handle_human_keeper_menu(input,event,content);
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::keeper_presentation_pending &&
    input.session.current.field.side==0 && input.session.action && input.session.action->action==0)
  return resolve_opponent_keeper_ai(input,event,content);
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::keeper_decided &&
    input.session.action && input.session.action->action==0 && (input.session.current.field.side==0 || (input.session.current.field.side==11 && input.session.keeper_decision && (input.session.keeper_decision->result_code==0 || input.session.keeper_decision->result_code>=2))))
  return score_human_shot(input,event,content);
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::keeper_possession && input.session.current.field.side==0)
  return handle_home_keeper_outlet(input,event,content);
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::goal_celebration_pending &&
    input.session.action && input.session.action->action==0)
  return restart_after_human_goal(input,event);
 if(input.mode==MatchTimingMode::provisional_demo && (input.session.phase==FieldSessionPhase::ball_recovery || input.session.phase==FieldSessionPhase::keeper_punch_recovery) && input.session.action && input.session.action->action==0)
  return recover_after_keeper_save(input,event,content);
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::recovered_owner && input.session.action && input.session.action->action==0)
  return continue_keeper_save_recovery(input,event);
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::keeper_interaction && input.session.action && input.session.action->action==0)
  return prepare_keeper_save_area(input,event);
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::keeper_area_prepared && input.session.action && input.session.action->menu_route==1 && input.session.current.field.side==11) {
  if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
  const auto& field=input.session.current.field;
  // With random above 85 the original goalkeeper-selection table cannot select the keeper.
  if(!input.session.scheduling || field.random_first<=85){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  DefenderSelectionInput scan{};scan.possession_side=field.side;scan.ball_x=field.ball_x;scan.ball_y=field.ball_y;scan.radius=10;
  for(unsigned i=0;i<field.motion.size();++i){const auto& m=field.motion[i];scan.players[i]={static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8),m.status!=0};}
  if(!select_field_defenders(scan).empty()){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  auto next=input;next.session.current.field.selected_defenders=0;next.session.scheduling->selected_ids.fill(0);
  next.session.phase=FieldSessionPhase::keeper_area_choices_done;next.last_sequence=event.sequence;
  validate_phase_session(next.session);project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::keeper_area_choices_done && input.session.action && input.session.action->menu_route==1 && input.session.current.field.side==11) {
  if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->ai_goal_area_choice || !content.data->ai_area_choice){result.unsupported=UnsupportedCapability::content;return result;}
  const auto& field=input.session.current.field;const auto owner=field.ball_owner;
  if(owner<12 || owner>21 || field.roster[owner].type!=0 || !input.session.recovery){result.unsupported=UnsupportedCapability::opponent_action;return result;}
  std::array<std::array<std::uint8_t,2>,10> positions{};for(unsigned id=12;id<=21;++id){const auto& m=field.motion[field_motion_index(static_cast<std::uint8_t>(id))];positions[id-12]={{static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8)}};}
  auto next=input;advance_demo_random(next);const auto& actor=field.roster[owner];
  try {const auto plan=plan_generic_goal_area_pass(owner,static_cast<std::uint8_t>(actor.stamina),static_cast<std::uint8_t>(actor.stamina>>8),field.ball_x,field.ball_y,
    next.session.current.field.random_first,next.session.current.field.random_second,input.session.recovery->ball_variant,positions,content.data->field_players,*content.data->ai_goal_area_choice,*content.data->ai_area_choice);
   if(plan.action!=1 || !plan.recipient || !plan.cell){result.unsupported=UnsupportedCapability::opponent_action;return result;}
   next.session.action->action=plan.action;next.session.recovery->attack_subtype=plan.subtype;next.session.current.field.random_first=plan.random_first;
   next.session.area_ai_target=PhaseSessionState::AreaAiTarget{*plan.recipient,*plan.cell};next.session.phase=FieldSessionPhase::keeper_area_ai_chosen;
   validate_phase_session(next.session);
  }catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::opponent_action;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::keeper_area_ai_chosen && input.session.action && input.session.action->menu_route==1 && input.session.current.field.side==11) {
  if(event.kind!=MatchInputKind::tick){result.unsupported=UnsupportedCapability::action;return result;}
  auto next=input;try {next.session=prepare_session_area_attack(next.session,*content.data);}
  catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::opponent_execution;return result;}
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && (input.session.phase==FieldSessionPhase::goal_kickoff_pending || input.session.phase==FieldSessionPhase::kickoff_formation_placed ||
    input.session.phase==FieldSessionPhase::kickoff_receiver_selected || input.session.phase==FieldSessionPhase::kickoff_ball_placed || input.session.phase==FieldSessionPhase::kickoff_control_pending) &&
    input.session.score && (input.session.score->kickoff==4 ||
     (input.session.score->kickoff==0 && input.session.score->left_goals==0 && input.session.score->right_goals==0 &&
      input.session.current.clock.remaining==180 && input.session.current.field.side==0 && input.session.experience &&
      input.session.experience->campaign==0 && input.session.experience->opponent_stage==0 && input.session.current.field.roster[9].type==1 &&
      (!input.session.kickoff_receiver || *input.session.kickoff_receiver==5 || *input.session.kickoff_receiver==6 ||
       *input.session.kickoff_receiver==7 || *input.session.kickoff_receiver==9 || *input.session.kickoff_receiver==10))))
  return advance_goal_kickoff(input,event,content);
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_human_pass_target && event.kind==MatchInputKind::tick) {
  if(input.session.current.field.ball_owner==0 && (event.pressed!=0 || (event.held!=0 && event.held!=1 && event.held!=2 && event.held!=4 && event.held!=8))){result.unsupported=UnsupportedCapability::action;return result;}
  if(input.target_chooser){
   if(!input.demo_rng || !input.session.ordinary_pass_candidates){result.unsupported=UnsupportedCapability::phase;return result;}
   const auto count=input.session.ordinary_pass_candidates->candidates.allied_count;
   if(count<2 || input.target_chooser->index>=count){result.unsupported=UnsupportedCapability::phase;return result;}
   auto next=input;advance_demo_random(next);
   if(event.pressed&64u){
    std::array<std::array<std::uint8_t,2>,22> positions{};
    for(unsigned id=1;id<22;++id){if(id==11)continue;const auto& p=next.session.current.field.motion[field_motion_index(static_cast<std::uint8_t>(id))];positions[id]={{static_cast<std::uint8_t>(p.x>>8),static_cast<std::uint8_t>(p.y>>8)}};}
    next.session.ordinary_pass_candidates=OrdinaryPassCandidateContext{true,select_ordinary_pass_candidates(next.session.current.field.ball_owner,*next.session.human_pass_cursor,positions)};
    next.target_chooser.reset();
   } else {
    auto& index=next.target_chooser->index;
    if(event.pressed&4u){if(index+1<count)++index;}
    else if(event.pressed&8u){if(index)--index;}
    if(event.pressed&128u){next.session=confirm_ordinary_human_pass_recipient(next.session,index);next.target_chooser.reset();}
   }
   next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
  }
  if(!(event.pressed&64u) && (event.pressed&128u) && !input.session.ordinary_pass_candidates) {
   result.unsupported=UnsupportedCapability::action;return result;
  }
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!(event.pressed&64u) && (event.pressed&128u) && input.session.ordinary_pass_candidates->candidates.allied_count>1){
   if(event.held&15u){result.unsupported=UnsupportedCapability::action;return result;}
   auto next=input;advance_demo_random(next);next.target_chooser=MatchTargetChooser{};next.retained_defense_action.reset();next.last_sequence=event.sequence;
   project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
  }
  auto next=input;advance_demo_random(next);
  next.session=(event.pressed&64u) ? cancel_ordinary_human_pass_target(next.session)
    : (event.pressed&128u) ? confirm_ordinary_human_pass_target(next.session)
    : poll_ordinary_human_pass_cursor(next.session,event.held,event.pressed).state;
  const auto direction=event.held&15u;
  if((event.pressed&64u) || (direction!=0 && direction!=1 && direction!=2 && direction!=4 && direction!=8) ||
     ((event.pressed&128u) && direction!=0) ||
     (next.session.ordinary_pass_recipient && *next.session.ordinary_pass_recipient!=255))next.retained_defense_action.reset();
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);
  result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo &&
    ((input.session.phase==FieldSessionPhase::ordinary_human_pass_confirmed && input.session.current.field.selected_defenders) ||
     input.session.phase==FieldSessionPhase::ordinary_pass_defense_selection || input.session.phase==FieldSessionPhase::ordinary_pass_defenses_recorded))
  return handle_contested_human_pass(input,event,content);
 if(input.mode==MatchTimingMode::provisional_demo &&
    ((input.session.phase>=FieldSessionPhase::ordinary_pass_defender_prepared && input.session.phase<=FieldSessionPhase::ordinary_pass_next_contact_clock) ||
     (input.session.phase>=FieldSessionPhase::ordinary_pass_dispatch_clock && input.session.phase<=FieldSessionPhase::ordinary_free_pass_contest_winner)))
  return handle_contested_pass_progress(input,event,content);
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_human_pass_confirmed &&
    !input.session.current.field.selected_defenders && event.kind==MatchInputKind::tick) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(content.data->field_players.skills.empty() || content.data->attack.moves[1].empty()) {result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  next.session=prepare_ordinary_unopposed_pass(next.session,content.data->field_players,content.data->attack);
  next.pass_stage=MatchPassStage::awaiting_presentation;
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);
  result.state=std::move(next);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::awaiting_presentation) {
  if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
  if(input.session.current.field.selected_defenders) {
   if(!input.session.recovery || !input.session.control || !input.session.ordinary_pass_defenses ||
      input.session.ordinary_pass_defenses->count!=input.session.current.field.selected_defenders || input.session.recovery->ball_variant ||
      input.session.ordinary_pass_defenses->actions[0]>2 || input.session.ordinary_pass_defenses->subtypes[0]!=0) {
    result.unsupported=UnsupportedCapability::defender_contact;return result;
   }
   auto next=input;
   next.session.control->facing=64;
   next.session.control->motion_requested=0;
   const auto variant=static_cast<std::uint8_t>(1+(next.session.current.field.random_first&1));
   try {next.session=prepare_first_ordinary_pass_defender(next.session,variant,content.data->field_players,content.data->defense);}
   catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::defender_contact;return result;}
   next.pass_stage=MatchPassStage::none;next.last_sequence=event.sequence;
   project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
  }
  if(!input.session.recovery || input.session.recovery->ball_variant || input.session.current.field.selected_defenders) {
   result.unsupported=UnsupportedCapability::phase;return result;
  }
  if(input.session.current.field.elapsed){
   if(!content.data->movement){result.unsupported=UnsupportedCapability::content;return result;}
   const auto clock=advance_match_clock(input.session.current.clock,0,input.session.current.field.random_first);
   if(clock.continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
   try {
    result.state.session.current.field=advance_field_phase(input.session.current.field,content.data->field_players,*content.data->movement).state;
    result.state.session.current.clock=clock.state;
   } catch(const std::invalid_argument&){result.state=input;result.unsupported=UnsupportedCapability::action;return result;}
  }
  result.state.session.recovery->ball_variant=1;
  validate_phase_session(result.state.session);
  result.state.pass_stage=MatchPassStage::handoff;result.state.last_sequence=event.sequence;
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::handoff && event.kind==MatchInputKind::tick) {
  validate_phase_session(input.session);
  if(!content.pass_distance){result.unsupported=UnsupportedCapability::content;return result;}
  const auto& field=input.session.current.field;
  if(field.elapsed || !input.session.ordinary_pass_recipient || !input.session.human_pass_cursor ||
     (*input.session.ordinary_pass_recipient!=255 && (*input.session.ordinary_pass_recipient<1 || *input.session.ordinary_pass_recipient>10))) {
   result.unsupported=UnsupportedCapability::phase;return result;
  }
  const auto recipient=*input.session.ordinary_pass_recipient;
  const auto* player=recipient==255 ? nullptr : &field.motion[field_motion_index(recipient)];
  const auto destination=resolve_pass_target_cell(recipient,*input.session.human_pass_cursor,
      player ? static_cast<std::uint8_t>(player->x>>8) : 0,player ? static_cast<std::uint8_t>(player->y>>8) : 0);
  if(destination==255){result.unsupported=UnsupportedCapability::action;return result;}
  const auto distance=calculate_pass_distance(field.ball_x,field.ball_y,destination,*content.pass_distance);
  result.state.pass_target=MatchPassTarget{recipient,destination,255,distance};
  result.state.pass_stage=MatchPassStage::target_ready;result.state.last_sequence=event.sequence;
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::target_ready && event.kind==MatchInputKind::tick) {
  validate_phase_session(input.session);
  if((!input.pass_target || input.session.current.field.ball_cell!=input.pass_target->destination) && (!content.data->movement || !content.ball_velocity)){result.unsupported=UnsupportedCapability::content;return result;}
  if(!input.ball_fraction){result.unsupported=UnsupportedCapability::ball_fraction;return result;}
  if(!input.pass_target || !input.session.ordinary_pass_recipient || !input.session.recovery || input.pass_target->recipient!=*input.session.ordinary_pass_recipient ||
     input.pass_target->destination>=240 || input.pass_target->counter!=255){
   result.unsupported=UnsupportedCapability::phase;return result;
  }
  const auto& field=input.session.current.field;
  BallTrajectoryContext trajectory{static_cast<std::uint16_t>((unsigned(field.ball_x)<<8)|input.ball_fraction->x),
    static_cast<std::uint16_t>((unsigned(field.ball_y)<<8)|input.ball_fraction->y),0,0,0,1};
  if(field.ball_cell==input.pass_target->destination){
   if(input.pass_target->recipient!=255){result.unsupported=UnsupportedCapability::phase;return result;}
   trajectory.direction=input.session.recovery->x_jitter_control;
   result.state.pass_trajectory=trajectory;result.state.pass_cell=field.ball_cell;result.state.session.saved_ball_flight_flag=1;
   result.state.pass_stage=MatchPassStage::flight_arrived;result.state.last_sequence=event.sequence;return result;
  }
  trajectory.direction=calculate_player_direction({trajectory.x,trajectory.y,input.pass_target->destination,0},field.ball_cell,content.data->movement->geometry);
  const auto velocity=calculate_ball_velocity(trajectory.direction,*content.ball_velocity);
  trajectory.vx=velocity.x;trajectory.vy=velocity.y;
  result.state.pass_trajectory=trajectory;result.state.pass_cell=field.ball_cell;result.state.session.recovery->x_jitter_control=trajectory.direction;
  result.state.session.saved_ball_flight_flag=1;result.state.pass_stage=MatchPassStage::flight_ready;
  result.state.last_sequence=event.sequence;validate_phase_session(result.state.session);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::flight_ready && event.kind==MatchInputKind::tick) {
  validate_phase_session(input.session);
  if(!input.pass_target || !input.pass_trajectory || !input.pass_cell){result.unsupported=UnsupportedCapability::phase;return result;}
  const auto& t=*input.pass_trajectory;
  const auto step=advance_ball_to_next_cell({t.x,t.y,{t.vx,t.vy},*input.pass_cell,input.pass_target->destination});
  result.state.pass_trajectory->x=step.x;result.state.pass_trajectory->y=step.y;result.state.pass_cell=step.cell;
  result.state.pass_stage=step.event==BallStepEvent::cell_changed ? MatchPassStage::flight_cell : MatchPassStage::flight_arrived;
  result.state.last_sequence=event.sequence;return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::flight_cell && event.kind==MatchInputKind::tick) {
  validate_phase_session(input.session);
  if(!input.pass_cell || !input.pass_target || !input.pass_trajectory){result.unsupported=UnsupportedCapability::phase;return result;}
  const auto& field=input.session.current.field;FlightDefenderSelectionInput selection{};
  selection.possession_side=field.side;selection.ball_cell=*input.pass_cell;selection.random_first=field.random_first;
  selection.random_second=field.random_second;selection.threshold=input.pass_target->counter;
  for(unsigned i=0;i<field.motion.size();++i){const auto& player=field.motion[i];
   selection.players[i]={static_cast<std::uint8_t>(player.x>>8),static_cast<std::uint8_t>(player.y>>8),player.status!=0};}
  result.state.flight_defenders=accumulate_flight_defenders(selection,input.flight_defenders);
  if(!result.state.flight_defenders.empty())result.state.retained_defense_action.reset();
  result.state.pass_stage=MatchPassStage::flight_ready;result.state.last_sequence=event.sequence;return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::flight_arrived) {
  if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
  if(!input.flight_defenders.empty())return resolve_pass_flight_contact(input,event,content);
  validate_phase_session(input.session);
  if(!input.pass_target || !input.pass_trajectory || !input.pass_cell || !input.session.ordinary_pass_recipient ||
     input.pass_target->recipient!=*input.session.ordinary_pass_recipient || input.pass_target->counter!=255 || input.pass_trajectory->flags!=1 ||
     *input.pass_cell!=input.pass_target->destination || input.session.current.field.selected_defenders){result.unsupported=UnsupportedCapability::phase;return result;}
  const auto recipient=input.pass_target->recipient;
  if(recipient==255)return resolve_free_pass_arrival(input,event,content);
  if(recipient<1 || recipient>10){result.unsupported=UnsupportedCapability::action;return result;}
  const auto& player=input.session.current.field.motion[field_motion_index(recipient)];
  const auto x=static_cast<std::uint8_t>(input.pass_trajectory->x>>8),y=static_cast<std::uint8_t>(input.pass_trajectory->y>>8);
  if((player.x>>8)!=x || (player.y>>8)!=y){result.unsupported=UnsupportedCapability::phase;return result;}
  auto& state=result.state;state.session.current.field.ball_owner=recipient;state.session.current.field.ball_x=x;state.session.current.field.ball_y=y;state.session.current.field.ball_cell=*input.pass_cell;
  state.session.phase=FieldSessionPhase::player_control;state.session.human_pass_cursor.reset();state.session.human_pass_steps=0;
  state.session.ordinary_pass_candidates.reset();state.session.ordinary_pass_recipient.reset();state.session.ordinary_pass_defenses.reset();
  state.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(input.pass_trajectory->x),static_cast<std::uint8_t>(input.pass_trajectory->y)};
  state.pass_target.reset();state.pass_trajectory.reset();state.pass_cell.reset();state.pass_stage=MatchPassStage::none;
  state.last_sequence=event.sequence;validate_phase_session(state.session);project_field_random_memory(state.session.current.field,state.random_memory);return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::control_clock_pending && event.kind==MatchInputKind::tick) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->world_step){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  auto clock=advance_control_tick(next.session,content.data->field_players,content.data->goalkeepers,*content.data->world_step);
  next.session=clock.state;next.last_sequence=event.sequence;
  if(next.session.phase!=FieldSessionPhase::field_round_pending && next.session.phase!=FieldSessionPhase::control_ready)next.retained_defense_action.reset();
  if(next.session.phase!=FieldSessionPhase::field_round_pending && next.session.phase!=FieldSessionPhase::control_ready && !(next.session.phase==FieldSessionPhase::field_contact_pending))next.ball_fraction.reset();
  project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);
  for(const auto& request:clock.presentation)result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::field_round_pending && event.kind==MatchInputKind::tick) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->movement || !content.data->control_movement || !content.data->world_step){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  std::array<LogicalButtons,22> buttons{};for(auto& item:buttons)item.held=event.held;buttons[0].pressed=event.pressed;
  auto round=advance_control_field_round(next.session,content.data->field_players,*content.data->movement,*content.data->control_movement,*content.data->world_step,buttons);
  next.session=round.state;next.last_sequence=event.sequence;
  if(next.session.phase!=FieldSessionPhase::control_ready || next.session.current.field.selected_defenders)next.retained_defense_action.reset();
  if((next.session.phase!=FieldSessionPhase::control_ready || next.session.current.field.selected_defenders) && !(next.session.phase==FieldSessionPhase::field_contact_pending))next.ball_fraction.reset();
  project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);
  for(const auto& request:round.presentation)result.events.push_back({request});
  return result;
 }
 if(input.session.phase==FieldSessionPhase::field_contact_pending && input.session.current.field.side==11){
  result.unsupported=UnsupportedCapability::defensive_prompt;return result;
 }
 if(input.session.phase==FieldSessionPhase::field){result.unsupported=UnsupportedCapability::initialization;return result;}
 if(event.kind==MatchInputKind::action)result.unsupported=UnsupportedCapability::action;
 else if(event.kind==MatchInputKind::presentation_complete)result.unsupported=UnsupportedCapability::presentation;
 else if(input.session.phase==FieldSessionPhase::control_ready)result.unsupported=input.random_index ? UnsupportedCapability::timing : UnsupportedCapability::random_state;
 else result.unsupported=UnsupportedCapability::phase;
 return result;
}
}

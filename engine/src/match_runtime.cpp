#include "captain/match_runtime.hpp"
#include "captain/control_session.hpp"
#include "captain/ball_step.hpp"
#include "captain/defender_selection.hpp"
#include "captain/field_player_profile.hpp"
#include "captain/action_strength.hpp"
#include "captain/ball_recovery.hpp"
#include <stdexcept>
namespace captain {
namespace {
void advance_demo_random(MatchRuntimeState& state) {
 auto random=state.demo_rng;random^=random<<13;random^=random>>17;random^=random<<5;
 state.demo_rng=random;state.session.current.field.random_first=static_cast<std::uint8_t>(random);
 state.session.current.field.random_second=static_cast<std::uint8_t>(random>>8);
}
}
const char* capability_name(UnsupportedCapability c) {
 switch(c) {case UnsupportedCapability::content:return "content";case UnsupportedCapability::initialization:return "match initialization";case UnsupportedCapability::input_sequence:return "input sequence";case UnsupportedCapability::ball_fraction:return "ball fractional position is unknown";case UnsupportedCapability::random_state:return "random index is not present in development snapshot";case UnsupportedCapability::timing:return "live timing and random scheduling";case UnsupportedCapability::action:return "action routing";case UnsupportedCapability::defender_contact:return "pass-flight defender contact";case UnsupportedCapability::defensive_prompt:return "opponent contact defensive prompt";case UnsupportedCapability::opponent_action:return "opponent attack choice";case UnsupportedCapability::opponent_execution:return "opponent attack execution and defensive resolution";case UnsupportedCapability::period_transition:return "period or added-time transition";case UnsupportedCapability::presentation:return "presentation completion";case UnsupportedCapability::phase:return "session phase";} return "unknown";
}
MatchBallView match_ball_view(const MatchRuntimeState& state) {
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
 result.state.session=*config.initial;
 result.state.random_index=config.random_index;
 result.state.mode=config.mode;result.state.demo_rng=config.seed;result.state.ball_fraction=config.ball_fraction;
 project_field_random_memory(result.state.session.current.field,result.state.random_memory);
 return result;
}
MatchStepResult step_match(const MatchRuntimeState& input,const MatchInput& event,const MatchContent& content) {
 MatchStepResult result{input};
 if(!content.data){result.unsupported=UnsupportedCapability::content;return result;}
 if(event.sequence<=input.last_sequence){result.unsupported=UnsupportedCapability::input_sequence;return result;}
 if(input.mode==MatchTimingMode::provisional_demo && input.session.current.field.side==11 &&
    (input.session.phase==FieldSessionPhase::field_contact_pending || (input.session.phase==FieldSessionPhase::field && input.defense_menu && static_cast<unsigned>(input.defense_menu->stage)>=static_cast<unsigned>(MatchDefenseStage::ai_flight_ready)))) {
  const auto count=input.session.current.field.selected_defenders;
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::dispatch_pending){
   if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
   if(!content.data->movement){result.unsupported=UnsupportedCapability::content;return result;}
   auto next=input;
   try {
    next.session.current.field=advance_field_phase(input.session.current.field,content.data->field_players,*content.data->movement).state;
    if(next.session.action && next.session.action->action==1){
     if(!next.defense_menu->ai_pass_target)throw std::invalid_argument("Missing opponent pass target");
     const auto recipient=next.defense_menu->ai_pass_target->recipient;
     const auto& player=next.session.current.field.motion[field_motion_index(recipient)];
     const auto cell=field_cell_at(static_cast<std::uint8_t>(player.x>>8),static_cast<std::uint8_t>(player.y>>8));
     if(cell==255)throw std::invalid_argument("Opponent pass recipient outside field");
     next.defense_menu->ai_pass_target->cell=cell;next.defense_menu->stage=MatchDefenseStage::ai_pass_handoff;
    } else {next.session.phase=FieldSessionPhase::player_control;next.defense_menu.reset();validate_phase_session(next.session);}
   } catch(const std::invalid_argument&){result.unsupported=UnsupportedCapability::opponent_execution;return result;}
   next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
  }
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::attack_prepared){
   if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
   if(input.session.action && input.session.action->action==1){
    if(!input.session.control || !input.session.recovery){result.unsupported=UnsupportedCapability::phase;return result;}
    result.state.session.recovery->ball_variant=1;result.state.session.control->last_carrier_cell=128;
   }
   result.state.defense_menu->stage=MatchDefenseStage::clock_pending;result.state.last_sequence=event.sequence;return result;
  }
  if(input.defense_menu && input.defense_menu->stage==MatchDefenseStage::ai_flight_arrived){
   if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
   if(!input.flight_defenders.empty()){
    const auto& data=*content.data;const auto& field=input.session.current.field;
    if(!data.pass_flight_comparison || !data.contact || !data.experience || !input.session.experience || !input.session.action || !input.session.recovery || !input.defense_menu->ai_trajectory || !input.defense_menu->ai_ball_cell){result.unsupported=UnsupportedCapability::content;return result;}
    const auto id=input.flight_defenders.front();if(id<1 || id>10 || field.ball_owner<12 || field.ball_owner>21){result.unsupported=UnsupportedCapability::defender_contact;return result;}
    try {
     const auto& player=field.roster[id];const FieldPlayerRecord record{id,player.type,static_cast<std::uint8_t>(player.stamina),static_cast<std::uint8_t>(player.stamina>>8),player.level};
     const auto prep=prepare_profiled_field_defender(record,data.field_players,2,0,0,data.defense);const auto& rules=*data.pass_flight_comparison;
     const auto contact=compare_defender_contact({input.session.action->power,prep.base_strength,field.random_first,field.random_second,rules.range,rules.offset,false,bool(rules.eligibility&128u),0,id,0,rules.thresholds});
     if(prep.move_effect || contact.subtype){result.unsupported=UnsupportedCapability::defender_contact;return result;}
     if(contact.result_code==2){
      if(!data.movement){result.unsupported=UnsupportedCapability::content;return result;}
      if(field.elapsed || data.contact->field_elapsed[5]!=1 || !input.session.scheduling){result.unsupported=UnsupportedCapability::defender_contact;return result;}
      if((input.session.recovery->special_owner_flags&128u) && field.ball_owner==20){result.unsupported=UnsupportedCapability::defender_contact;return result;}
      auto next=input;auto& session=next.session;const auto trajectory=*next.defense_menu->ai_trajectory;
      const auto& defender=field.motion[field_motion_index(id)];session.current.field.ball_x=static_cast<std::uint8_t>(defender.x>>8);session.current.field.ball_y=static_cast<std::uint8_t>(defender.y>>8);
      session.current.field.ball_cell=*next.defense_menu->ai_ball_cell;session.recovery->opposing_player=id;
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
      session.phase=FieldSessionPhase::player_control;next.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(trajectory.x),static_cast<std::uint8_t>(trajectory.y)};
      next.defense_menu.reset();next.flight_defenders.clear();next.last_sequence=event.sequence;validate_phase_session(session);
      project_field_random_memory(session.current.field,next.random_memory);result.state=std::move(next);return result;
     }
     if(contact.result_code==3){
      if(!data.movement){result.unsupported=UnsupportedCapability::content;return result;}
      if(field.elapsed || data.contact->field_elapsed[5]!=1 || !input.session.scheduling){result.unsupported=UnsupportedCapability::defender_contact;return result;}
      auto next=input;auto& session=next.session;const auto trajectory=*next.defense_menu->ai_trajectory;
      session.current.field.ball_x=static_cast<std::uint8_t>(trajectory.x>>8);session.current.field.ball_y=static_cast<std::uint8_t>(trajectory.y>>8);session.current.field.ball_cell=*next.defense_menu->ai_ball_cell;
      session.current.field.selected_defenders=static_cast<std::uint8_t>(next.flight_defenders.size());session.scheduling->selected_ids.fill(0);
      for(unsigned i=0;i<next.flight_defenders.size();++i)session.scheduling->selected_ids[i]=next.flight_defenders[i];
      session.recovery->opposing_player=id;session.experience=award_player_experience(*session.experience,player.type,2,*data.experience).state;
      auto clock=apply_contact_prelude({session.current,id,2,false},*data.contact);
      if(clock.clock.continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
      session.current=clock.state;session.current.field.side=0;session.current.field.ball_owner=id;
      session.current.field=advance_field_phase(session.current.field,data.field_players,*data.movement).state;
      session.phase=FieldSessionPhase::player_control;next.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(trajectory.x),static_cast<std::uint8_t>(trajectory.y)};
      next.defense_menu.reset();next.flight_defenders.clear();next.last_sequence=event.sequence;validate_phase_session(session);
      project_field_random_memory(session.current.field,next.random_memory);result.state=std::move(next);return result;
     }
     auto next=input;auto& session=next.session;const auto& trajectory=*next.defense_menu->ai_trajectory;
     session.current.field.ball_x=static_cast<std::uint8_t>(trajectory.x>>8);session.current.field.ball_y=static_cast<std::uint8_t>(trajectory.y>>8);session.current.field.ball_cell=*next.defense_menu->ai_ball_cell;
     session.recovery->opposing_player=id;
     if(contact.result_code==1){session.action->power=reduce_action_strength(session.action->power,contact.loss);
      session.experience=award_player_experience(*session.experience,field.roster[field.ball_owner].type,1,*data.experience).state;}
     auto clock=apply_contact_prelude({session.current,id,2,true},*data.contact);
     if(clock.clock.continuation!=ClockContinuation::ready){result.unsupported=UnsupportedCapability::period_transition;return result;}
     session.current=clock.state;next.flight_defenders.erase(next.flight_defenders.begin());
     if(next.flight_defenders.empty())session.current.field.elapsed=0;
     next.last_sequence=event.sequence;project_field_random_memory(session.current.field,next.random_memory);result.state=std::move(next);return result;
    } catch(const std::invalid_argument&) {result.unsupported=UnsupportedCapability::defender_contact;return result;}
   }
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
   if(!content.data->area_prefix_elapsed){result.unsupported=UnsupportedCapability::content;return result;}
   if(!input.session.action || (input.session.action->action!=1 && input.session.action->action!=2)){result.unsupported=UnsupportedCapability::phase;return result;}
   auto next=input;auto& current=next.session.current;const auto elapsed=(*content.data->area_prefix_elapsed)[input.session.action->action];
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
   if(!input.session.action || !input.session.recovery || input.session.recovery->ball_variant || (input.session.action->action!=1 && input.session.action->action!=2)){result.unsupported=UnsupportedCapability::phase;return result;}
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
   result.state.defense_menu->stage=MatchDefenseStage::attack_prepared;result.state.last_sequence=event.sequence;return result;
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
    const auto plan=plan_ai_field_pass_candidate(actor.type,static_cast<std::uint8_t>(actor.stamina),static_cast<std::uint8_t>(actor.stamina>>8),owner,
     field.ball_x,field.ball_y,next.session.current.field.random_first,next.session.current.field.random_second,positions,*content.data->ai_field_choice,*content.data->ai_area_choice);
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
  if(!input.demo_rng || !input.session.scheduling || !count || count>4){result.unsupported=UnsupportedCapability::phase;return result;}
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
  if(poll.boundary==DefenseMenuBoundary::choice_confirmed)next.defense_menu->stage=MatchDefenseStage::choice_confirmed;
  else if(poll.boundary==DefenseMenuBoundary::complete)next.defense_menu->stage=MatchDefenseStage::complete;
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);
  for(const auto& request:poll.presentation)result.events.push_back({request});
  return result;
 }
 if(input.session.phase==FieldSessionPhase::player_control && event.kind==MatchInputKind::tick) {
  auto prepared=prepare_session_control(input.session);
  result.state.session=prepared.state;result.state.last_sequence=event.sequence;
  if(result.state.session.current.field.ball_x!=input.session.current.field.ball_x || result.state.session.current.field.ball_y!=input.session.current.field.ball_y)result.state.ball_fraction.reset();
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);
  for(const auto& request:prepared.presentation) result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && event.kind==MatchInputKind::tick && input.session.current.field.side==0 &&
    (input.session.phase==FieldSessionPhase::action_menu_pending || input.session.phase==FieldSessionPhase::field_contact_pending || input.session.phase==FieldSessionPhase::human_action_menu)) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->action_menu){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  auto menu=(next.session.phase==FieldSessionPhase::action_menu_pending || next.session.phase==FieldSessionPhase::field_contact_pending)
    ? open_human_action_menu(next.session,*content.data->action_menu)
    : poll_human_action_menu(next.session,event.pressed,*content.data->action_menu);
  next.session=menu.state;next.last_sequence=event.sequence;
  project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);
  for(const auto& request:menu.presentation)result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::control_ready && event.kind==MatchInputKind::tick) {
  if(!input.demo_rng){result.unsupported=UnsupportedCapability::random_state;return result;}
  if(!content.data->movement || !content.data->control_movement){result.unsupported=UnsupportedCapability::content;return result;}
  auto next=input;advance_demo_random(next);
  auto poll=poll_session_control(next.session,event.held,event.pressed,content.data->field_players,*content.data->movement,*content.data->control_movement);
  next.session=poll.state;next.last_sequence=event.sequence;
  if((poll.moved_player && *poll.moved_player==input.session.current.field.ball_owner) || next.session.current.field.ball_x!=input.session.current.field.ball_x || next.session.current.field.ball_y!=input.session.current.field.ball_y)next.ball_fraction.reset();
  project_field_random_memory(next.session.current.field,next.random_memory);
  result.state=std::move(next);
  for(const auto& request:poll.presentation)result.events.push_back({request});
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::attack_choice_pending &&
    input.session.current.field.side==0 && input.session.action && input.session.action->action==1 &&
    input.session.action->menu_route==0 && event.kind==MatchInputKind::action && event.action==MatchAction::pass && !event.target) {
  result.state.session=begin_ordinary_human_pass_target(input.session);
  if(result.state.session.current.field.ball_x!=input.session.current.field.ball_x || result.state.session.current.field.ball_y!=input.session.current.field.ball_y)result.state.ball_fraction.reset();
  result.state.last_sequence=event.sequence;
  project_field_random_memory(result.state.session.current.field,result.state.random_memory);
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_human_pass_target && event.kind==MatchInputKind::tick) {
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
   auto next=input;advance_demo_random(next);next.target_chooser=MatchTargetChooser{};next.last_sequence=event.sequence;
   project_field_random_memory(next.session.current.field,next.random_memory);result.state=std::move(next);return result;
  }
  auto next=input;advance_demo_random(next);
  next.session=(event.pressed&64u) ? cancel_ordinary_human_pass_target(next.session)
    : (event.pressed&128u) ? confirm_ordinary_human_pass_target(next.session)
    : poll_ordinary_human_pass_cursor(next.session,event.held,event.pressed).state;
  next.last_sequence=event.sequence;project_field_random_memory(next.session.current.field,next.random_memory);
  result.state=std::move(next);return result;
 }
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
  if(!input.session.recovery || input.session.recovery->ball_variant || input.session.current.field.selected_defenders) {
   result.unsupported=UnsupportedCapability::phase;return result;
  }
  result.state.session.recovery->ball_variant=1;
  validate_phase_session(result.state.session);
  result.state.pass_stage=MatchPassStage::handoff;result.state.last_sequence=event.sequence;
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::handoff && event.kind==MatchInputKind::tick) {
  validate_phase_session(input.session);
  if(!content.pass_distance){result.unsupported=UnsupportedCapability::content;return result;}
  const auto& field=input.session.current.field;
  if(field.elapsed || !input.session.ordinary_pass_recipient || !input.session.human_pass_cursor ||
     *input.session.ordinary_pass_recipient==255 || *input.session.ordinary_pass_recipient<1 || *input.session.ordinary_pass_recipient>10) {
   result.unsupported=UnsupportedCapability::phase;return result;
  }
  const auto recipient=*input.session.ordinary_pass_recipient;
  const auto& player=field.motion[field_motion_index(recipient)];
  const auto destination=resolve_pass_target_cell(recipient,*input.session.human_pass_cursor,
      static_cast<std::uint8_t>(player.x>>8),static_cast<std::uint8_t>(player.y>>8));
  if(destination==255){result.unsupported=UnsupportedCapability::action;return result;}
  const auto distance=calculate_pass_distance(field.ball_x,field.ball_y,destination,*content.pass_distance);
  result.state.pass_target=MatchPassTarget{recipient,destination,255,distance};
  result.state.pass_stage=MatchPassStage::target_ready;result.state.last_sequence=event.sequence;
  return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::target_ready && event.kind==MatchInputKind::tick) {
  validate_phase_session(input.session);
  if(!content.data->movement || !content.ball_velocity){result.unsupported=UnsupportedCapability::content;return result;}
  if(!input.ball_fraction){result.unsupported=UnsupportedCapability::ball_fraction;return result;}
  if(!input.pass_target || !input.session.ordinary_pass_recipient || input.pass_target->recipient!=*input.session.ordinary_pass_recipient ||
     input.pass_target->destination>=240 || input.pass_target->counter!=255 || input.session.current.field.ball_cell==input.pass_target->destination){
   result.unsupported=UnsupportedCapability::phase;return result;
  }
  const auto& field=input.session.current.field;
  BallTrajectoryContext trajectory{static_cast<std::uint16_t>((unsigned(field.ball_x)<<8)|input.ball_fraction->x),
    static_cast<std::uint16_t>((unsigned(field.ball_y)<<8)|input.ball_fraction->y),0,0,0,1};
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
  result.state.pass_stage=MatchPassStage::flight_ready;result.state.last_sequence=event.sequence;return result;
 }
 if(input.mode==MatchTimingMode::provisional_demo && input.session.phase==FieldSessionPhase::ordinary_pass_prepared &&
    input.pass_stage==MatchPassStage::flight_arrived) {
  if(event.kind!=MatchInputKind::presentation_complete){result.unsupported=UnsupportedCapability::presentation;return result;}
  if(!input.flight_defenders.empty()){result.unsupported=UnsupportedCapability::defender_contact;return result;}
  validate_phase_session(input.session);
  if(!input.pass_target || !input.pass_trajectory || !input.pass_cell || !input.session.ordinary_pass_recipient ||
     input.pass_target->recipient!=*input.session.ordinary_pass_recipient || input.pass_target->counter!=255 || input.pass_trajectory->flags!=1 ||
     *input.pass_cell!=input.pass_target->destination || input.session.current.field.selected_defenders){result.unsupported=UnsupportedCapability::phase;return result;}
  const auto recipient=input.pass_target->recipient;
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
  next.session=round.state;next.last_sequence=event.sequence;next.ball_fraction.reset();
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

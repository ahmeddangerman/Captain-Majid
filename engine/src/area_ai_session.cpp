#include "captain/area_ai_session.hpp"
#include "captain/player_movement.hpp"
#include <stdexcept>
namespace captain {
PreparedAreaDefender prepare_area_defender(const FieldPlayerRecord& player,std::uint8_t attack,
 std::uint8_t attack_subtype,std::uint8_t variant,std::uint8_t defense,
 std::uint8_t defense_subtype,const ShotContent& content) {
 if(player.player_id<1 || player.player_id>10 || (attack!=1 && attack!=4 && attack!=6) ||
    (variant!=1 && variant!=2) || (defense!=2 && defense!=4) ||
    (defense==2 ? defense_subtype>1 : defense_subtype!=0))throw std::invalid_argument("Unsupported area defender preparation");
 const auto prepared=prepare_profiled_field_defender(player,content.field_players,defense,variant,defense_subtype,content.defense);
 const auto stamina=static_cast<std::uint16_t>(unsigned(player.flags)|(unsigned(player.alternate_type)<<8));
 const auto energy=calculate_energy({stamina,prepared.stamina_cost,player.type,11,attack,attack_subtype,true,false});
 return {prepared,energy};
}
PreparedAreaContact compare_area_defender(const FieldPlayerRecord& player,std::uint8_t attack,
 std::uint8_t attack_subtype,std::uint8_t variant,std::uint8_t defense,
 std::uint8_t defense_subtype,std::uint16_t incoming,std::uint8_t first,
 std::uint8_t second,const ShotContent& content) {
 if(!content.area_comparison)throw std::invalid_argument("Missing private area comparison content");
 const auto prepared=prepare_area_defender(player,attack,attack_subtype,variant,defense,defense_subtype,content);
 const auto selected=select_area_comparison(attack,defense,*content.area_comparison,content.field_comparison.eligibility);
 const auto comparison=compare_defender_contact({incoming,prepared.preparation.base_strength,first,second,selected.range,selected.offset,prepared.energy.weakened,selected.eligible,2,player.player_id,defense_subtype,selected.thresholds});
 return {prepared,comparison};
}
ContactPreludeResult calculate_area_prefix_clock(const PhaseSessionState& input,std::uint8_t elapsed) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_prefix_done)
  throw std::invalid_argument("Area prefix clock requires exhausted defenders");
 auto state=input.current;
 state.field.elapsed=static_cast<std::uint8_t>(unsigned(state.field.elapsed)+elapsed);
 const auto recovery=advance_keeper_recovery({state.field.keepers,state.shared_recovery_flags,state.shared_recovery_timer},elapsed);
 state.field.keepers=recovery.keepers;state.shared_recovery_flags=recovery.shared_flags;state.shared_recovery_timer=recovery.shared_timer;
 const auto clock=advance_match_clock(state.clock,elapsed,state.field.random_first);state.clock=clock.state;
 return {state,clock,elapsed};
}
ActionMenuResult present_session_area_contact_result(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_contact_clock_done)
  throw std::invalid_argument("Area result presentation requires completed contact timing");
 ActionMenuResult result{input};const bool terminal=input.area_contact->result>=2;
 result.presentation.push_back({ControlPresentationKind::script,static_cast<std::uint8_t>(terminal?16:17)});
 result.presentation.push_back({ControlPresentationKind::script,18});
 if(terminal){result.state.control->facing=0;result.state.control->motion_requested=0;}
 result.state.phase=FieldSessionPhase::keeper_area_contact_presented;
 validate_phase_session(result.state);return result;
}
ActionMenuResult advance_session_area_defender(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_contact_presented)
  throw std::invalid_argument("Area defender advance requires completed result presentation");
 ActionMenuResult result{input};auto& state=result.state;
 if(state.area_contact->result>=2)state.phase=FieldSessionPhase::keeper_area_terminal_result;
 else {
  ++state.area_defender_cursor;state.area_contact.reset();
  state.phase=state.area_defender_cursor==state.current.field.selected_defenders ?
   FieldSessionPhase::keeper_area_prefix_done : FieldSessionPhase::keeper_area_defender_next;
  if(state.phase==FieldSessionPhase::keeper_area_defender_next)
   result.presentation.push_back({ControlPresentationKind::wait_frames,1});
 }
 validate_phase_session(state);return result;
}
ActionMenuResult skip_session_area_defender(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_defenders_announced && input.phase!=FieldSessionPhase::keeper_area_defender_next)
  throw std::invalid_argument("Area skip requires the next selected defender");
 const auto action=input.area_defense->actions[input.area_defender_cursor];
 if(action!=5 && action!=6)throw std::invalid_argument("Selected area defense requires a contact calculation");
 ActionMenuResult result{input};auto& state=result.state;
 state.recovery->opposing_player=state.scheduling->selected_ids[state.area_defender_cursor];
 ++state.area_defender_cursor;
 state.phase=state.area_defender_cursor==state.current.field.selected_defenders ?
  FieldSessionPhase::keeper_area_prefix_done : FieldSessionPhase::keeper_area_defender_next;
 if(state.phase==FieldSessionPhase::keeper_area_defender_next)
  result.presentation.push_back({ControlPresentationKind::wait_frames,1});
 validate_phase_session(state);return result;
}
PhaseSessionResult advance_session_area_contact_clock(const PhaseSessionState& input,
 const ContactPreludeContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_defender_compared)
  throw std::invalid_argument("Area contact clock requires a saved comparison");
 auto state=input;const auto& contact=*input.area_contact;
 const auto applied=apply_contact_prelude({state.current,state.recovery->opposing_player,
  contact.defense_action,contact.result<2},content);
 state.current=applied.state;
 state.phase=applied.clock.continuation==ClockContinuation::period_end ? FieldSessionPhase::keeper_area_contact_period_end :
  applied.clock.continuation==ClockContinuation::added_time_presentation ? FieldSessionPhase::keeper_area_contact_added_time :
  FieldSessionPhase::keeper_area_contact_clock_done;
 validate_phase_session(state);return {state,applied,std::nullopt,{}};
}
PhaseSessionState resume_session_area_contact_clock(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_contact_added_time)
  throw std::invalid_argument("No area contact added-time presentation to resume");
 auto state=input;state.phase=FieldSessionPhase::keeper_area_contact_clock_done;
 validate_phase_session(state);return state;
}
PhaseSessionState resolve_session_area_defender(const PhaseSessionState& input,const ShotContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_defenders_announced && input.phase!=FieldSessionPhase::keeper_area_defender_next)throw std::invalid_argument("Area contact requires the next announced defender");
 const auto cursor=input.area_defender_cursor;const auto id=input.scheduling->selected_ids[cursor];const auto& p=input.current.field.roster[id];
 const auto defense=input.area_defense->actions[cursor];
 const auto result=compare_area_defender({id,p.type,static_cast<std::uint8_t>(p.stamina),static_cast<std::uint8_t>(p.stamina>>8),p.level},input.action->action,input.recovery->attack_subtype,input.recovery->ball_variant,defense,input.area_defense->subtypes[cursor],input.action->power,input.current.field.random_first,input.current.field.random_second,content);
 auto state=input;state.current.field.roster[id].stamina=result.defender.energy.stamina;
 state.recovery->opposing_player=id;const auto& c=result.comparison;
 state.area_contact=AreaContactContext{cursor,defense,c.subtype,result.defender.preparation.move_effect,c.defender_power,result.defender.energy.cost,c.strength,c.loss,c.score,c.result_code,result.defender.energy.weakened};
 state.phase=FieldSessionPhase::keeper_area_defender_compared;validate_phase_session(state);return state;
}
ActionMenuResult announce_prepared_area_defenders(const PhaseSessionState& input,std::uint8_t facing) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_defense_entry || (facing!=0 && facing!=64))throw std::invalid_argument("Defender announcements require prepared entry and captured facing");
 ActionMenuResult result{input};auto& state=result.state;
 for(unsigned slot=0;slot<state.current.field.selected_defenders;++slot) {
  const auto action=state.area_defense->actions[slot];
  if(action!=2 && action!=4 && action!=5 && action!=6)throw std::invalid_argument("Unrepresented area defender presentation action");
  if(action==6)continue;
  state.recovery->opposing_player=state.scheduling->selected_ids[slot];
  result.presentation.push_back({ControlPresentationKind::script,15});
  result.presentation.push_back({ControlPresentationKind::wait_frames,20});
 }
 result.presentation.push_back({ControlPresentationKind::script,4});
 result.presentation.push_back({ControlPresentationKind::wait_frames,1});
 state.control->facing=facing;state.control->motion_requested=16;
 state.phase=FieldSessionPhase::keeper_area_defenders_announced;
 validate_phase_session(state);return result;
}
ActionMenuResult begin_prepared_area_defense(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_attack_prepared)throw std::invalid_argument("Area defense entry requires prepared attack");
 ActionMenuResult result{input};result.presentation.push_back({ControlPresentationKind::script,14});
 result.state.control->motion_requested=input.recovery->ball_variant==1 ? 2 : 0;result.state.phase=FieldSessionPhase::keeper_area_defense_entry;
 validate_phase_session(result.state);return result;
}
PhaseSessionState prepare_session_area_attack(const PhaseSessionState& input,const ShotContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_ai_chosen)throw std::invalid_argument("Area attack requires a saved opponent choice");
 const auto id=input.current.field.ball_owner;const auto& p=input.current.field.roster[id];
 const auto result=prepare_opponent_area_action({id,p.type,static_cast<std::uint8_t>(p.stamina),static_cast<std::uint8_t>(p.stamina>>8),p.level},input.action->action,input.recovery->ball_variant,input.recovery->attack_subtype,input.current.field.random_first,input.current.field.random_second,content);
 auto state=input;state.action->power=result.action_power.power;state.action->move_effect=result.preparation.move_effect;
 state.action->move_flags=result.move_flags;state.recovery->attack_subtype=result.action_power.subtype;
 state.area_ai_target->cost=result.preparation.stamina_cost;state.phase=FieldSessionPhase::keeper_area_attack_prepared;
 validate_phase_session(state);return state;
}
PreparedAttackResult prepare_opponent_area_action(const FieldPlayerRecord& player,
 std::uint8_t action,std::uint8_t variant,std::uint8_t subtype,std::uint8_t first,
 std::uint8_t second,const ShotContent& content) {
 if(player.player_id<12 || player.player_id>21 || (variant!=1 && variant!=2) ||
    (action!=1 && action!=4 && action!=6) || (action==6 ? subtype>1 : subtype!=0))
  throw std::invalid_argument("Unsupported opponent area attack preparation");
 if(action==6 && !content.area_attack6)throw std::invalid_argument("Missing private area attack6 content");
 // Action6 uses its private selectors/moves in slot0 of the shared numeric loader.
 // Supported subtypes0/1 do not trigger the ordinary-shot subtype override.
 return prepare_attacking_action({player,static_cast<std::uint8_t>(action==6?0:action),variant,subtype,11,0,first,second,false},
  content.field_players,action==6?*content.area_attack6:content.attack);
}
PhaseSessionState choose_session_area_action(const PhaseSessionState& input,const AiAreaChoiceContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_defense_complete)throw std::invalid_argument("Area AI requires completed defensive choices");
 const auto& f=input.current.field;const auto& player=f.roster[f.ball_owner];
 std::array<std::array<std::uint8_t,2>,10> positions{};
 for(unsigned id=12;id<=21;++id){const auto& m=f.motion[field_motion_index(id)];positions[id-12]={{static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8)}};}
 const auto plan=plan_ai_area_action(player.type,static_cast<std::uint8_t>(player.stamina),static_cast<std::uint8_t>(player.stamina>>8),f.ball_owner,f.ball_x,f.ball_y,f.random_first,f.random_second,input.recovery->ball_variant,positions,content);
 auto state=input;state.action->action=plan.action;state.recovery->attack_subtype=plan.subtype;state.current.field.random_first=plan.random_first;
 state.area_ai_target=PhaseSessionState::AreaAiTarget{plan.recipient.value_or(0),plan.cell.value_or(0)};
 state.phase=FieldSessionPhase::keeper_area_ai_chosen;validate_phase_session(state);return state;
}
}

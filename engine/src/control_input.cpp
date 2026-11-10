#include "captain/control_input.hpp"
#include "captain/defender_selection.hpp"
#include "captain/prepared_attack.hpp"
#include "captain/field_state_io.hpp"
#include <sstream>
#include <stdexcept>
#include <utility>

namespace captain {
PhaseSessionState capture_sampled_score24_dribble_defense(const PhaseSessionState& input,
 std::uint8_t action,std::uint8_t subtype) {
 validate_phase_session(input);
 const auto& f=input.current.field;
 if(input.phase!=FieldSessionPhase::attack_choice_pending || !input.action || !input.recovery || !input.control || !input.score || !input.scheduling ||
    input.score->left_goals!=2 || input.score->right_goals!=4 || input.score->kickoff!=4 || input.current.clock.remaining!=342 || input.current.clock.extra_time!=13 ||
    f.side!=0 || f.ball_owner!=5 || f.ball_x!=108 || f.ball_y!=108 || f.ball_cell!=87 || f.elapsed!=1 || f.roster[5].type!=23 || f.roster[5].stamina ||
    f.selected_defenders!=1 || input.scheduling->selected_ids!=std::array<std::uint8_t,5>{17,0,0,0,0} ||
    action!=1 || subtype || input.action->action!=2 || input.action->cursor!=227 || input.area_defender_cursor ||
    f.random_first || f.random_second || input.saved_ball_flight_flag)
  throw std::invalid_argument("Unsupported sampled 2-4 dribble defense choice");
 auto state=input;state.recovery->opposing_player=17;state.recovery->attack_subtype=0;
 state.phase=FieldSessionPhase::sampled_score24_dribble_choice;validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_player5_defense_choices(const PhaseSessionState& input,
 const std::array<std::uint8_t,5>& actions,const std::array<std::uint8_t,5>& subtypes) {
 validate_phase_session(input);
 const auto& f=input.current.field;
 if(input.phase==FieldSessionPhase::attack_choice_pending && !input.human_defense_choices && input.score && input.action && input.recovery && input.scheduling &&
    input.score->left_goals==3 && input.score->right_goals==6 && input.score->kickoff==4 && input.current.clock.remaining==269 &&
    input.current.clock.extra_time==13 && f.side==0 && f.ball_owner==7 && f.ball_x==113 && f.ball_y==143 && f.ball_cell==103 &&
    f.roster[7].type==23 && f.roster[7].stamina==140 && f.selected_defenders==4 &&
    input.scheduling->selected_ids==std::array<std::uint8_t,5>{12,15,16,21,0} &&
    actions==std::array<std::uint8_t,5>{1,1,1,1,1} && subtypes==std::array<std::uint8_t,5>{} &&
    input.action->action==0 && input.action->cursor==224 && input.area_defender_cursor==3 &&
    !f.random_first && !f.random_second && input.saved_ball_flight_flag==1) {
  auto state=input;state.human_defense_choices=HumanDefenseChoices{actions,subtypes};state.recovery->opposing_player=21;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::attack_choice_pending && !input.human_defense_choices && input.score && input.scheduling &&
    input.score->left_goals==2 && input.score->right_goals==5 && input.score->kickoff==4 && input.current.clock.remaining==311 &&
    input.current.clock.extra_time==13 && f.side==0 && f.ball_owner==7 && f.ball_x==117 && f.ball_y==139 && f.ball_cell==103 &&
    f.roster[7].type==23 && f.roster[7].stamina==210 && f.selected_defenders==5 &&
    input.scheduling->selected_ids==std::array<std::uint8_t,5>{12,14,15,16,18} &&
    actions==std::array<std::uint8_t,5>{1,1,1,1,1} && subtypes==std::array<std::uint8_t,5>{} &&
    input.action->action==0 && input.action->cursor==224 && input.area_defender_cursor==3 &&
    !f.random_first && !f.random_second && input.saved_ball_flight_flag==1) {
  auto state=input;state.human_defense_choices=HumanDefenseChoices{actions,subtypes};state.recovery->opposing_player=18;
  validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::attack_choice_pending || input.human_defense_choices || !input.score || !input.scheduling ||
    input.score->left_goals!=2 || input.score->right_goals!=2 || input.score->kickoff!=4 ||
    input.current.clock.remaining!=367 || input.current.clock.extra_time!=13 || f.side!=0 || f.ball_owner!=5 ||
    f.ball_x!=108 || f.ball_y!=108 || f.ball_cell!=87 || f.roster[5].type!=23 || f.roster[5].stamina!=10 ||
    f.selected_defenders!=5 || input.scheduling->selected_ids!=std::array<std::uint8_t,5>{14,16,18,19,20} ||
    actions!=std::array<std::uint8_t,5>{1,1,1,1,1} || subtypes!=std::array<std::uint8_t,5>{} ||
    input.action->action!=2 || input.action->cursor!=227 || input.area_defender_cursor!=1 ||
    f.random_first || f.random_second || input.saved_ball_flight_flag!=1)
  throw std::invalid_argument("Unsupported sampled player5 five-defender choices");
 auto state=input;state.human_defense_choices=HumanDefenseChoices{actions,subtypes};
 state.recovery->opposing_player=20;validate_phase_session(state);return state;
}
PhaseSessionState prepare_sampled_player5_dribble(const PhaseSessionState& input,
 const FieldPlayerProfiles& profiles,const FieldDefenseContent& attack) {
 validate_phase_session(input);
 const bool score24=input.phase==FieldSessionPhase::sampled_score24_dribble_choice;
 if((input.phase!=FieldSessionPhase::attack_choice_pending || !input.human_defense_choices) && !score24)
  throw std::invalid_argument("Player5 dribble needs the captured five-defender choices");
 const auto& f=input.current.field;const auto& p=f.roster[5];
 const auto result=prepare_attacking_action({{5,p.type,static_cast<std::uint8_t>(p.stamina),
  static_cast<std::uint8_t>(p.stamina>>8),p.level},2,0,0,0,f.selected_defenders,f.random_first,f.random_second,false},profiles,attack);
 if(result.preparation.base_strength!=24 || result.preparation.stamina_cost!=40 ||
    !result.energy.weakened || result.energy.stamina || result.action_power.power || result.action_power.subtype ||
    result.preparation.move_effect || result.move_flags)
  throw std::invalid_argument("Unverified sampled player5 dribble preparation: base="+std::to_string(result.preparation.base_strength)+
    " cost="+std::to_string(result.preparation.stamina_cost)+" weakened="+std::to_string(result.energy.weakened)+
    " stamina="+std::to_string(result.energy.stamina)+" power="+std::to_string(result.action_power.power)+
    " subtype="+std::to_string(result.action_power.subtype)+" effect="+std::to_string(result.preparation.move_effect)+
    " flags="+std::to_string(result.move_flags));
 auto state=input;state.current.field.roster[5].stamina=result.energy.stamina;
 state.action->power=result.action_power.power;state.recovery->attack_subtype=result.action_power.subtype;
 state.action->move_effect=result.preparation.move_effect;state.action->move_flags=result.move_flags;
 state.phase=score24 ? FieldSessionPhase::sampled_score24_dribble_prepared : FieldSessionPhase::attack_prepared;validate_phase_session(state);return state;
}
PhaseSessionState begin_sampled_player5_dribble_defenders(const PhaseSessionState& input) {
 validate_phase_session(input);
 const bool player7=input.score && ((input.score->left_goals==2 && input.score->right_goals==5) || (input.score->left_goals==3 && input.score->right_goals==6)) && input.current.field.ball_owner==7;
 if(player7 && input.phase==FieldSessionPhase::attack_prepared && input.human_defense_choices && input.area_contact &&
    input.area_defender_cursor<input.current.field.selected_defenders && input.area_sequence_result==0 &&
    input.current.field.motion[field_motion_index(input.scheduling->selected_ids[input.area_defender_cursor])].status==6) {
  auto state=input;++state.area_defender_cursor;state.area_contact.reset();state.area_sequence_result=255;
  validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices || input.area_defender_cursor!=(player7 ? 3 : 1) ||
    input.current.field.random_first!=0)
  throw std::invalid_argument("Player5 dribble defender prefix needs its observed prepared state");
 auto state=input;state.area_defender_cursor=0;state.control->last_carrier_cell=0;
 validate_phase_session(state);return state;
}
PhaseSessionState prepare_sampled_player5_dribble_defender(const PhaseSessionState& input,
 const FieldPlayerProfiles& profiles,const FieldDefenseContent& defense) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::sampled_score24_dribble_prepared) {
  const auto& p=input.current.field.roster[17];
  const auto prepared=prepare_profiled_field_defender({17,p.type,static_cast<std::uint8_t>(p.stamina),
   static_cast<std::uint8_t>(p.stamina>>8),p.level},profiles,1,0,0,defense);
  if(prepared.base_strength!=18 || prepared.stamina_cost!=60 || prepared.move_effect)
   throw std::invalid_argument("Unverified sampled 2-4 defender content");
  auto state=input;state.control->last_carrier_cell=0;
  state.area_contact=AreaContactContext{0,1,0,0,18,60,0,0,0,0,false};
  state.phase=FieldSessionPhase::sampled_score24_dribble_defender;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::attack_prepared && input.human_defense_choices && input.score &&
    ((input.score->left_goals==2 && input.score->right_goals==5) || (input.score->left_goals==3 && input.score->right_goals==6)) && input.current.field.ball_owner==7 &&
    input.area_defender_cursor<input.current.field.selected_defenders && !input.area_contact) {
  const auto defender=input.scheduling->selected_ids[input.area_defender_cursor];
  const auto& p=input.current.field.roster[defender];
  const auto prepared=prepare_profiled_field_defender({defender,p.type,static_cast<std::uint8_t>(p.stamina),static_cast<std::uint8_t>(p.stamina>>8),p.level},profiles,1,0,0,defense);
  if(prepared.base_strength!=18 || prepared.stamina_cost!=60 || prepared.move_effect)
   throw std::invalid_argument("Unverified player7 first defender content");
  auto state=input;state.recovery->opposing_player=defender;state.recovery->ball_variant=1;
  state.control->last_carrier_cell=128;state.control->motion_requested=input.area_defender_cursor==0 ? 2 : 0;
  state.area_contact=AreaContactContext{input.area_defender_cursor,1,0,prepared.move_effect,prepared.base_strength,prepared.stamina_cost,0,0,0,0,false};
  validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices || input.area_defender_cursor ||
    input.area_contact || input.recovery->opposing_player!=20 || input.current.field.roster[14].type!=23 ||
    input.current.field.roster[14].stamina!=100)
  throw std::invalid_argument("Player5 dribble needs its first observed defender");
 const auto& p=input.current.field.roster[14];
 const auto prepared=prepare_profiled_field_defender({14,p.type,static_cast<std::uint8_t>(p.stamina),
  static_cast<std::uint8_t>(p.stamina>>8),p.level},profiles,1,0,0,defense);
 if(prepared.base_strength!=18 || prepared.stamina_cost!=60 || prepared.move_effect)
  throw std::invalid_argument("Unverified player5 dribble defender content");
 auto state=input;state.recovery->opposing_player=14;
 state.area_contact=AreaContactContext{0,1,0,0,18,60,0,0,0,0,false};
 validate_phase_session(state);return state;
}
PhaseSessionState compare_sampled_player5_dribble_defender(const PhaseSessionState& input,
 const FieldComparisonSelection& comparison) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::attack_prepared && input.human_defense_choices && input.area_contact && input.score &&
    ((input.score->left_goals==2 && input.score->right_goals==5) || (input.score->left_goals==3 && input.score->right_goals==6)) && input.current.field.ball_owner==7) {
  if(input.area_sequence_result!=255 || comparison.table_index!=1)throw std::invalid_argument("Unsupported player7 shot comparison row");
  DefenderContactInput contact{};contact.incoming_power=input.action->power;contact.defender_power=input.area_contact->defender_power;
  contact.random_first=input.current.field.random_first;contact.random_second=input.current.field.random_second;
  contact.range=comparison.range;contact.offset=comparison.offset;contact.eligible=comparison.eligible;
  contact.thresholds=comparison.thresholds;contact.defender_slot=input.recovery->opposing_player;
  const auto result=compare_defender_contact(contact);
  if(result.strength || result.loss || result.score!=255 || result.result_code)
   throw std::invalid_argument("Unverified player7 shot defender comparison");
  auto state=input;state.area_contact->strength=result.strength;state.area_contact->loss=result.loss;
  state.area_contact->score=result.score;state.area_contact->result=result.result_code;state.area_sequence_result=result.result_code;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_score24_dribble_defender) {
  if(comparison.table_index!=9)throw std::invalid_argument("Unsupported sampled 2-4 defender comparison row");
  DefenderContactInput contact{};contact.incoming_power=input.action->power;contact.defender_power=input.area_contact->defender_power;
  contact.random_first=input.current.field.random_first;contact.random_second=input.current.field.random_second;
  contact.range=comparison.range;contact.offset=comparison.offset;contact.eligible=comparison.eligible;
  contact.thresholds=comparison.thresholds;contact.defender_slot=17;
  const auto result=compare_defender_contact(contact);
  if(result.strength!=4 || result.loss!=1 || result.score!=0 || result.result_code!=3)
   throw std::invalid_argument("Unverified sampled 2-4 dribble defender comparison");
  auto state=input;state.area_contact->strength=result.strength;state.area_contact->loss=result.loss;
  state.area_contact->score=result.score;state.area_contact->result=result.result_code;state.area_sequence_result=result.result_code;
  state.phase=FieldSessionPhase::sampled_score24_dribble_compared;validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices || !input.area_contact ||
    input.area_sequence_result!=255 || comparison.table_index!=9)
  throw std::invalid_argument("Player5 dribble comparison needs the observed first defender and direct table row");
 DefenderContactInput contact{};contact.incoming_power=input.action->power;contact.defender_power=input.area_contact->defender_power;
 contact.random_first=input.current.field.random_first;contact.random_second=input.current.field.random_second;
 contact.range=comparison.range;contact.offset=comparison.offset;contact.eligible=comparison.eligible;
 contact.thresholds=comparison.thresholds;contact.defender_slot=14;
 const auto result=compare_defender_contact(contact);
 if(result.strength!=4 || result.loss!=1 || result.score!=0 || result.result_code!=3)
  throw std::invalid_argument("Unverified player5 dribble defender comparison");
 auto state=input;state.area_contact->strength=result.strength;state.area_contact->loss=result.loss;
 state.area_contact->score=result.score;state.area_contact->result=result.result_code;state.area_sequence_result=result.result_code;
 validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_player5_dribble_terminal(const PhaseSessionState& input,
 const ContactPreludeContent& prelude,const ExperienceContent& experience) {
 validate_phase_session(input);
 const bool score24=input.phase==FieldSessionPhase::sampled_score24_dribble_compared;
 if(((input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices) && !score24) || !input.area_contact ||
    input.area_sequence_result!=3 || input.current.clock.remaining!=(score24 ? 342 : 367) || input.current.field.elapsed!=1 ||
    input.current.field.motion[field_motion_index(5)].status || prelude.field_elapsed[2]!=1)
  throw std::invalid_argument("Player5 result3 needs the observed pre-presentation terminal state");
 const auto defender=static_cast<std::uint8_t>(score24 ? 17 : 14);
 const auto reward=award_player_experience(*input.experience,input.current.field.roster[defender].type,2,experience);
 if(!reward.slot || *reward.slot!=0 || reward.award!=1)
  throw std::invalid_argument("Unverified player5 result3 experience");
 const auto applied=apply_contact_prelude({input.current,defender,1,false},prelude);
 if(applied.elapsed_step!=1 || applied.clock.continuation!=ClockContinuation::ready)
  throw std::invalid_argument("Unverified player5 result3 contact clock");
 auto state=input;state.current=applied.state;state.experience=reward.state;state.control->facing=64;
 if(score24)state.phase=FieldSessionPhase::sampled_score24_dribble_clock;
 validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_player5_terminal_field(const PhaseSessionState& input,
 const PlayerProfileMappings& mappings,const FieldPhaseContent& movement) {
 validate_phase_session(input);
 const bool score24=input.phase==FieldSessionPhase::sampled_score24_dribble_clock;
 if(((input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices) && !score24) || !input.area_contact ||
    input.area_sequence_result!=3 || input.current.clock.remaining!=(score24 ? 341 : 366) || input.current.field.elapsed!=2)
  throw std::invalid_argument("Player5 result3 field movement needs its observed terminal clock");
 auto state=input;state.current.field=advance_field_phase(input.current.field,mappings,movement).state;
 if(score24)state.phase=FieldSessionPhase::sampled_score24_dribble_field;
 validate_phase_session(state);return state;
}
Player5PossessionResult transfer_sampled_player5_dribble_possession(const PhaseSessionState& input) {
 validate_phase_session(input);
 const bool score24=input.phase==FieldSessionPhase::sampled_score24_dribble_field;
 if(((input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices) && !score24) || !input.area_contact ||
    input.area_sequence_result!=3 || input.current.clock.remaining!=(score24 ? 341 : 366) || input.current.field.elapsed ||
    input.current.field.side!=0 || input.current.field.ball_owner!=5 || input.recovery->opposing_player!=(score24 ? 17 : 14))
  throw std::invalid_argument("Player5 result3 possession needs the completed observed field movement");
 auto state=input;state.current.field.ball_owner=score24 ? 17 : 14;state.current.field.side=11;
 if(score24)state.phase=FieldSessionPhase::sampled_score24_dribble_possession;
 validate_phase_session(state);return {std::move(state),true};
}
PhaseSessionState prepare_sampled_player3_pass_defender(const PhaseSessionState& input,const FieldPlayerProfiles& profiles,const FieldDefenseContent& defense) {
 validate_phase_session(input);
 const bool second=input.phase==FieldSessionPhase::sampled_score25_first_contact_clock;
 const bool third=input.phase==FieldSessionPhase::sampled_score25_second_contact_clock;
 const bool later=second || third;
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_pass_prepared;
 const bool score25=input.phase==FieldSessionPhase::sampled_score25_pass_prepared;
 if(!later && ((input.phase!=FieldSessionPhase::sampled_player3_pass_prepared && !score23 && !score25) ||
    input.current.field.roster[17].type!=23 || input.current.field.roster[17].stamina!=100))
  throw std::invalid_argument("Unsupported sampled player3 defender preparation");
 const std::uint8_t defender=third ? 20 : second ? 19 : 17;
 const std::uint8_t cursor=third ? 2 : second ? 1 : 0;
 const auto& p=input.current.field.roster[defender];
 const auto prepared=prepare_profiled_field_defender({defender,p.type,static_cast<std::uint8_t>(p.stamina),
  static_cast<std::uint8_t>(p.stamina>>8),p.level},profiles,1,0,0,defense);
 if(prepared.base_strength!=18 || prepared.stamina_cost!=60 || prepared.move_effect!=0)
  throw std::invalid_argument(later ? "Unverified sampled second defender content" : "Unverified sampled player3 defender content");
 auto state=input;
 if(later) {state.recovery->opposing_player=defender;state.area_defender_cursor=cursor;}
 else {state.recovery->ball_variant=1;state.control->facing=64;if(score25)state.recovery->opposing_player=17;}
 state.area_contact=AreaContactContext{cursor,1,0,prepared.move_effect,prepared.base_strength,prepared.stamina_cost,0,0,0,0,false};
 state.phase=third ? FieldSessionPhase::sampled_score25_third_defender_prepared :
  second ? FieldSessionPhase::sampled_score25_second_defender_prepared :
  score25 ? FieldSessionPhase::sampled_score25_first_defender_prepared :
  score23 ? FieldSessionPhase::sampled_score23_defender_prepared : FieldSessionPhase::sampled_player3_defender_prepared;
 validate_phase_session(state);return state;
}
PhaseSessionState compare_sampled_player3_pass_defender(const PhaseSessionState& input,const FieldComparisonSelection& comparison) {
 validate_phase_session(input);
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_defender_prepared;
 const bool third=input.phase==FieldSessionPhase::sampled_score25_third_defender_prepared;
 const bool second=input.phase==FieldSessionPhase::sampled_score25_second_defender_prepared;
 const bool score25=input.phase==FieldSessionPhase::sampled_score25_first_defender_prepared;
 if(input.phase!=FieldSessionPhase::sampled_player3_defender_prepared && !score23 && !score25 && !second && !third)
  throw std::invalid_argument("Unsupported sampled player3 defender comparison");
 DefenderContactInput contact{};contact.incoming_power=input.action->power;contact.defender_power=input.area_contact->defender_power;
 contact.random_first=input.current.field.random_first;contact.random_second=input.current.field.random_second;
 contact.weakened=false;contact.context=0;contact.subtype=0;
 if(comparison.table_index!=5)throw std::invalid_argument("Unsupported sampled player3 comparison table index");
 contact.range=comparison.range;contact.offset=comparison.offset;contact.eligible=comparison.eligible;
 contact.thresholds=comparison.thresholds;contact.defender_slot=third ? 20 : second ? 19 : 17;
 const auto result=compare_defender_contact(contact);
 if(result.score!=255 || result.result_code!=0 || result.strength!=0 || result.loss!=0)
  throw std::invalid_argument("Unverified sampled player3 defender result: score="+std::to_string(result.score)+" result="+std::to_string(result.result_code)+" strength="+std::to_string(result.strength)+" loss="+std::to_string(result.loss));
 auto state=input;state.area_contact->strength=result.strength;state.area_contact->loss=result.loss;
 state.area_contact->score=result.score;state.area_contact->result=result.result_code;state.area_sequence_result=result.result_code;
 state.phase=third ? FieldSessionPhase::sampled_score25_third_defender_compared : second ? FieldSessionPhase::sampled_score25_second_defender_compared : score25 ? FieldSessionPhase::sampled_score25_first_defender_compared : score23 ? FieldSessionPhase::sampled_score23_defender_compared : FieldSessionPhase::sampled_player3_defender_compared;validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_player3_pass_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude,const ExperienceContent& experience) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::attack_prepared && input.human_defense_choices && input.area_contact && input.score &&
    ((input.score->left_goals==2 && input.score->right_goals==5) || (input.score->left_goals==3 && input.score->right_goals==6)) && input.current.field.ball_owner==7) {
  const auto defender=input.recovery->opposing_player;
  if(input.area_sequence_result!=0 || input.current.field.motion[field_motion_index(defender)].status ||
     prelude.field_elapsed[3]!=1 || prelude.defender_status[1]!=6)
   throw std::invalid_argument("Unsupported player7 first defender clock");
  const auto reward=award_player_experience(*input.experience,23,1,experience);
  if(!reward.slot || *reward.slot!=0 || reward.award!=3)throw std::invalid_argument("Unverified player7 contact experience");
  const auto applied=apply_contact_prelude({input.current,defender,1,true},prelude);
  if(applied.elapsed_step!=1 || applied.clock.continuation!=ClockContinuation::ready)
   throw std::invalid_argument("Unverified player7 contact clock");
  auto state=input;state.current=applied.state;state.experience=reward.state;state.control->motion_requested=0;
  validate_phase_session(state);return state;
 }
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_defender_compared;
 const bool third=input.phase==FieldSessionPhase::sampled_score25_third_defender_compared;
 const bool second=input.phase==FieldSessionPhase::sampled_score25_second_defender_compared;
 const bool score25=input.phase==FieldSessionPhase::sampled_score25_first_defender_compared;
 const auto defender=third ? 20 : second ? 19 : 17;
 if((input.phase!=FieldSessionPhase::sampled_player3_defender_compared && !score23 && !score25 && !second && !third) || input.current.field.motion[field_motion_index(defender)].status ||
    prelude.field_elapsed[3]!=1 || prelude.defender_status[1]!=6)
  throw std::invalid_argument("Unsupported sampled player3 contact clock");
 const auto reward=award_player_experience(*input.experience,23,1,experience);
 if(!reward.slot || *reward.slot!=0 || reward.award!=3)throw std::invalid_argument("Unverified sampled player3 contact experience");
 const auto applied=apply_contact_prelude({input.current,static_cast<std::uint8_t>(defender),1,true},prelude);
 if(applied.elapsed_step!=1 || applied.clock.continuation!=ClockContinuation::ready)
  throw std::invalid_argument("Unverified sampled player3 contact clock");
 auto state=input;state.current=applied.state;state.experience=reward.state;
 state.phase=third ? FieldSessionPhase::sampled_score25_third_contact_clock : second ? FieldSessionPhase::sampled_score25_second_contact_clock : score25 ? FieldSessionPhase::sampled_score25_first_contact_clock : score23 ? FieldSessionPhase::sampled_score23_contact_clock : FieldSessionPhase::sampled_player3_contact_clock;validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_player3_pass_dispatch(const PhaseSessionState& input) {
 validate_phase_session(input);
 const bool score25=input.phase==FieldSessionPhase::sampled_score25_third_contact_clock;
 const bool player7=input.phase==FieldSessionPhase::attack_prepared && input.human_defense_choices && input.score &&
    input.current.field.ball_owner==7 && !input.area_contact &&
    ((input.score->left_goals==2 && input.score->right_goals==5 && input.area_defender_cursor==5 && input.current.clock.remaining==306 && input.current.field.elapsed==6) ||
     (input.score->left_goals==3 && input.score->right_goals==6 && input.area_defender_cursor==4 && input.current.clock.remaining==265 && input.current.field.elapsed==5));
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_contact_clock;
 if(input.phase!=FieldSessionPhase::sampled_player3_contact_clock && !score23 && !score25 && !player7)
  throw std::invalid_argument("Unsupported sampled player3 pass dispatch");
 auto state=input;const std::uint8_t elapsed=player7 ? 2 : 1;
 state.current.field.elapsed=static_cast<std::uint8_t>(unsigned(state.current.field.elapsed)+elapsed);
 const auto recovery=advance_keeper_recovery({state.current.field.keepers,state.current.shared_recovery_flags,state.current.shared_recovery_timer},elapsed);
 state.current.field.keepers=recovery.keepers;state.current.shared_recovery_flags=recovery.shared_flags;state.current.shared_recovery_timer=recovery.shared_timer;
 const auto clock=advance_match_clock(state.current.clock,elapsed,state.current.field.random_first);
 if(clock.continuation!=ClockContinuation::ready)throw std::invalid_argument("Unverified sampled player3 dispatch clock");
 state.current.clock=clock.state;state.area_defender_cursor=player7 ? input.area_defender_cursor : score25 ? 3 : 1;
 state.phase=player7 ? FieldSessionPhase::attack_prepared : score25 ? FieldSessionPhase::sampled_score25_dispatch_clock : score23 ? FieldSessionPhase::sampled_score23_dispatch_clock : FieldSessionPhase::sampled_player3_dispatch_clock;validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_player3_pass_handoff(const PhaseSessionState& input,const FieldPhaseState& captured) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_player3_dispatch_clock || captured.elapsed || captured.movement_region!=129)
  throw std::invalid_argument("Unsupported sampled player3 pass handoff");
 auto normalized=captured;normalized.motion=input.current.field.motion;normalized.elapsed=input.current.field.elapsed;
 normalized.movement_region=input.current.field.movement_region;
 std::ostringstream before(std::ios::out|std::ios::binary),after(std::ios::out|std::ios::binary);
 write_field_state(before,input.current.field);write_field_state(after,normalized);
 if(before.str()!=after.str())throw std::invalid_argument("Captured player3 pass handoff changed unrelated field state");
 auto state=input;state.current.field=captured;state.phase=FieldSessionPhase::sampled_player3_pass_handoff;
 validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_score23_pass_handoff(const PhaseSessionState& input,
 const PlayerProfileMappings& mappings,const FieldPhaseContent& movement) {
 validate_phase_session(input);
 const bool score25=input.phase==FieldSessionPhase::sampled_score25_dispatch_clock;
 const bool player7=input.phase==FieldSessionPhase::attack_prepared && input.human_defense_choices && input.score &&
    input.current.field.ball_owner==7 &&
    ((input.score->left_goals==2 && input.score->right_goals==5 && input.current.clock.remaining==304 && input.current.field.elapsed==8 && input.area_defender_cursor==5) ||
     (input.score->left_goals==3 && input.score->right_goals==6 && input.current.clock.remaining==263 && input.current.field.elapsed==7 && input.area_defender_cursor==4));
 if(input.phase!=FieldSessionPhase::sampled_score23_dispatch_clock && !score25 && !player7)
  throw std::invalid_argument("Unsupported sampled pass handoff");
 auto state=input;state.current.field=advance_field_phase(input.current.field,mappings,movement).state;
 state.phase=player7 ? FieldSessionPhase::attack_prepared : score25 ? FieldSessionPhase::sampled_score25_pass_handoff : FieldSessionPhase::sampled_score23_pass_handoff;validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_player3_pass_target(const PhaseSessionState& input,std::uint8_t recipient,std::uint8_t destination,std::uint8_t counter,std::uint8_t distance) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::sampled_score25_pass_handoff) {
  if(recipient!=7 || destination!=103 || counter!=255 || distance!=3 || destination!=*input.human_pass_cursor)
   throw std::invalid_argument("Unsupported sampled player7 pass target");
  auto state=input;state.sampled_field_pass=SampledFieldPassContext{recipient,destination,counter,distance};
  state.phase=FieldSessionPhase::sampled_score25_pass_target_captured;validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::sampled_player3_pass_handoff || recipient!=255 || destination!=87 || counter!=255 || distance!=2 || destination!=*input.human_pass_cursor)
  throw std::invalid_argument("Unsupported sampled player3 pass target");
 auto state=input;state.sampled_field_pass=SampledFieldPassContext{recipient,destination,counter,distance};
 state.phase=FieldSessionPhase::sampled_player3_empty_pass_destination;validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_home_shot_defender(const PhaseSessionState& input, std::uint8_t action, std::uint8_t subtype) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::attack_prepared || action!=1 || subtype!=0 || !input.action || !input.recovery || !input.control || !input.scheduling || !input.score ||
    input.current.field.side!=0 || input.current.field.ball_owner!=5 || input.current.field.ball_x!=108 || input.current.field.ball_y!=108 || input.current.field.ball_cell!=87 ||
    input.current.field.elapsed!=1 || input.current.field.selected_defenders!=1 || input.scheduling->selected_ids[0]!=17 ||
    input.current.field.roster[5].stamina!=15 || input.current.field.roster[17].type!=23 || input.current.field.roster[17].stamina!=100 ||
    input.current.clock.remaining!=411 || input.current.clock.extra_time!=9 || input.current.field.random_first || input.current.field.random_second ||
    input.score->left_goals!=1 || input.score->right_goals!=1 || input.score->kickoff!=4 || input.action->action!=0 || input.action->power!=22 ||
    input.recovery->attack_subtype!=128 || input.recovery->ball_variant!=0 || input.saved_ball_flight_flag!=1)
  throw std::invalid_argument("Unsupported sampled home shot defender choice");
 auto state=input;state.recovery->opposing_player=17;state.area_defense=KeeperAreaDefenseContext{};
 state.area_defense->actions={1,6,6,255};state.area_defense->blink=13;
 state.phase=FieldSessionPhase::sampled_home_shot_defender_choice;validate_phase_session(state);return state;
}
PhaseSessionState resolve_sampled_home_shot_defender(const PhaseSessionState& input,const FieldPlayerProfiles& profiles,
 const FieldDefenseContent& defense,const FieldComparisonContent& comparison) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_home_shot_defender_choice)
  throw std::invalid_argument("Unsupported sampled home shot defender comparison");
 const auto& player=input.current.field.roster[17];
 const auto preparation=prepare_profiled_field_defender({17,player.type,static_cast<std::uint8_t>(player.stamina),
  static_cast<std::uint8_t>(player.stamina>>8),player.level},profiles,1,0,0,defense);
 if(preparation.base_strength!=18 || preparation.stamina_cost!=60)
  throw std::invalid_argument("Unverified sampled home defender profile");
 DefenderContactInput contact{};contact.incoming_power=input.action->power;contact.defender_power=preparation.base_strength;
 contact.random_first=input.current.field.random_first;contact.random_second=input.current.field.random_second;
 contact.weakened=false;contact.context=0;contact.subtype=0;
 contact=configure_field_comparison(contact,0,1,17,comparison);
 const auto result=compare_defender_contact(contact);
 if(result.score!=255 || result.result_code!=0 || result.strength!=0 || result.loss!=0)
  throw std::invalid_argument("Unverified sampled home defender result");
 // Original defender presentation and ball variant are acknowledged; live timing is not simulated.
 auto state=input;state.recovery->ball_variant=1;state.control->motion_requested=2;state.area_sequence_result=0;
 state.phase=FieldSessionPhase::sampled_home_shot_defender_resolved;validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_home_shot_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude,const ExperienceContent& experience) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_home_shot_defender_resolved || input.current.field.roster[5].type!=23 ||
    input.current.field.motion[field_motion_index(17)].status!=0 || prelude.field_elapsed[3]!=1 || prelude.defender_status[1]!=6)
  throw std::invalid_argument("Unsupported sampled home shot contact clock");
 const auto reward=award_player_experience(*input.experience,23,1,experience);
 if(!reward.slot || *reward.slot!=0 || reward.award!=3)throw std::invalid_argument("Unverified sampled home shot experience");
 const auto applied=apply_contact_prelude({input.current,17,1,true},prelude);
 if(applied.elapsed_step!=1 || applied.clock.continuation!=ClockContinuation::ready)
  throw std::invalid_argument("Unverified sampled home shot clock");
 auto state=input;state.current=applied.state;state.experience=reward.state;
 // Original defender presentation completed; NMI/video timing is acknowledged.
 state.control->facing=64;state.control->motion_requested=0;
 state.phase=FieldSessionPhase::sampled_home_shot_contact_clock;validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_home_shot_dispatch(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_home_shot_contact_clock)
  throw std::invalid_argument("Unsupported sampled home shot dispatch clock");
 auto state=input;constexpr std::uint8_t elapsed=2;
 state.current.field.elapsed=static_cast<std::uint8_t>(unsigned(state.current.field.elapsed)+elapsed);
 const auto recovery=advance_keeper_recovery({state.current.field.keepers,state.current.shared_recovery_flags,state.current.shared_recovery_timer},elapsed);
 state.current.field.keepers=recovery.keepers;state.current.shared_recovery_flags=recovery.shared_flags;state.current.shared_recovery_timer=recovery.shared_timer;
 const auto clock=advance_match_clock(state.current.clock,elapsed,state.current.field.random_first);
 if(clock.continuation!=ClockContinuation::ready)throw std::invalid_argument("Unverified sampled home shot dispatch clock");
 state.current.clock=clock.state;state.area_defender_cursor=1;
 state.phase=FieldSessionPhase::sampled_home_shot_dispatch_clock;validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_home_shot_handoff(const PhaseSessionState& input,const FieldPhaseState& captured) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_home_shot_dispatch_clock || captured.elapsed!=0 || captured.movement_region!=129)
  throw std::invalid_argument("Unsupported sampled home shot handoff");
 auto normalized=captured;normalized.motion=input.current.field.motion;normalized.elapsed=input.current.field.elapsed;
 normalized.movement_region=input.current.field.movement_region;
 std::ostringstream before(std::ios::out|std::ios::binary),after(std::ios::out|std::ios::binary);
 write_field_state(before,input.current.field);write_field_state(after,normalized);
 if(before.str()!=after.str())throw std::invalid_argument("Captured home shot handoff changed unrelated field state");
 auto state=input;state.current.field=captured;state.phase=FieldSessionPhase::sampled_home_shot_handoff;
 validate_phase_session(state);return state;
}
PhaseSessionState prepare_sampled_home_shot_target(const PhaseSessionState& input,const ShotApproachContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_home_shot_handoff)
  throw std::invalid_argument("Unsupported sampled home shot target");
 const auto& field=input.current.field;
 const auto target=calculate_shot_approach({field.ball_x,field.ball_y,field.side,input.recovery->attack_subtype,field.random_first},content);
 if(target.distance_adjustment!=108 || target.destination_cell!=233 || target.random_first!=0 || target.loose_ball)
  throw std::invalid_argument("Unverified sampled home shot target");
 auto state=input;state.current.field.random_first=target.random_first;
 state.shot_flight=ShotFlightContext{target.distance_adjustment,target.destination_cell,255,target.loose_ball};
 state.phase=FieldSessionPhase::sampled_home_shot_target;validate_phase_session(state);return state;
}
PhaseSessionState capture_postgoal_opponent_choice(const PhaseSessionState& input, std::uint8_t action,
    std::uint8_t subtype, std::uint8_t recipient, std::uint8_t target) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ai_cell_decision || !input.action || !input.recovery || !input.score || !input.control ||
    input.area_ai_target || action!=3 || subtype!=0 || recipient!=14 || target!=161 ||
    input.current.field.side!=11 || input.current.field.ball_owner!=18 || input.current.field.ball_x!=141 ||
    input.current.field.ball_y!=108 || input.current.field.ball_cell!=135 || input.current.field.elapsed!=1 ||
    input.current.field.roster[18].type || input.current.field.roster[18].stamina || input.current.field.roster[18].level ||
    input.current.field.random_first!=93 || input.current.field.random_second!=128 || input.current.field.selected_defenders ||
    input.current.clock.remaining!=158 || input.current.clock.extra_time || input.current.clock.ball_mode!=128 ||
    input.score->left_goals!=1 || input.score->right_goals!=0 || input.score->kickoff!=4 ||
    input.control->controlled_player!=10 || input.control->last_carrier_cell!=135 ||
    input.action->action!=0 || input.action->power!=11 || input.action->move_flags || input.recovery->attack_subtype!=1)
  throw std::invalid_argument("Unsupported captured post-goal opponent choice");
 auto state=input;state.action->action=action;state.recovery->attack_subtype=subtype;
 state.area_ai_target=PhaseSessionState::AreaAiTarget{recipient,target,0};
 state.phase=FieldSessionPhase::postgoal_opponent_action_choice;validate_phase_session(state);return state;
}
PhaseSessionState prepare_postgoal_opponent_action(const PhaseSessionState& input,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_action_choice)
  throw std::invalid_argument("Post-goal opponent preparation requires its captured choice");
 const auto& player=input.current.field.roster[18];
 const auto result=prepare_attacking_action({{18,player.type,static_cast<std::uint8_t>(player.stamina),
  static_cast<std::uint8_t>(player.stamina>>8),player.level},3,0,0,11,0,
  input.current.field.random_first,input.current.field.random_second,false},profiles,content);
 if(result.preparation.skill_selector!=2 || result.preparation.skill_index!=17 || result.preparation.base_strength!=13 ||
    result.preparation.move_effect || result.preparation.stamina_cost!=60 || result.move_flags!=3 ||
    result.action_power.power!=12 || result.action_power.subtype!=0)
  throw std::invalid_argument("Unverified post-goal opponent action content");
 auto state=input;state.current.field.roster[18].stamina=result.energy.stamina;
 state.action->move_effect=result.preparation.move_effect;state.action->move_flags=result.move_flags;
 state.action->power=result.action_power.power;state.recovery->attack_subtype=result.action_power.subtype;
 state.area_ai_target->cost=result.preparation.stamina_cost;
 state.phase=FieldSessionPhase::postgoal_opponent_action_prepared;validate_phase_session(state);return state;
}
PhaseSessionState begin_postgoal_opponent_action_wait(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_action_prepared)
  throw std::invalid_argument("Post-goal action wait requires its prepared action");
 auto state=input;state.phase=FieldSessionPhase::postgoal_opponent_action_wait;
 validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_postgoal_opponent_action(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_action_wait || input.current.clock.ball_mode!=192 ||
    input.current.field.random_first!=191 || input.current.field.random_second!=238)
  throw std::invalid_argument("Post-goal action handoff requires the observed presentation boundary");
 auto state=input;state.current.clock.ball_mode=0;state.current.clock.script_flags=0;
 state.phase=FieldSessionPhase::postgoal_opponent_action_handoff;validate_phase_session(state);return state;
}
PhaseSessionState advance_postgoal_opponent_clock(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_action_handoff)
  throw std::invalid_argument("Post-goal opponent clock requires the observed action handoff");
 auto state=input;state.current.field.elapsed=static_cast<std::uint8_t>(unsigned(state.current.field.elapsed)+1u);
 const auto recovery=advance_keeper_recovery({state.current.field.keepers,state.current.shared_recovery_flags,state.current.shared_recovery_timer},1);
 state.current.field.keepers=recovery.keepers;state.current.shared_recovery_flags=recovery.shared_flags;state.current.shared_recovery_timer=recovery.shared_timer;
 const auto clock=advance_match_clock(state.current.clock,1,state.current.field.random_first);
 if(clock.continuation!=ClockContinuation::ready)throw std::invalid_argument("Unsupported post-goal clock continuation");
 state.current.clock=clock.state;state.phase=FieldSessionPhase::postgoal_opponent_clocked;
 validate_phase_session(state);return state;
}
PhaseSessionState dispatch_postgoal_opponent_action(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_clocked || input.current.field.random_first!=204 || input.current.field.random_second!=253)
  throw std::invalid_argument("Post-goal opponent dispatch requires the observed clock wait");
 auto state=input;state.control->last_carrier_cell=0;state.phase=FieldSessionPhase::postgoal_opponent_dispatch;
 validate_phase_session(state);return state;
}
PhaseSessionState handoff_postgoal_opponent_flight(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_field_moved || input.current.field.random_first!=176 ||
    input.current.field.random_second!=26 || input.control->facing!=64 || input.control->display_parameter ||
    input.control->motion_requested || input.recovery->ball_variant)
  throw std::invalid_argument("Post-goal flight handoff requires the observed presentation boundary");
 auto state=input;state.control->last_carrier_cell=128;state.recovery->ball_variant=1;
 state.phase=FieldSessionPhase::postgoal_opponent_flight_handoff;validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_field_ai_choice(const PhaseSessionState& input, std::uint8_t action, std::uint8_t subtype) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==2 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==18 &&
    input.current.field.ball_x==110 && input.current.field.ball_y==108 && input.current.field.ball_cell==87 &&
    input.current.field.elapsed==1 && !input.current.field.selected_defenders &&
    input.current.field.random_first==188 && input.current.field.random_second==86 &&
    input.current.clock.remaining==154 && !input.current.clock.extra_time &&
    input.control->controlled_player==10 && input.control->last_carrier_cell==87 &&
    input.score->left_goals==1 && input.score->right_goals==0 && input.score->kickoff==4 &&
    input.action->action==3 && input.action->power==12 && !input.recovery->attack_subtype) {
  auto state=input;state.action->action=2;state.phase=FieldSessionPhase::control_clock_pending;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==0 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==19 &&
    input.current.field.ball_x==85 && input.current.field.ball_y==157 && input.current.field.ball_cell==57 &&
    input.current.field.roster[19].type==23 && input.current.field.roster[19].stamina==100 && input.current.field.elapsed==1 &&
    !input.current.field.selected_defenders && !input.current.field.random_first && !input.current.field.random_second && !input.experience->opponent_stage &&
    input.current.clock.remaining==294 && input.current.clock.extra_time==13 && input.control->controlled_player==2 && input.control->last_carrier_cell==57 &&
    input.score->left_goals==3 && input.score->right_goals==5 && input.score->kickoff==4 && input.action->action==1 &&
    input.action->power==21 && input.recovery->attack_subtype==128 && input.saved_ball_flight_flag==1 && !input.area_defender_cursor) {
  auto state=input;state.action->action=0;state.recovery->attack_subtype=0;state.phase=FieldSessionPhase::sampled_score23_shot_choice;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==0 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==17 &&
    input.current.field.ball_x==94 && input.current.field.ball_y==106 && input.current.field.ball_cell==63 &&
    input.current.field.roster[17].type==23 && input.current.field.roster[17].stamina==100 && input.current.field.elapsed==1 &&
    !input.current.field.selected_defenders && !input.current.field.random_first && !input.current.field.random_second && !input.experience->opponent_stage &&
    input.current.clock.remaining==336 && input.current.clock.extra_time==13 && input.control->controlled_player==9 &&
    input.control->last_carrier_cell==63 && input.score->left_goals==2 && input.score->right_goals==4 && input.score->kickoff==4 &&
    input.action->action==2 && !input.action->power && !input.recovery->attack_subtype && !input.saved_ball_flight_flag) {
  auto state=input;state.action->action=0;state.phase=FieldSessionPhase::sampled_score23_shot_choice;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==2 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==17 &&
    input.current.field.ball_x==101 && input.current.field.ball_y==106 && input.current.field.ball_cell==75 &&
    input.current.field.roster[17].type==23 && input.current.field.roster[17].stamina==100 && input.current.field.elapsed==1 &&
    !input.current.field.selected_defenders && !input.current.field.random_first && !input.current.field.random_second && !input.experience->opponent_stage &&
    input.current.clock.remaining==338 && input.current.clock.extra_time==13 && input.control->controlled_player==9 &&
    input.control->last_carrier_cell==75 && input.score->left_goals==2 && input.score->right_goals==4 && input.score->kickoff==4 &&
    input.action->action==2 && !input.action->power && !input.recovery->attack_subtype && !input.saved_ball_flight_flag) {
  auto state=input;state.phase=FieldSessionPhase::control_clock_pending;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==0 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==19 &&
    input.current.field.ball_x==94 && input.current.field.ball_y==108 && input.current.field.ball_cell==63 &&
    input.current.field.roster[19].type==23 && input.current.field.roster[19].stamina==100 && input.current.field.elapsed==1 &&
    !input.current.field.selected_defenders && !input.current.field.random_first && !input.current.field.random_second && !input.experience->opponent_stage &&
    input.current.clock.remaining==349 && input.current.clock.extra_time==13 && input.control->controlled_player==5 &&
    input.control->last_carrier_cell==63 && input.score->left_goals==2 && input.score->right_goals==3 && input.score->kickoff==4 &&
    input.action->action==2 && !input.action->power && !input.recovery->attack_subtype && input.saved_ball_flight_flag==1) {
  auto state=input;state.action->action=0;state.phase=FieldSessionPhase::sampled_score23_shot_choice;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==2 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==19 &&
    input.current.field.ball_x==101 && input.current.field.ball_y==108 && input.current.field.ball_cell==75 &&
    input.current.field.roster[19].type==23 && input.current.field.elapsed==1 && !input.current.field.selected_defenders &&
    !input.current.field.random_first && !input.current.field.random_second && !input.experience->opponent_stage &&
    input.current.clock.remaining==351 && input.current.clock.extra_time==13 && input.control->controlled_player==5 &&
    input.control->last_carrier_cell==75 && input.score->left_goals==2 && input.score->right_goals==3 && input.score->kickoff==4 &&
    input.action->action==1 && !input.action->power && !input.recovery->attack_subtype && input.saved_ball_flight_flag==1) {
  auto state=input;state.action->action=2;state.phase=FieldSessionPhase::control_clock_pending;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==0 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==19 &&
    input.current.field.ball_x==85 && input.current.field.ball_y==109 && input.current.field.ball_cell==51 &&
    input.current.field.roster[19].type==23 && input.current.field.roster[19].stamina==100 && input.current.field.elapsed==1 &&
    !input.current.field.selected_defenders && !input.current.field.random_first && !input.current.field.random_second &&
    !input.experience->opponent_stage && input.current.clock.remaining==361 && input.current.clock.extra_time==13 &&
    input.control->controlled_player==7 && input.control->last_carrier_cell==51 && input.score->left_goals==2 &&
    input.score->right_goals==2 && input.score->kickoff==4 && input.action->action==1 && input.action->power==21 &&
    input.recovery->attack_subtype==128 && input.saved_ball_flight_flag==1) {
  auto state=input;state.action->action=action;state.recovery->attack_subtype=0;
  state.phase=FieldSessionPhase::sampled_player19_return_shot_choice;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==1 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==14 &&
    input.current.field.ball_x==100 && input.current.field.ball_y==107 && input.current.field.ball_cell==75 &&
    input.current.field.roster[14].type==23 && input.current.field.roster[14].stamina==100 && input.current.field.elapsed==1 &&
    !input.current.field.selected_defenders && !input.current.field.random_first && !input.current.field.random_second &&
    !input.experience->opponent_stage && input.current.clock.remaining==363 && input.current.clock.extra_time==13 &&
    input.control->controlled_player==9 && input.control->last_carrier_cell==75 && input.score->left_goals==2 &&
    input.score->right_goals==2 && input.score->kickoff==4 && input.action->action==2 && input.action->power==0 &&
    input.recovery->attack_subtype==0 && input.saved_ball_flight_flag==1) {
  auto state=input;state.action->action=action;state.phase=FieldSessionPhase::sampled_player14_ai_choice;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_field_defense_complete) {
  const bool third_return=input.score && input.score->left_goals==1 && !input.score->right_goals && input.current.clock.remaining==153 &&
   input.current.field.side==11 && input.current.field.ball_owner==18 && input.current.field.ball_cell==87 && input.current.field.selected_defenders==1 &&
   input.scheduling && input.scheduling->selected_ids[0]==5 && input.area_defense->actions==std::array<std::uint8_t,4>{0,255,255,255};
  if(third_return) {
   if(action!=2 || subtype!=0 || input.action->action!=2)throw std::invalid_argument("Unsupported third-return opponent choice");
   auto state=input;state.phase=FieldSessionPhase::sampled_field_contact_ai_choice;validate_phase_session(state);return state;
  }
  const bool mixed=input.area_defense->actions==std::array<std::uint8_t,4>{0,6,6,255};
  if(action!=2 || subtype!=0 || (!mixed && input.area_defense->actions!=std::array<std::uint8_t,4>{6,6,6,255}))
   throw std::invalid_argument("Unsupported captured field contact AI choice");
  auto state=input;state.action->action=action;state.recovery->attack_subtype=subtype;
  state.phase=mixed ? FieldSessionPhase::sampled_mixed_defense_ai_choice : FieldSessionPhase::sampled_field_contact_ai_choice;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==0 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==19 &&
    input.current.field.ball_cell==67 &&
    input.current.field.roster[19].type==23 && input.current.field.elapsed==1 && !input.current.field.selected_defenders &&
    !input.current.field.random_first && !input.current.field.random_second && !input.experience->opponent_stage &&
    ((input.current.field.ball_x==93 && input.current.field.ball_y==140 && input.current.clock.remaining==419 && input.current.clock.extra_time==9 && input.score->left_goals==1 && input.score->right_goals==0) ||
     (input.current.field.ball_x==92 && input.current.field.ball_y==141 && input.current.clock.remaining==382 && input.current.clock.extra_time==13 && input.score->left_goals==2 && input.score->right_goals==1)) &&
    input.control->controlled_player==4 && input.control->last_carrier_cell==67 && input.score->kickoff==4 &&
    input.action->action==1 && input.action->power==21 && input.recovery->attack_subtype==128 && input.saved_ball_flight_flag==1) {
  auto state=input;state.action->action=0;state.recovery->attack_subtype=0;
  state.phase=FieldSessionPhase::sampled_player19_shot_choice;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==1 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==12 &&
    input.current.field.ball_cell==130 &&
    input.current.field.roster[12].type==23 && input.current.field.elapsed==1 && !input.current.field.selected_defenders &&
    !input.current.field.random_first && !input.current.field.random_second && !input.experience->opponent_stage &&
    ((input.current.field.ball_x==134 && input.current.field.ball_y==160 && input.current.clock.remaining==421 && input.current.clock.extra_time==9 && input.score->left_goals==1 && input.score->right_goals==0) ||
     (input.current.field.ball_x==133 && input.current.field.ball_y==161 && input.current.clock.remaining==384 && input.current.clock.extra_time==13 && input.score->left_goals==2 && input.score->right_goals==1)) &&
    input.control->controlled_player==6 && input.control->last_carrier_cell==130 && input.score->kickoff==4 &&
    input.action->action==1 && input.action->power==24 && input.recovery->attack_subtype==0 && input.saved_ball_flight_flag==1) {
  auto state=input;state.phase=FieldSessionPhase::sampled_player12_ai_choice;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==2 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==19 &&
    input.current.field.ball_x==102 && input.current.field.ball_y==140 && input.current.field.ball_cell==79 &&
    input.current.field.roster[19].type==23 && input.current.field.elapsed==1 && !input.current.field.selected_defenders &&
    !input.current.field.random_first && !input.current.field.random_second && !input.experience->opponent_stage &&
    input.current.clock.remaining==437 && input.current.clock.extra_time==5 && input.control->controlled_player==2 &&
    input.control->last_carrier_cell==79 && input.score->left_goals==1 && input.score->right_goals==0 && input.score->kickoff==4 &&
    input.action->action==2 && input.action->power==24 && input.recovery->attack_subtype==128) {
  auto state=input;state.recovery->attack_subtype=0;state.phase=FieldSessionPhase::control_clock_pending;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==0 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==19 &&
    input.current.field.ball_x==95 && input.current.field.ball_y==140 && input.current.field.ball_cell==67 &&
    input.current.field.roster[19].type==23 && input.current.field.elapsed==1 && !input.current.field.selected_defenders &&
    !input.current.field.random_first && !input.current.field.random_second && !input.experience->opponent_stage &&
    input.current.clock.remaining==435 && input.current.clock.extra_time==5 && input.control->controlled_player==2 &&
    input.control->last_carrier_cell==67 && input.score->left_goals==1 && input.score->right_goals==0 && input.score->kickoff==4 &&
    input.action->action==2 && input.action->power==24 && input.recovery->attack_subtype==0) {
  auto state=input;state.action->action=0;state.phase=FieldSessionPhase::sampled_opponent_shot_choice;
  validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::ai_cell_decision && input.action && input.score && input.recovery && input.control && input.experience &&
    action==2 && subtype==0 && input.current.field.side==11 && input.current.field.ball_owner==19 &&
    input.current.field.ball_x==111 && input.current.field.ball_y==141 && input.current.field.ball_cell==91 &&
    input.current.field.roster[19].type==23 && input.current.field.elapsed==1 && !input.current.field.selected_defenders &&
    !input.current.field.random_first && !input.current.field.random_second && !input.experience->opponent_stage &&
    input.current.clock.remaining==404 && input.current.clock.extra_time==9 && input.control->controlled_player==7 &&
    input.control->last_carrier_cell==91 && input.score->left_goals==2 && input.score->right_goals==1 && input.score->kickoff==4 &&
    input.action->action==1 && input.action->power==21 && input.recovery->attack_subtype==128 && input.saved_ball_flight_flag==1) {
  auto state=input;state.action->action=2;state.recovery->attack_subtype=0;state.phase=FieldSessionPhase::control_clock_pending;
  validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::ai_cell_decision || action!=1 || subtype!=0 || !input.action || !input.score ||
    input.current.field.side!=11 || input.current.field.ball_owner!=16 || input.current.field.ball_x!=141 ||
    input.current.field.ball_y!=148 || input.current.field.ball_cell!=140 || input.current.field.roster[16].type!=23 ||
    input.current.field.selected_defenders || input.current.field.random_first || input.current.field.random_second ||
    input.experience->opponent_stage ||
    !((input.current.clock.remaining==443 && input.current.clock.extra_time==5 && input.score->left_goals==1 && input.score->right_goals==0) ||
      (input.current.clock.remaining==406 && input.current.clock.extra_time==9 && input.score->left_goals==2 && input.score->right_goals==1) ||
      (input.current.clock.remaining==296 && input.current.clock.extra_time==13 && input.score->left_goals==3 && input.score->right_goals==5)) ||
    input.control->last_carrier_cell!=140 || input.score->kickoff!=4 || input.action->action!=0 || input.action->power!=22 || input.recovery->attack_subtype!=128)
  throw std::invalid_argument("Unsupported sampled opponent AI choice");
 auto state=input;state.action->action=action;state.recovery->attack_subtype=subtype;
 state.phase=FieldSessionPhase::sampled_field_ai_choice;validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_field_ai_preparation(const PhaseSessionState& input, std::uint8_t subtype, std::uint16_t power) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::sampled_score23_shot_choice) {
  if(subtype!=128 || power!=22)throw std::invalid_argument("Unsupported captured 2-3 shot preparation");
  auto state=input;state.recovery->attack_subtype=subtype;state.action->power=power;
  state.phase=FieldSessionPhase::sampled_score23_shot_prepared;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_player19_return_shot_choice) {
  if(subtype!=128 || power!=22)throw std::invalid_argument("Unsupported captured player19 return shot preparation");
  auto state=input;state.recovery->attack_subtype=subtype;state.action->power=power;
  state.phase=FieldSessionPhase::sampled_player19_return_shot_prepared;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_player14_ai_choice) {
  if(subtype!=128 || power!=21)throw std::invalid_argument("Unsupported captured player14 pass preparation");
  auto state=input;state.recovery->attack_subtype=subtype;state.action->power=power;
  state.phase=FieldSessionPhase::sampled_player14_ai_prepared;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_player19_shot_choice) {
  if(subtype!=128 || power!=22)throw std::invalid_argument("Unsupported captured player19 shot preparation");
  auto state=input;state.recovery->attack_subtype=subtype;state.action->power=power;
  state.phase=FieldSessionPhase::sampled_player19_shot_prepared;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_player12_ai_choice) {
  if(subtype!=128 || power!=21)throw std::invalid_argument("Unsupported captured player12 pass preparation");
  auto state=input;state.recovery->attack_subtype=subtype;state.action->power=power;
  state.phase=FieldSessionPhase::sampled_player12_ai_prepared;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_mixed_defense_ai_choice) {
  if(subtype!=128 || power!=24)throw std::invalid_argument("Unsupported captured mixed defense preparation");
  auto state=input;state.recovery->attack_subtype=subtype;state.action->power=power;
  state.phase=FieldSessionPhase::sampled_mixed_defense_ai_prepared;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_opponent_shot_choice) {
  if(subtype!=128 || power!=22)throw std::invalid_argument("Unsupported captured opponent shot preparation");
  auto state=input;state.recovery->attack_subtype=subtype;state.action->power=power;
  state.phase=FieldSessionPhase::sampled_opponent_shot_prepared;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_field_contact_ai_choice) {
  if(subtype!=128 || power!=24)throw std::invalid_argument("Unsupported captured field contact AI preparation");
  auto state=input;state.recovery->attack_subtype=subtype;state.action->power=power;
  state.phase=FieldSessionPhase::sampled_field_contact_ai_prepared;validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::sampled_field_ai_choice || subtype!=128 || power!=21)
  throw std::invalid_argument("Unsupported sampled opponent AI preparation");
 auto state=input;state.recovery->attack_subtype=subtype;state.action->power=power;
 state.phase=FieldSessionPhase::sampled_field_ai_prepared;validate_phase_session(state);return state;
}
PhaseSessionState prepare_sampled_field_contact_attack(const PhaseSessionState& input,
 const FieldPlayerProfiles& profiles,const FieldDefenseContent& attack) {
 validate_phase_session(input);
 const bool shot=input.phase==FieldSessionPhase::sampled_opponent_shot_choice || input.phase==FieldSessionPhase::sampled_player19_shot_choice || input.phase==FieldSessionPhase::sampled_player19_return_shot_choice || input.phase==FieldSessionPhase::sampled_score23_shot_choice;
 const bool player12=input.phase==FieldSessionPhase::sampled_player12_ai_choice || input.phase==FieldSessionPhase::sampled_player14_ai_choice;
 if((input.phase!=FieldSessionPhase::sampled_field_contact_ai_choice && input.phase!=FieldSessionPhase::sampled_mixed_defense_ai_choice && !shot && !player12) || input.recovery->ball_variant!=0)
  throw std::invalid_argument("Unsupported sampled field contact attack");
 const auto id=input.current.field.ball_owner;const auto& p=input.current.field.roster[id];
 const auto result=prepare_attacking_action({{id,p.type,static_cast<std::uint8_t>(p.stamina),
  static_cast<std::uint8_t>(p.stamina>>8),p.level},static_cast<std::uint8_t>(player12 ? 1 : shot ? 0 : 2),0,input.recovery->attack_subtype,11,
  input.current.field.selected_defenders,input.current.field.random_first,input.current.field.random_second,false},profiles,attack);
 const bool third_return=input.phase==FieldSessionPhase::sampled_field_contact_ai_choice && input.score && input.score->left_goals==1 &&
  !input.score->right_goals && input.current.clock.remaining==153 && id==18 && input.current.field.ball_cell==87 &&
  input.current.field.selected_defenders==1 && input.scheduling && input.scheduling->selected_ids[0]==5;
 if(third_return) {
  if(p.type!=0 || p.level!=0 || p.stamina!=0 || result.preparation.base_strength!=9 || result.preparation.stamina_cost!=40 ||
     result.preparation.move_effect!=0 || result.move_flags!=0 || result.action_power.power!=4 || result.action_power.subtype!=0)
   throw std::invalid_argument("Unverified third-return attack preparation");
  auto state=input;state.action->move_flags=result.move_flags;state.action->power=result.action_power.power;
  state.control->last_carrier_cell=0;state.phase=FieldSessionPhase::sampled_field_contact_ai_prepared;
  validate_phase_session(state);return state;
 }
 if(result.preparation.base_strength!=(player12 ? 21 : shot ? 22 : 24) || result.preparation.stamina_cost!=(player12 ? 20 : shot ? 80 : 40) ||
    result.preparation.move_effect!=input.action->move_effect || result.move_flags!=input.action->move_flags)
  throw std::invalid_argument("Unverified sampled field attack content");
 return capture_sampled_field_ai_preparation(input,result.action_power.subtype,result.action_power.power);
}
PhaseSessionState prepare_sampled_mixed_defender(const PhaseSessionState& input,
 const FieldPlayerProfiles& profiles,const FieldDefenseContent& defense) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_mixed_defense_ai_prepared || input.scheduling->selected_ids[0]!=3)
  throw std::invalid_argument("Unsupported sampled mixed defender");
 const auto& player=input.current.field.roster[3];
 const auto result=prepare_profiled_field_defender({3,player.type,static_cast<std::uint8_t>(player.stamina),
  static_cast<std::uint8_t>(player.stamina>>8),player.level},profiles,0,0,0,defense);
 if(result.base_strength!=19 || result.stamina_cost!=70 || result.move_effect!=0)
  throw std::invalid_argument("Unverified sampled defender profile");
 auto state=input;state.recovery->opposing_player=3;state.control->last_carrier_cell=0;
 state.phase=FieldSessionPhase::sampled_mixed_defender_prepared;validate_phase_session(state);return state;
}
PhaseSessionState compare_third_return_defender(const PhaseSessionState& input, const FieldPlayerProfiles& profiles,
 const FieldDefenseContent& defense, const FieldComparisonContent& comparison) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_field_contact_ai_prepared || !input.score || input.score->left_goals!=1 || input.score->right_goals ||
    input.current.clock.remaining!=153 || input.current.field.ball_owner!=18 || input.current.field.ball_cell!=87 ||
    input.current.field.random_first!=75 || input.current.field.random_second!=195 || !input.scheduling || input.scheduling->selected_ids[0]!=5 ||
    !input.area_defense || input.area_defense->actions!=std::array<std::uint8_t,4>{0,255,255,255})
  throw std::invalid_argument("Unsupported third-return defender comparison");
 const auto& player=input.current.field.roster[5];
 if(player.type!=7 || player.level!=0 || player.stamina!=400)throw std::invalid_argument("Unsupported third-return defender profile");
 const auto prep=prepare_profiled_field_defender({5,player.type,static_cast<std::uint8_t>(player.stamina),
  static_cast<std::uint8_t>(player.stamina>>8),player.level},profiles,0,0,0,defense);
 const auto energy=calculate_energy({player.stamina,prep.stamina_cost,player.type,11,2,0,true,false});
 DefenderContactInput contact{};contact.incoming_power=input.action->power;contact.defender_power=prep.base_strength;
 contact.random_first=input.current.field.random_first;contact.random_second=input.current.field.random_second;
 contact.weakened=energy.weakened;contact.context=0;contact.subtype=0;
 contact=configure_field_comparison(contact,2,0,5,comparison);
 const auto result=compare_defender_contact(contact);
 if(prep.base_strength!=8 || prep.stamina_cost!=70 || prep.move_effect!=0 || energy.stamina!=330 || energy.weakened ||
    result.strength!=4 || result.loss!=1 || result.score!=64 || result.result_code!=2 || result.incoming_power!=4)
  throw std::invalid_argument("Unverified third-return defender result");
 auto state=input;state.current.field.roster[5].stamina=energy.stamina;
 state.phase=FieldSessionPhase::third_return_defender_compared;validate_phase_session(state);return state;
}
PhaseSessionState compare_sampled_mixed_defender(const PhaseSessionState& input,const FieldComparisonContent& comparison) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_mixed_defender_prepared)
  throw std::invalid_argument("Unsupported sampled mixed comparison");
 const auto& player=input.current.field.roster[3];
 const auto energy=calculate_energy({player.stamina,70,player.type,11,2,128,true,false});
 DefenderContactInput contact{};contact.incoming_power=input.action->power;contact.defender_power=19;
 contact.random_first=input.current.field.random_first;contact.random_second=input.current.field.random_second;
 contact.weakened=energy.weakened;contact.context=0;contact.subtype=0;
 contact=configure_field_comparison(contact,2,0,3,comparison);
 const auto result=compare_defender_contact(contact);
 if(energy.stamina!=0 || !energy.weakened || result.score!=255 || result.result_code!=0 || result.strength!=0)
  throw std::invalid_argument("Unverified sampled mixed comparison result");
 auto state=input;state.current.field.roster[3].stamina=energy.stamina;
 state.phase=FieldSessionPhase::sampled_mixed_defender_compared;validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_mixed_result4(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_mixed_defender_compared || input.area_sequence_result!=255 ||
    input.current.field.random_first!=0 || input.current.field.random_second!=0)
  throw std::invalid_argument("Unsupported sampled mixed defender result4");
 auto state=input;state.area_sequence_result=4;state.phase=FieldSessionPhase::sampled_mixed_defender_result4;
 validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_mixed_result4_pass(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_mixed_defender_result4)
  throw std::invalid_argument("Unsupported sampled mixed result4 pass");
 const bool postgoal=input.score && input.score->left_goals==2 && input.score->right_goals==1 &&
  input.current.clock.remaining==403 && input.current.clock.extra_time==9;
 auto state=input;
 state.current.field.elapsed=19;state.current.clock.remaining=postgoal ? 385 : 422;
 state.current.clock.extra_time=postgoal ? 13 : 9;
 state.action->action=1;state.recovery->attack_subtype=0;state.control->facing=postgoal ? 0 : 64;
 state.phase=FieldSessionPhase::sampled_mixed_result4_pass;validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_field_ai_presentation(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::sampled_score23_shot_prepared) {
  auto state=input;state.current.clock.script_flags=0;state.current.clock.ball_mode=0;
  state.phase=FieldSessionPhase::sampled_score23_shot_presented;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_player19_return_shot_prepared) {
  auto state=input;state.current.clock.script_flags=0;state.current.clock.ball_mode=0;
  state.phase=FieldSessionPhase::sampled_player19_return_shot_presented;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_player14_ai_prepared) {
  auto state=input;state.current.clock.script_flags=0;state.current.clock.ball_mode=0;
  state.phase=FieldSessionPhase::sampled_player14_ai_presented;validate_phase_session(state);return state;
 }
 const bool shot=input.phase==FieldSessionPhase::sampled_opponent_shot_prepared || input.phase==FieldSessionPhase::sampled_player19_shot_prepared;
 const bool player12=input.phase==FieldSessionPhase::sampled_player12_ai_prepared;
 if(input.phase!=FieldSessionPhase::sampled_field_ai_prepared && !shot && !player12)
  throw std::invalid_argument("Unsupported sampled opponent presentation");
 auto state=input;state.current.clock.script_flags=0;state.current.clock.ball_mode=0;
 state.phase=player12 ? FieldSessionPhase::sampled_player12_ai_presented : shot ? FieldSessionPhase::sampled_opponent_shot_presented : FieldSessionPhase::sampled_field_ai_presented;
 validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_field_ai_dispatch(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::sampled_score23_shot_presented) {
  auto state=input;constexpr std::uint8_t elapsed=1;
  state.current.field.elapsed=static_cast<std::uint8_t>(unsigned(state.current.field.elapsed)+elapsed);
  const auto recovered=advance_keeper_recovery({state.current.field.keepers,state.current.shared_recovery_flags,state.current.shared_recovery_timer},elapsed);
  state.current.field.keepers=recovered.keepers;state.current.shared_recovery_flags=recovered.shared_flags;state.current.shared_recovery_timer=recovered.shared_timer;
  const auto clock=advance_match_clock(state.current.clock,elapsed,state.current.field.random_first);
  if(clock.continuation!=ClockContinuation::ready)throw std::invalid_argument("Unverified 2-3 shot clock");
  state.current.clock=clock.state;state.phase=FieldSessionPhase::sampled_score23_shot_clock;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_player19_return_shot_presented) {
  auto state=input;constexpr std::uint8_t elapsed=1;
  state.current.field.elapsed=static_cast<std::uint8_t>(unsigned(state.current.field.elapsed)+elapsed);
  const auto recovered=advance_keeper_recovery({state.current.field.keepers,state.current.shared_recovery_flags,state.current.shared_recovery_timer},elapsed);
  state.current.field.keepers=recovered.keepers;state.current.shared_recovery_flags=recovered.shared_flags;state.current.shared_recovery_timer=recovered.shared_timer;
  const auto clock=advance_match_clock(state.current.clock,elapsed,state.current.field.random_first);
  if(clock.continuation!=ClockContinuation::ready)throw std::invalid_argument("Unverified player19 return shot clock");
  state.current.clock=clock.state;state.control->last_carrier_cell=0;
  state.phase=FieldSessionPhase::sampled_player19_return_shot_clock;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_player14_ai_presented) {
  auto state=input;state.control->last_carrier_cell=0;
  state.phase=FieldSessionPhase::sampled_player14_ai_dispatch;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_player12_ai_presented) {
  auto state=input;state.control->last_carrier_cell=0;
  state.phase=FieldSessionPhase::sampled_player12_ai_dispatch;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_opponent_shot_presented) {
  auto state=input;--state.current.clock.remaining;state.current.field.elapsed=2;state.control->last_carrier_cell=0;
  state.phase=FieldSessionPhase::sampled_opponent_shot_clock;validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::sampled_field_ai_presented)
  throw std::invalid_argument("Unsupported sampled opponent dispatch");
 auto state=input;state.control->last_carrier_cell=0;
 state.phase=FieldSessionPhase::sampled_field_ai_dispatch;validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_player14_pass_handoff(const PhaseSessionState& input,
 const PlayerProfileMappings& mappings,const FieldPhaseContent& movement) {
 validate_phase_session(input);
 const bool player16=input.phase==FieldSessionPhase::sampled_field_ai_dispatch && input.score &&
  input.score->left_goals==3 && input.score->right_goals==5 && input.current.clock.remaining==296;
 if((input.phase!=FieldSessionPhase::sampled_player14_ai_dispatch && !player16) || input.recovery->special_owner_flags || input.recovery->ball_variant)
  throw std::invalid_argument("Pass handoff needs the observed dispatch state");
 auto state=input;state.current.field=advance_field_phase(input.current.field,mappings,movement).state;
 if(player16)state.phase=FieldSessionPhase::sampled_field_ai_pass_presented;
 else {
  state.recovery->ball_variant=1;state.control->last_carrier_cell=128;state.control->display_parameter=0;state.control->motion_requested=0;
  state.phase=FieldSessionPhase::sampled_player14_pass_handoff;
 }
 validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_player19_return_shot_approach(const PhaseSessionState& input,
 const PlayerProfileMappings& mappings,const FieldPhaseContent& movement) {
 validate_phase_session(input);
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_shot_clock;
 if(input.phase!=FieldSessionPhase::sampled_player19_return_shot_clock && !score23)
  throw std::invalid_argument("Player19 return shot approach needs its observed dispatch clock");
 auto state=input;state.current.field=advance_field_phase(input.current.field,mappings,movement).state;
 if(score23)state.control->last_carrier_cell=0;
 state.phase=score23 ? FieldSessionPhase::sampled_score23_shot_approach : FieldSessionPhase::sampled_player19_return_shot_approach;validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_field_ai_pass_presentation(const PhaseSessionState& input, const FieldPhaseState& captured) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::sampled_opponent_shot_handoff || input.phase==FieldSessionPhase::sampled_player19_return_shot_handoff)
  return capture_sampled_shot_keeper_prompt(input,captured);
 const bool contact=input.phase==FieldSessionPhase::sampled_field_contact_ai_prepared;
 const bool shot=input.phase==FieldSessionPhase::sampled_opponent_shot_clock;
 const bool player12=input.phase==FieldSessionPhase::sampled_player12_ai_dispatch;
 if((input.phase!=FieldSessionPhase::sampled_field_ai_dispatch && !contact && !shot && !player12) || captured.elapsed!=0 ||
    captured.movement_region!=(contact ? 135 : shot ? 134 : 140))
  throw std::invalid_argument("Unsupported captured opponent pass presentation");
 auto normalized=captured;normalized.motion=input.current.field.motion;normalized.elapsed=input.current.field.elapsed;
 normalized.movement_region=input.current.field.movement_region;
 std::ostringstream before(std::ios::out|std::ios::binary),after(std::ios::out|std::ios::binary);
 write_field_state(before,input.current.field);write_field_state(after,normalized);
 if(before.str()!=after.str())throw std::invalid_argument("Captured pass presentation changed unrelated field state");
 auto state=input;state.current.field=captured;
 if(player12) {
  if(state.recovery->special_owner_flags || state.recovery->ball_variant)
   throw std::invalid_argument("Unsupported sampled player12 pass variant");
  state.recovery->ball_variant=1;state.control->facing=64;state.control->last_carrier_cell=128;
  state.control->display_parameter=0;state.control->motion_requested=0;
  state.phase=FieldSessionPhase::sampled_player12_pass_handoff;
 } else if(contact) {
  --state.current.clock.remaining;state.control->last_carrier_cell=0;
  state.phase=FieldSessionPhase::sampled_field_contact_presentation;
 } else state.phase=shot ? FieldSessionPhase::sampled_opponent_shot_approach : FieldSessionPhase::sampled_field_ai_pass_presented;
 validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_field_ai_pass_handoff(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::sampled_player19_return_shot_approach || input.phase==FieldSessionPhase::sampled_score23_shot_approach) {
  if(input.recovery->special_owner_flags || input.recovery->ball_variant)
   throw std::invalid_argument("Unsupported player19 return shot handoff");
  auto state=input;state.recovery->ball_variant=1;state.control->last_carrier_cell=128;
  state.control->display_parameter=0;state.control->motion_requested=2;
  state.phase=input.phase==FieldSessionPhase::sampled_score23_shot_approach ? FieldSessionPhase::sampled_score23_shot_handoff : FieldSessionPhase::sampled_player19_return_shot_handoff;validate_phase_session(state);return state;
 }
 if(input.phase==FieldSessionPhase::sampled_opponent_shot_approach) {
  if(input.recovery->special_owner_flags!=0 || input.recovery->ball_variant!=0 || input.control->facing!=0 ||
     input.control->display_parameter!=2 || input.control->motion_requested!=1)
   throw std::invalid_argument("Unsupported sampled opponent shot handoff");
  auto state=input;state.recovery->ball_variant=1;state.control->last_carrier_cell=128;
  state.control->display_parameter=0;state.control->motion_requested=2;
  state.phase=FieldSessionPhase::sampled_opponent_shot_handoff;validate_phase_session(state);return state;
 }
 if(input.phase!=FieldSessionPhase::sampled_field_ai_pass_presented || input.recovery->special_owner_flags!=0 || input.recovery->ball_variant!=0 ||
    input.control->facing!=0 || input.control->display_parameter!=2 || input.control->motion_requested!=1)
  throw std::invalid_argument("Unsupported sampled opponent pass handoff");
 auto state=input;state.recovery->ball_variant=1;
 state.control->facing=(input.score && input.score->left_goals==3 && input.score->right_goals==5 && input.current.clock.remaining==296) ? 0 : 64;
 state.control->last_carrier_cell=128;state.control->display_parameter=0;state.control->motion_requested=0;
 state.phase=FieldSessionPhase::sampled_field_ai_pass_handoff;validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_shot_keeper_prompt(const PhaseSessionState& input,const FieldPhaseState& captured) {
 validate_phase_session(input);
 const bool player19_return=input.phase==FieldSessionPhase::sampled_player19_return_shot_handoff;
 if((input.phase!=FieldSessionPhase::sampled_opponent_shot_handoff && !player19_return) || captured.ball_x!=52 || captured.ball_y!=124 ||
    captured.ball_cell!=5 || captured.selected_defenders!=1)
  throw std::invalid_argument("Unsupported captured goalkeeper prompt");
 auto normalized=captured;normalized.ball_x=input.current.field.ball_x;normalized.ball_y=input.current.field.ball_y;
 normalized.ball_cell=input.current.field.ball_cell;normalized.selected_defenders=input.current.field.selected_defenders;
 std::ostringstream before(std::ios::out|std::ios::binary),after(std::ios::out|std::ios::binary);
 write_field_state(before,input.current.field);write_field_state(after,normalized);
 if(before.str()!=after.str())throw std::invalid_argument("Captured goalkeeper prompt changed unrelated field state");
 auto state=input;state.current.field=captured;state.scheduling->selected_ids.fill(0);
 const bool postgoal=input.score && input.score->left_goals==2 && input.score->right_goals==1 && input.current.clock.remaining==381 && input.current.clock.extra_time==13;
 if(postgoal || player19_return)state.saved_ball_flight_flag=0;
 state.recovery->opposing_player=0;
 state.recovery->x_jitter_control=player19_return ? 110 : postgoal ? 144 : 142;
 state.action->menu_route=4;state.action->cursor=0;state.control->motion_requested=0;
 state.phase=player19_return ? FieldSessionPhase::sampled_player19_return_keeper_prompt : FieldSessionPhase::sampled_shot_keeper_prompt;validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_field_pass_target(const PhaseSessionState& input, std::uint8_t recipient, std::uint8_t destination, std::uint8_t counter, std::uint8_t distance) {
 validate_phase_session(input);
 const bool mixed=input.phase==FieldSessionPhase::sampled_mixed_result4_pass;
 const bool player12=input.phase==FieldSessionPhase::sampled_player12_pass_handoff;
 const bool player14=input.phase==FieldSessionPhase::sampled_player14_pass_handoff;
 const bool score35=input.phase==FieldSessionPhase::sampled_field_ai_pass_handoff && input.score &&
    input.score->left_goals==3 && input.score->right_goals==5 && input.current.clock.remaining==296;
 if((input.phase!=FieldSessionPhase::sampled_field_ai_pass_handoff && !mixed && !player12 && !player14) ||
    recipient!=(mixed ? 12 : 19) || destination!=(score35 ? 70 : player14 ? 63 : player12 ? 79 : mixed ? 142 : 103) || counter!=255 || distance!=(score35 ? 4 : 3))
  throw std::invalid_argument("Unsupported sampled opponent pass target");
 const auto& target=input.current.field.motion[field_motion_index(recipient)];
 if(field_cell_at(static_cast<std::uint8_t>(target.x>>8),static_cast<std::uint8_t>(target.y>>8))!=destination)
  throw std::invalid_argument("Captured pass destination disagrees with receiver position");
 auto state=input;state.sampled_field_pass=SampledFieldPassContext{recipient,destination,counter,distance};
 state.phase=player14 ? FieldSessionPhase::sampled_player14_pass_target : player12 ? FieldSessionPhase::sampled_player12_pass_target : mixed ? FieldSessionPhase::sampled_mixed_result4_target : FieldSessionPhase::sampled_field_ai_pass_target;validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_opponent_contact_presentation(const PhaseSessionState& input) {
 validate_phase_session(input);
 const bool third_return=input.phase==FieldSessionPhase::field_contact_pending && input.score && input.action && input.control && input.scheduling &&
    input.current.field.side==11 && input.current.field.ball_owner==18 && input.current.field.ball_x==110 && input.current.field.ball_y==108 &&
    input.current.field.ball_cell==87 && input.current.field.elapsed==1 && input.current.field.selected_defenders==1 &&
    input.scheduling->selected_ids==std::array<std::uint8_t,5>{5,0,0,0,0} && input.current.field.random_first==60 && input.current.field.random_second==131 &&
    input.current.clock.remaining==153 && !input.current.clock.extra_time && !input.current.clock.script_flags && !input.current.clock.ball_mode &&
    input.score->left_goals==1 && input.score->right_goals==0 && input.score->kickoff==4 &&
    !input.control->display_parameter && !input.control->motion_requested && input.action->action==2 && input.action->power==12 && !input.action->cursor;
 const bool postgoal=input.phase==FieldSessionPhase::field_contact_pending && input.score && input.action && input.kickoff_formation &&
    input.experience && input.scheduling && input.control && input.saved_ball_flight_flag==1 &&
    input.current.field.side==11 && input.current.field.ball_owner==19 && input.current.field.ball_x==111 &&
    input.current.field.ball_y==141 && input.current.field.ball_cell==91 && input.current.field.elapsed==1 &&
    input.current.field.selected_defenders==3 && input.scheduling->selected_ids==std::array<std::uint8_t,5>{3,8,9,0,0} &&
    !input.current.field.random_first && !input.current.field.random_second && !input.experience->opponent_stage &&
    input.current.clock.remaining==403 && input.current.clock.extra_time==9 && !input.current.clock.script_flags && !input.current.clock.ball_mode &&
    input.score->left_goals==2 && input.score->right_goals==1 && input.score->kickoff==4 &&
    input.control->display_parameter==2 && input.control->motion_requested==1 && input.action->action==2 && input.action->power==21;
 if(!third_return && !postgoal && (input.phase!=FieldSessionPhase::field_contact_pending || !input.score || !input.action || !input.kickoff_formation ||
    input.saved_ball_flight_flag!=1 || input.current.field.side!=11 || input.current.field.ball_owner!=19 ||
    input.current.field.ball_x!=112 || input.current.field.ball_y!=140 || input.current.field.ball_cell!=103 || input.current.field.elapsed!=1 ||
    input.current.field.selected_defenders!=3 || input.scheduling->selected_ids!=std::array<std::uint8_t,5>{3,7,8,0,0} ||
    input.current.field.random_first || input.current.field.random_second || input.experience->opponent_stage ||
    input.current.clock.remaining!=440 || input.current.clock.extra_time!=5 || input.current.clock.script_flags || input.current.clock.ball_mode ||
    input.score->left_goals!=1 || input.score->right_goals!=0 || input.score->kickoff!=4 ||
    input.control->display_parameter!=2 || input.control->motion_requested!=1 || input.action->action!=1 || input.action->power!=21))
  throw std::invalid_argument("Unsupported sampled opponent contact presentation");
 auto state=input;state.control->display_parameter=0;state.control->motion_requested=0;
 state.phase=FieldSessionPhase::sampled_opponent_contact_presented;validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_field_defense_prompt(const PhaseSessionState& input, std::uint8_t captured_blink) {
 validate_phase_session(input);
 const bool postgoal=input.score && input.score->left_goals==2 && input.score->right_goals==1 && input.current.clock.remaining==403;
 const bool third_return=input.score && input.score->left_goals==1 && !input.score->right_goals && input.current.clock.remaining==153 && input.current.field.ball_owner==18 && input.current.field.ball_cell==87 && input.action && input.action->action==2;
 if(input.phase!=FieldSessionPhase::sampled_opponent_contact_presented || captured_blink!=((postgoal || third_return) ? 13 : 7) || input.action->menu_route || input.action->auxiliary)
  throw std::invalid_argument("Unsupported sampled field defense prompt");
 auto state=input;state.action->cursor=0;state.area_defense=KeeperAreaDefenseContext{};state.area_defense->blink=captured_blink;
 if(third_return)state.action->flash_counter=192;
 state.phase=FieldSessionPhase::sampled_field_defense_prompt;validate_phase_session(state);return state;
}
ControlPollResult poll_session_area_action4_control(const PhaseSessionState& input,std::uint8_t held,std::uint8_t pressed,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement,const ControlMovementContent& control_content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_control_ready && input.phase!=FieldSessionPhase::area_action4_control_wait)
  throw std::invalid_argument("Area action4 polling requires a prepared input wait");
 auto prepared=input;prepared.phase=FieldSessionPhase::control_ready;
 prepared.area_ai_target.reset();prepared.area_defense.reset();prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
 auto result=poll_session_control(prepared,held,pressed,primary,movement,control_content);auto& state=result.state;
 if(state.phase==FieldSessionPhase::ai_cell_decision)
  throw std::invalid_argument("Area action4 AI cell interruption requires original tracing");
 state.area_ai_target=input.area_ai_target;state.area_defense=input.area_defense;
 state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 state.phase=result.decision_step ? FieldSessionPhase::area_action4_control_clock : FieldSessionPhase::area_action4_control_wait;
 validate_phase_session(state);return result;
}
ControlPollResult poll_session_area_pass_capture(const PhaseSessionState& input,std::uint8_t held,std::uint8_t pressed,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement,const ControlMovementContent& control_content) {
 validate_phase_session(input);
 const bool blocked=input.phase==FieldSessionPhase::area_pass_blocked_control_ready || input.phase==FieldSessionPhase::area_pass_blocked_control_wait;
 if((input.phase!=FieldSessionPhase::area_pass_capture_control_ready && input.phase!=FieldSessionPhase::area_pass_capture_control_wait && !blocked) ||
    (blocked && input.current.field.side!=0))
  throw std::invalid_argument("Captured pass polling needs prepared control");
 auto prepared=input;prepared.phase=FieldSessionPhase::control_ready;
 prepared.area_ai_target.reset();prepared.area_contact.reset();prepared.area_pass_contact.reset();
 prepared.shot_flight.reset();prepared.ball_trajectory.reset();prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
 auto result=poll_session_control(prepared,held,pressed,primary,movement,control_content);auto& state=result.state;
 state.area_ai_target=input.area_ai_target;state.area_contact=input.area_contact;state.area_pass_contact=input.area_pass_contact;
 state.shot_flight=input.shot_flight;state.ball_trajectory=input.ball_trajectory;
 state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 state.ball_trajectory->x=static_cast<std::uint16_t>((unsigned(state.current.field.ball_x)<<8)|(input.ball_trajectory->x&255u));
 state.ball_trajectory->y=static_cast<std::uint16_t>((unsigned(state.current.field.ball_y)<<8)|(input.ball_trajectory->y&255u));
 state.phase=blocked ? (result.menu_requested ? FieldSessionPhase::area_pass_blocked_action_menu :
  result.decision_step ? FieldSessionPhase::area_pass_blocked_clock_pending : FieldSessionPhase::area_pass_blocked_control_wait) :
  (result.menu_requested ? FieldSessionPhase::area_pass_capture_action_menu :
  result.decision_step ? FieldSessionPhase::area_pass_capture_clock_pending : FieldSessionPhase::area_pass_capture_control_wait);
 validate_phase_session(state);return result;
}
ControlButtonResult apply_control_buttons(const PhaseSessionState& input, std::uint8_t held, std::uint8_t pressed) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::control_ready && input.phase != FieldSessionPhase::field_round_pending)
        throw std::invalid_argument("Buttons require control-ready or field-round phase");
    ControlButtonResult result{input};
    auto& state = result.state;
    auto& field = state.current.field;
    auto& control = *state.control;
    control.motion_requested = 0;
    if (field.side == 0) {
        state.current.clock.script_flags |= 64u;
        if (held & 64u) {
            field.selected_defenders = 0;
            if (state.scheduling) state.scheduling->selected_ids.fill(0);
            state.current.clock.script_flags = 0;
            state.phase = FieldSessionPhase::action_menu_pending;
            result.menu_requested = true;
            return result;
        }
        if (held & 15u) {
            control.motion_requested = 1;
            control.facing = held & 2u ? 0 : 64;
            state.current.clock.script_flags &= 191u;
        }
    } else {
        control.motion_requested = 1;
        if (pressed & 192u) {
            auto selected = static_cast<std::uint8_t>(unsigned(control.controlled_player) +
                ((pressed & 128u) ? 1u : 255u));
            if (selected == 0) selected = 10;
            if (selected >= 11) selected = 1;
            control.controlled_player = selected;
            result.presentation = {{ControlPresentationKind::display, 49},
                                   {ControlPresentationKind::display, 50}};
        }
    }
    return result;
}

ControlPollResult poll_session_control(const PhaseSessionState& input,
    std::uint8_t held, std::uint8_t pressed, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ControlMovementContent& control_content) {
    if (input.phase != FieldSessionPhase::control_ready)
        throw std::invalid_argument("Input poll requires control-ready phase");
    auto buttons = apply_control_buttons(input, held, pressed);
    ControlPollResult result{std::move(buttons.state), std::move(buttons.presentation), {}, 0, false, buttons.menu_requested};
    if (result.menu_requested) return result;
    auto& state = result.state;
    auto& field = state.current.field;
    auto& control = *state.control;
    if (control.decision_timer != 0) {
        --control.decision_timer;
        return result;
    }
    control.decision_timer = 10;
    result.decision_step = true;
    if (held & 15u) {
        const auto id = field.side == 0 ? field.ball_owner : control.controlled_player;
        const auto& stats = field.roster[id];
        const auto profile = load_movement_profile({id, stats.type, static_cast<std::uint8_t>(stats.stamina),
            static_cast<std::uint8_t>(stats.stamina >> 8), stats.level}, field.side, primary, movement.profiles);
        const auto speed = field.side == 0 ? control_content.carrier_speeds[profile.speed_profile] : profile.speed;
        auto& actor = field.motion[field_motion_index(id)];
        const auto step = [speed](std::uint16_t value, std::uint8_t direction, FieldAxis axis) {
            const auto bits = direction & 3u;
            if (bits == 0) return value;
            const auto displacement = static_cast<std::uint16_t>((bits & 1u) ? unsigned(speed) : 0u - speed);
            return advance_field_coordinate(value, displacement, axis);
        };
        actor.x = step(actor.x, held, FieldAxis::horizontal);
        actor.y = step(actor.y, static_cast<std::uint8_t>(held >> 2), FieldAxis::vertical);
        result.moved_player = id;
        result.movement_speed = speed;
    }
    const auto& carrier = field.motion[field_motion_index(field.ball_owner)];
    field.ball_x = static_cast<std::uint8_t>(carrier.x >> 8);
    field.ball_y = static_cast<std::uint8_t>(carrier.y >> 8);
    field.ball_cell = field_cell_at(field.ball_x, field.ball_y);
    if (field.ball_owner >= 11 && field.ball_cell != control.last_carrier_cell) {
        control.last_carrier_cell = field.ball_cell;
        state.phase = FieldSessionPhase::ai_cell_decision;
    } else state.phase = FieldSessionPhase::control_clock_pending;
    validate_phase_session(state);
    return result;
}
}

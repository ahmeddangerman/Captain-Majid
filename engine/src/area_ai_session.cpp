#include "captain/area_ai_session.hpp"
#include "captain/player_movement.hpp"
#include "captain/defender_selection.hpp"
#include "captain/ball_recovery.hpp"
#include "captain/field_state_io.hpp"
#include "captain/prepared_attack.hpp"
#include <sstream>
#include <stdexcept>
namespace captain {
PhaseSessionState acknowledge_session_area_action4_restart_pass(const PhaseSessionState& input) {
 validate_phase_session(input);const auto& clock=input.current.clock;
 const bool observed=(clock.remaining==428 && clock.extra_time==1 && clock.flags==0) || (clock.remaining==0 && clock.extra_time==33 && clock.flags==0);
 if(input.phase!=FieldSessionPhase::area_action4_restart_action1_entry || !observed)
  throw std::invalid_argument("Untraced action4 restart pass entry");
 auto state=input;state.area_action4_restart_pass_counter=77;state.phase=FieldSessionPhase::area_action4_restart_pass_entry;
 validate_phase_session(state);return state;
}
PhaseSessionState resume_session_area_action4_restart_added_time(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_restart_added_time || input.current.clock.remaining || input.current.clock.extra_time!=13 || input.current.clock.flags!=128)
  throw std::invalid_argument("Untraced action4 restart added-time resume");
 auto state=input;state.phase=FieldSessionPhase::area_action4_restart_clock_done;validate_phase_session(state);return state;
}
PhaseSessionState dispatch_session_area_action4_restart(const PhaseSessionState& input) {
 validate_phase_session(input);const auto& clock=input.current.clock;
 const bool observed=(clock.remaining==428 && clock.extra_time==1 && clock.flags==0) ||
                     (clock.remaining==0 && clock.extra_time==33 && clock.flags==0) ||
                     (clock.remaining==0 && clock.extra_time==13 && clock.flags==128);
 if(input.phase!=FieldSessionPhase::area_action4_restart_clock_done || !observed || !input.area_action4_restart || input.area_action4_restart->action!=1)
  throw std::invalid_argument("Untraced action4 restart action dispatch");
 auto state=input;state.phase=FieldSessionPhase::area_action4_restart_action1_entry;validate_phase_session(state);return state;
}
PhaseSessionResult advance_session_area_action4_restart_clock(const PhaseSessionState& input) {
 validate_phase_session(input);const auto& s=input.current;const auto& f=s.field;const auto& k0=f.keepers[0];const auto& k1=f.keepers[1];
 const bool observed=(s.clock.remaining==446 && s.clock.extra_time==0) || ((s.clock.remaining==18 || s.clock.remaining==17) && s.clock.extra_time==0) ||
                     (s.clock.remaining==0 && (s.clock.extra_time==50 || s.clock.extra_time==1));
 if(input.phase!=FieldSessionPhase::area_action4_keeper_restart_power || !observed || s.clock.flags || s.clock.script_flags || s.clock.ball_mode ||
    s.shared_recovery_flags || s.shared_recovery_timer || f.elapsed!=1 || f.random_first!=255 || f.random_second!=255 ||
    k0.timer || k0.penalty_duration || k0.penalty || k0.stun || k1.timer!=7 || k1.penalty_duration!=1 || k1.penalty || k1.stun)
  throw std::invalid_argument("Untraced action4 restart clock inputs");
 auto state=input;auto& current=state.current;constexpr std::uint8_t elapsed=18;
 current.clock.extra_time=static_cast<std::uint8_t>(unsigned(current.clock.extra_time)+1u);
 current.field.elapsed=static_cast<std::uint8_t>(unsigned(current.field.elapsed)+elapsed);
 const auto recovery=advance_keeper_recovery({current.field.keepers,current.shared_recovery_flags,current.shared_recovery_timer},elapsed);
 current.field.keepers=recovery.keepers;current.shared_recovery_flags=recovery.shared_flags;current.shared_recovery_timer=recovery.shared_timer;
 const auto clock=advance_match_clock(current.clock,elapsed,current.field.random_first);current.clock=clock.state;
 state.phase=clock.continuation==ClockContinuation::period_end ? FieldSessionPhase::area_action4_restart_period_end :
             clock.continuation==ClockContinuation::added_time_presentation ? FieldSessionPhase::area_action4_restart_added_time : FieldSessionPhase::area_action4_restart_clock_done;
 validate_phase_session(state);return {state,ContactPreludeResult{current,clock,elapsed},std::nullopt,{}};
}
PhaseSessionState prepare_session_area_action4_restart_power(const PhaseSessionState& input,bool carry,const ShotContent& content) {
 validate_phase_session(input);const auto& f=input.current.field;
 if(input.phase!=FieldSessionPhase::area_action4_keeper_restart_formation || !carry || f.side!=11 || f.ball_owner!=14 ||
    f.random_first!=255 || f.random_second!=255 || !input.area_action4_restart || input.area_action4_restart->action!=1 || input.area_action4_restart->subtype)
  throw std::invalid_argument("Untraced action4 restart power inputs");
 const auto& player=f.roster[f.ball_owner];
 const auto prepared=prepare_attacking_action({{f.ball_owner,player.type,static_cast<std::uint8_t>(player.stamina),static_cast<std::uint8_t>(player.stamina>>8),player.level},1,0,0,11,f.selected_defenders,f.random_first,f.random_second,carry},content.field_players,content.attack);
 if(prepared.preparation.stamina_cost!=20 || prepared.action_power.power!=21 || prepared.action_power.subtype || prepared.energy.stamina!=player.stamina)
  throw std::invalid_argument("Untraced action4 restart power result");
 auto state=input;state.area_action4_restart_power=AreaAction4RestartPowerContext{prepared.preparation.stamina_cost,prepared.action_power.power};
 state.phase=FieldSessionPhase::area_action4_keeper_restart_power;validate_phase_session(state);return state;
}
PhaseSessionState compute_session_area_action4_restart_formation(const PhaseSessionState& input,const AreaAction4RestartFormationContent& content) {
 validate_phase_session(input);const auto& f=input.current.field;
 if(input.phase!=FieldSessionPhase::area_action4_keeper_restart_prepared || f.side!=11 || f.ball_owner!=14 ||
    !input.area_action4_restart || input.area_action4_restart->choice!=6 || content.cells[0]!=255 || content.cells[11]!=255 || content.owner_cell>=240)
  throw std::invalid_argument("Untraced action4 restart formation setup");
 auto captured=f;
 for(unsigned id=1;id<22;++id) {
  if(id==11)continue;
  const auto index=id<11 ? id+11 : id-11;
  const auto cell=static_cast<std::uint8_t>(id==f.ball_owner ? content.owner_cell : content.cells[index]);
  if(cell==255)continue;
  if(cell>=240)throw std::invalid_argument("Invalid captured restart formation cell");
  auto x=static_cast<std::uint8_t>(52u+8u*(cell/12u));auto y=static_cast<std::uint8_t>(84u+8u*(cell%12u));
  x=static_cast<std::uint8_t>(0u-x);y=static_cast<std::uint8_t>(0u-y);
  auto& player=captured.motion[field_motion_index(static_cast<std::uint8_t>(id))];
  player.x=static_cast<std::uint16_t>((player.x&255u)|(unsigned(x)<<8));player.y=static_cast<std::uint16_t>((player.y&255u)|(unsigned(y)<<8));
 }
 const auto& owner=captured.motion[field_motion_index(f.ball_owner)];
 captured.ball_x=static_cast<std::uint8_t>(owner.x>>8);captured.ball_y=static_cast<std::uint8_t>(owner.y>>8);
 captured.ball_cell=field_cell_at(captured.ball_x,captured.ball_y);
 return acknowledge_session_area_action4_restart_formation(input,captured);
}
PhaseSessionState acknowledge_session_area_action4_restart_formation(const PhaseSessionState& input,const FieldPhaseState& captured) {
 validate_phase_session(input);const auto& f=input.current.field;
 if(input.phase!=FieldSessionPhase::area_action4_keeper_restart_prepared || f.ball_x!=32 || f.ball_y!=100 ||
    captured.ball_x!=52 || captured.ball_y!=172 || captured.ball_cell!=11)
  throw std::invalid_argument("Untraced captured action4 restart formation");
 for(std::size_t i=0;i<f.motion.size();++i) {
  const auto& a=f.motion[i];const auto& b=captured.motion[i];
  if((a.x&255u)!=(b.x&255u) || (a.y&255u)!=(b.y&255u) || a.target_cell!=b.target_cell || a.status!=b.status)
   throw std::invalid_argument("Captured formation changed an unobserved motion field");
 }
 auto normalized=captured;normalized.ball_x=f.ball_x;normalized.ball_y=f.ball_y;normalized.ball_cell=f.ball_cell;normalized.motion=f.motion;
 std::ostringstream expected(std::ios::out|std::ios::binary),actual(std::ios::out|std::ios::binary);
 write_field_state(expected,f);write_field_state(actual,normalized);
 if(expected.str()!=actual.str())throw std::invalid_argument("Captured formation changed unrelated field state");
 auto state=input;state.current.field=captured;state.phase=FieldSessionPhase::area_action4_keeper_restart_formation;
 validate_phase_session(state);return state;
}
PhaseSessionState prepare_session_area_action4_keeper_restart(const PhaseSessionState& input,std::uint8_t recipient,std::uint8_t action,std::uint8_t subtype,std::uint8_t choice) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_keeper_result1_restart_entry || recipient!=14 || action!=1 || subtype!=0 || choice!=6)
  throw std::invalid_argument("Untraced captured action4 keeper restart choice");
 auto state=input;state.current.field.ball_owner=recipient;state.area_action4_restart=AreaAction4RestartContext{action,subtype,choice};
 state.phase=FieldSessionPhase::area_action4_keeper_restart_prepared;validate_phase_session(state);return state;
}
PhaseSessionState route_session_area_action4_keeper_result1_restart(const PhaseSessionState& input) {
 validate_phase_session(input);const auto& f=input.current.field;const auto& left=f.keepers[0];const auto& right=f.keepers[1];
 if(input.phase!=FieldSessionPhase::area_action4_keeper_result1_boundary_check || f.side!=11 || f.ball_owner!=12 ||
    f.ball_x!=32 || f.ball_y!=100 || f.random_first!=255 || f.random_second!=255 || input.recovery->x_jitter_control!=131 ||
    left.timer || left.penalty_duration || left.penalty || left.stun!=6 || right.timer!=7 || right.penalty_duration!=1 || right.penalty!=79 || right.stun)
  throw std::invalid_argument("Untraced action4 keeper result1 restart route");
 auto state=input;state.current.field.keepers[0].stun=0;state.current.field.keepers[1].penalty=0;
 state.phase=FieldSessionPhase::area_action4_keeper_result1_restart_entry;validate_phase_session(state);return state;
}
PhaseSessionState advance_session_area_action4_keeper_result1_jitter(const PhaseSessionState& input) {
 validate_phase_session(input);
 const auto& f=input.current.field;
 if(input.phase!=FieldSessionPhase::area_action4_keeper_result1_recovery || f.ball_owner!=12 ||
    f.ball_x!=56 || f.ball_y!=124 || f.random_first!=255 || f.random_second!=255 || input.recovery->x_jitter_control!=0)
  throw std::invalid_argument("Untraced action4 keeper result1 jitter");
 const auto position=jitter_loose_ball(f.ball_x,f.ball_y,f.random_first,f.random_second);
 auto state=input;state.current.field.ball_x=position.x;state.current.field.ball_y=position.y;
 state.recovery->x_jitter_control=position.x_jitter_control;state.phase=FieldSessionPhase::area_action4_keeper_result1_boundary_check;
 validate_phase_session(state);return state;
}
PhaseSessionState recover_session_sampled_keeper_result4_action(const PhaseSessionState& input,const ExperienceContent& experience) {
 validate_phase_session(input);const auto& f=input.current.field;const auto& c=input.current.clock;const auto& keeper=f.keepers[0];
 if(input.phase!=FieldSessionPhase::sampled_keeper_result4_extra_time || !input.experience || !input.recovery || !input.control ||
    c.remaining!=446 || c.extra_time!=5 || c.flags || c.script_flags || c.ball_mode ||
    f.side!=11 || f.ball_owner!=20 || f.ball_x!=68 || f.ball_y!=132 || f.ball_cell!=30 || f.random_first || f.random_second ||
    keeper.timer || keeper.penalty_duration || keeper.penalty || keeper.stun)
  throw std::invalid_argument("Unsupported sampled keeper result4 recovery");
 auto state=input;state.experience=award_player_experience(*input.experience,f.roster[0].type,3,experience).state;
 state.current.field.ball_x=80;state.current.field.ball_y=128;state.current.field.selected_defenders=0;state.current.field.keepers[0]={7,4,79,0};
 state.scheduling->selected_ids.fill(0);state.recovery->attack_subtype=128;state.control->display_parameter=0;state.action=ActionContext{9,5,128,128,0,0,0,21};
 state.area_ai_target.reset();state.area_defense.reset();state.area_contact.reset();state.area_action4_restart.reset();state.area_action4_restart_power.reset();
 state.area_defender_cursor=0;state.area_sequence_result=255;state.phase=FieldSessionPhase::ball_recovery;
 validate_phase_session(state);return state;
}
PhaseSessionState advance_session_sampled_keeper_result4_extra_time(const PhaseSessionState& input) {
 validate_phase_session(input);const auto& c=input.current.clock;
 const bool observed=(c.remaining==446 && (c.extra_time==0 || c.extra_time==250 || c.extra_time==251 || c.extra_time==255)) ||
    ((c.remaining==5 || c.remaining==4) && c.extra_time==0) || (c.remaining==0 && (c.extra_time==50 || c.extra_time==1));
 if(input.phase!=FieldSessionPhase::sampled_keeper_result4_action || !observed || c.flags || c.script_flags || c.ball_mode)
  throw std::invalid_argument("Unsupported sampled result4 extra-time input");
 auto state=input;state.current.clock.extra_time=static_cast<std::uint8_t>(unsigned(c.extra_time)+5u);
 state.phase=FieldSessionPhase::sampled_keeper_result4_extra_time;validate_phase_session(state);return state;
}
PhaseSessionState prepare_session_sampled_keeper_result4_action(const PhaseSessionState& input,std::uint8_t action,std::uint8_t subtype,std::uint8_t context,std::uint16_t cost,std::uint16_t power) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_keeper_result4_formation || action!=9 || subtype!=128 || context!=5 || cost!=40 || power!=21)
  throw std::invalid_argument("Unsupported captured keeper result4 action");
 auto state=input;state.area_action4_restart=AreaAction4RestartContext{action,subtype,context};
 state.area_action4_restart_power=AreaAction4RestartPowerContext{cost,power};state.phase=FieldSessionPhase::sampled_keeper_result4_action;
 validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_session_sampled_keeper_result4_formation(const PhaseSessionState& input,const FieldPhaseState& captured) {
 validate_phase_session(input);const auto& f=input.current.field;
 if(input.phase!=FieldSessionPhase::sampled_keeper_result4_restart || captured.ball_owner!=20 || captured.ball_x!=68 || captured.ball_y!=132 || captured.ball_cell!=30)
  throw std::invalid_argument("Unsupported captured keeper result4 formation");
 const auto& owner=captured.motion[field_motion_index(20)];
 if((owner.x>>8)!=captured.ball_x || (owner.y>>8)!=captured.ball_y)throw std::invalid_argument("Captured result4 owner position disagrees with ball");
 for(std::size_t i=0;i<f.motion.size();++i) {
  const auto& a=f.motion[i];const auto& b=captured.motion[i];
  if((a.x&255u)!=(b.x&255u) || (a.y&255u)!=(b.y&255u) || a.target_cell!=b.target_cell || a.status!=b.status)
   throw std::invalid_argument("Captured result4 formation changed unsupported motion fields");
 }
 auto normalized=captured;normalized.ball_owner=f.ball_owner;normalized.ball_x=f.ball_x;normalized.ball_y=f.ball_y;normalized.ball_cell=f.ball_cell;normalized.motion=f.motion;
 std::ostringstream expected(std::ios::out|std::ios::binary),actual(std::ios::out|std::ios::binary);
 write_field_state(expected,f);write_field_state(actual,normalized);
 if(expected.str()!=actual.str())throw std::invalid_argument("Captured result4 formation changed unrelated field state");
 auto state=input;state.current.field=captured;state.phase=FieldSessionPhase::sampled_keeper_result4_formation;
 validate_phase_session(state);return state;
}
PhaseSessionState route_session_sampled_keeper_result4(const PhaseSessionState& input,bool captured_carry,std::uint8_t captured_side) {
 validate_phase_session(input);const auto& f=input.current.field;const auto& c=input.current.clock;
 if(input.phase!=FieldSessionPhase::area_action4_keeper_dispatch_entry || input.area_contact->result!=4 || !captured_carry || captured_side!=0 ||
    f.side!=11 || f.ball_owner!=12 || f.ball_x!=56 || f.ball_y!=124 || f.ball_cell!=17 || f.random_first || f.random_second ||
    f.roster[0].stamina || f.keepers[1].penalty || c.remaining!=446 || c.extra_time || c.flags || c.script_flags || c.ball_mode)
  throw std::invalid_argument("Unsupported sampled keeper result4 restart branch");
 auto state=input;state.phase=FieldSessionPhase::sampled_keeper_result4_restart;validate_phase_session(state);return state;
}
PhaseSessionState continue_session_area_action4_keeper_result0_goal(const PhaseSessionState& input) {
 validate_phase_session(input);
 const auto& f=input.current.field;
 if(input.phase!=FieldSessionPhase::area_action4_keeper_result0_goal || f.side!=11 || f.ball_owner!=12 ||
    f.ball_x!=56 || f.ball_y!=124 || f.ball_cell!=17 || f.random_first!=127 || f.random_second!=128 ||
    input.current.clock.remaining!=446 || input.current.clock.extra_time || input.current.clock.ball_mode ||
    f.keepers[0].timer || f.keepers[0].penalty || f.keepers[0].stun!=6)
  throw std::invalid_argument("Unsupported sampled result0 post-goal handoff");
 auto state=input;state.current.field.side=0;state.current.field.keepers[0].stun=0;state.score->kickoff=4;
 state.area_ai_target.reset();state.area_defense.reset();state.area_contact.reset();state.action=ActionContext{0,3,128,128,0,0,0,22};state.area_defender_cursor=0;state.area_sequence_result=255;
 state.phase=FieldSessionPhase::goal_kickoff_pending;validate_phase_session(state);return state;
}
PhaseSessionState prepare_session_area_action4_keeper_result0_goal(const PhaseSessionState& input,const ExperienceContent& experience) {
 validate_phase_session(input);
 const auto& f=input.current.field;const auto& c=input.current.clock;
 if(input.phase!=FieldSessionPhase::area_action4_keeper_result0_entry || !input.score || !input.experience ||
    input.score->left_goals || input.score->right_goals || input.score->kickoff || f.side!=11 || f.ball_owner!=12 ||
    f.ball_x!=56 || f.ball_y!=124 || f.ball_cell!=17 || f.random_first!=127 || f.random_second!=128 ||
    c.remaining!=446 || c.extra_time || c.script_flags || c.ball_mode)
  throw std::invalid_argument("Unsupported sampled action4 keeper result0 goal");
 auto state=input;state.experience=award_player_experience(*input.experience,f.roster[f.ball_owner].type,0,experience).state;
 state.score->right_goals=1;state.phase=FieldSessionPhase::area_action4_keeper_result0_goal;
 validate_phase_session(state);return state;
}
PhaseSessionState continue_session_area_action4_keeper_result(const PhaseSessionState& input,std::uint8_t attack_action,const ExperienceContent& experience) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_keeper_dispatch_entry || !input.experience || attack_action!=0 ||
    input.current.field.ball_owner!=12 || input.area_contact->result>1 ||
    !((input.current.field.random_first==127 && input.current.field.random_second==128) ||
      (input.current.field.random_first==255 && input.current.field.random_second==255)))
  throw std::invalid_argument("Untraced action4 goalkeeper result handler");
 auto state=input;
 state.experience=award_player_experience(*input.experience,state.current.field.roster[12].type,1,experience).state;
 state.current.field.keepers[0].stun=6;
 state.phase=state.area_contact->result==0 ? FieldSessionPhase::area_action4_keeper_result0_entry : FieldSessionPhase::area_action4_keeper_result1_recovery;
 validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_session_area_action4_keeper_result(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_keeper_compared)
  throw std::invalid_argument("Keeper result dispatch needs its saved comparison");
 auto state=input;state.phase=FieldSessionPhase::area_action4_keeper_dispatch_entry;
 validate_phase_session(state);return state;
}
PhaseSessionState compare_session_area_action4_keeper(const PhaseSessionState& input,std::uint16_t attack_power,std::uint8_t attack_action,std::uint8_t attack_subtype,std::uint8_t adjustment) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_keeper_stamina_done)
  throw std::invalid_argument("Keeper comparison requires its saved stamina boundary");
 const auto& f=input.current.field;const auto& c=*input.area_contact;
 const bool ordinary_random=(f.random_first==127 && f.random_second==128) || (f.random_first==255 && f.random_second==255);
 const auto penalty=f.keepers[1].penalty;
 const bool observed_override=(f.random_first==0 && f.random_second==0) || (f.random_first==1 && f.random_second==1);
 const bool observed_penalty=penalty==0 || penalty==1 || penalty==25 || penalty==79 || penalty==80 || penalty==127 || penalty==255;
 if((!ordinary_random && !(observed_override && f.ball_owner==12 && observed_penalty)) || attack_power!=22 || attack_action!=0 || attack_subtype!=128 || adjustment ||
    f.roster[0].type!=1 || c.cost!=50 || c.defender_power!=8)
  throw std::invalid_argument("Keeper comparison inputs have not been traced");
 const auto comparison=compare_defender_contact({attack_power,c.defender_power,f.random_first,f.random_second,218,37,c.weakened,true,3,0,0,{168,90,82,0}});
 auto state=input;auto& saved=*state.area_contact;
 saved.defense_subtype=comparison.subtype;saved.strength=comparison.strength;saved.loss=comparison.loss;
 saved.score=comparison.score;saved.result=observed_override ? 4 : comparison.result_code;state.area_sequence_result=saved.result;
 if(observed_override)state.current.field.keepers[1].penalty=0;
 state.phase=FieldSessionPhase::area_action4_keeper_compared;validate_phase_session(state);return state;
}
PhaseSessionState spend_session_area_action4_keeper_stamina(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_keeper_prepared)
  throw std::invalid_argument("Keeper stamina requires its prepared action4 boundary");
 const auto& keeper=input.current.field.roster[0];
 if(keeper.type!=1 || input.area_contact->cost!=50 || input.area_contact->defender_power!=8 ||
    (keeper.stamina!=0 && keeper.stamina!=49 && keeper.stamina!=50 && keeper.stamina!=51))
  throw std::invalid_argument("Keeper stamina case has not been traced");
 auto state=input;
 const auto energy=calculate_energy({keeper.stamina,input.area_contact->cost,keeper.type,0,6,0,false,false});
 state.current.field.roster[0].stamina=energy.stamina;state.area_contact->weakened=energy.weakened;
 state.phase=FieldSessionPhase::area_action4_keeper_stamina_done;validate_phase_session(state);return state;
}
PhaseSessionState prepare_session_area_action4_keeper(const PhaseSessionState& input,std::uint16_t cost,std::uint16_t power) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_keeper_presented)
  throw std::invalid_argument("Area action4 goalkeeper preparation requires acknowledged presentation");
 auto state=input;state.area_contact=AreaContactContext{0,6,0,0,power,cost,0,0,0,0,false};
 state.phase=FieldSessionPhase::area_action4_keeper_prepared;validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_session_area_action4_keeper(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_keeper_entry || (facing!=0 && facing!=64))
  throw std::invalid_argument("Area action4 keeper presentation requires its saved entry and captured facing");
 auto state=input;state.control->facing=facing;state.control->motion_requested=motion;
 state.phase=FieldSessionPhase::area_action4_keeper_presented;validate_phase_session(state);return state;
}
PhaseSessionState begin_session_area_action4_keeper(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_goal_area)
  throw std::invalid_argument("Area action4 keeper entry requires its goal-area boundary");
 auto state=input;state.action->menu_route=3;state.current.field.selected_defenders=1;
 state.scheduling->selected_ids.fill(0);state.recovery->opposing_player=0;
 state.phase=FieldSessionPhase::area_action4_keeper_entry;validate_phase_session(state);return state;
}
PhaseSessionState continue_session_area_action4_control(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_control_pending)
  throw std::invalid_argument("Area action4 control entry requires its saved no-encounter route");
 auto state=input;state.phase=FieldSessionPhase::area_action4_control_entry;validate_phase_session(state);return state;
}
PhaseSessionState begin_session_area_action4_encounters(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_encounters_pending)
  throw std::invalid_argument("Area action4 field entry requires the filtered encounter prefix");
 auto state=input;state.recovery->ball_variant=0;state.action->menu_route=0;
 state.phase=FieldSessionPhase::area_action4_field_entry;validate_phase_session(state);return state;
}
AreaDispatchPreview preview_area_result_dispatch(const PhaseSessionState& input,const ShotContent& content) {
 validate_phase_session(input);
 if((input.phase!=FieldSessionPhase::keeper_area_prefix_clock_done && input.phase!=FieldSessionPhase::keeper_area_terminal_result) || !content.movement)
  throw std::invalid_argument("Area dispatch needs a ready result and original movement content");
 const auto route=input.phase==FieldSessionPhase::keeper_area_terminal_result ? input.area_contact->result : 0;
 return {advance_field_phase(input.current.field,content.field_players,*content.movement),static_cast<std::uint8_t>(route)};
}
AreaResultDispatch advance_session_area_result_dispatch(const PhaseSessionState& input,const ShotContent& content) {
 validate_phase_session(input);
 if((input.phase!=FieldSessionPhase::keeper_area_prefix_clock_done && input.phase!=FieldSessionPhase::keeper_area_terminal_result) || !content.movement)
  throw std::invalid_argument("Area dispatch needs a ready result and original movement content");
 const bool terminal=input.phase==FieldSessionPhase::keeper_area_terminal_result;
 const auto result=terminal ? input.area_contact->result : input.area_sequence_result;
 if((terminal && result<2) || (!terminal && result>1))
  throw std::invalid_argument("Area dispatch needs a known final sequence result");
 const auto movement=advance_field_phase(input.current.field,content.field_players,*content.movement);
 AreaResultDispatch dispatched{input,movement,std::nullopt,0,false};
 auto& state=dispatched.state;
 state.current.field=movement.state;
 if(!terminal) {
  if(input.action->action!=4 && input.action->action!=6)state.area_defense.reset();
  state.phase=FieldSessionPhase::keeper_area_attack_continuation;
 } else {
  const auto& contact=*input.area_contact;
  const auto defender=input.scheduling->selected_ids[contact.cursor];
  if(result==3) {
   if(!input.experience || !content.experience)
    throw std::invalid_argument("Area result3 dispatch requires experience state and content");
   auto reward=award_player_experience(*input.experience,state.current.field.roster[defender].type,2,*content.experience);
   state.experience=reward.state;
   dispatched.reward=reward;
  }
  if(result==3 && contact.defense_action==2) {
   const auto side=static_cast<std::uint8_t>(defender<11 ? 0 : 11);
   dispatched.request_possession_sound=side!=state.current.field.side;
   state.current.field.ball_owner=defender;
   state.current.field.side=side;
   state.keeper_area.reset();state.area_defense.reset();state.area_ai_target.reset();state.area_contact.reset();
   state.area_defender_cursor=0;state.area_sequence_result=255;
   state.action=ActionContext{};
   state.phase=FieldSessionPhase::player_control;
   dispatched.route=3;
  } else {
   state.phase=FieldSessionPhase::keeper_area_recovery_ready;
   dispatched.route=2;
  }
 }
 validate_phase_session(state);
 return dispatched;
}
PhaseSessionState continue_session_area_action6(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion,const AreaAction6DestinationContent& content,const ExperienceContent* experience) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_attack_continuation || input.action->action!=6 || (facing!=0 && facing!=64))
  throw std::invalid_argument("Area action6 needs a saved continuation and acknowledged presentation");
 const auto& f=input.current.field;const auto& p=content.positions[f.random_first&7u];
 const auto x=static_cast<std::uint8_t>((f.ball_x&128u)?p[0]^255u:p[0]);
 const auto y=static_cast<std::uint8_t>((f.ball_y&128u)?p[1]^255u:p[1]);
 const auto cell=field_cell_at(x,y);if(cell==255)throw std::invalid_argument("Area action6 destination is outside the represented field");
 auto state=input;
 if(input.current.field.selected_defenders) {
  if(!experience || !input.experience)throw std::invalid_argument("Area continuation requires experience content");
  state.experience=award_player_experience(*input.experience,f.roster[f.ball_owner].type,1,*experience).state;
 }
 state.current.field.ball_x=x;state.current.field.ball_y=y;state.current.field.ball_cell=cell;
 state.area_ai_target->cell=cell;state.control->facing=facing;state.control->motion_requested=motion;
 state.keeper_area.reset();state.phase=FieldSessionPhase::area_action6_destination_pending;
 validate_phase_session(state);return state;
}
PhaseSessionState continue_session_area_action4(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion,const ExperienceContent* experience) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_attack_continuation || input.action->action!=4 || !input.area_defense || (facing!=0 && facing!=64))
  throw std::invalid_argument("Area action4 needs retained defender choices and acknowledged presentation");
 auto state=input;
 if(input.current.field.selected_defenders) {
  if(!experience || !input.experience)throw std::invalid_argument("Area continuation requires experience content");
  state.experience=award_player_experience(*input.experience,input.current.field.roster[input.current.field.ball_owner].type,1,*experience).state;
 }
 std::array<std::uint8_t,5> filtered{};unsigned count=0;
 for(unsigned i=0;i<input.current.field.selected_defenders;++i)
  if(input.area_defense->actions[i]==5)filtered[count++]=input.scheduling->selected_ids[i];
 if(count){state.current.field.selected_defenders=static_cast<std::uint8_t>(count);state.scheduling->selected_ids=filtered;}
 state.recovery->ball_variant=0;state.control->facing=facing;state.control->motion_requested=motion;
 state.keeper_area.reset();
 state.phase=count?FieldSessionPhase::area_action4_encounters_pending:FieldSessionPhase::area_action4_control_pending;
 validate_phase_session(state);return state;
}
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
PhaseSessionResult advance_session_area_prefix_clock(const PhaseSessionState& input,std::uint8_t elapsed) {
 const auto calculated=calculate_area_prefix_clock(input,elapsed);auto state=input;state.current=calculated.state;
 state.phase=calculated.clock.continuation==ClockContinuation::period_end ? FieldSessionPhase::keeper_area_prefix_period_end :
  calculated.clock.continuation==ClockContinuation::added_time_presentation ? FieldSessionPhase::keeper_area_prefix_added_time : FieldSessionPhase::keeper_area_prefix_clock_done;
 validate_phase_session(state);return {state,calculated,std::nullopt,{}};
}
PhaseSessionResult advance_profiled_area_prefix_clock(const PhaseSessionState& input,const ShotContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_prefix_done || !content.area_prefix_elapsed)
  throw std::invalid_argument("Area prefix clock requires saved prefix and original timing content");
 return advance_session_area_prefix_clock(input,(*content.area_prefix_elapsed)[input.action->action]);
}
PhaseSessionState resume_session_area_prefix_clock(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_prefix_added_time)throw std::invalid_argument("No area prefix added-time presentation to resume");
 auto state=input;state.phase=FieldSessionPhase::keeper_area_prefix_clock_done;validate_phase_session(state);return state;
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
 state.area_sequence_result=c.result_code;
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
 state.area_sequence_result=0;
 state.control->facing=facing;state.control->motion_requested=16;
 state.phase=FieldSessionPhase::keeper_area_defenders_announced;
 validate_phase_session(state);return result;
}
ActionMenuResult begin_prepared_area_defense(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_attack_prepared)throw std::invalid_argument("Area defense entry requires prepared attack");
 ActionMenuResult result{input};result.presentation.push_back({ControlPresentationKind::script,14});
 result.state.area_sequence_result=0;result.state.area_defender_cursor=0;
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

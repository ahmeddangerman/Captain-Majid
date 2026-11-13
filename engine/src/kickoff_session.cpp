#include "captain/kickoff_session.hpp"
#include "captain/kickoff_selection.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState continue_prepared_session_kickoff(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::kickoff_ball_placed)
  throw std::invalid_argument("Kickoff handoff requires ball placement and acknowledged presentation");
 auto state=input;
 state.current.field.ball_owner=*state.kickoff_receiver;
 state.control->controlled_player=1;
 // Captured interval kickoff display leaves this travel scratch before control resumes.
 if(state.score && state.score->kickoff==1 && state.score->left_goals==1 && !state.score->right_goals &&
    state.current.clock.remaining==180 && state.current.field.side==11 && *state.kickoff_receiver==18) {
  state.control->travel_accumulator=0x1bff;state.control->facing=64;
 }
 // Captured first kickoff: the acknowledged original display sets facing64.
 if(state.score && state.score->kickoff==0 && !state.score->left_goals && !state.score->right_goals &&
    state.current.clock.remaining==180 && state.current.field.side==0 &&
    (*state.kickoff_receiver==5 || *state.kickoff_receiver==6 || *state.kickoff_receiver==7 || *state.kickoff_receiver==9 || *state.kickoff_receiver==10) &&
    state.current.field.roster[9].type==1 && state.experience && state.experience->campaign==0 && state.experience->opponent_stage==0)
  state.control->facing=64;
 // The sampled 1-1 restart acknowledges the original kickoff display's facing write.
 if(state.score && state.score->kickoff==4 &&
    ((state.score->left_goals==1 && state.score->right_goals==1 && ((state.current.clock.remaining==434 && state.current.clock.extra_time==5) || (state.current.clock.remaining==418 && state.current.clock.extra_time==9))) ||
     (state.score->left_goals==2 && state.score->right_goals==2 && state.current.clock.remaining==381 && state.current.clock.extra_time==13) ||
     (state.score->left_goals==2 && state.score->right_goals==3 && state.current.clock.remaining==360 && state.current.clock.extra_time==13) ||
     (state.score->left_goals==2 && state.score->right_goals==4 && state.current.clock.remaining==348 && state.current.clock.extra_time==13) ||
     (state.score->left_goals==2 && state.score->right_goals==5 && state.current.clock.remaining==335 && state.current.clock.extra_time==13) ||
     (state.score->left_goals==3 && state.score->right_goals==6 && state.current.clock.remaining==293 && state.current.clock.extra_time==13)) && state.current.field.side==0 && *state.kickoff_receiver==5 &&
    state.current.field.selected_defenders==1 && state.scheduling->selected_ids[0]==0)
  state.control->facing=0;
 state.phase=FieldSessionPhase::kickoff_control_pending;
 validate_phase_session(state);
 return state;
}
PhaseSessionState prepare_session_kickoff_ball(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::kickoff_receiver_selected)
  throw std::invalid_argument("Ball placement requires a selected kickoff receiver");
 auto state=input;
 auto& field=state.current.field;
 const auto& owner=field.motion[field_motion_index(field.ball_owner)];
 field.ball_x=static_cast<std::uint8_t>(owner.x>>8);
 field.ball_y=static_cast<std::uint8_t>(owner.y>>8);
 field.ball_cell=field_cell_at(field.ball_x,field.ball_y);
 state.phase=FieldSessionPhase::kickoff_ball_placed;
 validate_phase_session(state);
 return state;
}
PhaseSessionState prepare_session_kickoff_receiver(const PhaseSessionState& input,std::uint8_t random,const std::array<std::uint8_t,5>& candidates) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::kickoff_formation_placed)
  throw std::invalid_argument("Receiver selection requires a placed kickoff formation");
 if(input.current.field.side==0 && input.current.field.elapsed==1 && input.current.field.selected_defenders==1 &&
    input.score && input.score->left_goals==0 && input.score->right_goals==1 &&
    (random!=127 || candidates!=std::array<std::uint8_t,5>{5,6,7,9,10}))
  throw std::invalid_argument("Sampled right-side goal kickoff needs its observed receiver inputs");
 auto state=input;
 state.kickoff_receiver=select_kickoff_receiver(state.current.field.side,random,candidates);
 state.phase=FieldSessionPhase::kickoff_receiver_selected;
 validate_phase_session(state);
 return state;
}
PhaseSessionState prepare_session_kickoff_formation(const PhaseSessionState& input,std::uint8_t selected_side,const KickoffFormationContent& content,bool runtime_reset_completed) {
 validate_phase_session(input);
 const bool sampled_draw=input.phase==FieldSessionPhase::sampled_opponent_goal_restart || input.phase==FieldSessionPhase::sampled_player19_return_goal_restart || input.phase==FieldSessionPhase::sampled_score24_goal_restart;
 if((input.phase!=FieldSessionPhase::goal_kickoff_pending && !sampled_draw) ||
    (selected_side!=0 && selected_side!=11) || (sampled_draw && selected_side!=0))
  throw std::invalid_argument("Formation requires prepared kickoff and selected side0/11");
 const auto& field=input.current.field;
 if(field.side==0 && field.ball_owner==12 && field.elapsed==1 && input.score && input.score->left_goals==0 && input.score->right_goals==1 && selected_side!=0)
  throw std::invalid_argument("Sampled right-side goal requires the observed side0 kickoff");
 std::array<KickoffPosition,22> positions{};
 for(unsigned slot=0;slot<22;++slot){
  if(slot==0 || slot==11){const auto& k=field.keepers[slot==0?0:1];
   // Original overwrites keeper Y high and clears its low byte; old Y high is immaterial.
   positions[slot]={static_cast<std::uint16_t>(k.timer|(unsigned(k.penalty_duration)<<8)),k.penalty};
  }else{const auto& m=field.motion[field_motion_index(slot)];positions[slot]={m.x,m.y};}
 }
 const auto placed=place_kickoff_formation(positions,selected_side,field.teams[0].formation,field.teams[1].formation,content);
 auto state=input;state.current.field.side=selected_side;state.current.field.ball_owner=selected_side+8;
 if(sampled_draw)state.keeper_menu_action.reset();
 if(input.phase==FieldSessionPhase::sampled_score24_goal_restart) {
  for(auto& player:state.current.field.motion)player.status=0;
  for(auto& keeper:state.current.field.keepers)keeper.stun=0;
 }
 // The captured 1-0 restart resets these statuses before the formation routine.
 if(!runtime_reset_completed && input.phase==FieldSessionPhase::goal_kickoff_pending && selected_side==11 && input.score &&
    input.score->left_goals==1 && input.score->right_goals==0 && input.score->kickoff==4 &&
    input.current.clock.remaining==160 && input.current.clock.extra_time==0 && field.side==11 &&
    field.ball_owner==8 && field.ball_cell==234 && field.selected_defenders==1 &&
    input.scheduling && input.scheduling->selected_ids[0]==11) {
  for(unsigned id=1;id<=21;++id) {
   if(id==11)continue;
   const auto expected=id==3?5u:(id==14 || id==16)?6u:(id==19 || id==20)?1u:0u;
   if(field.motion[field_motion_index(id)].status!=expected)
    throw std::invalid_argument("Unobserved 1-0 restart status pattern");
  }
  if(field.keepers[0].stun || field.keepers[1].stun)
   throw std::invalid_argument("Unobserved 1-0 restart goalkeeper status");
  for(auto& player:state.current.field.motion)player.status=0;
 }
 KickoffFormationContext captured{};
 for(unsigned slot=0;slot<22;++slot){
  if(slot==0 || slot==11){const auto i=slot==0?0u:1u;auto& k=state.current.field.keepers[i];
   k.timer=placed[slot].x&255;k.penalty_duration=placed[slot].x>>8;k.penalty=placed[slot].y&255;
   captured.keeper_y[i]=placed[slot].y>>8;
  }else{auto& m=state.current.field.motion[field_motion_index(slot)];m.x=placed[slot].x;m.y=placed[slot].y;}
 }
 // The observed 2-1 restart runs the original status reset before formation placement.
 if(!runtime_reset_completed && input.phase==FieldSessionPhase::goal_kickoff_pending && selected_side==11 && input.score &&
    input.score->left_goals==2 && input.score->right_goals==1 && input.score->kickoff==4 &&
    input.current.clock.remaining==408 && input.current.clock.extra_time==9 && field.side==11 &&
    field.ball_owner==5 && field.ball_cell==233 && field.selected_defenders==1 &&
    input.scheduling && input.scheduling->selected_ids[0]==11 && field.roster[5].type==23 &&
    field.motion[field_motion_index(17)].status==6)
 {
  for(auto& player:state.current.field.motion)player.status=0;
  for(auto& keeper:state.current.field.keepers)keeper.stun=0;
 }
 state.kickoff_formation=captured;state.phase=FieldSessionPhase::kickoff_formation_placed;
 validate_phase_session(state);return state;
}
}

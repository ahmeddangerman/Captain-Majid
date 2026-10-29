#include "captain/keeper_save_session.hpp"
#include <stdexcept>
namespace captain {
KeeperSaveResult resolve_session_keeper_save(const PhaseSessionState& input,const ShotContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_decided || input.keeper_decision->result_code<2 || !content.experience)
  throw std::invalid_argument("Keeper save requires a prepared normal result2/3 and experience content");
 const auto& f=input.current.field;const auto& decision=*input.keeper_decision;
 const auto slot=static_cast<std::uint8_t>(f.side^11u);const auto index=f.side==0?1u:0u;
 const auto& keeper=f.keepers[index];
 ShotOutcomeInput outcome{};
 // The saved decision already applied story rules. This guarded result2/3
 // caller uses only non-goal handlers; score/kickoff fields are unused.
 outcome.comparison_result=decision.result_code;outcome.possession_side=f.side;
 outcome.attack_subtype=input.recovery->attack_subtype;outcome.defense_action=decision.defense_action;
 outcome.random_first=f.random_first;outcome.random_second=f.random_second;
 outcome.timer_selector=input.action->move_flags;outcome.keeper_timer=keeper.timer;
 outcome.keeper_penalty=keeper.penalty;outcome.keeper_penalty_duration=keeper.penalty_duration;
 outcome.keeper_stun=keeper.stun;outcome.ball_owner=f.ball_owner;outcome.ball_x=f.ball_x;
 outcome.ball_y=f.ball_y;outcome.defenders=f.selected_defenders;outcome.ball_active=f.elapsed;
 outcome.punch_intervals=content.punch_intervals;
 const auto resolved=resolve_shot_outcome(outcome);
 auto state=input;const auto reward=award_player_experience(*state.experience,f.roster[slot].type,3,*content.experience);
 state.experience=reward.state;state.current.field.side=resolved.possession_side;
 state.current.field.ball_owner=resolved.ball_owner;state.current.field.ball_x=resolved.ball_x;
 state.current.field.ball_y=resolved.ball_y;state.current.field.elapsed=resolved.ball_active;
 state.current.field.selected_defenders=resolved.defenders;
 state.current.field.keepers[index]={resolved.keeper_timer,resolved.keeper_penalty_duration,resolved.keeper_penalty,resolved.keeper_stun};
 if(!resolved.defenders)state.scheduling->selected_ids.fill(0);
 state.shot_flight.reset();state.ball_trajectory.reset();state.keeper_decision.reset();
 state.phase=resolved.event==ShotEvent::rebound ? FieldSessionPhase::ball_recovery :
  resolved.event==ShotEvent::keeper_catch ? FieldSessionPhase::keeper_possession : FieldSessionPhase::keeper_punch_recovery;
 validate_phase_session(state);return {std::move(state),resolved.event,reward,f.side!=resolved.possession_side};
}
}

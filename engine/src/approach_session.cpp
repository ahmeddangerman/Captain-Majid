#include "captain/approach_session.hpp"
#include <stdexcept>
namespace captain {
SessionApproachResult prepare_session_shot_approach(const PhaseSessionState& input,const ShotApproachContent& content) {
 validate_phase_session(input);
 const bool returned=input.phase==FieldSessionPhase::ordinary_return_shot_field_moved;
 const bool returned_goal=input.phase==FieldSessionPhase::ordinary_return_goal_area_handoff;
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_shot_handoff;
 const bool player7=input.phase==FieldSessionPhase::attack_prepared && input.human_defense_choices && input.score &&
  input.current.field.ball_owner==7 && input.current.field.elapsed==0 &&
  ((input.score->left_goals==2 && input.score->right_goals==5 && input.current.clock.remaining==304 && input.area_defender_cursor==5) ||
   (input.score->left_goals==3 && input.score->right_goals==6 && input.current.clock.remaining==263 && input.area_defender_cursor==4));
 if(input.phase!=FieldSessionPhase::shot_approach_ready && !score23 && !player7 && !returned && !returned_goal)
  throw std::invalid_argument("Shot approach requires completed attacking clock");
 const auto& field=input.current.field;
 const auto approach=calculate_shot_approach({field.ball_x,field.ball_y,field.side,
  input.recovery->attack_subtype,field.random_first},content);
 const bool score35=score23 && input.score && input.score->left_goals==3 && input.score->right_goals==5;
 if(score23 && (approach.distance_adjustment!=(score35 ? 36 : 45) || approach.destination_cell!=5 || approach.random_first || approach.loose_ball))
  throw std::invalid_argument("Unverified 2-3 shot destination");
 if(player7 && (approach.distance_adjustment!=99 || approach.destination_cell!=233 || approach.random_first || approach.loose_ball))
  throw std::invalid_argument("Unverified player7 shot target");
 if(returned_goal && (approach.loose_ball || approach.destination_cell!=field.ball_cell))
  throw std::invalid_argument("Only the observed same-cell goal-area shot is supported");
 auto state=input;state.current.field.random_first=approach.random_first;
 if(player7)state.human_defense_choices.reset();
 state.shot_flight=ShotFlightContext{approach.distance_adjustment,approach.destination_cell,255,approach.loose_ball};
 state.phase=returned_goal ? FieldSessionPhase::ordinary_return_goal_area_target : returned ? FieldSessionPhase::ordinary_return_shot_target : player7 ? FieldSessionPhase::sampled_score25_shot_target : score23 ? FieldSessionPhase::sampled_score23_shot_target : FieldSessionPhase::shot_flight_pending;
 validate_phase_session(state);return {std::move(state),approach};
}
}

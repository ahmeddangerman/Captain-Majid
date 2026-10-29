#include "captain/approach_session.hpp"
#include <stdexcept>
namespace captain {
SessionApproachResult prepare_session_shot_approach(const PhaseSessionState& input,const ShotApproachContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::shot_approach_ready)
  throw std::invalid_argument("Shot approach requires completed attacking clock");
 const auto& field=input.current.field;
 const auto approach=calculate_shot_approach({field.ball_x,field.ball_y,field.side,
  input.recovery->attack_subtype,field.random_first},content);
 auto state=input;state.current.field.random_first=approach.random_first;
 state.shot_flight=ShotFlightContext{approach.distance_adjustment,approach.destination_cell,255,approach.loose_ball};
 state.phase=FieldSessionPhase::shot_flight_pending;
 validate_phase_session(state);return {std::move(state),approach};
}
}

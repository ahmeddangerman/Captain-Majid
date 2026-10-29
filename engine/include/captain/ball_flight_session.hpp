#pragma once
#include "captain/phase_session.hpp"
#include "captain/ball_step.hpp"
namespace captain {
PhaseSessionState begin_prepared_session_ball_flight(const PhaseSessionState& input,
 std::uint8_t x_fraction,std::uint8_t y_fraction,const MovementGeometryContent& geometry,
 const BallVelocityContent& velocity);
PhaseSessionState resolve_session_ball_cell(const PhaseSessionState& input);
struct SessionBallStepResult { PhaseSessionState state; BallStepResult step; };
SessionBallStepResult advance_prepared_session_ball_cell(const PhaseSessionState& input);
}

#include "captain/ball_flight_session.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState begin_prepared_session_ball_flight(const PhaseSessionState& input,
 std::uint8_t low_x,std::uint8_t low_y,const MovementGeometryContent& geometry,const BallVelocityContent& velocity) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::shot_flight_pending)
  throw std::invalid_argument("Flight setup requires a saved shot destination");
 const auto& f=input.current.field;
 if(field_cell_at(f.ball_x,f.ball_y)!=f.ball_cell)
  throw std::invalid_argument("Flight requires coherent current ball coordinates/cell");
 auto state=input;
 BallTrajectoryContext t{static_cast<std::uint16_t>((unsigned(f.ball_x)<<8)|low_x),
  static_cast<std::uint16_t>((unsigned(f.ball_y)<<8)|low_y),0,0,input.recovery->x_jitter_control,0};
 state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);
 state.current.field.elapsed=0;state.shot_progress.reset();
 if(f.ball_cell==input.shot_flight->destination)state.phase=FieldSessionPhase::ball_already_arrived;
 else {
  t.direction=calculate_player_direction({t.x,t.y,input.shot_flight->destination,0},f.ball_cell,geometry);
  const auto v=calculate_ball_velocity(t.direction,velocity);t.vx=v.x;t.vy=v.y;
  state.recovery->x_jitter_control=t.direction;state.phase=FieldSessionPhase::ball_advance_ready;
 }
 state.ball_trajectory=t;validate_phase_session(state);return state;
}
PhaseSessionState resolve_session_ball_cell(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ball_cell_callback)
  throw std::invalid_argument("No pending ball cell callback");
 FlightDefenderSelectionInput selection{};
 const auto& field=input.current.field;
 selection.possession_side=field.side;selection.ball_cell=field.ball_cell;
 selection.random_first=field.random_first;selection.random_second=field.random_second;
 selection.threshold=input.shot_flight->counter;
 for(unsigned i=0;i<field.motion.size();++i){const auto& m=field.motion[i];
  selection.players[i]={static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8),m.status!=0};}
 const auto selected=select_flight_defenders(selection);
 auto state=input;auto& count=state.current.field.selected_defenders;
 const auto capacity=field.side==0 ? 5u : 4u;
 for(auto id:selected){if(count>=capacity)break;state.scheduling->selected_ids[count++]=id;}
 state.phase=FieldSessionPhase::ball_advance_ready;validate_phase_session(state);return state;
}
SessionBallStepResult advance_prepared_session_ball_cell(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ball_advance_ready)
  throw std::invalid_argument("Ball step requires acknowledged frame and pending numeric flight");
 auto state=input;auto& t=*state.ball_trajectory;
 const auto step=advance_ball_to_next_cell({t.x,t.y,{t.vx,t.vy},state.current.field.ball_cell,state.shot_flight->destination});
 t.x=step.x;t.y=step.y;state.current.field.ball_x=step.x>>8;state.current.field.ball_y=step.y>>8;
 state.current.field.ball_cell=step.cell;
 state.phase=step.event==BallStepEvent::cell_changed ? FieldSessionPhase::ball_cell_callback : FieldSessionPhase::ball_arrival_callback;
 validate_phase_session(state);return {std::move(state),step};
}
}

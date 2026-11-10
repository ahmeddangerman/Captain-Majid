#pragma once
#include "captain/phase_session.hpp"
#include "captain/ball_step.hpp"
#include "captain/defender_contact.hpp"
#include "captain/flight_contact.hpp"
namespace captain {
struct SessionBallStepResult;
struct AreaFlightAcknowledgement {
 std::uint8_t x_fraction,y_fraction,distance,facing,motion;
 std::uint16_t prior_vx,prior_vy;
};
PhaseSessionState begin_postgoal_opponent_first_flight(const PhaseSessionState& input,
 std::uint8_t x_fraction,std::uint8_t y_fraction,std::uint8_t captured_distance,
 const MovementGeometryContent& geometry,const BallVelocityContent& velocity);
SessionBallStepResult advance_postgoal_opponent_first_flight(const PhaseSessionState& input);
PhaseSessionState resolve_postgoal_opponent_first_flight_cell(const PhaseSessionState& input);
PhaseSessionState move_postgoal_opponent_runner(const PhaseSessionState& input);
PhaseSessionState exchange_postgoal_opponent_carrier(const PhaseSessionState& input);
PhaseSessionState begin_postgoal_opponent_return_flight(const PhaseSessionState& input,
 const MovementGeometryContent& geometry,const BallVelocityContent& velocity);
SessionBallStepResult advance_postgoal_opponent_return_flight(const PhaseSessionState& input);
PhaseSessionState resolve_postgoal_opponent_return_flight_cell(const PhaseSessionState& input);
PhaseSessionState snap_postgoal_opponent_return_arrival(const PhaseSessionState& input);
PhaseSessionState begin_session_area_action4_restart_flight(const PhaseSessionState& input,std::uint8_t captured_recipient,std::uint8_t captured_x_fraction,std::uint8_t captured_y_fraction,std::uint8_t captured_distance,const MovementGeometryContent& geometry,const BallVelocityContent& velocity);
PhaseSessionState begin_prepared_area_action_flight(const PhaseSessionState& input,
 const AreaFlightAcknowledgement& acknowledgement,const MovementGeometryContent& geometry,
 const BallVelocityContent& velocity,const ExperienceContent* experience);
PhaseSessionState begin_sampled_field_pass_flight(const PhaseSessionState& input, std::uint8_t x_fraction, std::uint8_t y_fraction, const MovementGeometryContent& geometry, const BallVelocityContent& velocity);
PhaseSessionState begin_ordinary_pass_flight(const PhaseSessionState& input, std::uint8_t x_fraction, std::uint8_t y_fraction, const MovementGeometryContent& geometry, const BallVelocityContent& velocity);
SessionBallStepResult advance_ordinary_pass_ball_cell(const PhaseSessionState& input);
PhaseSessionState resolve_ordinary_pass_ball_cell(const PhaseSessionState& input);
PhaseSessionState continue_ordinary_pass_reception(const PhaseSessionState& input);
PhaseSessionState position_ordinary_pass_recipient(const PhaseSessionState& input);
PhaseSessionState continue_ordinary_pass_control(const PhaseSessionState& input,std::uint8_t observed_facing);
PhaseSessionState begin_prepared_session_ball_flight(const PhaseSessionState& input,
 std::uint8_t x_fraction,std::uint8_t y_fraction,const MovementGeometryContent& geometry,
 const BallVelocityContent& velocity);
PhaseSessionState continue_session_unopposed_arrival(const PhaseSessionState& input);
PhaseSessionState prepare_ordinary_return_flight_defender(const PhaseSessionState& input,const FieldPlayerProfiles& profiles,const FieldDefenseContent& defense);
PhaseSessionState compare_ordinary_return_flight_defender(const PhaseSessionState& input,const FlightContactContent& content);
PhaseSessionState position_ordinary_return_blocked_shot(const PhaseSessionState& input,const ExperienceContent& experience);
PhaseSessionState advance_ordinary_return_blocked_shot_clock(const PhaseSessionState& input,const ContactPreludeContent& clock,const ExperienceContent& experience);
PhaseSessionState resolve_session_ball_cell(const PhaseSessionState& input);
PhaseSessionState resolve_session_area_ball_cell(const PhaseSessionState& input);
PhaseSessionState continue_area_already_arrived(const PhaseSessionState& input);
PhaseSessionState recover_session_sampled_loose_shot_result2(const PhaseSessionState& input,const ContactPreludeContent& clock,const ExperienceContent& experience);
PhaseSessionState compare_session_sampled_loose_shot_defender(const PhaseSessionState& input,std::uint16_t captured_power,const FlightContactContent& content);
PhaseSessionState prepare_session_sampled_loose_shot_defender(const PhaseSessionState& input);
PhaseSessionState acknowledge_area_pass_defender_announcement(const PhaseSessionState& input);
PhaseSessionState prepare_area_pass_defender(const PhaseSessionState& input,std::uint16_t captured_cost);
struct AreaPassComparisonContent { std::uint8_t range,offset,eligibility; std::array<std::uint8_t,4> thresholds; };
PhaseSessionState compare_postgoal_return_defender(const PhaseSessionState& input,const FieldPlayerProfiles& profiles,
 const FieldDefenseContent& defense,const AreaPassComparisonContent& comparison);
PhaseSessionState compare_area_pass_defender(const PhaseSessionState& input,std::uint16_t captured_power,
 std::uint8_t captured_context,const AreaPassComparisonContent& content);
PhaseSessionState restore_area_pass_ball_variant(const PhaseSessionState& input);
PhaseSessionState acknowledge_area_pass_result(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion);
PhaseSessionState advance_area_pass_nonterminal(const PhaseSessionState& input,const ContactPreludeContent& clock_content,
 const ExperienceContent& experience_content);
PhaseSessionState advance_area_pass_defender(const PhaseSessionState& input);
PhaseSessionState advance_area_pass_blocked(const PhaseSessionState& input,const ExperienceContent& experience_content);
PhaseSessionState advance_area_pass_terminal(const PhaseSessionState& input,const ContactPreludeContent& clock_content,
 const ExperienceContent& experience_content);
PhaseSessionState resume_area_pass_added_time(const PhaseSessionState& input);
struct AreaPassCaptureResult { PhaseSessionState state; bool request_possession_sound; };
AreaPassCaptureResult capture_area_pass_defender(const PhaseSessionState& input);
PhaseSessionState enter_area_pass_blocked_recovery(const PhaseSessionState& input);
PhaseSessionState continue_area_pass_capture(const PhaseSessionState& input,const PlayerProfileMappings& profiles,
 const FieldPhaseContent& movement,std::uint8_t facing,std::uint8_t motion);
PhaseSessionState continue_area_pass_received(const PhaseSessionState& input);
struct SessionBallStepResult { PhaseSessionState state; BallStepResult step; };
SessionBallStepResult advance_prepared_session_ball_cell(const PhaseSessionState& input);
SessionBallStepResult advance_prepared_area_ball_cell(const PhaseSessionState& input);
}

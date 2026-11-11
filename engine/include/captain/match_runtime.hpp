#pragma once
#include "captain/phase_session.hpp"
#include "captain/shot_content.hpp"
#include "captain/control_session.hpp"
#include "captain/runtime_random.hpp"
#include "captain/pass_distance.hpp"
#include "captain/ball_velocity.hpp"
#include <cstdint>
#include <optional>
#include <vector>

namespace captain {
enum class UnsupportedCapability { content, initialization, input_sequence, random_state, ball_fraction, timing, action, defender_contact, defensive_prompt, opponent_action, presentation, phase };
enum class MatchTimingMode { original_backed, provisional_demo };
enum class MatchInputKind { tick, action, presentation_complete };
enum class MatchAction { none, pass };
enum class MatchPassStage { none, awaiting_presentation, handoff, target_ready, flight_ready, flight_cell, flight_arrived };
struct MatchBallFraction { std::uint8_t x=0,y=0; };
struct MatchPassTarget { std::uint8_t recipient=0,destination=0,counter=255,distance=0; };
enum class MatchDefenseStage { polling, choice_confirmed, complete };
struct MatchDefenseMenu { MatchDefenseStage stage=MatchDefenseStage::polling; DefenseMenuPollState poll{}; };
// Inputs express intent only; action outcomes and random samples are never supplied.
struct MatchInput { MatchInputKind kind; std::uint64_t sequence; std::uint8_t held=0, pressed=0; MatchAction action=MatchAction::none; std::optional<std::uint8_t> target{}; };
struct MatchEvent { ControlPresentationRequest presentation; };
// Initial state is a development snapshot, not independently verified match startup.
struct MatchConfig { std::optional<PhaseSessionState> initial{}; std::uint32_t seed=0; std::uint16_t clock=0; std::optional<std::uint8_t> random_index{}; MatchTimingMode mode=MatchTimingMode::original_backed; std::optional<MatchBallFraction> ball_fraction{}; };
struct MatchContent { std::optional<ShotContent> data{}; std::optional<PassDistanceContent> pass_distance{}; std::optional<BallVelocityContent> ball_velocity{}; };
struct MatchRuntimeState { PhaseSessionState session{}; std::uint64_t last_sequence=0; std::optional<std::uint8_t> random_index{}; RuntimeRandomMemory random_memory{}; MatchTimingMode mode=MatchTimingMode::original_backed; std::uint32_t demo_rng=0; MatchPassStage pass_stage=MatchPassStage::none; std::optional<MatchPassTarget> pass_target{}; std::optional<MatchBallFraction> ball_fraction{}; std::optional<BallTrajectoryContext> pass_trajectory{}; std::optional<std::uint8_t> pass_cell{}; std::optional<MatchDefenseMenu> defense_menu{}; };
struct MatchBallView { std::uint8_t x=0,y=0,cell=0; bool in_flight=false; };
struct MatchStepResult { MatchRuntimeState state{}; std::vector<MatchEvent> events{}; std::optional<UnsupportedCapability> unsupported{}; };
// Runtime trajectory is authoritative for the visible ball while a pass is in flight.
MatchBallView match_ball_view(const MatchRuntimeState&);
MatchStepResult create_match(const MatchConfig&,const MatchContent&);
MatchStepResult step_match(const MatchRuntimeState&,const MatchInput&,const MatchContent&);
const char* capability_name(UnsupportedCapability);
}

#pragma once
#include "captain/contact_prelude.hpp"
#include "captain/terminal_flight_phase.hpp"
#include "captain/experience.hpp"
#include <vector>

namespace captain {
enum class FieldSessionPhase : std::uint8_t {
    field = 0, player_control = 1, ball_recovery = 2, added_time = 3, period_end = 4,
    boundary_restart = 5, recovered_owner = 6, keeper_interaction = 7, story_transition = 8,
    control_ready = 9, control_clock_pending = 10, action_menu_pending = 11, ai_cell_decision = 12,
    field_round_pending = 13, control_added_time = 14, field_contact_pending = 15,
    goal_area_action = 16, special_callback = 17, human_action_menu = 18,
    attack_choice_pending = 19, attack_prepared = 20,
    shot_field_complete = 21, shot_restart_pending = 22,
    shot_defender_next = 23, shot_contact_added_time = 24, shot_dispatch_clock = 25,
    shot_dispatch_added_time = 26, shot_dispatch_ready = 27,
    selected_ball_recovery = 28, selected_defense_animation = 29, selected_restart_entry = 30,
    selected_recovered_owner = 31, selected_boundary_restart = 32, capture_clearance_flight = 33, clearance_race_winner = 34, clearance_race_timeout = 35, clearance_recovery_ready = 36, clearance_recovered_owner = 37, selected_field_restart = 38, selected_goal_restart = 39, restart_options = 40, restart_tactics = 41, restart_options_done = 42, selected_attack_presentation = 43, attack_clock_added_time = 44, shot_approach_ready = 45, shot_flight_pending = 46, ball_advance_ready = 47, ball_cell_callback = 48, ball_arrival_callback = 49, ball_already_arrived = 50, shot_loose_recovery = 51, shot_keeper_entry = 52, keeper_presentation_pending = 53, stunned_keeper_pending = 54, stunned_keeper_decided = 55, keeper_decided = 56, keeper_possession = 57, keeper_punch_recovery = 58, goal_celebration_pending = 59, goal_kickoff_pending = 60, kickoff_formation_placed = 61, kickoff_receiver_selected = 62
};
struct PendingTerminalContact {
    std::uint8_t result_code, defender_id, defense_action, defense_subtype;
};
struct RecoveryContext {
    std::uint8_t opposing_player = 0, attack_subtype = 0, special_owner_flags = 0;
    std::uint8_t ball_variant = 0, game_mode = 0, x_jitter_control = 0;
};
struct ControlContext {
    std::uint8_t decision_timer = 0, facing = 0, controlled_player = 0, last_carrier_cell = 0;
    std::uint16_t travel_accumulator = 0;
    std::uint8_t display_parameter = 0;
    std::uint8_t motion_requested = 0;
};
struct SchedulingContext {
    std::uint8_t field_cycle = 0, energy_cycle = 0, special_flags = 0;
    std::array<std::uint8_t, 5> selected_ids{};
};
struct ActionContext {
    std::uint8_t action = 0, menu_route = 0, cursor = 0, flash_counter = 0, auxiliary = 0;
    std::uint8_t move_effect = 0, move_flags = 0;
    std::uint16_t power = 0;
};
struct ShotContactProgress {
    std::uint8_t cursor = 0, result_code = 0, defender_id = 0, defense_action = 0, defense_subtype = 0;
};
struct ClearanceFlightContext { std::uint8_t distance_adjustment = 0; };
struct ClearanceRaceContext { std::uint8_t remaining = 0, active_candidate = 0, opposing_candidate = 0; };
struct RestartOptionsContext { bool goal_area = false; std::uint8_t cursor = 4; };
struct ShotFlightContext { std::uint8_t distance = 0, destination = 0, counter = 255; bool loose_ball = false; };
struct BallTrajectoryContext { std::uint16_t x=0,y=0,vx=0,vy=0; std::uint8_t direction=0,flags=0; };
struct KeeperDecisionContext { std::uint8_t cursor=0, result_code=0, defense_action=0, defense_subtype=0, effect=0; };
struct MatchScoreContext { std::uint8_t left_goals=0,right_goals=0,kickoff=0; };
struct KickoffFormationContext { std::array<std::uint8_t,2> keeper_y{}; };
struct PhaseSessionState {
    ContactPreludeState current;
    FieldSessionPhase phase = FieldSessionPhase::field;
    std::optional<PendingTerminalContact> pending{};
    std::optional<ExperienceState> experience{};
    std::optional<RecoveryContext> recovery{};
    std::optional<ControlContext> control{};
    std::optional<SchedulingContext> scheduling{};
    std::optional<ActionContext> action{};
    std::optional<ShotContactProgress> shot_progress{};
    std::optional<ClearanceFlightContext> clearance_flight{};
    std::optional<ClearanceRaceContext> clearance_race{};
    std::optional<RestartOptionsContext> restart_options{};
    std::optional<ShotFlightContext> shot_flight{};
    std::optional<BallTrajectoryContext> ball_trajectory{};
    std::optional<KeeperDecisionContext> keeper_decision{};
    std::optional<MatchScoreContext> score{};
    std::optional<KickoffFormationContext> kickoff_formation{};
    std::optional<std::uint8_t> kickoff_receiver{};
};
struct PhaseSessionResult {
    PhaseSessionState state;
    std::optional<ContactPreludeResult> prelude{};
    std::optional<TerminalFlightPhaseResult> terminal{};
    std::vector<ExperienceRewardResult> rewards{};
};
void validate_phase_session(const PhaseSessionState& state);
// Input is after rewards/prior presentation, before original9095 status/time.
// Numeric clock pauses are retained; no presentation/NMI acknowledgement is implicit.
PhaseSessionResult resolve_session_terminal(const PhaseSessionState& input,
    PendingTerminalContact contact, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ContactPreludeContent& prelude);
// Caller explicitly supplies/accepts state after the added-time presentation
// and any original wait. The development lab keeps otherwise frozen inputs.
PhaseSessionResult resume_session_terminal(const PhaseSessionState& post_presentation,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement);
// Entry before the terminal reward caller. Result2 invokes that caller twice;
// result3 once. Resume never repeats these rewards.
PhaseSessionResult resolve_session_rewarded_terminal(const PhaseSessionState& input,
    PendingTerminalContact contact, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ContactPreludeContent& prelude,
    const ExperienceContent& experience);
}

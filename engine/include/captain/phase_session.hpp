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
    attack_choice_pending = 19, attack_prepared = 20
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
struct PhaseSessionState {
    ContactPreludeState current;
    FieldSessionPhase phase = FieldSessionPhase::field;
    std::optional<PendingTerminalContact> pending{};
    std::optional<ExperienceState> experience{};
    std::optional<RecoveryContext> recovery{};
    std::optional<ControlContext> control{};
    std::optional<SchedulingContext> scheduling{};
    std::optional<ActionContext> action{};
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

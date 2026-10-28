#pragma once
#include "captain/contact_prelude.hpp"
#include "captain/terminal_flight_phase.hpp"
#include "captain/experience.hpp"
#include <vector>

namespace captain {
enum class FieldSessionPhase : std::uint8_t {
    field = 0, player_control = 1, ball_recovery = 2, added_time = 3, period_end = 4,
    boundary_restart = 5, recovered_owner = 6, keeper_interaction = 7, story_transition = 8,
    control_ready = 9, control_clock_pending = 10, action_menu_pending = 11, ai_cell_decision = 12
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
struct PhaseSessionState {
    ContactPreludeState current;
    FieldSessionPhase phase = FieldSessionPhase::field;
    std::optional<PendingTerminalContact> pending{};
    std::optional<ExperienceState> experience{};
    std::optional<RecoveryContext> recovery{};
    std::optional<ControlContext> control{};
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

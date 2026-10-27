#pragma once
#include "captain/contact_prelude.hpp"
#include "captain/terminal_flight_phase.hpp"

namespace captain {
enum class FieldSessionPhase : std::uint8_t {
    field = 0, player_control = 1, ball_recovery = 2, added_time = 3, period_end = 4
};
struct PendingTerminalContact {
    std::uint8_t result_code, defender_id, defense_action, defense_subtype;
};
struct PhaseSessionState {
    ContactPreludeState current;
    FieldSessionPhase phase = FieldSessionPhase::field;
    std::optional<PendingTerminalContact> pending{};
};
struct PhaseSessionResult {
    PhaseSessionState state;
    std::optional<ContactPreludeResult> prelude{};
    std::optional<TerminalFlightPhaseResult> terminal{};
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
}

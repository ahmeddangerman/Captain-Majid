#pragma once
#include "captain/field_phase.hpp"
#include <optional>

namespace captain {
enum class TerminalFlightNextPhase : std::uint8_t { ball_recovery, player_control };
struct PreparedTerminalFlightInput {
    FieldPhaseState state;
    std::uint8_t result_code, defender_id, defense_action, defense_subtype;
};
struct TerminalFlightPhaseResult {
    FieldPhaseState state;
    TerminalFlightNextPhase next_phase;
    std::optional<FieldPhaseResult> movement{};
};
// Caller supplies coherent post-prelude stats/status/elapsed/RNG. Reward,
// presentation and clock processing are not inferred from a classification.
TerminalFlightPhaseResult resolve_prepared_terminal_flight(const PreparedTerminalFlightInput& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& content);
}

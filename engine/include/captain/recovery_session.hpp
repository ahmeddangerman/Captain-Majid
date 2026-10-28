#pragma once
#include "captain/phase_session.hpp"
#include "captain/ball_recovery.hpp"

namespace captain {
struct RecoverySessionResult {
    PhaseSessionState state;
    std::optional<FieldPhaseResult> movement{};
    std::optional<BallRecoveryResult> recipient{};
    bool special_owner_cleared = false;
    bool request_possession_sound = false;
};
// Supported original8BDF..8C31 sequence; selected-defender count must be zero.
// Uses staged inputs without synthesizing frame/NMI randomness.
RecoverySessionResult advance_session_recovery(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement);
// Explicit prepared continuation after ownership writes. Unknown goalkeeper,
// restart and story handling remain typed boundaries; waits do not run here.
PhaseSessionState continue_session_recovery(const PhaseSessionState& post_presentation);
}

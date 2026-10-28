#pragma once
#include "captain/phase_session.hpp"
namespace captain {
struct StagedShotResult {
    PhaseSessionState state;
    std::optional<PreparedDefenderContact> contact{};
    std::optional<MatchClockResult> clock{};
};
StagedShotResult step_session_shot_defender(const PhaseSessionState& input,
    const FieldDefenseChoice& choice, bool energy_carry,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& defense,
    const FieldComparisonContent& comparison, const ContactPreludeContent& prelude);
// elapsed is the explicit action-specific original table entry, not wall time.
StagedShotResult dispatch_session_shot(const PhaseSessionState& input, std::uint8_t elapsed);
// Explicit presentation acknowledgement with captured numeric state retained.
PhaseSessionState resume_session_shot_clock(const PhaseSessionState& input);
}

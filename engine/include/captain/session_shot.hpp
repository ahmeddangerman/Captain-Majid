#pragma once
#include "captain/phase_session.hpp"

namespace captain {
struct SessionShotDefenseResult {
    PhaseSessionState state;
    PreparedDefenderSequenceResult defenses;
    std::optional<PhaseSessionResult> terminal{};
    std::optional<FieldRestartDecision> restart{};
};
// Explicit ordered choices/randomness; roster, power, subtype, comparison
// coefficients and interruption state come from the saved session/content.
SessionShotDefenseResult resolve_session_shot_defenders(const PhaseSessionState& input,
    const std::vector<FieldDefenseChoice>& choices, bool energy_carry,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& defense,
    const FieldComparisonContent& comparison, const FieldPhaseContent& movement,
    const ContactPreludeContent& prelude, const ExperienceContent& experience);
}

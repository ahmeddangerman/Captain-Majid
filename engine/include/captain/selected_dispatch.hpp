#pragma once
#include "captain/phase_session.hpp"
namespace captain {
struct SelectedDispatchResult {
    PhaseSessionState state;
    FieldPhaseResult movement;
    std::optional<ExperienceRewardResult> reward{};
    bool request_possession_sound = false;
};
SelectedDispatchResult resolve_session_selected_result(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ExperienceContent& experience);
}

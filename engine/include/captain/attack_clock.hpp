#pragma once
#include "captain/staged_shot.hpp"
namespace captain {
// Unopposed shots require the explicitly captured043D action; selected shots use saved progress.
// Explicit post-script8 input; no script/render/frame execution is inferred.
StagedShotResult continue_prepared_session_attack(const PhaseSessionState& input,
    const ContactPreludeContent& content, std::optional<std::uint8_t> unopposed_defense_action = std::nullopt);
PhaseSessionState resume_session_attack_clock(const PhaseSessionState& input);
}

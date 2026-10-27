#pragma once

#include "captain/ball_recovery.hpp"

#include <cstdint>
#include <optional>

namespace captain {
enum class DefenderRoute : std::uint8_t {
    continue_action, recover_ball, keeper_control, player_control, defense_animation, restart_decision
};

struct DefenderOutcomeInput {
    std::uint8_t result_code;
    std::uint8_t possession_side;
    std::uint8_t ball_owner;
    std::uint8_t defender_id;
    std::uint8_t defense_action;
    std::optional<BallRecoveryInput> recovery{};
};

struct DefenderOutcomeResult {
    DefenderRoute route;
    std::uint8_t possession_side;
    std::uint8_t ball_owner;
    std::optional<BallRecoveryResult> recovery{};
};

// Applies the original immediate handoff. Supplying recovery state resolves
// result two through the ordinary interior-field ball-recovery branch.
DefenderOutcomeResult resolve_defender_outcome(const DefenderOutcomeInput& input);
}

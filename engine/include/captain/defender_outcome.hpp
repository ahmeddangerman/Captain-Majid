#pragma once

#include <cstdint>

namespace captain {
enum class DefenderRoute : std::uint8_t {
    continue_action, recover_ball, keeper_control, player_control, defense_animation
};

struct DefenderOutcomeInput {
    std::uint8_t result_code;
    std::uint8_t possession_side;
    std::uint8_t ball_owner;
    std::uint8_t defender_id;
    std::uint8_t defense_action;
};

struct DefenderOutcomeResult {
    DefenderRoute route;
    std::uint8_t possession_side;
    std::uint8_t ball_owner;
};

// Applies only the immediate writes in the original field-defender result
// dispatcher. Ball recovery and animation continuation remain separate.
DefenderOutcomeResult resolve_defender_outcome(const DefenderOutcomeInput& input);
}

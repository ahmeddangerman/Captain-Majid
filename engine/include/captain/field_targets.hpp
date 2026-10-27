#pragma once
#include "captain/player_movement.hpp"

namespace captain {
struct FieldTeamShape {
    std::uint8_t formation = 0, defense_tactic = 0;
};
struct FieldTargetContent {
    std::array<std::uint8_t, 15> region_mirror;
    std::array<std::array<std::uint8_t, 300>, 4> on_ball;
    std::array<std::array<std::uint8_t, 300>, 12> off_ball;
};
struct FieldTargetInput {
    FieldMotionState player;
    std::uint8_t player_id, possession_side, ball_x, ball_y, random_first, decision_threshold;
    std::array<FieldTeamShape, 2> teams;
};
struct FieldTargetResult {
    std::uint8_t target_cell, random_first, ball_region, template_region;
    bool used_override;
};
std::uint8_t field_ball_region(std::uint8_t x, std::uint8_t y);
FieldTargetResult select_field_target(const FieldTargetInput& input,
                                     const FieldTargetContent& content);
}

#pragma once
#include <array>
#include <cstdint>

namespace captain {
struct ShotApproachContent {
    std::array<std::uint8_t, 14> outer_distance;
    std::array<std::uint8_t, 36> inner_distance;
};
struct ShotApproachInput {
    std::uint8_t ball_x, ball_y, side, attack_subtype, random_first;
};
struct ShotApproachContext {
    std::uint8_t ball_x, ball_y, random_first;
    ShotApproachContent content;
};
struct ShotApproachResult {
    std::uint8_t distance_adjustment, destination_cell, target_x, target_y, random_first;
    bool loose_ball;
};
ShotApproachResult calculate_shot_approach(const ShotApproachInput& input,
                                         const ShotApproachContent& content);
}

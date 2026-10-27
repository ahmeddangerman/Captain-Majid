#pragma once
#include "captain/complete_prepared_shot.hpp"
#include "captain/shot_content.hpp"
#include "captain/keeper_state.hpp"
#include <vector>

namespace captain {
// Explicit resettable development scene, not a persistent match command.
struct ShotSceneOptions {
    unsigned level = 20, keeper_level = 20, subtype = 0, defenders = 2, keeper_action = 1;
    unsigned ball_x = 160, ball_y = 100;
    unsigned keeper_timer = 0, keeper_duration = 0, keeper_penalty = 0, keeper_stun = 0;
    std::vector<unsigned> recovery_steps{};
};
KeeperRecoveryState resolve_shot_scene_keeper_state(const ShotSceneOptions& options);
CompletePreparedShotResult calculate_shot_scene(const ShotContent& content,
                                               const ShotSceneOptions& options);
}

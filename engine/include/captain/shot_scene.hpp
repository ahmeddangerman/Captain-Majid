#pragma once
#include "captain/complete_prepared_shot.hpp"
#include "captain/shot_content.hpp"

namespace captain {
// Explicit resettable development scene, not a persistent match command.
struct ShotSceneOptions {
    unsigned level = 20, keeper_level = 20, subtype = 0, defenders = 2, keeper_action = 1;
    unsigned ball_x = 160, ball_y = 100;
};
CompletePreparedShotResult calculate_shot_scene(const ShotContent& content,
                                               const ShotSceneOptions& options);
}

#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace captain {
struct RecoveryPlayer {
    std::uint8_t x;
    std::uint8_t y;
    bool unavailable;
};

struct RecoveryDefender {
    std::uint8_t player_id;
    std::uint8_t defense_action;
};

struct BallRecoveryInput {
    std::uint8_t ball_x;
    std::uint8_t ball_y;
    std::uint8_t ball_owner;
    std::uint8_t opposing_player;
    std::uint8_t random_e2;
    std::uint8_t random_e3;
    bool ball_update_active;
    std::array<RecoveryPlayer, 20> players; // IDs 1–10, then 12–21.
    std::vector<RecoveryDefender> selected_defenders;
};

struct BallRecoveryResult {
    std::uint8_t ball_x;
    std::uint8_t ball_y;
    std::uint8_t ball_owner;
    std::uint8_t possession_side;
    std::uint8_t attack_subtype;
    bool direct_defender_capture;
};

struct LooseBallPosition {
    std::uint8_t x, y, x_jitter_control;
    bool inside_field;
};
LooseBallPosition jitter_loose_ball(std::uint8_t x, std::uint8_t y,
    std::uint8_t random_e2, std::uint8_t random_e3);
// Input coordinates are already jittered; positions/random may have advanced
// through the intervening field phase. Does not apply jitter a second time.
BallRecoveryResult recover_stationary_ball(const BallRecoveryInput& input);

// Resolves the ordinary interior-field handoff after a loose ball. Player
// coordinates and availability are the records at the recovery decision.
// Active ball movement and outer-field restart routes are separate stages.
BallRecoveryResult recover_ball(const BallRecoveryInput& input);
}

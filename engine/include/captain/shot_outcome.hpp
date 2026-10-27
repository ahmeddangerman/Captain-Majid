#pragma once

#include <array>
#include <cstdint>

namespace captain {
enum class ShotEvent : std::uint8_t { goal, special_turnover, rebound, keeper_catch, keeper_punch };

// Explicit state at the goalkeeper decision. All values are game bytes.
struct ShotOutcomeInput {
    std::uint8_t comparison_result, possession_side, opponent_team, story_state;
    std::uint8_t attack_subtype, defense_action, random_first, random_second;
    std::uint8_t timer_selector, keeper_timer, keeper_penalty, keeper_penalty_duration;
    std::uint8_t keeper_stun, left_goals, right_goals, ball_owner;
    std::uint8_t ball_x, ball_y, defenders, ball_active, kickoff;
    std::array<std::uint8_t, 8> punch_intervals;
};

struct ShotOutcomeResult {
    ShotEvent event;
    std::uint8_t result_code, possession_side, ball_owner;
    std::uint8_t left_goals, right_goals, keeper_timer, keeper_penalty;
    std::uint8_t keeper_penalty_duration, keeper_stun, ball_x, ball_y;
    std::uint8_t defenders, ball_active, kickoff;
};

// Resolves writes through the original goalkeeper handler's handoff to the
// next match phase. Animation, experience, and that next phase are separate.
ShotOutcomeResult resolve_shot_outcome(const ShotOutcomeInput& input);
}

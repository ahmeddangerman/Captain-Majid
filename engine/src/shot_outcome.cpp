#include "captain/shot_outcome.hpp"
#include "captain/keeper_state.hpp"

#include <stdexcept>

namespace captain {
ShotOutcomeResult resolve_shot_outcome(const ShotOutcomeInput& input) {
    if (input.comparison_result > 3 || (input.possession_side != 0 && input.possession_side != 11)) {
        throw std::invalid_argument("Shot outcome needs a keeper result and a valid side");
    }
    const auto story = apply_keeper_story_override({input.comparison_result, input.possession_side,
        input.opponent_team, input.story_state, input.attack_subtype, input.random_first});
    ShotOutcomeResult result{ShotEvent::rebound, story.result_code, input.possession_side,
        input.ball_owner, input.left_goals, input.right_goals, input.keeper_timer,
        input.keeper_penalty, input.keeper_penalty_duration, input.keeper_stun,
        input.ball_x, input.ball_y, input.defenders, input.ball_active, input.kickoff};

    if (story.result_code == 0 ||
        (story.result_code == 1 && (input.attack_subtype & 63u) < 3u)) {
        result.event = ShotEvent::goal;
        auto& goals = input.possession_side == 0 ? result.left_goals : result.right_goals;
        goals = static_cast<std::uint8_t>(unsigned(goals) + 1u);
        result.possession_side = input.possession_side ^ 11u;
        result.keeper_timer = 0;
        result.keeper_penalty = 0;
        result.keeper_stun = 0;
        result.kickoff = 4;
        return result;
    }
    if (story.result_code == 1) {
        result.event = ShotEvent::special_turnover;
        result.possession_side = input.possession_side ^ 11u;
        return result;
    }
    if (story.result_code == 2) {
        const auto keeper = update_keeper_state({input.keeper_timer, input.keeper_penalty,
            input.keeper_penalty_duration, input.timer_selector, input.random_first, true, true});
        result.keeper_timer = keeper.timer;
        result.keeper_penalty = keeper.penalty;
        result.keeper_penalty_duration = keeper.penalty_duration;
        result.defenders = 0;
        return result;
    }

    const bool punched = input.defense_action == 1;
    const auto keeper = update_keeper_state({input.keeper_timer, input.keeper_penalty,
        input.keeper_penalty_duration, input.timer_selector, input.random_first,
        true, punched});
    result.keeper_timer = keeper.timer;
    result.keeper_penalty = keeper.penalty;
    result.keeper_penalty_duration = keeper.penalty_duration;
    if (!punched) {
        result.event = ShotEvent::keeper_catch;
        result.possession_side = input.possession_side ^ 11u;
        result.ball_owner = result.possession_side;
        return result;
    }
    result.event = ShotEvent::keeper_punch;
    unsigned index = input.random_first & 7u;
    if (input.possession_side == 0) index ^= 7u;
    const unsigned interval = input.punch_intervals[index];
    if (interval == 0) throw std::invalid_argument("Punch interval must be nonzero");
    result.ball_x = static_cast<std::uint8_t>((input.possession_side == 0 ? 0x90u : 0x30u) + index * 8u);
    const unsigned vertical = (input.random_second & 15u) % interval;
    std::uint8_t y = static_cast<std::uint8_t>(0x50u + vertical * 8u);
    if (input.random_second & 0x80u) y ^= 0xffu;
    result.ball_y = y;
    result.ball_active = 1;
    result.defenders = 0;
    return result;
}
}

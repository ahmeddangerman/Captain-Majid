#include "captain/keeper_state.hpp"

namespace captain {
StoryOverrideResult apply_keeper_story_override(const StoryOverrideInput& input) {
    const bool overridden = input.possession_side == 0 && input.opponent_team == 5 &&
        input.story_state != 0 && input.attack_subtype != 0 &&
        (input.attack_subtype == 3 || input.story_state < 4);
    const auto result = overridden
        ? static_cast<std::uint8_t>(2 + (input.random_first >> 7))
        : input.comparison_result;
    return {result, overridden, 0};
}

KeeperStateResult update_keeper_state(const KeeperStateInput& input) {
    KeeperStateResult result{input.stored_timer, input.stored_penalty,
                             input.stored_penalty_duration};
    if (input.invoke_timer) {
        result.timer = static_cast<std::uint8_t>(result.timer +
            (input.timer_selector < 2 ? 7 : 11));
    }
    if (input.invoke_penalty && input.random_first < 64 && input.stored_penalty < 80) {
        const unsigned penalty = unsigned(input.stored_penalty) + 79;
        result.penalty = static_cast<std::uint8_t>(penalty < 128 ? penalty : 127);
        result.penalty_duration = 4;
    }
    return result;
}
}

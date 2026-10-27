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

KeeperRecoverySnapshot advance_keeper_recovery(const KeeperRecoverySnapshot& state,
                                               std::uint8_t elapsed) {
    auto result = state;
    if (result.shared_flags & 128u) {
        result.shared_timer = static_cast<std::uint8_t>(unsigned(result.shared_timer) - elapsed);
        if (result.shared_timer & 128u) result.shared_flags = 0;
    }
    for (auto& keeper : result.keepers) {
        if (keeper.stun != 0) {
            const auto remaining = static_cast<std::uint8_t>(unsigned(keeper.stun) - elapsed);
            keeper.stun = remaining & 128u ? 0 : remaining;
            continue;
        }
        if (keeper.penalty == 0) continue;
        auto duration = static_cast<std::uint8_t>(unsigned(keeper.penalty_duration) - elapsed);
        if (duration & 128u) {
            duration = static_cast<std::uint8_t>(unsigned(duration) + 3u);
            const auto penalty = static_cast<std::uint8_t>(unsigned(keeper.penalty) - 25u);
            keeper.penalty = penalty & 128u ? 0 : penalty;
        }
        keeper.penalty_duration = duration;
    }
    return result;
}
}

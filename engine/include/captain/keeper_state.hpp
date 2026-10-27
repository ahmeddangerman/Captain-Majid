#pragma once

#include <cstdint>

namespace captain {
struct StoryOverrideInput {
    std::uint8_t comparison_result;
    std::uint8_t possession_side;
    std::uint8_t opponent_team;
    std::uint8_t story_state;
    std::uint8_t attack_subtype;
    std::uint8_t random_first;
};
struct StoryOverrideResult {
    std::uint8_t result_code;
    bool overridden;
    std::uint8_t defender_index;
};
StoryOverrideResult apply_keeper_story_override(const StoryOverrideInput& input);

struct KeeperStateInput {
    std::uint8_t stored_timer;
    std::uint8_t stored_penalty;
    std::uint8_t stored_penalty_duration;
    std::uint8_t timer_selector;
    std::uint8_t random_first;
    // The shot handler selects these helpers; this value API does not select an outcome.
    bool invoke_timer;
    bool invoke_penalty;
};
struct KeeperStateResult {
    std::uint8_t timer;
    std::uint8_t penalty;
    std::uint8_t penalty_duration;
};
KeeperStateResult update_keeper_state(const KeeperStateInput& input);
}

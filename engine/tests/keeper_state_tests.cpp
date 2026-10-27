#include "captain/keeper_state.hpp"

#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
}

int main() {
    captain::StoryOverrideInput story{0, 0, 5, 1, 1, 127};
    auto result = captain::apply_keeper_story_override(story);
    require(result.result_code == 2 && result.overridden && result.defender_index == 0,
            "eligible story forces classification two and clears defender index");
    story.random_first = 128;
    result = captain::apply_keeper_story_override(story);
    require(result.result_code == 3 && result.overridden, "story uses random sign bit");
    story.comparison_result = 3;
    result = captain::apply_keeper_story_override(story);
    require(result.result_code == 3 && result.overridden, "override reports eligibility even if result agrees");
    story.possession_side = 11;
    require(!captain::apply_keeper_story_override(story).overridden, "story requires left possession");
    story.possession_side = 0; story.opponent_team = 4;
    require(!captain::apply_keeper_story_override(story).overridden, "story requires designated opponent");
    story.opponent_team = 5; story.story_state = 0;
    require(!captain::apply_keeper_story_override(story).overridden, "inactive story preserves comparison");
    story.story_state = 1; story.attack_subtype = 0;
    result = captain::apply_keeper_story_override(story);
    require(result.result_code == 3 && !result.overridden && result.defender_index == 0,
            "zero subtype preserves comparison and clears defender index");
    story.story_state = 4; story.attack_subtype = 1;
    require(!captain::apply_keeper_story_override(story).overridden, "late story excludes ordinary nonzero subtype");
    story.attack_subtype = 3;
    require(captain::apply_keeper_story_override(story).overridden, "raw subtype three remains eligible late");
    story.attack_subtype = 131;
    require(!captain::apply_keeper_story_override(story).overridden, "story does not mask subtype flags");
    story.story_state = 3;
    require(captain::apply_keeper_story_override(story).overridden, "early story accepts flagged nonzero subtype");

    captain::KeeperStateInput input{250, 49, 99, 0, 63, false, false};
    auto state = captain::update_keeper_state(input);
    require(state.timer == 250 && state.penalty == 49 && state.penalty_duration == 99,
            "uninvoked helpers preserve all state");
    input.invoke_timer = true;
    state = captain::update_keeper_state(input);
    require(state.timer == 1 && state.penalty == 49 && state.penalty_duration == 99,
            "short timer increment wraps and preserves penalty fields");
    input.stored_timer = 0; input.timer_selector = 1;
    require(captain::update_keeper_state(input).timer == 7, "selector one uses short timer increment");
    input.timer_selector = 2;
    require(captain::update_keeper_state(input).timer == 11, "selector two uses long timer increment");
    input.stored_timer = 255; input.timer_selector = 255;
    require(captain::update_keeper_state(input).timer == 10, "long timer increment wraps");
    input.invoke_timer = false; input.invoke_penalty = true;
    state = captain::update_keeper_state(input);
    require(state.timer == 255 && state.penalty == 127 && state.penalty_duration == 4,
            "penalty saturates independently of timer");
    input.stored_penalty = 0;
    require(captain::update_keeper_state(input).penalty == 79, "zero penalty receives fixed increment");
    input.stored_penalty = 48;
    require(captain::update_keeper_state(input).penalty == 127, "penalty reaches cap exactly");
    input.stored_penalty = 79;
    require(captain::update_keeper_state(input).penalty == 127, "largest eligible penalty clamps");
    input.stored_penalty = 80;
    state = captain::update_keeper_state(input);
    require(state.penalty == 80 && state.penalty_duration == 99, "penalty gate excludes eighty");
    input.stored_penalty = 255;
    state = captain::update_keeper_state(input);
    require(state.penalty == 255 && state.penalty_duration == 99, "ineligible large penalty is preserved");
    input.stored_penalty = 0; input.random_first = 64;
    state = captain::update_keeper_state(input);
    require(state.penalty == 0 && state.penalty_duration == 99, "random gate excludes sixty-four");
    input.random_first = 0; input.invoke_timer = true;
    state = captain::update_keeper_state(input);
    require(state.timer == 10 && state.penalty == 79 && state.penalty_duration == 4,
            "both helpers update independent state fields");
    std::cout << "Keeper story and state checks passed\n";
}

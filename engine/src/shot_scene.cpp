#include "captain/shot_scene.hpp"
#include <stdexcept>

namespace captain {
CompletePreparedShotResult calculate_shot_scene(const ShotContent& content,
                                               const ShotSceneOptions& options) {
    if (options.level > 255 || options.keeper_level > 255 || options.subtype > 255 ||
        options.defenders > 5 || options.keeper_action > 3)
        throw std::invalid_argument("Unsupported development scene options");
    const auto index = options.subtype & 63u;
    if (index >= content.attack.moves[0].size())
        throw std::invalid_argument("Shot subtype unavailable in this content pack");
    captain::CompletePreparedShotInput input{};
    for (auto& player : input.field.roster) player = {1, 20, 1000};
    input.field.roster[3].level = static_cast<std::uint8_t>(options.level);
    input.field.roster[11] = {33, static_cast<std::uint8_t>(options.keeper_level), 1000};
    input.field.ball_owner = 3;
    input.field.context = {0, 0, static_cast<std::uint8_t>(options.subtype), 0, 0};
    input.field.context.comparison_content = content.field_comparison;
    input.field.context.interruption = captain::FieldInterruptionContext{160, 100, {}};
    for (unsigned i = 0; i < options.defenders; ++i)
        input.field.choices.push_back({static_cast<std::uint8_t>(14 + i), 2, 0, 0,
            {0, 0, 100, 100, 1, 0, false, false, 0, 0, 0, {0, 0, 0, 0}}});
    input.attack_random_first = 100;
    input.attack_random_second = 100;
    input.keeper_action = static_cast<std::uint8_t>(options.keeper_action);
    auto& keeper = input.field.continuation.keeper;
    keeper.roll.random_first = 0;
    keeper.roll.random_second = 100;
    keeper.roll.range = content.keeper_range;
    keeper.roll.offset = content.keeper_offset;
    keeper.elapsed = 45;
    keeper.thresholds = content.keeper_thresholds;
    auto& outcome = input.field.continuation.outcome;
    outcome.ball_owner = 3;
    outcome.ball_x = 160;
    outcome.ball_y = 100;
    outcome.defenders = static_cast<std::uint8_t>(options.defenders);
    outcome.timer_selector = static_cast<std::uint8_t>(
        (content.attack.moves[0][index].cost_high_flags & 0xF8u) >> 3);
    outcome.punch_intervals = content.punch_intervals;
    return resolve_complete_prepared_shot(input, content.field_players,
        content.attack, content.defense, content.goalkeepers, content.keeper);
}
}

#include "captain/shot_scene.hpp"
#include <stdexcept>

namespace captain {
KeeperRecoveryState resolve_shot_scene_keeper_state(const ShotSceneOptions& options) {
    if (options.side != 0 && options.side != 11)
        throw std::invalid_argument("Development scene side must be0 or11");
    if (options.keeper_timer > 255 || options.keeper_duration > 255 ||
        options.keeper_penalty > 255 || options.keeper_stun > 255)
        throw std::invalid_argument("Keeper state and elapsed step must be bytes");
    KeeperRecoverySnapshot snapshot{};
    const auto keeper_index = options.side == 0 ? 1u : 0u;
    snapshot.keepers[keeper_index] = {static_cast<std::uint8_t>(options.keeper_timer),
        static_cast<std::uint8_t>(options.keeper_duration),
        static_cast<std::uint8_t>(options.keeper_penalty),
        static_cast<std::uint8_t>(options.keeper_stun)};
    for (const auto elapsed : options.recovery_steps) {
        if (elapsed > 255)
            throw std::invalid_argument("Keeper elapsed step must be a byte");
        snapshot = advance_keeper_recovery(snapshot, static_cast<std::uint8_t>(elapsed));
    }
    return snapshot.keepers[keeper_index];
}

CompletePreparedShotResult calculate_shot_scene(const ShotContent& content,
                                               const ShotSceneOptions& options) {
    if (options.level > 255 || options.keeper_level > 255 || options.subtype > 255 ||
        options.defenders > 5 || options.keeper_action > 3 || options.ball_x > 255 || options.ball_y > 255)
        throw std::invalid_argument("Unsupported development scene options");
    const auto index = options.subtype & 63u;
    if (index >= content.attack.moves[0].size())
        throw std::invalid_argument("Shot subtype unavailable in this content pack");
    const auto keeper_state = resolve_shot_scene_keeper_state(options);
    const auto side = static_cast<std::uint8_t>(options.side);
    const std::uint8_t owner = side == 0 ? 3 : 14;
    const auto keeper_slot = side ^ 11u;
    captain::CompletePreparedShotInput input{};
    for (auto& player : input.field.roster) player = {1, 20, 1000};
    input.field.roster[owner].level = static_cast<std::uint8_t>(options.level);
    input.field.roster[keeper_slot] = {33, static_cast<std::uint8_t>(options.keeper_level), 1000};
    input.field.ball_owner = owner;
    input.field.context = {side, 0, static_cast<std::uint8_t>(options.subtype), 0, 0};
    input.field.context.comparison_content = content.field_comparison;
    const auto ball_x = static_cast<std::uint8_t>(options.ball_x);
    const auto ball_y = static_cast<std::uint8_t>(options.ball_y);
    input.field.context.interruption = captain::FieldInterruptionContext{ball_x, ball_y, {}};
    if (content.approach)
        input.approach = ShotApproachContext{ball_x, ball_y, 0, *content.approach};
    else if (options.side != 0 || options.ball_x != 160 || options.ball_y != 100)
        throw std::invalid_argument("Side/ball-position changes require version2 approach content");
    for (unsigned i = 0; i < options.defenders; ++i)
        input.field.choices.push_back({static_cast<std::uint8_t>(keeper_slot + 3 + i), 2, 0, 0,
            {0, 0, 100, 100, 1, 0, false, false, 0, 0, 0, {0, 0, 0, 0}}});
    input.attack_random_first = 100;
    input.attack_random_second = 100;
    input.keeper_action = static_cast<std::uint8_t>(options.keeper_action);
    auto& keeper = input.field.continuation.keeper;
    input.field.continuation.opposing_keeper_status = keeper_state.stun;
    keeper.stored_timer = keeper_state.timer;
    keeper.stored_penalty = keeper_state.penalty;
    keeper.roll.random_first = 0;
    keeper.roll.random_second = 100;
    keeper.roll.range = content.keeper_range;
    keeper.roll.offset = content.keeper_offset;
    keeper.elapsed = 45;
    keeper.thresholds = content.keeper_thresholds;
    auto& outcome = input.field.continuation.outcome;
    outcome.ball_owner = owner;
    outcome.ball_x = ball_x;
    outcome.ball_y = ball_y;
    // The keeper continuation replaces field choices with one selected keeper
    // at original84FA. This outcome input is at that later checkpoint.
    outcome.defenders = 1;
    outcome.timer_selector = static_cast<std::uint8_t>(
        (content.attack.moves[0][index].cost_high_flags & 0xF8u) >> 3);
    outcome.punch_intervals = content.punch_intervals;
    outcome.keeper_timer = keeper_state.timer;
    outcome.keeper_penalty = keeper_state.penalty;
    outcome.keeper_penalty_duration = keeper_state.penalty_duration;
    outcome.keeper_stun = keeper_state.stun;
    return resolve_complete_prepared_shot(input, content.field_players,
        content.attack, content.defense, content.goalkeepers, content.keeper);
}
}

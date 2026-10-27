#include "captain/field_phase.hpp"
#include <stdexcept>

namespace captain {
std::size_t field_motion_index(std::uint8_t player_id) {
    if (player_id == 0 || player_id == 11 || player_id > 21)
        throw std::invalid_argument("Expected a field player ID");
    return player_id < 11 ? player_id - 1u : player_id - 2u;
}

FieldPhaseResult advance_field_phase(const FieldPhaseState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& content) {
    FieldPhaseResult result{input, {}};
    if (input.elapsed == 0) return result;
    if ((input.side != 0 && input.side != 11) || input.ball_owner > 21)
        throw std::invalid_argument("Unsupported field phase side or owner");
    const auto region = field_ball_region(input.ball_x, input.ball_y);
    result.state.movement_region = static_cast<std::uint8_t>(region | 128u);
    result.players.reserve(20);
    for (unsigned slot = 1; slot <= 21; ++slot) {
        if (slot == 11 || slot == input.ball_owner) continue;
        const auto id = static_cast<std::uint8_t>(slot);
        const auto& stats = input.roster[slot];
        const FieldPlayerRecord record{id, stats.type, static_cast<std::uint8_t>(stats.stamina),
            static_cast<std::uint8_t>(stats.stamina >> 8), stats.level};
        const auto profile = load_movement_profile(record, input.side, primary, content.profiles);
        auto& motion = result.state.motion[field_motion_index(id)];
        const auto target = select_field_target({motion, id, input.side, input.ball_x, input.ball_y,
            result.state.random_first, profile.decision_threshold, input.teams}, content.targets);
        result.state.random_first = target.random_first;
        motion.target_cell = target.target_cell;
        const auto movement = advance_field_player({motion, profile.speed, input.elapsed, input.ball_cell},
                                                   content.geometry);
        motion = movement.player;
        result.players.push_back({id, profile, target, movement});
    }
    result.state.elapsed = 0;
    return result;
}
}

#include "captain/field_phase.hpp"
#include <stdexcept>
#include <utility>

namespace captain {
std::size_t field_motion_index(std::uint8_t player_id) {
    if (player_id == 0 || player_id == 11 || player_id > 21)
        throw std::invalid_argument("Expected a field player ID");
    return player_id < 11 ? player_id - 1u : player_id - 2u;
}
FieldAdvancedPlayer advance_field_phase_player(FieldPhaseState& state,std::uint8_t id,
    const PlayerProfileMappings& primary,const FieldPhaseContent& content) {
    const auto index=field_motion_index(id);
    if ((state.side!=0 && state.side!=11) || state.ball_owner>21 || id==state.ball_owner || !state.elapsed)
        throw std::invalid_argument("Unsupported field phase player");
    auto next=state;
    const auto& stats=next.roster[id];
    const FieldPlayerRecord record{id,stats.type,static_cast<std::uint8_t>(stats.stamina),
        static_cast<std::uint8_t>(stats.stamina>>8),stats.level};
    const auto profile=load_movement_profile(record,next.side,primary,content.profiles);
    auto& motion=next.motion[index];
    const auto target=select_field_target({motion,id,next.side,next.ball_x,next.ball_y,
        next.random_first,profile.decision_threshold,next.teams},content.targets);
    next.random_first=target.random_first;
    motion.target_cell=target.target_cell;
    const auto movement=advance_field_player({motion,profile.speed,next.elapsed,next.ball_cell},content.geometry);
    motion=movement.player;
    state=std::move(next);
    return {id,profile,target,movement};
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
        result.players.push_back(advance_field_phase_player(result.state,id,primary,content));
    }
    result.state.elapsed = 0;
    return result;
}
}

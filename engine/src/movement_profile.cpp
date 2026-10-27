#include "captain/movement_profile.hpp"
#include <stdexcept>

namespace captain {
MovementProfileResult load_movement_profile(const FieldPlayerRecord& player,
    std::uint8_t possession_side, const PlayerProfileMappings& primary,
    const MovementProfiles& content) {
    if ((possession_side != 0 && possession_side != 11) ||
        player.player_id == 0 || player.player_id == 11 || player.player_id > 21)
        throw std::invalid_argument("Movement profile needs a field player and side0/11");
    const auto reference = resolve_player_profile_reference(player, primary);
    const MovementProfileEntry* entry;
    if (reference.effective_type < 35) entry = &content.ordinary[reference.effective_type];
    else {
        const auto index = unsigned(reference.effective_type) - 35u;
        if (index >= content.alternate.size())
            throw std::invalid_argument("Missing alternate movement metadata");
        entry = &content.alternate[index];
    }
    if (entry->speed_profile >= content.speeds.size())
        throw std::invalid_argument("Missing movement speed profile");
    const bool on_ball = (player.player_id < 11 ? 0 : 11) == possession_side;
    return {reference.effective_type, entry->speed_profile,
        on_ball ? entry->on_ball_threshold : entry->off_ball_threshold,
        content.speeds[entry->speed_profile][on_ball ? 0 : 1], on_ball};
}
}

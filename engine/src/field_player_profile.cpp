#include "captain/field_player_profile.hpp"
#include <stdexcept>

namespace captain {
FieldPlayerProfile resolve_field_player_profile(
    const FieldPlayerRecord& player, const FieldPlayerProfiles& profiles) {
    if (player.player_id == 0 || player.player_id == 11 || player.player_id > 21)
        throw std::invalid_argument("Expected a field player ID");
    auto type = player.type;
    if (type == 0) {
        if (player.player_id < 12)
            throw std::invalid_argument("Generic profile requires an opposing field player");
        const auto offset = profiles.generic_slot_offsets[player.player_id - 11];
        if (offset >= profiles.team_types.size())
            throw std::invalid_argument("Missing generic team profile entry");
        type = profiles.team_types[offset];
    }
    std::uint8_t profile;
    if (type >= 35) {
        if (player.flags & 128u) type = player.alternate_type;
        if (type < 35 || static_cast<unsigned>(type - 35) >= profiles.alternate.size())
            throw std::invalid_argument("Missing alternate player profile");
        profile = profiles.alternate[type - 35];
    } else {
        // A generic team entry may itself resolve to ordinary type zero.
        profile = profiles.ordinary[type];
    }
    if (profile >= profiles.skills.size())
        throw std::invalid_argument("Missing field skill profile");
    return {type, profile, profiles.skills[profile]};
}

FieldDefenderPreparationResult prepare_profiled_field_defender(
    const FieldPlayerRecord& player, const FieldPlayerProfiles& profiles,
    std::uint8_t action, std::uint8_t situation, std::uint8_t subtype,
    const FieldDefenseContent& defense) {
    const auto profile = resolve_field_player_profile(player, profiles);
    return prepare_field_defender({profile.skills, player.level, action, situation, subtype},
                                 defense);
}
}

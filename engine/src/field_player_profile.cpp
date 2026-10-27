#include "captain/field_player_profile.hpp"
#include <stdexcept>

namespace captain {
PlayerProfileReference resolve_player_profile_reference(
    const FieldPlayerRecord& player, const PlayerProfileMappings& profiles) {
    if (player.player_id > 21)
        throw std::invalid_argument("Expected a player ID 0..21");
    auto type = player.type;
    if (type == 0) {
        if (player.player_id < 11)
            throw std::invalid_argument("Generic profile requires an opposing player");
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
    return {type, profile};
}

FieldPlayerProfile resolve_field_player_profile(
    const FieldPlayerRecord& player, const FieldPlayerProfiles& profiles) {
    if (player.player_id == 0 || player.player_id == 11 || player.player_id > 21)
        throw std::invalid_argument("Expected a field player ID");
    const auto reference = resolve_player_profile_reference(player, profiles);
    const auto profile = reference.profile_id;
    if (profile >= profiles.skills.size())
        throw std::invalid_argument("Missing field skill profile");
    return {reference.effective_type, profile, profiles.skills[profile]};
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

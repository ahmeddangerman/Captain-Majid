#pragma once
#include "captain/field_defender_preparation.hpp"

namespace captain {
// flags/alternate_type represent raw record+1/+2, also the stamina bytes.
struct FieldPlayerRecord {
    std::uint8_t player_id, type, flags, alternate_type, level;
};
struct PlayerProfileMappings {
    std::array<std::uint8_t, 35> ordinary;
    std::vector<std::uint8_t> alternate;
    std::array<std::uint8_t, 11> generic_slot_offsets;
    std::vector<std::uint8_t> team_types;
};
struct FieldPlayerProfiles : PlayerProfileMappings {
    std::vector<std::array<std::uint8_t, 31>> skills;
};
struct PlayerProfileReference {
    std::uint8_t effective_type, profile_id;
};
PlayerProfileReference resolve_player_profile_reference(
    const FieldPlayerRecord& player, const PlayerProfileMappings& profiles);
struct FieldPlayerProfile {
    std::uint8_t effective_type, profile_id;
    std::array<std::uint8_t, 31> skills;
};
FieldPlayerProfile resolve_field_player_profile(
    const FieldPlayerRecord& player, const FieldPlayerProfiles& profiles);
FieldDefenderPreparationResult prepare_profiled_field_defender(
    const FieldPlayerRecord& player, const FieldPlayerProfiles& profiles,
    std::uint8_t action, std::uint8_t situation, std::uint8_t subtype,
    const FieldDefenseContent& defense);
}

#pragma once
#include "captain/field_player_profile.hpp"

namespace captain {
struct MovementProfileEntry {
    std::uint8_t speed_profile, on_ball_threshold, off_ball_threshold;
};
struct MovementProfiles {
    std::array<MovementProfileEntry, 35> ordinary;
    std::vector<MovementProfileEntry> alternate;
    std::vector<std::array<std::uint16_t, 2>> speeds; // On/off-possession raw words.
};
struct MovementProfileResult {
    std::uint8_t effective_type, speed_profile, decision_threshold;
    std::uint16_t speed;
    bool in_possession;
};
MovementProfileResult load_movement_profile(const FieldPlayerRecord& player,
    std::uint8_t possession_side, const PlayerProfileMappings& primary,
    const MovementProfiles& content);
}

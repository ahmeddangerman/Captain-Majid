#pragma once
#include "captain/field_player_profile.hpp"
namespace captain {
struct DefensiveMoveInspection { std::uint16_t cost; bool allowed; std::uint8_t effect; };
// The caller supplies the raw player record, including stamina's low/high bytes.
DefensiveMoveInspection inspect_defensive_move(const FieldPlayerRecord& player, std::uint16_t stamina,
    std::uint8_t action, std::uint8_t variant, std::uint8_t subtype,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& defense);
}

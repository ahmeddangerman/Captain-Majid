#pragma once
#include "captain/energy.hpp"
#include "captain/field_player_profile.hpp"
#include "captain/shot_power.hpp"
namespace captain {
struct PreparedAttackInput {
    FieldPlayerRecord player;
    std::uint8_t action, situation, subtype, side, selected_defenders;
    std::uint8_t random_first, random_second;
    bool incoming_carry;
};
struct PreparedAttackResult {
    FieldPlayerProfile profile;
    FieldDefenderPreparationResult preparation;
    EnergyResult energy;
    std::uint8_t move_flags;
    ActionPowerResult action_power;
};
PreparedAttackResult prepare_attacking_action(const PreparedAttackInput& input,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& content);
}

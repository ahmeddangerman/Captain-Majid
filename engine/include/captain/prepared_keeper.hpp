#pragma once
#include "captain/field_player_profile.hpp"
#include "captain/energy.hpp"
#include "captain/goalkeeper.hpp"
namespace captain {
struct GoalkeeperProfiles : PlayerProfileMappings {
    std::vector<std::array<std::uint8_t,8>> skills;
};
struct GoalkeeperMoveContent {
    std::array<std::uint8_t,4> skill_selectors;
    std::array<std::vector<FieldDefenseMove>,4> moves;
    std::array<std::uint8_t,192> strength_curve;
};
struct PreparedKeeperInput {
    FieldPlayerRecord player;
    std::uint8_t side, defense_action, defense_subtype, attack_action, attack_subtype;
    bool incoming_energy_carry;
    GoalkeeperInput comparison;
};
struct PreparedKeeperResult {
    PlayerProfileReference profile;
    FieldDefenderPreparationResult preparation;
    EnergyResult energy;
    GoalkeeperResult comparison;
};
PreparedKeeperResult prepare_goalkeeper(const PreparedKeeperInput& input,
    const GoalkeeperProfiles& profiles, const GoalkeeperMoveContent& content);
}

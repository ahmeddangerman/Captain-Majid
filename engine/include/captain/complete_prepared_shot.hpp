#pragma once
#include "captain/prepared_field_shot.hpp"
#include "captain/prepared_attack.hpp"
#include "captain/prepared_keeper.hpp"
namespace captain {
struct CompletePreparedShotInput {
    PreparedFieldShotInput field;
    std::uint8_t attack_situation, attack_random_first, attack_random_second;
    bool attack_carry;
    std::uint8_t keeper_action, keeper_subtype;
    bool keeper_energy_carry;
};
struct CompletePreparedShotResult {
    PreparedAttackResult attack;
    PreparedFieldShotResult flow;
    std::optional<PreparedKeeperResult> keeper;
};
CompletePreparedShotResult resolve_complete_prepared_shot(
    const CompletePreparedShotInput& input, const FieldPlayerProfiles& profiles,
    const FieldDefenseContent& attack_content, const FieldDefenseContent& defense_content,
    const GoalkeeperProfiles& keeper_profiles, const GoalkeeperMoveContent& keeper_content);
}

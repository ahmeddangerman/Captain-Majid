#pragma once
#include "captain/prepared_field_shot.hpp"
#include "captain/prepared_attack.hpp"
#include "captain/prepared_keeper.hpp"
#include "captain/shot_approach.hpp"
#include "captain/flight_contact.hpp"
namespace captain {
struct CompletePreparedShotInput {
    PreparedFieldShotInput field;
    std::uint8_t attack_situation, attack_random_first, attack_random_second;
    bool attack_carry;
    std::uint8_t keeper_action, keeper_subtype;
    bool keeper_energy_carry;
    std::optional<ShotApproachContext> approach{};
    std::optional<FlightContactBurst> flight{};
};
struct CompletePreparedShotResult {
    PreparedAttackResult attack;
    PreparedFieldShotResult flow;
    std::optional<PreparedKeeperResult> keeper;
    std::optional<ShotApproachResult> approach{};
    std::optional<PreparedFlightResult> flight{};
};
struct PreparedShotContinuationInput {
    // The owner already paid the attacking cost. context.incoming_power and
    // context.attack_subtype are the retained action0 preparation result.
    PreparedFieldShotInput field;
    std::uint8_t keeper_action, keeper_subtype;
    bool keeper_energy_carry;
    std::optional<ShotApproachContext> approach{};
    std::optional<FlightContactBurst> flight{};
};
struct PreparedShotContinuationResult {
    PreparedFieldShotResult flow;
    std::optional<PreparedKeeperResult> keeper;
    std::optional<ShotApproachResult> approach{};
    std::optional<PreparedFlightResult> flight{};
};
// Continues after attacker preparation. The supplied roster must already
// contain the attacker's post-cost stamina; this function never charges it.
PreparedShotContinuationResult resolve_prepared_shot_continuation(
    const PreparedShotContinuationInput& input, const FieldPlayerProfiles& profiles,
    const FieldDefenseContent& defense_content,
    const GoalkeeperProfiles& keeper_profiles, const GoalkeeperMoveContent& keeper_content);
CompletePreparedShotResult resolve_complete_prepared_shot(
    const CompletePreparedShotInput& input, const FieldPlayerProfiles& profiles,
    const FieldDefenseContent& attack_content, const FieldDefenseContent& defense_content,
    const GoalkeeperProfiles& keeper_profiles, const GoalkeeperMoveContent& keeper_content);
}

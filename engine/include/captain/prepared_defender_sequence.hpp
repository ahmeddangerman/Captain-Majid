#pragma once
#include "captain/field_player_profile.hpp"
#include "captain/energy.hpp"
#include "captain/field_interruption.hpp"

namespace captain {
struct FieldPlayerState {
    std::uint8_t type, level;
    std::uint16_t stamina;
};
using FieldRoster = std::array<FieldPlayerState, 22>;
struct FieldDefenseChoice {
    std::uint8_t player_id, action, situation, subtype;
    DefenderContactInput comparison;
};
struct FieldDefenseContext {
    std::uint8_t side, attack_action, attack_subtype;
    std::uint16_t incoming_power;
    std::uint8_t initial_result_code;
    bool incoming_energy_carry = false;
    std::optional<FieldInterruptionContext> interruption{};
};
struct PreparedDefenderContact {
    std::uint8_t player_id;
    FieldDefenderPreparationResult preparation;
    EnergyResult energy;
    DefenderContactResult contact;
    std::optional<FieldInterruptionResult> interruption{};
};
struct PreparedDefenderSequenceResult {
    FieldRoster roster;
    std::vector<PreparedDefenderContact> contacts;
    std::uint16_t remaining_power;
    std::uint8_t result_code;
    bool stopped;
    std::optional<FieldInterruptionState> interruption_state{};
};
PreparedDefenderSequenceResult resolve_prepared_field_contacts(
    const FieldRoster& roster, const std::vector<FieldDefenseChoice>& choices,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& defense,
    const FieldDefenseContext& context);
}

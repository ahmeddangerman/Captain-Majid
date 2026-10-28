#pragma once
#include "captain/control_input.hpp"
#include "captain/prepared_keeper.hpp"

namespace captain {
struct WorldStepContent {
    std::array<std::uint16_t, 256> chase_speeds;
    std::array<std::uint16_t, 96> stamina_curve;
};
struct ControlTickResult {
    PhaseSessionState state;
    std::optional<MatchClockResult> clock{};
    std::vector<ControlPresentationRequest> presentation{};
};
ControlTickResult advance_control_tick(const PhaseSessionState& input,
    const FieldPlayerProfiles& field, const GoalkeeperProfiles& keepers, const WorldStepContent& content);
ControlTickResult resume_control_added_time(const PhaseSessionState& post_presentation,
    const FieldPlayerProfiles& field, const GoalkeeperProfiles& keepers, const WorldStepContent& content);
struct LogicalButtons { std::uint8_t held = 0, pressed = 0; };
struct FieldRoundResult {
    PhaseSessionState state;
    std::vector<ControlPresentationRequest> presentation{};
    unsigned consumed_polls = 0, moved_players = 0, decayed_statuses = 0;
};
FieldRoundResult advance_control_field_round(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ControlMovementContent& control, const WorldStepContent& world,
    const std::array<LogicalButtons, 22>& inputs);
}

#pragma once
#include "captain/control_input.hpp"
#include "captain/prepared_keeper.hpp"
#include "captain/reference_random.hpp"
#include <istream>

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
ControlTickResult advance_area_pass_control_tick(const PhaseSessionState& input,
    const FieldPlayerProfiles& field, const GoalkeeperProfiles& keepers, const WorldStepContent& content);
ControlTickResult advance_area_action4_control_tick(const PhaseSessionState& input,
    const FieldPlayerProfiles& field, const GoalkeeperProfiles& keepers, const WorldStepContent& content);
ControlTickResult resume_area_action4_added_time(const PhaseSessionState& input,
    const FieldPlayerProfiles& field, const GoalkeeperProfiles& keepers, const WorldStepContent& content);
ControlTickResult resume_control_added_time(const PhaseSessionState& post_presentation,
    const FieldPlayerProfiles& field, const GoalkeeperProfiles& keepers, const WorldStepContent& content);
struct LogicalButtons { std::uint8_t held = 0, pressed = 0; };
struct ObservedFieldPoll {
    LogicalButtons buttons{};
    std::uint8_t random_first = 0, random_second = 0;
    bool travel_observed = false;
    std::uint8_t travel_held = 0;
    std::uint16_t travel_speed = 0;
};
std::array<ObservedFieldPoll, 22> load_observed_field_polls(std::istream& input);
struct FieldRoundResult {
    PhaseSessionState state;
    std::vector<ControlPresentationRequest> presentation{};
    unsigned consumed_polls = 0, moved_players = 0, decayed_statuses = 0;
};
struct ObservedNmiFieldPoll {
    LogicalButtons buttons{};
    ReferenceRandomInputs random{};
    bool travel_observed = false;
    std::uint8_t travel_held = 0;
    std::uint16_t travel_speed = 0;
};
struct NmiFieldRoundResult {
    FieldRoundResult round;
    std::uint8_t random_index = 0;
};
struct NmiControlPollResult {
    ControlPollResult poll;
    std::uint8_t random_index = 0;
};
enum class ObservedClockEventKind { random, travel, schedule, energy };
struct ObservedClockEvent {
    ObservedClockEventKind kind;
    ReferenceRandomInputs random{};
    std::uint8_t held = 0;
    std::uint16_t speed = 0;
};
struct NmiClockResult {
    ControlTickResult tick;
    std::uint8_t random_index = 0;
};
std::vector<ObservedClockEvent> load_observed_clock_events(std::istream& input);
std::vector<ObservedClockEvent> load_observed_nmi_events(std::istream& input);
struct NmiEventSequenceResult {
    PhaseSessionState state;
    std::uint8_t random_index = 0;
};
NmiEventSequenceResult advance_observed_nmi_events(const PhaseSessionState& input,
    std::uint8_t initial_random_index, const std::vector<ObservedClockEvent>& events);
NmiClockResult advance_observed_nmi_control_clock(const PhaseSessionState& input,
    const FieldPlayerProfiles& field, const GoalkeeperProfiles& keepers, const WorldStepContent& content,
    std::uint8_t initial_random_index, const std::vector<ObservedClockEvent>& events);
ObservedNmiFieldPoll load_observed_nmi_poll(std::istream& input);
NmiControlPollResult advance_observed_nmi_control_poll(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ControlMovementContent& control, std::uint8_t initial_random_index,
    const ObservedNmiFieldPoll& sample);
std::array<ObservedNmiFieldPoll, 22> load_observed_nmi_field_polls(std::istream& input);
NmiFieldRoundResult advance_observed_nmi_field_round(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ControlMovementContent& control, const WorldStepContent& world,
    std::uint8_t initial_random_index, const std::array<ObservedNmiFieldPoll, 22>& inputs);
FieldRoundResult advance_control_field_round(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ControlMovementContent& control, const WorldStepContent& world,
    const std::array<LogicalButtons, 22>& inputs);
// Explicit captured pre-poll events; does not generate frames, random or audio.
FieldRoundResult advance_observed_control_field_round(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ControlMovementContent& control, const WorldStepContent& world,
    const std::array<ObservedFieldPoll, 22>& inputs);
FieldRoundResult advance_area_pass_field_round(const PhaseSessionState& input,
    const PlayerProfileMappings& primary,const FieldPhaseContent& movement,
    const ControlMovementContent& control,const WorldStepContent& world,
    const std::array<LogicalButtons,22>& inputs);
FieldRoundResult advance_area_action4_field_round(const PhaseSessionState& input,
    const PlayerProfileMappings& primary,const FieldPhaseContent& movement,
    const ControlMovementContent& control,const WorldStepContent& world,
    const std::array<LogicalButtons,22>& inputs);
}

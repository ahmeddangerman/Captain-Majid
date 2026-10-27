#pragma once
#include "captain/prepared_defender_sequence.hpp"

namespace captain {
struct FlightContactContent {
    std::array<std::uint8_t, 2> defense_actions;
    std::array<std::uint8_t, 2> eligibility;
    std::array<std::array<std::uint8_t, 2>, 2> variation;
    std::array<std::array<std::uint8_t, 4>, 2> thresholds;
};
struct FlightContactChoice {
    std::uint8_t player_id, action, random_first, random_second;
};
struct FlightContactBurst {
    std::vector<FlightContactChoice> choices;
    FlightContactContent content;
};
struct FlightContactInput {
    std::uint8_t side;
    std::uint16_t incoming_power;
};
struct PreparedFlightContact {
    std::uint8_t player_id, action;
    FieldDefenderPreparationResult preparation;
    DefenderContactResult contact;
};
struct PreparedFlightResult {
    std::vector<PreparedFlightContact> contacts;
    std::uint16_t remaining_power;
    std::uint8_t result_code;
    bool stopped;
};
// One explicit later-flight action0 burst. No movement selection, stamina debit,
// weakening, frame scheduling or terminal-handler side effects are inferred.
PreparedFlightResult resolve_prepared_flight_contacts(const FieldRoster& roster,
    const std::vector<FlightContactChoice>& choices, const FieldPlayerProfiles& profiles,
    const FieldDefenseContent& defense, const FlightContactContent& content,
    const FlightContactInput& input);
}

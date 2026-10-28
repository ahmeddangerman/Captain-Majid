#pragma once
#include "captain/control_session.hpp"

namespace captain {
struct ControlMovementContent {
    std::array<std::uint16_t, 256> carrier_speeds; // Original selector32 raw words.
};
struct ControlPollResult {
    PhaseSessionState state;
    std::vector<ControlPresentationRequest> presentation{};
    std::optional<std::uint8_t> moved_player{};
    std::uint16_t movement_speed = 0;
    bool decision_step = false, menu_requested = false;
};
struct ControlButtonResult {
    PhaseSessionState state;
    std::vector<ControlPresentationRequest> presentation{};
    bool menu_requested = false;
};
// E349 effects only: no decision-timer decrement or controlled movement.
ControlButtonResult apply_control_buttons(const PhaseSessionState& input, std::uint8_t held, std::uint8_t pressed);
// One staged post-yield E14A input poll; masks are supplied logical inputs.
// Stops before the subsequent clock, AI decision or action-menu handler.
ControlPollResult poll_session_control(const PhaseSessionState& input,
    std::uint8_t held, std::uint8_t pressed, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ControlMovementContent& control_content);
}

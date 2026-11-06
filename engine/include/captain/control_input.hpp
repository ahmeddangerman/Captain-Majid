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
// Capture the explicitly observed field AI choice at the sampled opponent-cell boundary.
PhaseSessionState capture_sampled_field_ai_choice(const PhaseSessionState& input, std::uint8_t action, std::uint8_t subtype);
PhaseSessionState capture_sampled_field_ai_preparation(const PhaseSessionState& input, std::uint8_t subtype, std::uint16_t power);
PhaseSessionState acknowledge_sampled_field_ai_presentation(const PhaseSessionState& input);
PhaseSessionState acknowledge_sampled_opponent_contact_presentation(const PhaseSessionState& input);
PhaseSessionState advance_sampled_field_ai_dispatch(const PhaseSessionState& input);
PhaseSessionState acknowledge_sampled_field_ai_pass_presentation(const PhaseSessionState& input, const FieldPhaseState& captured);
PhaseSessionState acknowledge_sampled_field_ai_pass_handoff(const PhaseSessionState& input);
PhaseSessionState capture_sampled_field_pass_target(const PhaseSessionState& input, std::uint8_t recipient, std::uint8_t destination, std::uint8_t counter, std::uint8_t distance);
// E349 effects only: no decision-timer decrement or controlled movement.
ControlButtonResult apply_control_buttons(const PhaseSessionState& input, std::uint8_t held, std::uint8_t pressed);
// One staged post-yield E14A input poll; masks are supplied logical inputs.
// Stops before the subsequent clock, AI decision or action-menu handler.
ControlPollResult poll_session_control(const PhaseSessionState& input,
    std::uint8_t held, std::uint8_t pressed, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ControlMovementContent& control_content);
ControlPollResult poll_session_area_pass_capture(const PhaseSessionState& input,std::uint8_t held,std::uint8_t pressed,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement,const ControlMovementContent& control_content);
ControlPollResult poll_session_area_action4_control(const PhaseSessionState& input,std::uint8_t held,std::uint8_t pressed,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement,const ControlMovementContent& control_content);
}

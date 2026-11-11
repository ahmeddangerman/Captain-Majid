#pragma once
#include "captain/control_session.hpp"
#include "captain/field_comparison_content.hpp"
#include "captain/shot_approach.hpp"

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
PhaseSessionState capture_postgoal_opponent_choice(const PhaseSessionState& input, std::uint8_t action,
    std::uint8_t subtype, std::uint8_t recipient, std::uint8_t target);
PhaseSessionState prepare_postgoal_opponent_action(const PhaseSessionState& input,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& content);
PhaseSessionState begin_postgoal_opponent_action_wait(const PhaseSessionState& input);
PhaseSessionState acknowledge_postgoal_opponent_action(const PhaseSessionState& input);
PhaseSessionState advance_postgoal_opponent_clock(const PhaseSessionState& input);
PhaseSessionState dispatch_postgoal_opponent_action(const PhaseSessionState& input);
PhaseSessionState handoff_postgoal_opponent_flight(const PhaseSessionState& input);
PhaseSessionState capture_sampled_field_ai_preparation(const PhaseSessionState& input, std::uint8_t subtype, std::uint16_t power);
PhaseSessionState advance_sampled_player14_pass_handoff(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement);
PhaseSessionState advance_sampled_score23_pass_handoff(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement);
PhaseSessionState advance_sampled_player19_return_shot_approach(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement);
// Compute the guarded sampled action2 preparation from supplied private profile data.
PhaseSessionState prepare_sampled_field_contact_attack(const PhaseSessionState& input, const FieldPlayerProfiles& profiles, const FieldDefenseContent& attack);
PhaseSessionState compare_third_return_defender(const PhaseSessionState& input, const FieldPlayerProfiles& profiles,
    const FieldDefenseContent& defense, const FieldComparisonContent& comparison);
PhaseSessionState classify_third_return_defender(const PhaseSessionState& input);
PhaseSessionState advance_third_return_contact(const PhaseSessionState& input, const ContactPreludeContent& prelude,
    const ExperienceContent& experience);
PhaseSessionState jitter_third_return_ball(const PhaseSessionState& input);
PhaseSessionState recover_third_return_ball(const PhaseSessionState& input);
PhaseSessionState prepare_sampled_mixed_defender(const PhaseSessionState& input, const FieldPlayerProfiles& profiles, const FieldDefenseContent& defense);
PhaseSessionState compare_sampled_mixed_defender(const PhaseSessionState& input, const FieldComparisonContent& comparison);
PhaseSessionState capture_sampled_home_shot_defender(const PhaseSessionState& input, std::uint8_t action, std::uint8_t subtype);
PhaseSessionState resolve_sampled_home_shot_defender(const PhaseSessionState& input, const FieldPlayerProfiles& profiles, const FieldDefenseContent& defense, const FieldComparisonContent& comparison);
PhaseSessionState prepare_sampled_player3_pass_defender(const PhaseSessionState& input,const FieldPlayerProfiles& profiles,const FieldDefenseContent& defense);
PhaseSessionState compare_sampled_player3_pass_defender(const PhaseSessionState& input,const FieldComparisonSelection& comparison);
PhaseSessionState advance_sampled_player3_pass_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude,const ExperienceContent& experience);
PhaseSessionState advance_sampled_player3_pass_dispatch(const PhaseSessionState& input);
PhaseSessionState capture_sampled_player3_pass_handoff(const PhaseSessionState& input,const FieldPhaseState& captured);
PhaseSessionState capture_sampled_player3_pass_target(const PhaseSessionState& input,std::uint8_t recipient,std::uint8_t destination,std::uint8_t counter,std::uint8_t distance);
PhaseSessionState capture_sampled_player5_defense_choices(const PhaseSessionState& input,
    const std::array<std::uint8_t,5>& actions,const std::array<std::uint8_t,5>& subtypes);
PhaseSessionState capture_sampled_score24_dribble_defense(const PhaseSessionState& input,
    std::uint8_t action,std::uint8_t subtype);
PhaseSessionState prepare_sampled_player5_dribble(const PhaseSessionState& input,
    const FieldPlayerProfiles& profiles,const FieldDefenseContent& attack);
PhaseSessionState begin_sampled_player5_dribble_defenders(const PhaseSessionState& input);
PhaseSessionState prepare_sampled_player5_dribble_defender(const PhaseSessionState& input,
    const FieldPlayerProfiles& profiles,const FieldDefenseContent& defense);
PhaseSessionState compare_sampled_player5_dribble_defender(const PhaseSessionState& input,
    const FieldComparisonSelection& comparison);
PhaseSessionState advance_sampled_player5_dribble_terminal(const PhaseSessionState& input,
    const ContactPreludeContent& prelude,const ExperienceContent& experience);
PhaseSessionState advance_sampled_player5_terminal_field(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement);
struct Player5PossessionResult { PhaseSessionState state; bool request_side_music=false; };
Player5PossessionResult transfer_sampled_player5_dribble_possession(const PhaseSessionState& input);
PhaseSessionState advance_sampled_home_shot_contact(const PhaseSessionState& input, const ContactPreludeContent& prelude, const ExperienceContent& experience);
PhaseSessionState advance_sampled_home_shot_dispatch(const PhaseSessionState& input);
PhaseSessionState capture_sampled_home_shot_handoff(const PhaseSessionState& input, const FieldPhaseState& captured);
PhaseSessionState prepare_sampled_home_shot_target(const PhaseSessionState& input, const ShotApproachContent& content);
PhaseSessionState advance_sampled_mixed_result4(const PhaseSessionState& input);
PhaseSessionState acknowledge_sampled_mixed_result4_pass(const PhaseSessionState& input);
PhaseSessionState acknowledge_sampled_field_ai_presentation(const PhaseSessionState& input);
PhaseSessionState acknowledge_sampled_opponent_contact_presentation(const PhaseSessionState& input);
PhaseSessionState acknowledge_sampled_field_defense_prompt(const PhaseSessionState& input, std::uint8_t captured_blink);
PhaseSessionState advance_sampled_field_ai_dispatch(const PhaseSessionState& input);
PhaseSessionState acknowledge_sampled_field_ai_pass_presentation(const PhaseSessionState& input, const FieldPhaseState& captured);
PhaseSessionState acknowledge_sampled_field_ai_pass_handoff(const PhaseSessionState& input);
PhaseSessionState capture_sampled_shot_keeper_prompt(const PhaseSessionState& input, const FieldPhaseState& captured);
PhaseSessionState capture_sampled_field_pass_target(const PhaseSessionState& input, std::uint8_t recipient, std::uint8_t destination, std::uint8_t counter, std::uint8_t distance);
// E349 effects only: no decision-timer decrement or controlled movement.
ControlButtonResult apply_control_buttons(const PhaseSessionState& input, std::uint8_t held, std::uint8_t pressed);
// One staged post-yield E14A input poll; masks are supplied logical inputs.
// Stops before the subsequent clock, AI decision or action-menu handler.
ControlPollResult poll_session_control(const PhaseSessionState& input,
    std::uint8_t held, std::uint8_t pressed, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ControlMovementContent& control_content);
PhaseSessionState capture_player2_opponent_action(const PhaseSessionState& input,std::uint8_t action);
PhaseSessionState capture_player2_opponent_pass(const PhaseSessionState& input,std::uint8_t recipient,std::uint8_t target);
PhaseSessionState prepare_player2_opponent_pass(const PhaseSessionState& input,const FieldPlayerProfiles& profiles,const FieldDefenseContent& content);
PhaseSessionState handoff_player2_opponent_pass(const PhaseSessionState& input);
ControlPollResult poll_session_area_pass_capture(const PhaseSessionState& input,std::uint8_t held,std::uint8_t pressed,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement,const ControlMovementContent& control_content);
ControlPollResult poll_session_area_action4_control(const PhaseSessionState& input,std::uint8_t held,std::uint8_t pressed,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement,const ControlMovementContent& control_content);
}

#pragma once
#include "captain/control_session.hpp"
#include "captain/prepared_attack.hpp"
#include "captain/pass_distance.hpp"

namespace captain {
struct ActionMenuContent {
    std::array<std::array<std::uint8_t, 4>, 6> directions;
    std::array<std::uint8_t, 10> icons;
    std::array<std::uint8_t, 6> titles, backgrounds;
};
struct AreaDefenseSpecialOption {
    std::uint8_t subtype=0;
    std::optional<std::uint8_t> teammate_type{};
};
struct AreaDefenseSpecialOptions {
    std::vector<std::optional<AreaDefenseSpecialOption>> field_action2;
};
struct AreaDefenseMenuContent {
    std::array<std::uint8_t,3> titles{};
    std::array<std::array<std::uint8_t,4>,3> directions{};
    std::optional<std::array<std::uint8_t,7>> field_icons{};
    std::optional<AreaDefenseSpecialOptions> special_options{};
    std::optional<std::array<std::uint8_t,2>> special_cursor{}; // tile, x
    std::optional<std::uint8_t> special_cursor_y_base{};
    std::optional<std::uint8_t> action2_special_icon{};
};
struct SpecialCursorPresentation { std::uint8_t tile, x, attribute; };
struct ActionMenuResult {
    PhaseSessionState state;
    std::vector<ControlPresentationRequest> presentation{};
    bool confirmed = false;
    std::optional<SpecialCursorPresentation> special_cursor{};
    std::optional<std::uint8_t> special_cursor_y{};
};
ActionMenuResult open_human_action_menu(const PhaseSessionState& input, const ActionMenuContent& content);
ActionMenuResult open_ordinary_return_action_menu(const PhaseSessionState& input, const ActionMenuContent& content);
ActionMenuResult poll_ordinary_return_action_menu(const PhaseSessionState& input, std::uint8_t pressed,const ActionMenuContent& content);
ActionMenuResult poll_ordinary_return_goal_area_menu(const PhaseSessionState& input,std::uint8_t pressed,const ActionMenuContent& content);
ActionMenuResult open_area_pass_human_action_menu(const PhaseSessionState& input,const ActionMenuContent& content);
// Route2 only, after acknowledged area presentation; facing is captured explicitly.
ActionMenuResult open_prepared_keeper_area_menu(const PhaseSessionState& input,
 std::uint8_t facing,const ActionMenuContent& content);
// Captured0620 is needed because earlier menu states do not represent it.
PhaseSessionState begin_keeper_area_defense(const PhaseSessionState& input,std::uint8_t blink);
ActionMenuResult open_keeper_area_defense_prompt(const PhaseSessionState& input,const AreaDefenseMenuContent& content);
ActionMenuResult poll_keeper_area_defense(const PhaseSessionState& input,std::uint8_t pressed,const AreaDefenseMenuContent& content);
// Query the original area-specific option gate without changing state.
std::optional<std::uint8_t> keeper_area_defense_special_option(const PhaseSessionState& input,const AreaDefenseMenuContent& content);
ActionMenuResult open_keeper_area_special(const PhaseSessionState& input,const AreaDefenseMenuContent& content);
ActionMenuResult poll_keeper_area_special(const PhaseSessionState& input,std::uint8_t pressed,const AreaDefenseMenuContent& content);
ActionMenuResult inspect_keeper_area_special(const PhaseSessionState& input,const FieldPlayerProfiles& profiles,const FieldDefenseContent& defense);
ActionMenuResult redraw_keeper_area_special(const PhaseSessionState& input,const AreaDefenseMenuContent& content);
ActionMenuResult continue_keeper_area_special(const PhaseSessionState& input,const AreaDefenseMenuContent& content);
// Ordinary field choices only; available special options stay pending. No energy charge.
ActionMenuResult prepare_keeper_area_defense(const PhaseSessionState& input,const AreaDefenseMenuContent& content);
// Bounded initial human-keeper menu input; icon IDs are supplied by private content.
ActionMenuResult poll_sampled_goalkeeper_prompt(const PhaseSessionState& input, std::uint8_t pressed,
 const std::array<std::uint8_t,2>& icons);
ActionMenuResult acknowledge_sampled_goalkeeper_overlay(const PhaseSessionState& input);
ActionMenuResult poll_human_action_menu(const PhaseSessionState& input, std::uint8_t pressed,
                                      const ActionMenuContent& content);
struct SessionAttackResult { PhaseSessionState state; PreparedAttackResult preparation; };
SessionAttackResult prepare_ordinary_return_goal_area_shot(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& attack);
PhaseSessionState begin_ordinary_return_goal_area_shot_clock(const PhaseSessionState& input);
PhaseSessionState advance_ordinary_return_goal_area_shot_clock(const PhaseSessionState& input);
PhaseSessionState advance_ordinary_return_goal_area_shot_dispatch(const PhaseSessionState& input,const ContactPreludeContent& content);
PhaseSessionState handoff_ordinary_return_goal_area_shot(const PhaseSessionState& input);
PhaseSessionState record_ordinary_return_shot_defense(const PhaseSessionState& input,std::uint8_t defender,std::uint8_t action,std::uint8_t subtype);
// Records only the captured opposing-player handoff; the supplied choice must
// also be supplied to the later staged defender comparison.
PhaseSessionState record_player7_shot_defender(const PhaseSessionState& input,std::uint8_t defender,std::uint8_t action,std::uint8_t subtype);
PhaseSessionState prepare_player7_shot_defender(const PhaseSessionState& input,std::uint8_t observed_variant,
    const FieldPlayerProfiles& players,const FieldDefenseContent& defense);
PhaseSessionState compare_player7_shot_defender(const PhaseSessionState& input,const FieldComparisonContent& comparison);
PhaseSessionState advance_player7_shot_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude);
PhaseSessionState jitter_player7_shot_ball(const PhaseSessionState& input);
PhaseSessionState recover_player7_shot_ball(const PhaseSessionState& input);
PhaseSessionState capture_player3_one_two_defenders(const PhaseSessionState& input,const std::array<std::uint8_t,3>& actions);
SessionAttackResult prepare_player3_one_two(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& attack);
PhaseSessionState prepare_player3_one_two_defender(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& defense);
PhaseSessionState compare_player3_one_two_defender(const PhaseSessionState& input,const FieldComparisonSelection& comparison);
PhaseSessionState advance_player3_one_two_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude,const ExperienceContent& experience);
PhaseSessionState advance_player3_one_two_next_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude,const ExperienceContent& experience);
PhaseSessionState advance_player3_one_two_terminal_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude);
PhaseSessionState jitter_player3_one_two_ball(const PhaseSessionState& input);
PhaseSessionState recover_player3_one_two_ball(const PhaseSessionState& input);
PhaseSessionState prepare_player3_one_two_next_defender(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& defense);
PhaseSessionState compare_player3_one_two_next_defender(const PhaseSessionState& input,const FieldComparisonSelection& comparison);
SessionAttackResult prepare_ordinary_return_shot(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& attack);
PhaseSessionState prepare_ordinary_return_shot_defender(const PhaseSessionState& input,std::uint8_t observed_variant,const FieldPlayerProfiles& players,const FieldDefenseContent& defense);
PhaseSessionState compare_ordinary_return_shot_defender(const PhaseSessionState& input,const FieldComparisonSelection& comparison);
PhaseSessionState advance_ordinary_return_shot_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude,const ExperienceContent& experience);
PhaseSessionState advance_ordinary_return_shot_dispatch(const PhaseSessionState& input);
// Explicit prepared ordinary UI choice; special moves and other actions stay pending.
SessionAttackResult prepare_session_basic_shot(const PhaseSessionState& input,
    const FieldPlayerProfiles& players, const FieldDefenseContent& attack);
// Acknowledge the captured return to the menu after an unaffordable ordinary shot.
PhaseSessionState acknowledge_sampled_unaffordable_shot(const PhaseSessionState& input,
    const FieldPlayerProfiles& players, const FieldDefenseContent& attack);
// Acknowledges an explicitly selected ordinary pass type; does not run the submenu.
PhaseSessionState begin_ordinary_human_pass_target(const PhaseSessionState& input);
struct OrdinaryPassCursorResult { PhaseSessionState state; bool cursor_changed = false; };
OrdinaryPassCursorResult poll_ordinary_human_pass_cursor(const PhaseSessionState& input, std::uint8_t held, std::uint8_t pressed);
PhaseSessionState confirm_ordinary_human_pass_target(const PhaseSessionState& input);
PhaseSessionState begin_ordinary_pass_defense_selection(const PhaseSessionState& input);
PhaseSessionState record_ordinary_pass_defense(const PhaseSessionState& input,
    std::uint8_t defender, std::uint8_t action, std::uint8_t subtype);
PhaseSessionState prepare_ordinary_pass(const PhaseSessionState& input,
    const FieldPlayerProfiles& players, const FieldDefenseContent& attack);
PhaseSessionState prepare_first_ordinary_pass_defender(const PhaseSessionState& input,
    std::uint8_t observed_ball_variant, const FieldPlayerProfiles& players, const FieldDefenseContent& defense);
PhaseSessionState compare_first_ordinary_pass_defender(const PhaseSessionState& input,
    const FieldComparisonSelection& comparison);
PhaseSessionState advance_first_ordinary_pass_contact(const PhaseSessionState& input,
    const ContactPreludeContent& prelude, const ExperienceContent& experience);
PhaseSessionState prepare_next_ordinary_pass_defender(const PhaseSessionState& input,
    const FieldPlayerProfiles& players, const FieldDefenseContent& defense);
PhaseSessionState compare_next_ordinary_pass_defender(const PhaseSessionState& input,
    const FieldComparisonSelection& comparison);
PhaseSessionState advance_next_ordinary_pass_contact(const PhaseSessionState& input,
    const ContactPreludeContent& prelude, const ExperienceContent& experience);
PhaseSessionState advance_ordinary_pass_dispatch_clock(const PhaseSessionState& input);
PhaseSessionState resolve_ordinary_pass_flight_target(const PhaseSessionState& input,const PassDistanceContent& distance);
PhaseSessionState begin_sampled_human_pass_target(const PhaseSessionState& input);
PhaseSessionState poll_sampled_human_pass_target(const PhaseSessionState& input,std::uint8_t held,std::uint8_t pressed);
// Explicit defender choices observed for the bounded player7 pass; no AI is inferred.
PhaseSessionState capture_sampled_score25_pass_defenses(const PhaseSessionState& input,
    const std::array<std::uint8_t,3>& actions,const std::array<std::uint8_t,3>& subtypes);
PhaseSessionState prepare_sampled_score25_player7_pass(const PhaseSessionState& input,
    const FieldPlayerProfiles& players,const FieldDefenseContent& attack);
PhaseSessionState prepare_sampled_human_empty_pass(const PhaseSessionState& input,
    const FieldPlayerProfiles& players,const FieldDefenseContent& attack);
PhaseSessionState acknowledge_sampled_human_empty_pass_handoff(const PhaseSessionState& input);
PhaseSessionState capture_sampled_human_empty_pass_recovery(const PhaseSessionState& input,const FieldPhaseState& captured);
}

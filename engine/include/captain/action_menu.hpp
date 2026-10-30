#pragma once
#include "captain/control_session.hpp"
#include "captain/prepared_attack.hpp"

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
};
struct SpecialCursorPresentation { std::uint8_t tile, x, attribute; };
struct ActionMenuResult {
    PhaseSessionState state;
    std::vector<ControlPresentationRequest> presentation{};
    bool confirmed = false;
    std::optional<SpecialCursorPresentation> special_cursor{};
};
ActionMenuResult open_human_action_menu(const PhaseSessionState& input, const ActionMenuContent& content);
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
// Ordinary field choices only; available special options stay pending. No energy charge.
ActionMenuResult prepare_keeper_area_defense(const PhaseSessionState& input,const AreaDefenseMenuContent& content);
ActionMenuResult poll_human_action_menu(const PhaseSessionState& input, std::uint8_t pressed,
                                      const ActionMenuContent& content);
struct SessionAttackResult { PhaseSessionState state; PreparedAttackResult preparation; };
// Explicit prepared ordinary UI choice; special moves and other actions stay pending.
SessionAttackResult prepare_session_basic_shot(const PhaseSessionState& input,
    const FieldPlayerProfiles& players, const FieldDefenseContent& attack);
}

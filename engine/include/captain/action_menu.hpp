#pragma once
#include "captain/control_session.hpp"
#include "captain/prepared_attack.hpp"

namespace captain {
struct ActionMenuContent {
    std::array<std::array<std::uint8_t, 4>, 6> directions;
    std::array<std::uint8_t, 10> icons;
    std::array<std::uint8_t, 6> titles, backgrounds;
};
struct ActionMenuResult {
    PhaseSessionState state;
    std::vector<ControlPresentationRequest> presentation{};
    bool confirmed = false;
};
ActionMenuResult open_human_action_menu(const PhaseSessionState& input, const ActionMenuContent& content);
ActionMenuResult poll_human_action_menu(const PhaseSessionState& input, std::uint8_t pressed,
                                      const ActionMenuContent& content);
struct SessionAttackResult { PhaseSessionState state; PreparedAttackResult preparation; };
// Explicit prepared ordinary UI choice; special moves and other actions stay pending.
SessionAttackResult prepare_session_basic_shot(const PhaseSessionState& input,
    const FieldPlayerProfiles& players, const FieldDefenseContent& attack);
}

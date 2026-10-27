#pragma once
#include <array>
#include <cstdint>
#include <optional>
namespace captain {
enum class FieldRestartRoute : std::uint8_t { field_restart, goal_area_restart };
struct FieldInterruptionState {
    std::array<std::uint8_t,22> status;
    std::array<std::uint8_t,2> keeper_penalties;
};
struct FieldInterruptionContext {
    std::uint8_t ball_x, ball_y;
    FieldInterruptionState state;
};
struct FieldInterruptionInput {
    std::uint8_t result_code, selected_defenders, defender_id, defense_action;
    std::uint8_t possession_side, ball_x, ball_y, random_first;
    FieldInterruptionState state;
};
struct FieldRestartDecision {
    FieldRestartRoute route;
    std::uint8_t situation;
};
struct FieldInterruptionResult {
    std::uint8_t result_code;
    bool triggered;
    FieldInterruptionState state;
    std::optional<FieldRestartDecision> restart;
};
FieldInterruptionResult resolve_field_interruption(const FieldInterruptionInput& input);
}

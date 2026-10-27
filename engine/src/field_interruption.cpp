#include "captain/field_interruption.hpp"
#include <stdexcept>
namespace captain {
FieldInterruptionResult resolve_field_interruption(const FieldInterruptionInput& input) {
    if ((input.possession_side != 0 && input.possession_side != 11) ||
        input.defender_id > 21 || input.defense_action > 6 || input.result_code > 4)
        throw std::invalid_argument("Invalid field interruption context");
    const bool keeper = input.defender_id == 0 || input.defender_id == 11;
    const bool bypass = input.selected_defenders == 0 ||
        (keeper ? input.defense_action == 4 : input.defense_action == 5 || input.defense_action == 6);
    const bool goal_area = input.ball_y >= 0x60 && input.ball_y < 0xa0 &&
        (input.ball_x < 0x50 || input.ball_x >= 0xb0);
    const auto threshold = goal_area ? 15 : 63;
    FieldInterruptionResult result{input.result_code,false,input.state,std::nullopt};
    if (!bypass && input.result_code == 0 && input.random_first <= threshold) {
        result.result_code = 4;
        result.triggered = true;
        result.state.status.fill(0);
        result.state.keeper_penalties.fill(0);
    }
    if (result.result_code == 4) {
        const auto area_side = input.ball_x < 0x50 ? 0 : 11;
        result.restart = FieldRestartDecision{goal_area && area_side != input.possession_side
            ? FieldRestartRoute::goal_area_restart : FieldRestartRoute::field_restart,0};
    }
    return result;
}
}

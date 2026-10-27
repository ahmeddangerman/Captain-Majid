#include "captain/defender_outcome.hpp"

#include <stdexcept>

namespace captain {
DefenderOutcomeResult resolve_defender_outcome(const DefenderOutcomeInput& input) {
    if ((input.possession_side != 0 && input.possession_side != 11) || input.result_code > 3) {
        throw std::invalid_argument("Defender outcome needs a valid side and result code");
    }
    DefenderOutcomeResult result{DefenderRoute::continue_action,
        input.possession_side, input.ball_owner};
    if (input.result_code < 2) return result;
    if (input.result_code == 2) {
        result.route = DefenderRoute::recover_ball;
        return result;
    }
    if (input.defender_id == 0 || input.defender_id == 11) {
        result.route = DefenderRoute::keeper_control;
    } else if (input.defense_action == 2) {
        result.route = DefenderRoute::player_control;
    } else {
        result.route = DefenderRoute::defense_animation;
        return result;
    }
    result.ball_owner = input.defender_id;
    result.possession_side = input.defender_id < 11 ? 0 : 11;
    return result;
}
}

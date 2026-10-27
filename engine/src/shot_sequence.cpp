#include "captain/shot_sequence.hpp"

namespace captain {
ShotSequenceResult resolve_explicit_shot(const ShotSequenceInput& input) {
    const auto energy = calculate_energy(input.energy);
    auto action_input = input.action;
    action_input.weakened = energy.weakened;
    const auto action = calculate_action_power(action_input);
    auto keeper_input = input.keeper;
    keeper_input.incoming_power = action.power;
    keeper_input.roll.subtype = action.subtype;
    const auto keeper = calculate_goalkeeper(keeper_input);
    auto outcome_input = input.outcome;
    outcome_input.comparison_result = keeper.result_code;
    outcome_input.attack_subtype = keeper.subtype;
    outcome_input.keeper_timer = keeper_input.stored_timer;
    outcome_input.keeper_penalty = keeper_input.stored_penalty;
    const auto outcome = resolve_shot_outcome(outcome_input);
    return {energy, action, keeper, outcome};
}
}

#include "captain/shot_sequence.hpp"

#include <stdexcept>

namespace captain {
namespace {
struct KeeperOutcome {
    GoalkeeperResult keeper;
    ShotOutcomeResult outcome;
};

KeeperOutcome resolve_keeper_outcome(std::uint16_t power, std::uint8_t subtype,
                                     GoalkeeperInput keeper_input,
                                     ShotOutcomeInput outcome_input) {
    keeper_input.incoming_power = power;
    const auto keeper = calculate_goalkeeper(keeper_input);
    outcome_input.comparison_result = keeper.result_code;
    outcome_input.attack_subtype = subtype;
    outcome_input.keeper_timer = keeper_input.stored_timer;
    outcome_input.keeper_penalty = keeper_input.stored_penalty;
    const auto outcome = resolve_shot_outcome(outcome_input);
    return {keeper, outcome};
}
}

ShotSequenceResult resolve_explicit_shot(const ShotSequenceInput& input) {
    const auto energy = calculate_energy(input.energy);
    auto action_input = input.action;
    action_input.weakened = energy.weakened;
    const auto action = calculate_action_power(action_input);
    const auto keeper_outcome = resolve_keeper_outcome(action.power, action.subtype,
                                                        input.keeper, input.outcome);
    return {energy, action, keeper_outcome.keeper, keeper_outcome.outcome};
}

Action0ShotContinuationResult resolve_action0_shot_continuation(
    const Action0ShotContinuationInput& input) {
    if (input.field_result_code > 1) {
        throw std::invalid_argument("Action-0 keeper continuation accepts field results 0 or 1");
    }
    if (input.action_code != 0) {
        return {ShotContinuationRoute::unsupported_action, std::nullopt, std::nullopt};
    }
    if (input.post_loop_active_defenders != 0) {
        return {ShotContinuationRoute::other_continuation, std::nullopt, std::nullopt};
    }
    if (input.field_proximity_carry) {
        return {ShotContinuationRoute::loose_ball, std::nullopt, std::nullopt};
    }
    if (input.opposing_keeper_status != 0) {
        return {ShotContinuationRoute::other_continuation, std::nullopt, std::nullopt};
    }

    const auto keeper_outcome = resolve_keeper_outcome(input.power_after_contacts,
        input.subtype_after_contacts, input.keeper, input.outcome);
    return {ShotContinuationRoute::keeper_attempt, keeper_outcome.keeper,
            keeper_outcome.outcome};
}
}

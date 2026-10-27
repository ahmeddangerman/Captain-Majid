#include "captain/complete_prepared_shot.hpp"
#include <stdexcept>
#include <utility>
namespace captain {
namespace {
FieldPlayerRecord record(std::uint8_t slot, const FieldRoster& roster) {
    const auto& p = roster[slot];
    return {slot,p.type,static_cast<std::uint8_t>(p.stamina),
        static_cast<std::uint8_t>(p.stamina >> 8),p.level};
}
}
CompletePreparedShotResult resolve_complete_prepared_shot(
    const CompletePreparedShotInput& input, const FieldPlayerProfiles& profiles,
    const FieldDefenseContent& attack_content, const FieldDefenseContent& defense_content,
    const GoalkeeperProfiles& keeper_profiles, const GoalkeeperMoveContent& keeper_content) {
    auto field_input = input.field;
    const auto side = field_input.context.side;
    const auto owner = field_input.ball_owner;
    if ((side != 0 && side != 11) || field_input.context.attack_action != 0 ||
        owner == 0 || owner == 11 || owner > 21 || (owner < 11 ? 0 : 11) != side)
        throw std::invalid_argument("Prepared shot requires action0 and a field owner on its side");
    if (field_input.choices.size() > 10)
        throw std::invalid_argument("At most ten selected field defenders supported");
    const auto attack = prepare_attacking_action({record(owner,field_input.roster),
        0,input.attack_situation,field_input.context.attack_subtype,side,
        static_cast<std::uint8_t>(field_input.choices.size()),input.attack_random_first,
        input.attack_random_second,input.attack_carry},profiles,attack_content);
    field_input.roster[owner].stamina = attack.energy.stamina;
    field_input.context.incoming_power = attack.action_power.power;
    field_input.context.attack_subtype = attack.action_power.subtype;
    auto flow = resolve_prepared_field_handoff(field_input,profiles,defense_content);
    std::optional<PreparedKeeperResult> keeper;
    std::optional<ShotApproachResult> approach;
    if (flow.field.result_code < 2) {
        auto next = field_input.continuation;
        next.action_code = 0;
        next.field_result_code = flow.field.result_code;
        next.power_after_contacts = flow.field.remaining_power;
        next.subtype_after_contacts = attack.action_power.subtype;
        if (input.approach && next.post_loop_active_defenders == 0) {
            approach = calculate_shot_approach({input.approach->ball_x,
                input.approach->ball_y, side, attack.action_power.subtype,
                input.approach->random_first}, input.approach->content);
            next.field_proximity_carry = approach->loose_ball;
            next.keeper.elapsed = approach->distance_adjustment;
            next.outcome.ball_x = approach->target_x;
            next.outcome.ball_y = approach->target_y;
        }
        const auto route = select_action0_shot_route(next);
        flow.continuation = Action0ShotContinuationResult{route,std::nullopt,std::nullopt};
        if (route == ShotContinuationRoute::keeper_attempt) {
            const auto slot = static_cast<std::uint8_t>(side ^ 11u);
            auto comparison = next.keeper;
            comparison.incoming_power = flow.field.remaining_power;
            keeper = prepare_goalkeeper({record(slot,flow.field.roster),side,input.keeper_action,
                input.keeper_subtype,0,attack.action_power.subtype,input.keeper_energy_carry,
                comparison},keeper_profiles,keeper_content);
            auto outcome = next.outcome;
            outcome.comparison_result = keeper->comparison.result_code;
            outcome.attack_subtype = attack.action_power.subtype;
            outcome.defense_action = input.keeper_action;
            outcome.possession_side = side;
            outcome.ball_owner = owner;
            outcome.random_first = comparison.roll.random_first;
            outcome.random_second = comparison.roll.random_second;
            outcome.keeper_timer = comparison.stored_timer;
            outcome.keeper_penalty = comparison.stored_penalty;
            const auto resolved = resolve_shot_outcome(outcome);
            flow.field.roster[slot].stamina = keeper->energy.stamina;
            flow.continuation->keeper = keeper->comparison;
            flow.continuation->outcome = resolved;
        }
    }
    return {attack,std::move(flow),std::move(keeper),std::move(approach)};
}
}

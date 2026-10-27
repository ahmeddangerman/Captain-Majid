#include "captain/prepared_field_shot.hpp"
#include <stdexcept>
#include <utility>

namespace captain {
PreparedFieldShotResult resolve_prepared_field_shot(
    const PreparedFieldShotInput& input, const FieldPlayerProfiles& profiles,
    const FieldDefenseContent& defense) {
    if (input.context.side != 0 && input.context.side != 11)
        throw std::invalid_argument("Field shot needs a valid possession side");
    auto context = input.context;
    context.initial_result_code = 0; // Original no-defender branch writes zero.
    auto field = resolve_prepared_field_contacts(input.roster, input.choices,
                                                 profiles, defense, context);
    const auto* last = field.contacts.empty() ? nullptr
        : &input.choices[field.contacts.size() - 1];
    std::optional<BallRecoveryInput> recovery;
    if (field.result_code == 2 && input.recovery) {
        recovery = input.recovery;
        recovery->ball_owner = input.ball_owner;
        recovery->opposing_player = last->player_id;
    }
    const auto handoff = resolve_defender_outcome({field.result_code, context.side,
        input.ball_owner, static_cast<std::uint8_t>(last ? last->player_id : 0),
        static_cast<std::uint8_t>(last ? last->action : 0), recovery});
    std::optional<Action0ShotContinuationResult> continuation;
    if (field.result_code < 2) {
        auto next = input.continuation;
        next.action_code = context.attack_action;
        next.field_result_code = field.result_code;
        next.power_after_contacts = field.remaining_power;
        next.subtype_after_contacts = context.attack_subtype;
        next.keeper.roll.defender_slot = context.side ^ 11u;
        next.outcome.possession_side = handoff.possession_side;
        next.outcome.ball_owner = handoff.ball_owner;
        continuation = resolve_action0_shot_continuation(next);
    }
    return {std::move(field), handoff, std::move(continuation)};
}
}

#pragma once

#include "captain/energy.hpp"
#include "captain/goalkeeper.hpp"
#include "captain/shot_outcome.hpp"
#include "captain/shot_power.hpp"

#include <optional>

namespace captain {
struct ShotSequenceInput {
    EnergyInput energy;
    ActionPowerInput action;
    GoalkeeperInput keeper;
    ShotOutcomeInput outcome;
};
struct ShotSequenceResult {
    EnergyResult energy;
    ActionPowerResult action;
    GoalkeeperResult keeper;
    ShotOutcomeResult outcome;
};

enum class ShotContinuationRoute : std::uint8_t {
    keeper_attempt,
    loose_ball,
    other_continuation,
    unsupported_action
};

// Inputs at the action-0 continuation after field contacts. The route gates
// correspond to the post-loop count, field-proximity carry, and opposing
// goalkeeper status checks in the original flow.
struct Action0ShotContinuationInput {
    std::uint8_t action_code;
    std::uint8_t field_result_code;
    std::uint8_t post_loop_active_defenders;
    bool field_proximity_carry;
    std::uint8_t opposing_keeper_status;
    std::uint16_t power_after_contacts;
    std::uint8_t subtype_after_contacts; // Attacking subtype; distinct from keeper.roll.subtype.
    GoalkeeperInput keeper;
    ShotOutcomeInput outcome;
};

struct Action0ShotContinuationResult {
    ShotContinuationRoute route;
    std::optional<GoalkeeperResult> keeper;
    std::optional<ShotOutcomeResult> outcome;
};

// Runs the supported shot stages in order using caller-selected content and
// random bytes. Match setup, field contacts, and phase continuation are separate.
ShotSequenceResult resolve_explicit_shot(const ShotSequenceInput& input);

// Routes only action-0 field results 0/1. Diverted routes carry no keeper or
// outcome result; the caller handles the next gameplay phase.
ShotContinuationRoute select_action0_shot_route(const Action0ShotContinuationInput& input);

Action0ShotContinuationResult resolve_action0_shot_continuation(
    const Action0ShotContinuationInput& input);
}

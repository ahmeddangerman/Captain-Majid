#pragma once
#include "captain/prepared_defender_sequence.hpp"
#include "captain/defender_outcome.hpp"
#include "captain/shot_sequence.hpp"

namespace captain {
struct PreparedFieldShotInput {
    FieldRoster roster;
    std::vector<FieldDefenseChoice> choices;
    FieldDefenseContext context;
    std::uint8_t ball_owner;
    std::optional<BallRecoveryInput> recovery;
    Action0ShotContinuationInput continuation;
};
struct PreparedFieldShotResult {
    PreparedDefenderSequenceResult field;
    DefenderOutcomeResult field_outcome;
    std::optional<Action0ShotContinuationResult> continuation;
};
PreparedFieldShotResult resolve_prepared_field_shot(
    const PreparedFieldShotInput& input, const FieldPlayerProfiles& profiles,
    const FieldDefenseContent& defense);
}

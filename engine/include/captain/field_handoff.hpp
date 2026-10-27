#pragma once

#include "captain/defender_outcome.hpp"
#include "captain/defender_sequence.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace captain {
struct FieldContactHandoffInput {
    std::vector<DefenderContactInput> contacts;
    std::uint16_t incoming_power;
    std::uint8_t initial_result_code;
    std::uint8_t possession_side;
    std::uint8_t ball_owner;
    std::uint8_t defender_id;
    std::uint8_t defense_action;
    std::optional<BallRecoveryInput> recovery{};
};

struct FieldContactHandoffResult {
    DefenderSequenceResult field;
    DefenderOutcomeResult outcome;
};

// Calculates caller-selected contacts in order, then dispatches the final
// field result through its immediate recovery/possession handoff.
FieldContactHandoffResult resolve_field_contact_handoff(
    const FieldContactHandoffInput& input);
}

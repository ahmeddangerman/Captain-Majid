#pragma once
#include "captain/field_phase.hpp"
#include "captain/match_clock.hpp"

namespace captain {
struct ContactPreludeContent {
    std::array<std::uint8_t, 14> field_elapsed, keeper_elapsed;
    std::array<std::uint8_t, 7> defender_status;
};
struct ContactPreludeState {
    FieldPhaseState field;
    MatchClockState clock;
    std::uint8_t shared_recovery_flags = 0, shared_recovery_timer = 0;
};
struct ContactPreludeInput {
    ContactPreludeState state;
    std::uint8_t defender_id, defense_action;
    bool affect_defender;
};
struct ContactPreludeResult {
    ContactPreludeState state;
    MatchClockResult clock;
    std::uint8_t elapsed_step;
};
// Original9095 carry/status selection and D193 recovery/clock effects under
// staged interrupt-free inputs; experience and prior scripts are not inferred.
ContactPreludeResult apply_contact_prelude(const ContactPreludeInput& input,
                                         const ContactPreludeContent& content);
}

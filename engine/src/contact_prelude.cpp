#include "captain/contact_prelude.hpp"
#include <stdexcept>
#include <utility>

namespace captain {
ContactPreludeResult apply_contact_prelude(const ContactPreludeInput& input,
                                         const ContactPreludeContent& content) {
    if ((input.state.field.side != 0 && input.state.field.side != 11) ||
        input.state.field.ball_owner > 21 || input.defender_id > 21 || input.defense_action >= 7)
        throw std::invalid_argument("Unsupported contact prelude side/owner/defender/action");
    auto state = input.state;
    const auto index = unsigned(input.defense_action) * 2u + (input.affect_defender ? 1u : 0u);
    const bool keeper = input.defender_id == 0 || input.defender_id == 11;
    const auto elapsed = keeper ? content.keeper_elapsed[index] : content.field_elapsed[index];
    if (!keeper) {
        if (input.affect_defender)
            state.field.motion[field_motion_index(input.defender_id)].status = content.defender_status[input.defense_action];
        else if (state.field.ball_owner != 0 && state.field.ball_owner != 11)
            state.field.motion[field_motion_index(state.field.ball_owner)].status = 5;
    }
    state.field.elapsed = static_cast<std::uint8_t>(unsigned(state.field.elapsed) + elapsed);
    const auto recovered = advance_keeper_recovery({state.field.keepers,
        state.shared_recovery_flags, state.shared_recovery_timer}, elapsed);
    state.field.keepers = recovered.keepers;
    state.shared_recovery_flags = recovered.shared_flags;
    state.shared_recovery_timer = recovered.shared_timer;
    const auto clock = advance_match_clock(state.clock, elapsed, state.field.random_first);
    state.clock = clock.state;
    return {std::move(state), clock, elapsed};
}
}

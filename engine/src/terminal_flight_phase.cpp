#include "captain/terminal_flight_phase.hpp"
#include <stdexcept>
#include <utility>

namespace captain {
TerminalFlightPhaseResult resolve_prepared_terminal_flight(const PreparedTerminalFlightInput& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& content) {
    if ((input.state.side != 0 && input.state.side != 11) || input.state.ball_owner > 21 ||
        (input.result_code != 2 && input.result_code != 3) ||
        (input.defense_action != 0 && input.defense_action != 2) ||
        (input.defense_subtype != 0 && input.defense_subtype != 128))
        throw std::invalid_argument("Unsupported prepared terminal flight context");
    const auto index = field_motion_index(input.defender_id);
    const std::uint8_t defender_side = input.defender_id < 11 ? 0 : 11;
    if (defender_side == input.state.side)
        throw std::invalid_argument("Terminal flight defender must oppose possession");
    auto state = input.state;
    if (input.result_code == 2) {
        state.ball_x = static_cast<std::uint8_t>(state.motion[index].x >> 8);
        state.ball_y = static_cast<std::uint8_t>(state.motion[index].y >> 8);
        state.selected_defenders = 0;
        // Original recovery handoff retains elapsed and the prior ball-cell
        // byte. The next recovery phase owns its recalculation.
        return {std::move(state), TerminalFlightNextPhase::ball_recovery, std::nullopt};
    }
    state.side = defender_side;
    state.ball_owner = input.defender_id;
    auto movement = advance_field_phase(state, primary, content);
    auto final_state = movement.state;
    return {std::move(final_state), TerminalFlightNextPhase::player_control, std::move(movement)};
}
}

#include "captain/control_session.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>

namespace captain {
ControlSetupResult prepare_session_control(const PhaseSessionState& input) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::player_control || !input.control ||
        !input.recovery || !input.experience)
        throw std::invalid_argument("Control setup requires a player-control handoff and captured contexts");
    const auto owner_index = field_motion_index(input.current.field.ball_owner);
    const auto& carrier = input.current.field.motion[owner_index];
    const auto x = static_cast<std::uint8_t>(carrier.x >> 8);
    const auto y = static_cast<std::uint8_t>(carrier.y >> 8);
    const auto cell = field_cell_at(x, y);
    if (cell == 255) throw std::invalid_argument("Control setup requires an interior field carrier");
    ControlSetupResult result{input};
    auto& state = result.state;
    auto& field = state.current.field;
    auto& recovery = *state.recovery;
    auto& control = *state.control;
    result.presentation = {{ControlPresentationKind::display, 0}, {ControlPresentationKind::display, 1},
        {ControlPresentationKind::script, 30}, {ControlPresentationKind::field_map, 0}};
    const auto status_requests = [&result](std::uint8_t side) {
        if (side == 0) result.presentation.push_back({ControlPresentationKind::display, 48});
        else {
            result.presentation.push_back({ControlPresentationKind::display, 49});
            result.presentation.push_back({ControlPresentationKind::display, 50});
        }
    };
    status_requests(field.side);
    state.current.clock.script_flags = 128;
    state.current.clock.ball_mode = 128;
    control.travel_accumulator = 0;
    control.display_parameter = 2;
    // E233 writes0469, represented as the high byte of existing slot10.
    state.experience->totals[10] = static_cast<std::uint16_t>((state.experience->totals[10] & 255u) | 256u);
    control.decision_timer = 10;
    field.movement_region = 255;
    field.ball_x = x; field.ball_y = y; field.ball_cell = cell;
    recovery.ball_variant = 0;
    field.selected_defenders = 0;
    field.side = field.ball_owner < 11 ? 0 : 11;
    control.facing = field.side == 0 ? 64 : 0;
    if (field.side == 0) {
        if (recovery.special_owner_flags & 128u) {
            recovery.special_owner_flags = 0;
            field.roster[20].stamina &= 0xFF00u;
            result.special_owner_cleared = true;
        }
    } else {
        recovery.opposing_player = 0;
        NearestFieldPlayerInput nearest{0, x, y, field.ball_owner, 0, {}};
        for (std::size_t i = 0; i < field.motion.size(); ++i) {
            const auto& actor = field.motion[i];
            nearest.players[i] = {static_cast<std::uint8_t>(actor.x >> 8),
                static_cast<std::uint8_t>(actor.y >> 8), actor.status != 0};
        }
        result.defender = find_nearest_field_player(nearest);
        control.controlled_player = result.defender->player_id;
        field.motion[owner_index].target_cell = 5;
        control.last_carrier_cell = field.ball_cell;
    }
    status_requests(field.side);
    state.phase = FieldSessionPhase::control_ready;
    validate_phase_session(state);
    return result;
}
}

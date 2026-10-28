#include "captain/control_input.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>
#include <utility>

namespace captain {
ControlButtonResult apply_control_buttons(const PhaseSessionState& input, std::uint8_t held, std::uint8_t pressed) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::control_ready && input.phase != FieldSessionPhase::field_round_pending)
        throw std::invalid_argument("Buttons require control-ready or field-round phase");
    ControlButtonResult result{input};
    auto& state = result.state;
    auto& field = state.current.field;
    auto& control = *state.control;
    control.motion_requested = 0;
    if (field.side == 0) {
        state.current.clock.script_flags |= 64u;
        if (held & 64u) {
            field.selected_defenders = 0;
            if (state.scheduling) state.scheduling->selected_ids.fill(0);
            state.current.clock.script_flags = 0;
            state.phase = FieldSessionPhase::action_menu_pending;
            result.menu_requested = true;
            return result;
        }
        if (held & 15u) {
            control.motion_requested = 1;
            control.facing = held & 2u ? 0 : 64;
            state.current.clock.script_flags &= 191u;
        }
    } else {
        control.motion_requested = 1;
        if (pressed & 192u) {
            auto selected = static_cast<std::uint8_t>(unsigned(control.controlled_player) +
                ((pressed & 128u) ? 1u : 255u));
            if (selected == 0) selected = 10;
            if (selected >= 11) selected = 1;
            control.controlled_player = selected;
            result.presentation = {{ControlPresentationKind::display, 49},
                                   {ControlPresentationKind::display, 50}};
        }
    }
    return result;
}

ControlPollResult poll_session_control(const PhaseSessionState& input,
    std::uint8_t held, std::uint8_t pressed, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ControlMovementContent& control_content) {
    if (input.phase != FieldSessionPhase::control_ready)
        throw std::invalid_argument("Input poll requires control-ready phase");
    auto buttons = apply_control_buttons(input, held, pressed);
    ControlPollResult result{std::move(buttons.state), std::move(buttons.presentation), {}, 0, false, buttons.menu_requested};
    if (result.menu_requested) return result;
    auto& state = result.state;
    auto& field = state.current.field;
    auto& control = *state.control;
    if (control.decision_timer != 0) {
        --control.decision_timer;
        return result;
    }
    control.decision_timer = 10;
    result.decision_step = true;
    if (held & 15u) {
        const auto id = field.side == 0 ? field.ball_owner : control.controlled_player;
        const auto& stats = field.roster[id];
        const auto profile = load_movement_profile({id, stats.type, static_cast<std::uint8_t>(stats.stamina),
            static_cast<std::uint8_t>(stats.stamina >> 8), stats.level}, field.side, primary, movement.profiles);
        const auto speed = field.side == 0 ? control_content.carrier_speeds[profile.speed_profile] : profile.speed;
        auto& actor = field.motion[field_motion_index(id)];
        const auto step = [speed](std::uint16_t value, std::uint8_t direction, FieldAxis axis) {
            const auto bits = direction & 3u;
            if (bits == 0) return value;
            const auto displacement = static_cast<std::uint16_t>((bits & 1u) ? unsigned(speed) : 0u - speed);
            return advance_field_coordinate(value, displacement, axis);
        };
        actor.x = step(actor.x, held, FieldAxis::horizontal);
        actor.y = step(actor.y, static_cast<std::uint8_t>(held >> 2), FieldAxis::vertical);
        result.moved_player = id;
        result.movement_speed = speed;
    }
    const auto& carrier = field.motion[field_motion_index(field.ball_owner)];
    field.ball_x = static_cast<std::uint8_t>(carrier.x >> 8);
    field.ball_y = static_cast<std::uint8_t>(carrier.y >> 8);
    field.ball_cell = field_cell_at(field.ball_x, field.ball_y);
    if (field.ball_owner >= 11 && field.ball_cell != control.last_carrier_cell) {
        control.last_carrier_cell = field.ball_cell;
        state.phase = FieldSessionPhase::ai_cell_decision;
    } else state.phase = FieldSessionPhase::control_clock_pending;
    validate_phase_session(state);
    return result;
}
}

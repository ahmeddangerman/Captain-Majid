#include "captain/restart_options.hpp"
#include <stdexcept>
#include <utility>
namespace captain {
namespace {
void redraw(std::vector<ControlPresentationRequest>& requests) {
    requests.push_back({ControlPresentationKind::clear_overlay,0});
    requests.push_back({ControlPresentationKind::display,0});
    requests.push_back({ControlPresentationKind::display,1});
}
}
RestartOptionsResult open_prepared_restart_options(const PhaseSessionState& input) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::selected_field_restart &&
        input.phase != FieldSessionPhase::selected_goal_restart)
        throw std::invalid_argument("Prepared restart options require a routed restart");
    RestartOptionsResult result{input};
    result.state.restart_options = RestartOptionsContext{
        input.phase == FieldSessionPhase::selected_goal_restart,4};
    result.state.phase = FieldSessionPhase::restart_options;
    result.state.current.clock.ball_mode = 0;
    redraw(result.presentation);
    result.presentation.push_back({ControlPresentationKind::display,51});
    result.presentation.push_back({ControlPresentationKind::sprite,4});
    validate_phase_session(result.state);
    return result;
}
RestartOptionsResult poll_prepared_restart_options(const PhaseSessionState& input,
    std::uint8_t held, std::uint8_t pressed) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::restart_options)
        throw std::invalid_argument("Restart input requires the options loop");
    RestartOptionsResult result{input};
    auto& options = *result.state.restart_options;
    if (pressed & 12u) {
        options.cursor ^= 64u;
        result.presentation.push_back({ControlPresentationKind::sprite,options.cursor});
    }
    // Original98BE tests held A, after applying both vertical pressed bits once.
    if (held & 128u) {
        redraw(result.presentation);
        if (options.cursor & 64u) {
            options.cursor = 0;
            result.state.phase = FieldSessionPhase::restart_tactics;
        } else result.state.phase = FieldSessionPhase::restart_options_done;
    }
    validate_phase_session(result.state);
    return result;
}
}

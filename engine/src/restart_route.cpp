#include "captain/restart_route.hpp"
#include <stdexcept>
#include <utility>
namespace captain {
SelectedRestartRouteResult route_session_selected_restart(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::selected_restart_entry)
        throw std::invalid_argument("Restart routing requires the selected restart entry");
    const auto& f=input.current.field;const auto& p=*input.shot_progress;
    const auto resolved=resolve_field_interruption({4,f.selected_defenders,p.defender_id,
        p.defense_action,f.side,f.ball_x,f.ball_y,f.random_first,{}});
    const auto restart=resolved.restart.value();
    auto state=input;state.recovery->ball_variant=restart.situation;
    state.phase=restart.route==FieldRestartRoute::field_restart
        ? FieldSessionPhase::selected_field_restart : FieldSessionPhase::selected_goal_restart;
    validate_phase_session(state);return {std::move(state),restart};
}
}

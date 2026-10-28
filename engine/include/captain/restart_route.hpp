#pragma once
#include "captain/phase_session.hpp"
#include "captain/field_interruption.hpp"
namespace captain {
struct SelectedRestartRouteResult { PhaseSessionState state; FieldRestartDecision restart; };
SelectedRestartRouteResult route_session_selected_restart(const PhaseSessionState& input);
}

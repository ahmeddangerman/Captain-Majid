#pragma once
#include "captain/control_session.hpp"
namespace captain {
struct RestartOptionsResult {
    PhaseSessionState state;
    std::vector<ControlPresentationRequest> presentation{};
};
// Caller supplies state after restart opening scripts43/31 and55. Presentation
// waits are explicit acknowledgements; this does not run live frame tasks.
RestartOptionsResult open_prepared_restart_options(const PhaseSessionState& input);
RestartOptionsResult poll_prepared_restart_options(const PhaseSessionState& input,
    std::uint8_t held, std::uint8_t pressed);
}

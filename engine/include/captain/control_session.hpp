#pragma once
#include "captain/phase_session.hpp"
#include "captain/ball_recovery.hpp"

namespace captain {
enum class ControlPresentationKind : std::uint8_t { display, script, field_map, sprite, clear_overlay };
struct ControlPresentationRequest {
    ControlPresentationKind kind;
    std::uint8_t id;
};
struct ControlSetupResult {
    PhaseSessionState state;
    std::vector<ControlPresentationRequest> presentation{};
    std::optional<NearestFieldPlayerResult> defender{};
    bool special_owner_cleared = false;
};
// Supported field-carrier E0DF..E145 setup with frozen presentation inputs.
// Stops before live controller sampling, AI decisions and frame advancement.
ControlSetupResult prepare_session_control(const PhaseSessionState& input);
}

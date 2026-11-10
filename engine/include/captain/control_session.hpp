#pragma once
#include "captain/phase_session.hpp"
#include "captain/ball_recovery.hpp"

namespace captain {
struct ControlSetupResult;
ControlSetupResult prepare_session_area_action4_restart_pass_control(const PhaseSessionState& input);
ControlSetupResult prepare_session_area_action4_control(const PhaseSessionState& input);
enum class ControlPresentationKind : std::uint8_t { display, script, field_map, sprite, clear_overlay, wait_frames };
struct ControlPresentationRequest {
    ControlPresentationKind kind;
    std::uint8_t id;
    std::uint8_t slot=0;
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
ControlSetupResult prepare_postgoal_return_control(const PhaseSessionState& input);
ControlSetupResult prepare_sampled_player5_result3_control(const PhaseSessionState& input);
ControlSetupResult prepare_session_area_action6_control(const PhaseSessionState& input);
ControlSetupResult prepare_session_area_pass_capture_control(const PhaseSessionState& input);
ControlSetupResult prepare_session_area_pass_blocked_control(const PhaseSessionState& input);
}

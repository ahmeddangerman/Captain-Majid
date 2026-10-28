#pragma once
#include "captain/phase_session.hpp"
namespace captain {
struct CaptureClearanceContent {
    std::array<std::array<std::uint8_t,2>,8> destinations;
    std::uint8_t distance_adjustment;
};
struct CaptureClearanceResult {
    PhaseSessionState state;
    bool request_side_music = true;
    std::uint8_t requested_task = 2;
};
CaptureClearanceResult prepare_session_capture_clearance(const PhaseSessionState& input,
    const CaptureClearanceContent& content);
}

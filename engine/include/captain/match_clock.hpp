#pragma once
#include <cstdint>

namespace captain {
struct MatchClockState {
    std::uint16_t remaining = 0;
    std::uint8_t extra_time = 0, flags = 0, script_flags = 0, ball_mode = 0;
};
enum class ClockContinuation : std::uint8_t { ready, added_time_presentation, period_end };
struct MatchClockResult {
    MatchClockState state;
    ClockContinuation continuation = ClockContinuation::ready;
    bool request_clock_display = false, request_warning_cue = false;
};
// Numeric clock decisions through return, pre-added-time presentation, or
// period-end boundary. Presentation/NMI refresh is explicitly outside this API.
MatchClockResult advance_match_clock(const MatchClockState& state,
    std::uint8_t elapsed, std::uint8_t random_first);
}

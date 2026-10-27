#include "captain/match_clock.hpp"

namespace captain {
MatchClockResult advance_match_clock(const MatchClockState& state,
    std::uint8_t elapsed, std::uint8_t random_first) {
    MatchClockResult result{state};
    if (state.remaining == 0) {
        const auto extra = static_cast<std::uint8_t>(unsigned(state.extra_time) - elapsed);
        if (extra & 128u) {
            result.continuation = ClockContinuation::period_end;
            return result; // Original branches before storing the wrapped byte.
        }
        result.state.extra_time = extra;
        return result;
    }
    if (elapsed == 0) return result;
    auto remaining = static_cast<std::uint16_t>(unsigned(state.remaining) - elapsed);
    if (remaining & 32768u) remaining = 0;
    result.state.remaining = remaining;
    result.request_clock_display = true;
    if (!(state.flags & 128u) && static_cast<std::uint8_t>(remaining) < 30) {
        result.state.flags |= 128u;
        result.request_warning_cue = true;
    }
    if (remaining != 0) return result;
    result.state.extra_time = static_cast<std::uint8_t>(
        unsigned(state.extra_time) + ((random_first & 128u) ? 12u : 0u));
    if (result.state.extra_time == 0) return result;
    result.state.ball_mode = 0;
    result.state.script_flags &= 191u;
    result.continuation = ClockContinuation::added_time_presentation;
    return result;
}
}

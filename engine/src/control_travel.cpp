#include "captain/control_travel.hpp"
#include <stdexcept>

namespace captain {
std::uint16_t advance_control_travel(std::uint16_t travel, std::uint16_t speed,
                                     std::uint8_t side, std::uint8_t held,
                                     std::uint8_t facing) {
    if (side != 0 && side != 11) throw std::invalid_argument("Control travel requires side 0 or 11");
    if (side == 0 && !(held & 15u)) return travel;
    const auto quarter = static_cast<std::uint16_t>(speed >> 2);
    const auto delta = static_cast<std::uint16_t>((facing & 64u) ? quarter : ~quarter);
    return static_cast<std::uint16_t>(unsigned(travel) + delta);
}

PhaseSessionState advance_session_control_travel(const PhaseSessionState& input,
                                                 std::uint16_t speed, std::uint8_t held) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::control_ready || !input.control)
        throw std::invalid_argument("Control travel requires a control-ready session");
    auto state = input;
    state.control->travel_accumulator = advance_control_travel(
        state.control->travel_accumulator, speed, state.current.field.side, held, state.control->facing);
    validate_phase_session(state);
    return state;
}
}

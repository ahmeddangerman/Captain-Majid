#include "captain/control_travel.hpp"
#include "captain/control_input.hpp"
#include "captain/field_phase.hpp"
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

ProfiledControlTravelResult advance_profiled_session_control_travel(
    const PhaseSessionState& input, std::uint8_t held, const PlayerProfileMappings& profiles,
    const FieldPhaseContent& movement, const ControlMovementContent& control) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::control_ready || !input.control)
        throw std::invalid_argument("Control travel requires a control-ready session");
    const auto& field = input.current.field;
    if (field.side == 0 && !(held & 15u)) return {input, std::nullopt};
    const auto id = field.ball_owner;
    const auto& stats = field.roster[id];
    const auto profile = load_movement_profile(
        {id, stats.type, static_cast<std::uint8_t>(stats.stamina),
         static_cast<std::uint8_t>(stats.stamina >> 8), stats.level},
        field.side, profiles, movement.profiles);
    const auto speed = control.carrier_speeds[profile.speed_profile];
    return {advance_session_control_travel(input, speed, held), speed};
}
}

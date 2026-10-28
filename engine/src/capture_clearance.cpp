#include "captain/capture_clearance.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>
#include <utility>
namespace captain {
CaptureClearanceResult prepare_session_capture_clearance(const PhaseSessionState& input,
    const CaptureClearanceContent& content) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::selected_defense_animation)
        throw std::invalid_argument("Clearance setup requires the selected capture animation entry");
    auto state=input;
    auto& field=state.current.field;
    const auto& target=content.destinations[field.random_first & 7u];
    const auto x=static_cast<std::uint8_t>(target[0] ^ ((field.ball_x & 128u) ? 255u : 0u));
    const auto y=static_cast<std::uint8_t>(target[1] ^ ((field.ball_y & 128u) ? 255u : 0u));
    const auto cell=field_cell_at(x,y);
    if(cell==255) throw std::invalid_argument("Clearance content produced an outside-field destination");
    field.side ^= 11u;
    field.ball_x=x;field.ball_y=y;field.ball_cell=cell;
    state.clearance_flight=ClearanceFlightContext{content.distance_adjustment};
    state.phase=FieldSessionPhase::capture_clearance_flight;
    validate_phase_session(state);
    return {std::move(state),true,2};
}
}

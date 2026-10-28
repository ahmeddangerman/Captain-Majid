#include "captain/clearance_handoff.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState continue_session_clearance(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::clearance_race_winner && input.phase!=FieldSessionPhase::clearance_race_timeout)
        throw std::invalid_argument("No pending clearance presentation to continue");
    auto state=input;
    if(input.phase==FieldSessionPhase::clearance_race_timeout) {
        state.phase=FieldSessionPhase::clearance_recovery_ready;
    } else {
        const auto& f=state.current.field;
        if(f.ball_y>=96 && f.ball_y<160 && (f.ball_x<80 || f.ball_x>=176))
            throw std::invalid_argument("Clearance winner awaits keeper-area continuation");
        state.shot_progress.reset();state.clearance_flight.reset();state.clearance_race.reset();
        state.phase=FieldSessionPhase::player_control;
    }
    validate_phase_session(state);return state;
}
}

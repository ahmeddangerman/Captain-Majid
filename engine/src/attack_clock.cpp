#include "captain/attack_clock.hpp"
#include <stdexcept>
namespace captain {
StagedShotResult continue_prepared_session_attack(const PhaseSessionState& input,
    const ContactPreludeContent& content, std::optional<std::uint8_t> unopposed_action) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::selected_attack_presentation ||
    input.control->last_carrier_cell || input.action->menu_route ||
    (input.shot_progress->cursor && input.recovery->opposing_player!=input.shot_progress->defender_id) ||
    (bool(input.shot_progress->cursor) == bool(unopposed_action)) ||
    (unopposed_action && *unopposed_action > 6))
  throw std::invalid_argument("Prepared attack requires a selected field defender, ordinary menu and no active keeper");
 auto state=input;const auto& p=*input.shot_progress;
 const auto defender=p.cursor ? p.defender_id : state.recovery->opposing_player;
 const auto action=p.cursor ? p.defense_action : *unopposed_action;
 auto applied=apply_contact_prelude({state.current,defender,action,true},content);
 state.current=applied.state;
 state.phase=applied.clock.continuation==ClockContinuation::period_end ? FieldSessionPhase::period_end :
  applied.clock.continuation==ClockContinuation::added_time_presentation ? FieldSessionPhase::attack_clock_added_time :
  FieldSessionPhase::shot_approach_ready;
 validate_phase_session(state);return {std::move(state),std::nullopt,applied.clock};
}
PhaseSessionState resume_session_attack_clock(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::attack_clock_added_time)
  throw std::invalid_argument("No attack clock presentation to resume");
 auto state=input;state.phase=FieldSessionPhase::shot_approach_ready;
 validate_phase_session(state);return state;
}
}

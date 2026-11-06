#include "captain/attack_clock.hpp"
#include <stdexcept>
namespace captain {
StagedShotResult continue_prepared_session_attack(const PhaseSessionState& input,
    const ContactPreludeContent& content, std::optional<std::uint8_t> unopposed_action) {
 validate_phase_session(input);
 const bool sampled_action9=input.phase==FieldSessionPhase::selected_attack_presentation && unopposed_action && *unopposed_action==9 &&
    !input.shot_progress->cursor && input.current.field.side==0 && input.current.field.ball_owner==3 &&
    input.current.field.ball_x==80 && input.current.field.ball_y==128 &&
    input.current.clock.remaining==445 && input.current.clock.extra_time==5 && input.current.field.elapsed==0 &&
    input.current.field.random_first==0 && input.current.field.random_second==0 &&
    input.recovery->opposing_player==0 && input.recovery->attack_subtype==128 &&
    input.action->power==22 && input.control->motion_requested==2 && content.keeper_elapsed[1]==0;
 if(input.phase!=FieldSessionPhase::selected_attack_presentation ||
    input.control->last_carrier_cell || input.action->menu_route ||
    (input.shot_progress->cursor && input.recovery->opposing_player!=input.shot_progress->defender_id) ||
    (bool(input.shot_progress->cursor) == bool(unopposed_action)) ||
    (unopposed_action && *unopposed_action > 6 && !sampled_action9))
  throw std::invalid_argument("Prepared attack requires a selected field defender, ordinary menu and no active keeper");
 auto state=input;const auto& p=*input.shot_progress;
 const auto defender=p.cursor ? p.defender_id : state.recovery->opposing_player;
 // For this captured keeper0 case the original action9 table entry is zero;
 // action0 has the same checked zero keeper elapsed and no field status effect.
 const auto action=p.cursor ? p.defense_action : static_cast<std::uint8_t>(sampled_action9 ? 0 : *unopposed_action);
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

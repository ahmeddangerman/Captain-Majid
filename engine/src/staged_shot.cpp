#include "captain/staged_shot.hpp"
#include <stdexcept>
#include <utility>
namespace captain {
namespace {
FieldSessionPhase after_contact(const PhaseSessionState& s) {
    const auto& p = *s.shot_progress;
    if (p.result_code >= 2) return FieldSessionPhase::shot_dispatch_ready;
    return p.cursor == s.current.field.selected_defenders
        ? FieldSessionPhase::shot_dispatch_clock : FieldSessionPhase::shot_defender_next;
}
void clock_boundary(PhaseSessionState& s, const MatchClockResult& clock,
                    FieldSessionPhase paused, FieldSessionPhase ready) {
    s.phase = clock.continuation == ClockContinuation::period_end ? FieldSessionPhase::period_end
        : clock.continuation == ClockContinuation::added_time_presentation ? paused : ready;
}
}
StagedShotResult step_session_shot_defender(const PhaseSessionState& input,
    const FieldDefenseChoice& choice, bool energy_carry,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& defense,
    const FieldComparisonContent& comparison, const ContactPreludeContent& prelude) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::attack_prepared && input.phase != FieldSessionPhase::shot_defender_next)
        throw std::invalid_argument("Shot is not waiting for a selected defender");
    auto state = input;
    if (!state.shot_progress) {
        state.shot_progress = ShotContactProgress{};
        state.control->last_carrier_cell = 0; // Original617 now stores active selected keepers.
    }
    auto& progress = *state.shot_progress;
    auto& field = state.current.field;
    if (progress.cursor >= field.selected_defenders ||
        choice.player_id != state.scheduling->selected_ids[progress.cursor] ||
        (choice.action != 0 && choice.action != 2) || choice.situation != 0 ||
        (choice.subtype != 0 && choice.subtype != 128))
        throw std::invalid_argument("Expected the next ordinary saved defender choice");
    FieldInterruptionContext interruption{field.ball_x,field.ball_y,{}};
    for (unsigned id=1;id<=21;++id) if(id!=11)
        interruption.state.status[id]=field.motion[field_motion_index(id)].status;
    interruption.state.status[0]=field.keepers[0].stun;
    interruption.state.status[11]=field.keepers[1].stun;
    interruption.state.keeper_penalties={field.keepers[0].penalty,field.keepers[1].penalty};
    const FieldDefenseContext context{field.side,0,state.recovery->attack_subtype,state.action->power,0,
        energy_carry,interruption,comparison};
    auto resolved=resolve_prepared_field_contacts(field.roster,{choice},profiles,defense,context);
    field.roster=resolved.roster;
    state.action->power=resolved.remaining_power;
    field.random_first=choice.comparison.random_first;field.random_second=choice.comparison.random_second;
    state.recovery->opposing_player=choice.player_id;
    for (unsigned id=1;id<=21;++id) if(id!=11)
        field.motion[field_motion_index(id)].status=resolved.interruption_state->status[id];
    for(unsigned i=0;i<2;++i) {
        field.keepers[i].stun=resolved.interruption_state->status[i==0?0:11];
        field.keepers[i].penalty=resolved.interruption_state->keeper_penalties[i];
    }
    const auto& contact=resolved.contacts.front();
    progress={static_cast<std::uint8_t>(progress.cursor+1),resolved.result_code,choice.player_id,
        choice.action,contact.contact.subtype};
    const auto applied=apply_contact_prelude({state.current,choice.player_id,choice.action,
        resolved.result_code<2},prelude);
    state.current=applied.state;
    clock_boundary(state,applied.clock,FieldSessionPhase::shot_contact_added_time,after_contact(state));
    validate_phase_session(state);
    return {std::move(state),contact,applied.clock};
}
StagedShotResult dispatch_session_shot(const PhaseSessionState& input,std::uint8_t elapsed) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::shot_dispatch_clock &&
        !(input.phase == FieldSessionPhase::attack_prepared && input.current.field.selected_defenders == 0))
        throw std::invalid_argument("Shot is not waiting for the final dispatch clock");
    auto state=input;
    if (!state.shot_progress) {
        state.shot_progress=ShotContactProgress{};state.control->last_carrier_cell=0;
    }
    state.current.field.elapsed=static_cast<std::uint8_t>(unsigned(state.current.field.elapsed)+elapsed);
    const auto recovery=advance_keeper_recovery({state.current.field.keepers,
        state.current.shared_recovery_flags,state.current.shared_recovery_timer},elapsed);
    state.current.field.keepers=recovery.keepers;
    state.current.shared_recovery_flags=recovery.shared_flags;state.current.shared_recovery_timer=recovery.shared_timer;
    const auto clock=advance_match_clock(state.current.clock,elapsed,state.current.field.random_first);
    state.current.clock=clock.state;
    clock_boundary(state,clock,FieldSessionPhase::shot_dispatch_added_time,FieldSessionPhase::shot_dispatch_ready);
    validate_phase_session(state);
    return {std::move(state),std::nullopt,clock};
}
PhaseSessionState resume_session_shot_clock(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::shot_contact_added_time && input.phase!=FieldSessionPhase::shot_dispatch_added_time)
        throw std::invalid_argument("No added-time shot clock to resume");
    auto state=input;
    state.phase=input.phase==FieldSessionPhase::shot_contact_added_time ? after_contact(state) : FieldSessionPhase::shot_dispatch_ready;
    validate_phase_session(state);return state;
}
}

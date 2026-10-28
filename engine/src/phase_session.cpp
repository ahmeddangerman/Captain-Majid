#include "captain/phase_session.hpp"
#include <stdexcept>
#include <utility>

namespace captain {
namespace {
void validate_contact(const FieldPhaseState& field, const PendingTerminalContact& contact) {
    if ((contact.result_code != 2 && contact.result_code != 3) ||
        (contact.defense_action != 0 && contact.defense_action != 2) ||
        (contact.defense_subtype != 0 && contact.defense_subtype != 128))
        throw std::invalid_argument("Unsupported terminal contact");
    field_motion_index(contact.defender_id);
    if ((contact.defender_id < 11 ? 0 : 11) == field.side)
        throw std::invalid_argument("Terminal contact requires an opposing field player");
}
PhaseSessionResult finish(PhaseSessionState state, PendingTerminalContact contact,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement) {
    state.pending.reset();
    TerminalFlightPhaseResult terminal;
    if (contact.result_code == 2) {
        // Placement happened before the clock. Do not re-read the defender's
        // position after a deferred presentation; preserve that earlier write.
        state.current.field.selected_defenders = 0;
        if (state.scheduling) state.scheduling->selected_ids.fill(0);
        terminal = {state.current.field, TerminalFlightNextPhase::ball_recovery, std::nullopt};
        state.phase = FieldSessionPhase::ball_recovery;
    } else {
        terminal = resolve_prepared_terminal_flight({state.current.field, contact.result_code,
            contact.defender_id, contact.defense_action, contact.defense_subtype}, primary, movement);
        state.current.field = terminal.state;
        state.phase = FieldSessionPhase::player_control;
    }
    return {std::move(state), std::nullopt, std::move(terminal)};
}
}

void validate_phase_session(const PhaseSessionState& state) {
    if ((state.current.field.side != 0 && state.current.field.side != 11) ||
        state.current.field.ball_owner > 21 || static_cast<unsigned>(state.phase) > 30)
        throw std::invalid_argument("Unsupported phase session state");
    const auto phase = static_cast<unsigned>(state.phase);
    const bool staged = phase >= 23 && phase <= 30;
    if (staged != bool(state.shot_progress) && !(state.phase == FieldSessionPhase::period_end && state.shot_progress))
        throw std::invalid_argument("Staged shot phase requires exactly one progress context");
    if (state.shot_progress) {
        const auto& p = *state.shot_progress;
        const auto count = state.current.field.selected_defenders;
        if (!state.scheduling || !state.action || !state.recovery || !state.control || !state.experience ||
            state.current.field.side != 0 || state.action->action != 0 || count > 5 ||
            p.cursor > count || p.result_code > 4 ||
            (p.defense_action != 0 && p.defense_action != 2) ||
            (p.defense_subtype != 0 && p.defense_subtype != 128))
            throw std::invalid_argument("Unsupported shot progress context");
        if ((state.phase == FieldSessionPhase::selected_ball_recovery && p.result_code != 2) ||
            (state.phase == FieldSessionPhase::selected_defense_animation &&
             (p.result_code != 3 || p.defense_action != 0)) ||
            (state.phase == FieldSessionPhase::selected_restart_entry && p.result_code != 4) ||
            (phase >= 28 && p.cursor == 0))
            throw std::invalid_argument("Selected handoff phase does not match its terminal result");
        if (p.cursor == 0) {
            if (p.result_code || p.defender_id || p.defense_action || p.defense_subtype)
                throw std::invalid_argument("Unused shot contact data must be zero");
        } else if (p.defender_id != state.scheduling->selected_ids[p.cursor - 1])
            throw std::invalid_argument("Shot cursor must retain the last selected defender");
        if (state.phase == FieldSessionPhase::shot_defender_next && (p.cursor >= count || p.result_code >= 2))
            throw std::invalid_argument("No next selected defender");
        if (state.phase == FieldSessionPhase::shot_contact_added_time && p.cursor == 0)
            throw std::invalid_argument("Contact clock pause requires a completed contact");
        if ((state.phase == FieldSessionPhase::shot_dispatch_clock ||
             state.phase == FieldSessionPhase::shot_dispatch_added_time ||
             state.phase == FieldSessionPhase::shot_dispatch_ready) &&
            p.result_code < 2 && p.cursor != count)
            throw std::invalid_argument("Nonterminal dispatch requires the completed prefix");
        if ((state.phase == FieldSessionPhase::shot_dispatch_clock ||
             state.phase == FieldSessionPhase::shot_dispatch_added_time) && p.result_code >= 2)
            throw std::invalid_argument("Terminal contact skips final dispatch clock");
    }
    if ((state.phase == FieldSessionPhase::added_time) != bool(state.pending))
        throw std::invalid_argument("Added-time session needs exactly one pending terminal contact");
    if (state.pending) validate_contact(state.current.field, *state.pending);
    if (state.experience) validate_experience_state(*state.experience);
    if (state.recovery && state.recovery->opposing_player > 21)
        throw std::invalid_argument("Unsupported recovery opposing player");
    if (static_cast<unsigned>(state.phase) >= 5 && !state.recovery)
        throw std::invalid_argument("Recovery continuation needs captured recovery context");
    const bool selected_phase = state.phase == FieldSessionPhase::field_contact_pending || static_cast<unsigned>(state.phase) >= 18;
    if (static_cast<unsigned>(state.phase) >= 5 && !selected_phase &&
        state.current.field.selected_defenders != 0)
        throw std::invalid_argument("Recovery continuation requires zero selected defenders");
    if (static_cast<unsigned>(state.phase) >= 6) {
        field_motion_index(state.current.field.ball_owner);
        if ((state.current.field.ball_owner < 11 ? 0 : 11) != state.current.field.side)
            throw std::invalid_argument("Recovered owner must belong to the possession side");
        const auto& field = state.current.field;
        if (field.ball_x < 48 || field.ball_x >= 208 || field.ball_y < 80 || field.ball_y >= 176)
            throw std::invalid_argument("Recovered-owner continuation requires an interior ball");
    }
    if (static_cast<unsigned>(state.phase) >= 9 && (!state.control || !state.experience))
        throw std::invalid_argument("Control-ready state needs captured control/experience context");
    if (static_cast<unsigned>(state.phase) >= 9 && state.current.field.side == 11 &&
        (state.control->controlled_player < 1 || state.control->controlled_player > 10))
        throw std::invalid_argument("Opposing control-ready state needs a side0 field defender");
    if (static_cast<unsigned>(state.phase) >= 13 && !state.scheduling)
        throw std::invalid_argument("Control-cycle continuation needs captured scheduling context");
    if ((state.phase == FieldSessionPhase::field_round_pending || state.phase == FieldSessionPhase::control_added_time) &&
        state.current.field.elapsed != 1)
        throw std::invalid_argument("Control-cycle continuation requires elapsed1");
    if (state.scheduling) {
        const auto count = state.current.field.selected_defenders;
        if (count > 5) throw std::invalid_argument("Selected defender prefix exceeds five slots");
        for (unsigned i = 0; i < 5; ++i) {
            if (i < count) field_motion_index(state.scheduling->selected_ids[i]);
            else if (state.scheduling->selected_ids[i] != 0)
                throw std::invalid_argument("Nonzero unused selected defender slot");
        }
    }
    if (selected_phase) {
        const auto& field = state.current.field;
        if ((state.phase == FieldSessionPhase::field_contact_pending && field.selected_defenders == 0) ||
            field.selected_defenders > (field.side == 0 ? 5 : 4))
            throw std::invalid_argument("Field-contact continuation needs a selected opposing prefix");
        for (unsigned i = 0; i < field.selected_defenders; ++i)
            if ((state.scheduling->selected_ids[i] < 11 ? 0 : 11) == field.side)
                throw std::invalid_argument("Selected contact player must oppose possession");
    }
    if (state.action && state.action->action > 9 &&
        (static_cast<unsigned>(state.phase) >= 19 ||
         (state.phase == FieldSessionPhase::human_action_menu && (state.action->cursor & 128u))))
        throw std::invalid_argument("Unsupported captured action ID");
    if (static_cast<unsigned>(state.phase) >= 18 && (!state.action || state.current.field.side != 0 || state.action->menu_route >= 6))
        throw std::invalid_argument("Human attacking menu requires captured action context and side0");
    if (static_cast<unsigned>(state.phase) >= 20 && state.action->action != 0)
        throw std::invalid_argument("Only ordinary shots are supported at attack-prepared phase");
}

PhaseSessionResult resolve_session_terminal(const PhaseSessionState& input,
    PendingTerminalContact contact, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ContactPreludeContent& prelude) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::field && input.phase != FieldSessionPhase::player_control &&
        input.phase != FieldSessionPhase::ball_recovery && input.phase != FieldSessionPhase::attack_prepared)
        throw std::invalid_argument("Session is waiting at a continuation boundary");
    validate_contact(input.current.field, contact);
    auto state = input;
    if (state.recovery) state.recovery->opposing_player = contact.defender_id;
    if (contact.result_code == 2) {
        const auto& defender = state.current.field.motion[field_motion_index(contact.defender_id)];
        state.current.field.ball_x = static_cast<std::uint8_t>(defender.x >> 8);
        state.current.field.ball_y = static_cast<std::uint8_t>(defender.y >> 8);
    }
    auto applied = apply_contact_prelude({state.current, contact.defender_id, contact.defense_action, false}, prelude);
    state.current = applied.state;
    if (applied.clock.continuation == ClockContinuation::period_end) {
        state.phase = FieldSessionPhase::period_end;
        state.pending.reset();
        return {std::move(state), std::move(applied), std::nullopt};
    }
    if (applied.clock.continuation == ClockContinuation::added_time_presentation) {
        state.phase = FieldSessionPhase::added_time;
        state.pending = contact;
        return {std::move(state), std::move(applied), std::nullopt};
    }
    auto result = finish(std::move(state), contact, primary, movement);
    result.prelude = std::move(applied);
    return result;
}

PhaseSessionResult resume_session_terminal(const PhaseSessionState& post_presentation,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement) {
    validate_phase_session(post_presentation);
    if (post_presentation.phase != FieldSessionPhase::added_time || !post_presentation.pending)
        throw std::invalid_argument("No added-time terminal contact to resume");
    return finish(post_presentation, *post_presentation.pending, primary, movement);
}

PhaseSessionResult resolve_session_rewarded_terminal(const PhaseSessionState& input,
    PendingTerminalContact contact, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ContactPreludeContent& prelude,
    const ExperienceContent& experience) {
    validate_phase_session(input);
    validate_contact(input.current.field, contact);
    if (!input.experience) throw std::invalid_argument("Rewarded contact needs captured experience context");
    auto state = input;
    std::vector<ExperienceRewardResult> rewards;
    const auto count = contact.result_code == 2 ? 2u : 1u;
    for (unsigned i = 0; i < count; ++i) {
        auto reward = award_player_experience(*state.experience,
            state.current.field.roster[contact.defender_id].type, 2, experience);
        state.experience = reward.state;
        rewards.push_back(std::move(reward));
    }
    auto result = resolve_session_terminal(state, contact, primary, movement, prelude);
    result.rewards = std::move(rewards);
    return result;
}
}

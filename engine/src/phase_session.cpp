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
        state.current.field.ball_owner > 21 || static_cast<unsigned>(state.phase) > 12)
        throw std::invalid_argument("Unsupported phase session state");
    if ((state.phase == FieldSessionPhase::added_time) != bool(state.pending))
        throw std::invalid_argument("Added-time session needs exactly one pending terminal contact");
    if (state.pending) validate_contact(state.current.field, *state.pending);
    if (state.experience) validate_experience_state(*state.experience);
    if (state.recovery && state.recovery->opposing_player > 21)
        throw std::invalid_argument("Unsupported recovery opposing player");
    if (static_cast<unsigned>(state.phase) >= 5 && !state.recovery)
        throw std::invalid_argument("Recovery continuation needs captured recovery context");
    if (static_cast<unsigned>(state.phase) >= 5 && state.current.field.selected_defenders != 0)
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
}

PhaseSessionResult resolve_session_terminal(const PhaseSessionState& input,
    PendingTerminalContact contact, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ContactPreludeContent& prelude) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::field && input.phase != FieldSessionPhase::player_control &&
        input.phase != FieldSessionPhase::ball_recovery)
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

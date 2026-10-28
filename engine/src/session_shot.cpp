#include "captain/session_shot.hpp"
#include <stdexcept>
#include <utility>
namespace captain {
SessionShotDefenseResult resolve_session_shot_defenders(const PhaseSessionState& input,
    const std::vector<FieldDefenseChoice>& choices, bool energy_carry,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& defense,
    const FieldComparisonContent& comparison, const FieldPhaseContent& movement,
    const ContactPreludeContent& prelude, const ExperienceContent& experience) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::attack_prepared)
        throw std::invalid_argument("Shot defenses require an already prepared ordinary shot");
    const auto& field = input.current.field;
    if (choices.size() != field.selected_defenders)
        throw std::invalid_argument("Shot choices must match the saved selected prefix");
    std::array<bool,22> seen{};
    for (std::size_t i = 0; i < choices.size(); ++i) {
        const auto& choice = choices[i];
        if (choice.player_id != input.scheduling->selected_ids[i] || seen[choice.player_id])
            throw std::invalid_argument("Shot choices require distinct defenders in saved order");
        seen[choice.player_id] = true;
        // Only ordinary field block/tackle decisions reach the supported
        // terminal continuation. Other actions need their own original caller.
        if ((choice.action != 0 && choice.action != 2) || choice.situation != 0 ||
            (choice.subtype != 0 && choice.subtype != 128))
            throw std::invalid_argument("Unsupported ordinary shot defense choice");
    }
    FieldInterruptionContext interruption{field.ball_x,field.ball_y,{}};
    for (unsigned id = 1; id <= 21; ++id) {
        if (id != 11) interruption.state.status[id] = field.motion[field_motion_index(id)].status;
    }
    interruption.state.status[0] = field.keepers[0].stun;
    interruption.state.status[11] = field.keepers[1].stun;
    interruption.state.keeper_penalties = {field.keepers[0].penalty, field.keepers[1].penalty};
    FieldDefenseContext context{field.side,0,input.recovery->attack_subtype,input.action->power,0,
        energy_carry,interruption,comparison};
    auto defenses = resolve_prepared_field_contacts(field.roster,choices,profiles,defense,context);
    auto state = input;
    state.current.field.roster = defenses.roster;
    state.action->power = defenses.remaining_power;
    if (!defenses.contacts.empty()) {
        const auto& sample = choices[defenses.contacts.size() - 1].comparison;
        state.current.field.random_first = sample.random_first;
        state.current.field.random_second = sample.random_second;
    }
    if (defenses.interruption_state) {
        for (unsigned id = 1; id <= 21; ++id) {
            if (id != 11) state.current.field.motion[field_motion_index(id)].status = defenses.interruption_state->status[id];
        }
        for (unsigned i = 0; i < 2; ++i) {
            state.current.field.keepers[i].penalty = defenses.interruption_state->keeper_penalties[i];
            state.current.field.keepers[i].stun = defenses.interruption_state->status[i == 0 ? 0 : 11];
        }
    }
    if (defenses.result_code == 2 || defenses.result_code == 3) {
        const auto& last = choices[defenses.contacts.size() - 1];
        // The rolled subtype, including any special bit, belongs to the
        // reached defender; never read a stale caller comparison subtype.
        const auto subtype = defenses.contacts.back().contact.subtype;
        auto terminal = resolve_session_rewarded_terminal(state,
            {defenses.result_code,last.player_id,last.action,subtype},profiles,movement,prelude,experience);
        state = terminal.state;
        validate_phase_session(state);
        return {std::move(state),std::move(defenses),std::move(terminal),std::nullopt};
    }
    std::optional<FieldRestartDecision> restart;
    if (defenses.result_code == 4) {
        restart = defenses.contacts.back().interruption->restart;
        state.recovery->ball_variant = restart->situation;
        state.phase = FieldSessionPhase::shot_restart_pending;
    } else {
        state.phase = FieldSessionPhase::shot_field_complete;
    }
    validate_phase_session(state);
    return {std::move(state),std::move(defenses),std::nullopt,restart};
}
}

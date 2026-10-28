#include "captain/selected_dispatch.hpp"
#include <stdexcept>
#include <utility>
namespace captain {
SelectedDispatchResult resolve_session_selected_result(const PhaseSessionState& input,
    const PlayerProfileMappings& primary,const FieldPhaseContent& content,
    const ExperienceContent& experience) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::shot_dispatch_ready || input.shot_progress->result_code < 2)
        throw std::invalid_argument("Selected dispatch requires a timed terminal field result");
    const auto progress = *input.shot_progress;
    // Original83F5 advances E074 before any reward or ownership write.
    auto movement = advance_field_phase(input.current.field,primary,content);
    auto state = input;
    state.current.field = movement.state;
    std::optional<ExperienceRewardResult> reward;
    if (progress.result_code == 2 || progress.result_code == 3) {
        reward = award_player_experience(*state.experience,
            state.current.field.roster[progress.defender_id].type,2,experience);
        state.experience = reward->state;
    }
    bool sound = false;
    if (progress.result_code == 2) {
        // Initial selected contact: no later-flight ball placement, no second
        // reward or clock step.8BDF owns the next recovery operation.
        state.phase = FieldSessionPhase::selected_ball_recovery;
    } else if (progress.result_code == 4) {
        state.phase = FieldSessionPhase::selected_restart_entry;
    } else if (progress.defense_action == 2) {
        const auto side = static_cast<std::uint8_t>(progress.defender_id < 11 ? 0 : 11);
        sound = side != state.current.field.side;
        state.current.field.ball_owner = progress.defender_id;
        state.current.field.side = side;
        state.shot_progress.reset();
        state.phase = FieldSessionPhase::player_control;
    } else {
        // E678 runs the action0 defense animation before its eventual handoff.
        state.phase = FieldSessionPhase::selected_defense_animation;
    }
    validate_phase_session(state);
    return {std::move(state),std::move(movement),std::move(reward),sound};
}
}

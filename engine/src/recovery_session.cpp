#include "captain/recovery_session.hpp"
#include <stdexcept>
#include <utility>

namespace captain {
RecoverySessionResult advance_session_recovery(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::ball_recovery || !input.recovery)
        throw std::invalid_argument("Recovery requires ball-recovery phase and captured context");
    if (input.current.field.selected_defenders != 0)
        throw std::invalid_argument("Connected recovery requires zero selected defenders");
    RecoverySessionResult result{input};
    auto& state = result.state;
    auto& field = state.current.field;
    auto& context = *state.recovery;
    // Original9070 cleans the special owner; it is not a random-generator call.
    if ((context.special_owner_flags & 128u) && field.ball_owner == 20) {
        context.special_owner_flags = 0;
        field.roster[20].stamina &= 0xFF00u;
        result.special_owner_cleared = true;
    }
    const auto ball = jitter_loose_ball(field.ball_x, field.ball_y, field.random_first, field.random_second);
    field.ball_x = ball.x;
    field.ball_y = ball.y;
    context.x_jitter_control = ball.x_jitter_control;
    if (!ball.inside_field) {
        state.phase = FieldSessionPhase::boundary_restart;
        return result;
    }
    result.movement = advance_field_phase(field, primary, movement);
    field = result.movement->state;
    BallRecoveryInput choice{};
    choice.ball_x = field.ball_x; choice.ball_y = field.ball_y;
    choice.ball_owner = field.ball_owner; choice.opposing_player = context.opposing_player;
    choice.random_e2 = field.random_first; choice.random_e3 = field.random_second;
    choice.ball_update_active = field.elapsed != 0;
    for (std::size_t i = 0; i < field.motion.size(); ++i) {
        const auto& actor = field.motion[i];
        choice.players[i] = {static_cast<std::uint8_t>(actor.x >> 8),
            static_cast<std::uint8_t>(actor.y >> 8), actor.status != 0};
    }
    result.recipient = recover_stationary_ball(choice);
    const auto& recovered = *result.recipient;
    result.request_possession_sound = field.side != recovered.possession_side;
    field.ball_owner = recovered.ball_owner;
    field.side = recovered.possession_side;
    auto& owner = field.motion[field_motion_index(field.ball_owner)];
    owner.x = static_cast<std::uint16_t>((owner.x & 255u) | (unsigned(field.ball_x) << 8));
    owner.y = static_cast<std::uint16_t>((owner.y & 255u) | (unsigned(field.ball_y) << 8));
    context.attack_subtype = recovered.attack_subtype;
    if (context.ball_variant == 0)
        context.ball_variant = static_cast<std::uint8_t>((field.random_first & 1u) + 1u);
    state.phase = FieldSessionPhase::recovered_owner;
    return result;
}

PhaseSessionState continue_session_recovery(const PhaseSessionState& post_presentation) {
    validate_phase_session(post_presentation);
    if (post_presentation.phase != FieldSessionPhase::recovered_owner || !post_presentation.recovery)
        throw std::invalid_argument("No recovered-owner handoff to continue");
    auto state = post_presentation;
    const auto& field = state.current.field;
    if (field.ball_y >= 96 && field.ball_y < 160 && (field.ball_x < 80 || field.ball_x >= 176)) {
        state.phase = FieldSessionPhase::keeper_interaction;
        return state;
    }
    const auto& owner = field.motion[field_motion_index(field.ball_owner)];
    if (state.recovery->game_mode == 4 && field.side == 0 &&
        field.roster[field.ball_owner].type != 1 && (owner.x >> 8) >= 128) {
        state.phase = FieldSessionPhase::story_transition;
        return state;
    }
    state.phase = FieldSessionPhase::player_control;
    return state;
}
}

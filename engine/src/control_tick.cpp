#include "captain/control_tick.hpp"
#include "captain/defender_selection.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace captain {
namespace {
std::uint16_t maximum_stamina(const FieldPhaseState& field, std::uint8_t id,
    const FieldPlayerProfiles& profiles, const GoalkeeperProfiles& keepers, const WorldStepContent& content) {
    const auto& stats = field.roster[id];
    const FieldPlayerRecord record{id, stats.type, static_cast<std::uint8_t>(stats.stamina),
        static_cast<std::uint8_t>(stats.stamina >> 8), stats.level};
    const auto& mappings = id == 0 || id == 11
        ? static_cast<const PlayerProfileMappings&>(keepers) : static_cast<const PlayerProfileMappings&>(profiles);
    const auto reference = resolve_player_profile_reference(record, mappings);
    std::uint8_t base;
    if (id == 0 || id == 11) {
        if (reference.profile_id >= keepers.skills.size()) throw std::invalid_argument("Missing keeper stamina profile");
        base = keepers.skills[reference.profile_id][0];
    } else {
        if (reference.profile_id >= profiles.skills.size()) throw std::invalid_argument("Missing field stamina profile");
        base = profiles.skills[reference.profile_id][0];
    }
    const auto index = static_cast<std::uint8_t>(unsigned(base) + stats.level);
    return content.stamina_curve[std::min<unsigned>(index, 95)];
}
ControlTickResult after_clock(PhaseSessionState state, const FieldPlayerProfiles& profiles,
    const GoalkeeperProfiles& keepers, const WorldStepContent& content) {
    auto& field = state.current.field;
    const auto forward_x = field.side == 0 ? field.ball_x : static_cast<std::uint8_t>(0u - field.ball_x);
    const auto& defending_keeper = field.keepers[field.side == 0 ? 1 : 0];
    if (defending_keeper.stun == 0 && forward_x >= 196 && field.ball_y >= 116 && field.ball_y < 140) {
        state.current.clock.ball_mode = 0; state.current.clock.script_flags = 0;
        state.phase = FieldSessionPhase::goal_area_action;
        return {std::move(state)};
    }
    auto& schedule = *state.scheduling;
    ++schedule.field_cycle;
    const auto energy_cycle = static_cast<std::uint8_t>(unsigned(schedule.energy_cycle) + 1u);
    schedule.energy_cycle = 0;
    if (energy_cycle != 0) {
        for (unsigned slot = 0; slot <= 10; ++slot) {
            if (slot == field.ball_owner) continue;
            auto& player = field.roster[slot];
            if (player.type == 32 && player.stamina == 0) continue;
            const auto maximum = maximum_stamina(field, static_cast<std::uint8_t>(slot), profiles, keepers, content);
            const auto candidate = static_cast<std::uint16_t>(unsigned(player.stamina) + (player.type == 32 ? 1u : 2u));
            player.stamina = std::min(candidate, maximum);
        }
    }
    ControlTickResult result{std::move(state)};
    if (result.state.current.field.ball_owner < 11) {
        auto& owner = result.state.current.field.roster[result.state.current.field.ball_owner];
        const auto cost = owner.type == 32 ? 5u : 3u;
        owner.stamina = static_cast<std::uint16_t>(owner.stamina < cost ? 0 : owner.stamina - cost);
        result.presentation.push_back({ControlPresentationKind::display, 48});
    }
    result.state.phase = FieldSessionPhase::field_round_pending;
    validate_phase_session(result.state);
    return result;
}
}

ControlTickResult advance_control_tick(const PhaseSessionState& input,
    const FieldPlayerProfiles& field_profiles, const GoalkeeperProfiles& keepers, const WorldStepContent& content) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::control_clock_pending || !input.scheduling)
        throw std::invalid_argument("Control tick requires clock-pending phase and captured scheduling context");
    auto state = input;
    state.current.field.elapsed = 1; // Original resets to0, then D193 adds1.
    const auto recovered = advance_keeper_recovery({state.current.field.keepers,
        state.current.shared_recovery_flags, state.current.shared_recovery_timer}, 1);
    state.current.field.keepers = recovered.keepers;
    state.current.shared_recovery_flags = recovered.shared_flags;
    state.current.shared_recovery_timer = recovered.shared_timer;
    const auto clock = advance_match_clock(state.current.clock, 1, state.current.field.random_first);
    state.current.clock = clock.state;
    if (clock.continuation != ClockContinuation::ready) {
        state.phase = clock.continuation == ClockContinuation::period_end
            ? FieldSessionPhase::period_end : FieldSessionPhase::control_added_time;
        return {std::move(state), clock};
    }
    auto result = after_clock(std::move(state), field_profiles, keepers, content);
    result.clock = clock;
    return result;
}

ControlTickResult resume_control_added_time(const PhaseSessionState& post_presentation,
    const FieldPlayerProfiles& profiles, const GoalkeeperProfiles& keepers, const WorldStepContent& content) {
    validate_phase_session(post_presentation);
    if (post_presentation.phase != FieldSessionPhase::control_added_time)
        throw std::invalid_argument("No control added-time wait to resume");
    auto state = post_presentation;
    std::vector<ControlPresentationRequest> requests;
    if (state.current.clock.script_flags & 128u) {
        requests = {{ControlPresentationKind::script, 30}, {ControlPresentationKind::field_map, 0}};
        if (state.current.field.side == 0) requests.push_back({ControlPresentationKind::display, 48});
        else {
            requests.push_back({ControlPresentationKind::display, 49});
            requests.push_back({ControlPresentationKind::display, 50});
        }
        state.current.clock.script_flags = 128; state.current.clock.ball_mode = 128;
        state.control->travel_accumulator = 0; state.control->display_parameter = 2;
        state.control->motion_requested = 0;
        state.experience->totals[10] = static_cast<std::uint16_t>((state.experience->totals[10] & 255u) | 256u);
    }
    auto result = after_clock(std::move(state), profiles, keepers, content);
    result.presentation.insert(result.presentation.begin(), requests.begin(), requests.end());
    return result;
}

FieldRoundResult advance_control_field_round(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ControlMovementContent& control_content, const WorldStepContent& world,
    const std::array<LogicalButtons, 22>& inputs) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::field_round_pending)
        throw std::invalid_argument("Field round requires a completed control clock/energy step");
    FieldRoundResult result{input};
    auto& state = result.state;
    const auto region = field_ball_region(state.current.field.ball_x, state.current.field.ball_y);
    auto& old_region = state.current.field.movement_region;
    old_region &= 127u;
    if (region != old_region) old_region = static_cast<std::uint8_t>(region | 128u);
    for (unsigned slot = 0; slot < 22; ++slot) {
        const auto buttons = apply_control_buttons(state, inputs[slot].held, inputs[slot].pressed);
        state = buttons.state;
        ++result.consumed_polls;
        result.presentation.insert(result.presentation.end(), buttons.presentation.begin(), buttons.presentation.end());
        if (buttons.menu_requested) return result;
        auto& field = state.current.field;
        if (slot == 0 || slot == 11 || (field.side != 0 && slot == state.control->controlled_player) ||
            (slot == field.ball_owner && slot < 11)) continue;
        const auto id = static_cast<std::uint8_t>(slot);
        const auto& stats = field.roster[slot];
        const auto profile = load_movement_profile({id, stats.type, static_cast<std::uint8_t>(stats.stamina),
            static_cast<std::uint8_t>(stats.stamina >> 8), stats.level}, field.side, primary, movement.profiles);
        auto& actor = field.motion[field_motion_index(id)];
        if ((field.movement_region & 128u) && slot != field.ball_owner) {
            const auto target = select_field_target({actor, id, field.side, field.ball_x, field.ball_y,
                field.random_first, profile.decision_threshold, field.teams}, movement.targets);
            actor.target_cell = target.target_cell;
            field.random_first = target.random_first;
        }
        const auto speed = slot == field.ball_owner ? control_content.carrier_speeds[profile.speed_profile]
            : !profile.in_possession && actor.target_cell == 240 ? world.chase_speeds[profile.speed_profile] : profile.speed;
        if (actor.status != 0) { --actor.status; ++result.decayed_statuses; continue; }
        actor = advance_field_player({actor, speed, 1, field.ball_cell}, movement.geometry).player;
        ++result.moved_players;
    }
    auto& field = state.current.field;
    field.selected_defenders = 0;
    state.scheduling->selected_ids.fill(0);
    if (state.scheduling->field_cycle >= 5) {
        state.scheduling->field_cycle = 0;
        DefenderSelectionInput selection{field.side, field.ball_x, field.ball_y, 7, {}};
        for (std::size_t i = 0; i < field.motion.size(); ++i) {
            const auto& actor = field.motion[i];
            selection.players[i] = {static_cast<std::uint8_t>(actor.x >> 8),
                static_cast<std::uint8_t>(actor.y >> 8), actor.status != 0};
        }
        const auto selected = select_field_defenders(selection);
        field.selected_defenders = static_cast<std::uint8_t>(selected.size());
        std::copy(selected.begin(), selected.end(), state.scheduling->selected_ids.begin());
    }
    if (field.selected_defenders != 0) {
        state.current.clock.ball_mode = 0; state.current.clock.script_flags = 0;
        state.phase = FieldSessionPhase::field_contact_pending;
    } else if ((state.scheduling->special_flags & 128u) && field.side == 0 && field.ball_x >= 128)
        state.phase = FieldSessionPhase::special_callback;
    else if (state.recovery->game_mode == 4 && field.side == 0 && field.roster[field.ball_owner].type != 1 &&
        (field.motion[field_motion_index(field.ball_owner)].x >> 8) >= 128)
        state.phase = FieldSessionPhase::story_transition;
    else state.phase = FieldSessionPhase::control_ready;
    validate_phase_session(state);
    return result;
}
}

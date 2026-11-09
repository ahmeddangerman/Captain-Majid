#include "captain/control_tick.hpp"
#include "captain/control_travel.hpp"
#include "captain/defender_selection.hpp"
#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace captain {
ControlTickResult resume_area_action4_added_time(const PhaseSessionState& input,
 const FieldPlayerProfiles& profiles,const GoalkeeperProfiles& keepers,const WorldStepContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_added_time)
  throw std::invalid_argument("Area action4 added-time resume needs a saved presentation boundary");
 auto prepared=input;prepared.phase=FieldSessionPhase::control_added_time;
 prepared.area_ai_target.reset();prepared.area_defense.reset();prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
 auto result=resume_control_added_time(prepared,profiles,keepers,content);auto& state=result.state;
 if(state.phase!=FieldSessionPhase::field_round_pending)
  throw std::invalid_argument("Area action4 added-time goal interruption requires original tracing");
 state.area_ai_target=input.area_ai_target;state.area_defense=input.area_defense;
 state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 state.phase=FieldSessionPhase::area_action4_field_round;validate_phase_session(state);return result;
}
FieldRoundResult advance_area_action4_field_round(const PhaseSessionState& input,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement,const ControlMovementContent& control,const WorldStepContent& world,
 const std::array<LogicalButtons,22>& inputs) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_field_round)
  throw std::invalid_argument("Area action4 field round requires a completed control clock");
 auto prepared=input;prepared.phase=FieldSessionPhase::field_round_pending;
 prepared.area_ai_target.reset();prepared.area_defense.reset();prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
 auto result=advance_control_field_round(prepared,primary,movement,control,world,inputs);auto& state=result.state;
 if(state.phase!=FieldSessionPhase::control_ready)
  throw std::invalid_argument("Area action4 field round interruption requires original tracing");
 state.area_ai_target=input.area_ai_target;state.area_defense=input.area_defense;
 state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 state.phase=FieldSessionPhase::area_action4_control_wait;validate_phase_session(state);return result;
}
ControlTickResult advance_area_action4_control_tick(const PhaseSessionState& input,
 const FieldPlayerProfiles& profiles,const GoalkeeperProfiles& keepers,const WorldStepContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_control_clock)
  throw std::invalid_argument("Area action4 control clock requires a saved input decision");
 auto prepared=input;prepared.phase=FieldSessionPhase::control_clock_pending;
 prepared.area_ai_target.reset();prepared.area_defense.reset();prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
 auto result=advance_control_tick(prepared,profiles,keepers,content);auto& state=result.state;
 state.area_ai_target=input.area_ai_target;state.area_defense=input.area_defense;
 state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 switch(state.phase) {
  case FieldSessionPhase::field_round_pending:state.phase=FieldSessionPhase::area_action4_field_round;break;
  case FieldSessionPhase::goal_area_action:state.phase=FieldSessionPhase::area_action4_goal_area;break;
  case FieldSessionPhase::control_added_time:state.phase=FieldSessionPhase::area_action4_added_time;break;
  case FieldSessionPhase::period_end:state.phase=FieldSessionPhase::area_action4_period_end;break;
  default:throw std::invalid_argument("Unsupported area action4 control clock boundary");
 }
 validate_phase_session(state);return result;
}
FieldRoundResult advance_area_pass_field_round(const PhaseSessionState& input,const PlayerProfileMappings& primary,const FieldPhaseContent& movement,
 const ControlMovementContent& control,const WorldStepContent& world,const std::array<LogicalButtons,22>& inputs) {
 validate_phase_session(input);
 const bool blocked=input.phase==FieldSessionPhase::area_pass_blocked_field_round;
 if(!blocked && input.phase!=FieldSessionPhase::area_pass_capture_field_round)
  throw std::invalid_argument("Area pass field round needs a completed control tick");
 auto prepared=input;prepared.phase=FieldSessionPhase::field_round_pending;
 prepared.area_ai_target.reset();prepared.area_contact.reset();prepared.area_pass_contact.reset();
 prepared.shot_flight.reset();prepared.ball_trajectory.reset();prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
 auto result=advance_control_field_round(prepared,primary,movement,control,world,inputs);auto& state=result.state;
 if(state.phase!=FieldSessionPhase::control_ready && state.phase!=FieldSessionPhase::action_menu_pending)
  throw std::invalid_argument("Area pass field round awaits a contact, story or special continuation");
 state.area_ai_target=input.area_ai_target;state.area_contact=input.area_contact;state.area_pass_contact=input.area_pass_contact;
 state.shot_flight=input.shot_flight;state.ball_trajectory=input.ball_trajectory;
 state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 state.ball_trajectory->x=static_cast<std::uint16_t>((unsigned(state.current.field.ball_x)<<8)|(input.ball_trajectory->x&255u));
 state.ball_trajectory->y=static_cast<std::uint16_t>((unsigned(state.current.field.ball_y)<<8)|(input.ball_trajectory->y&255u));
 const bool menu=state.phase==FieldSessionPhase::action_menu_pending;
 state.phase=blocked ? (menu ? FieldSessionPhase::area_pass_blocked_action_menu : FieldSessionPhase::area_pass_blocked_control_wait) :
  (menu ? FieldSessionPhase::area_pass_capture_action_menu : FieldSessionPhase::area_pass_capture_control_wait);
 validate_phase_session(state);return result;
}
ControlTickResult advance_area_pass_control_tick(const PhaseSessionState& input,const FieldPlayerProfiles& profiles,
 const GoalkeeperProfiles& keepers,const WorldStepContent& content) {
 validate_phase_session(input);
 const bool blocked=input.phase==FieldSessionPhase::area_pass_blocked_clock_pending;
 if(!blocked && input.phase!=FieldSessionPhase::area_pass_capture_clock_pending)
  throw std::invalid_argument("Area pass control clock requires a saved input decision");
 auto prepared=input;prepared.phase=FieldSessionPhase::control_clock_pending;
 prepared.area_ai_target.reset();prepared.area_contact.reset();prepared.area_pass_contact.reset();
 prepared.shot_flight.reset();prepared.ball_trajectory.reset();prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
 auto result=advance_control_tick(prepared,profiles,keepers,content);auto& state=result.state;
 state.area_ai_target=input.area_ai_target;state.area_contact=input.area_contact;state.area_pass_contact=input.area_pass_contact;
 state.shot_flight=input.shot_flight;state.ball_trajectory=input.ball_trajectory;
 state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 switch(state.phase) {
  case FieldSessionPhase::field_round_pending: state.phase=blocked ? FieldSessionPhase::area_pass_blocked_field_round : FieldSessionPhase::area_pass_capture_field_round;break;
  case FieldSessionPhase::goal_area_action: state.phase=blocked ? FieldSessionPhase::area_pass_blocked_goal_area : FieldSessionPhase::area_pass_capture_goal_area;break;
  case FieldSessionPhase::control_added_time: state.phase=blocked ? FieldSessionPhase::area_pass_blocked_added_time : FieldSessionPhase::area_pass_capture_added_time;break;
  case FieldSessionPhase::period_end: state.phase=blocked ? FieldSessionPhase::area_pass_blocked_period_end : FieldSessionPhase::area_pass_capture_period_end;break;
  default:throw std::invalid_argument("Unsupported area pass control clock continuation");
 }
 validate_phase_session(state);return result;
}
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
bool begin_after_clock(PhaseSessionState& state) {
    auto& field = state.current.field;
    const auto forward_x = field.side == 0 ? field.ball_x : static_cast<std::uint8_t>(0u - field.ball_x);
    const auto& defending_keeper = field.keepers[field.side == 0 ? 1 : 0];
    if (defending_keeper.stun == 0 && forward_x >= 196 && field.ball_y >= 116 && field.ball_y < 140) {
        if (state.kickoff_formation)
            throw std::invalid_argument("Kickoff keeper positions are not yet supported at goal-area action");
        state.current.clock.ball_mode = 0; state.current.clock.script_flags = 0;
        state.phase = FieldSessionPhase::goal_area_action;
        return false;
    }
    auto& schedule = *state.scheduling;
    ++schedule.field_cycle;
    return true;
}
ControlTickResult finish_after_clock(PhaseSessionState state, const FieldPlayerProfiles& profiles,
    const GoalkeeperProfiles& keepers, const WorldStepContent& content) {
    auto& field = state.current.field;
    auto& schedule = *state.scheduling;
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
ControlTickResult after_clock(PhaseSessionState state, const FieldPlayerProfiles& profiles,
    const GoalkeeperProfiles& keepers, const WorldStepContent& content) {
    if (!begin_after_clock(state)) return {std::move(state)};
    return finish_after_clock(std::move(state), profiles, keepers, content);
}
}

static ControlTickResult begin_control_clock(const PhaseSessionState& input) {
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
        if (state.kickoff_formation)
            throw std::invalid_argument("Kickoff keeper positions are not yet supported at clock presentation");
        state.phase = clock.continuation == ClockContinuation::period_end
            ? FieldSessionPhase::period_end : FieldSessionPhase::control_added_time;
        return {std::move(state), clock};
    }
    return {std::move(state), clock};
}

ControlTickResult advance_control_tick(const PhaseSessionState& input,
    const FieldPlayerProfiles& field_profiles, const GoalkeeperProfiles& keepers, const WorldStepContent& content) {
    auto initial = begin_control_clock(input);
    if (initial.state.phase != FieldSessionPhase::control_clock_pending) return initial;
    auto result = after_clock(std::move(initial.state), field_profiles, keepers, content);
    result.clock = initial.clock;
    return result;
}

NmiClockResult advance_observed_nmi_control_clock(const PhaseSessionState& input,
    const FieldPlayerProfiles& field_profiles, const GoalkeeperProfiles& keepers, const WorldStepContent& content,
    std::uint8_t initial_random_index, const std::vector<ObservedClockEvent>& events) {
    if (input.current.clock.remaining <= 30)
        throw std::invalid_argument("Observed clock events do not cover warning, added time or period end");
    auto initial = begin_control_clock(input);
    if (initial.state.phase != FieldSessionPhase::control_clock_pending)
        throw std::invalid_argument("Observed clock events require a normal clock continuation");
    auto state = std::move(initial.state);
    auto index = initial_random_index;
    unsigned stage = 0;
    ControlTickResult final{};
    for (const auto& event : events) {
        switch (event.kind) {
        case ObservedClockEventKind::random: {
            const auto next = advance_reference_random(
                {index, state.current.field.random_first, state.current.field.random_second}, event.random);
            index = next.state.index;
            state.current.field.random_first = next.state.first;
            state.current.field.random_second = next.state.second;
            break;
        }
        case ObservedClockEventKind::travel:
            state.control->travel_accumulator = advance_control_travel(state.control->travel_accumulator,
                event.speed, state.current.field.side, event.held, state.control->facing);
            break;
        case ObservedClockEventKind::schedule:
            if (stage != 0 || !begin_after_clock(state))
                throw std::invalid_argument("Unsupported or out-of-order observed clock schedule");
            stage = 1;
            break;
        case ObservedClockEventKind::energy:
            if (stage != 1) throw std::invalid_argument("Out-of-order observed clock energy");
            final = finish_after_clock(std::move(state), field_profiles, keepers, content);
            state = final.state;
            stage = 2;
            break;
        default:
            throw std::invalid_argument("Unknown observed clock event");
        }
    }
    if (stage != 2) throw std::invalid_argument("Observed clock events require schedule and energy boundaries");
    final.state = std::move(state);
    final.clock = initial.clock;
    validate_phase_session(final.state);
    return {std::move(final), index};
}

static std::vector<ObservedClockEvent> read_observed_events(std::istream& input, const char* magic, bool milestones) {
    std::string token;
    if (!(input >> token) || token != magic)
        throw std::invalid_argument("Invalid observed event header");
    std::vector<ObservedClockEvent> events;
    while (input >> token) {
        if (token == "R") {
            unsigned player, sound, tail, carry;
            if (!(input >> player >> sound >> tail >> carry) || player > 255 || sound > 255 || tail > 255 || carry > 1)
                throw std::invalid_argument("Invalid observed clock random event");
            events.push_back({ObservedClockEventKind::random,
                {static_cast<std::uint8_t>(player), static_cast<std::uint8_t>(sound),
                 static_cast<std::uint8_t>(tail), carry != 0}});
        } else if (token == "T") {
            unsigned held, low, high;
            if (!(input >> held >> low >> high) || held > 255 || low > 255 || high > 255)
                throw std::invalid_argument("Invalid observed clock travel event");
            events.push_back({ObservedClockEventKind::travel, {}, static_cast<std::uint8_t>(held),
                              static_cast<std::uint16_t>(low | (high << 8))});
        } else if (token == "S" && milestones) events.push_back({ObservedClockEventKind::schedule});
        else if (token == "E" && milestones) events.push_back({ObservedClockEventKind::energy});
        else throw std::invalid_argument("Unknown observed clock event");
    }
    if (input.bad() || !input.eof()) throw std::invalid_argument("Unreadable observed clock events");
    return events;
}
std::vector<ObservedClockEvent> load_observed_clock_events(std::istream& input) {
    return read_observed_events(input, "CMJCLOCKNMI1", true);
}
std::vector<ObservedClockEvent> load_observed_nmi_events(std::istream& input) {
    return read_observed_events(input, "CMJNMISEQ1", false);
}
NmiEventSequenceResult advance_observed_nmi_events(const PhaseSessionState& input,
    std::uint8_t initial_random_index, const std::vector<ObservedClockEvent>& events) {
    validate_phase_session(input);
    if ((input.phase != FieldSessionPhase::field_contact_pending && input.phase != FieldSessionPhase::human_action_menu) || !input.control)
        throw std::invalid_argument("Observed NMI sequence requires a field contact or human menu boundary");
    auto state = input;
    auto index = initial_random_index;
    for (const auto& event : events) {
        if (event.kind == ObservedClockEventKind::random) {
            const auto next = advance_reference_random(
                {index, state.current.field.random_first, state.current.field.random_second}, event.random);
            index = next.state.index;
            state.current.field.random_first = next.state.first;
            state.current.field.random_second = next.state.second;
        } else if (event.kind == ObservedClockEventKind::travel) {
            state.control->travel_accumulator = advance_control_travel(state.control->travel_accumulator,
                event.speed, state.current.field.side, event.held, state.control->facing);
        } else throw std::invalid_argument("Observed NMI sequence cannot contain clock milestones");
    }
    validate_phase_session(state);
    return {std::move(state), index};
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

static FieldRoundResult run_control_field_round(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ControlMovementContent& control_content, const WorldStepContent& world,
    const std::array<LogicalButtons, 22>& inputs,
    const std::array<ObservedFieldPoll, 22>* observed,
    const std::array<ObservedNmiFieldPoll, 22>* nmi, std::uint8_t* random_index) {
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
        if (nmi) {
            const auto& sample = (*nmi)[slot];
            const auto result = advance_reference_random(
                {*random_index, state.current.field.random_first, state.current.field.random_second}, sample.random);
            *random_index = result.state.index;
            state.current.field.random_first = result.state.first;
            state.current.field.random_second = result.state.second;
            if (sample.travel_observed) {
                state.control->travel_accumulator = advance_control_travel(
                    state.control->travel_accumulator, sample.travel_speed, state.current.field.side,
                    sample.travel_held, state.control->facing);
            }
        }
        if (observed) {
            const auto& sample = (*observed)[slot];
            state.current.field.random_first = sample.random_first;
            state.current.field.random_second = sample.random_second;
            if (sample.travel_observed) {
                state.control->travel_accumulator = advance_control_travel(
                    state.control->travel_accumulator, sample.travel_speed, state.current.field.side,
                    sample.travel_held, state.control->facing);
            }
        }
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

FieldRoundResult advance_control_field_round(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ControlMovementContent& control, const WorldStepContent& world,
    const std::array<LogicalButtons, 22>& inputs) {
    return run_control_field_round(input, primary, movement, control, world, inputs, nullptr, nullptr, nullptr);
}

std::array<ObservedFieldPoll, 22> load_observed_field_polls(std::istream& input) {
    std::string magic;
    if (!(input >> magic) || magic != "CMJWORLDOBS1")
        throw std::invalid_argument("Invalid observed world file header");
    std::array<ObservedFieldPoll, 22> samples{};
    for (auto& sample : samples) {
        unsigned held, pressed, first, second, present, travel_held, low, high;
        if (!(input >> held >> pressed >> first >> second >> present >> travel_held >> low >> high) ||
            held > 255 || pressed > 255 || first > 255 || second > 255 || present > 1 ||
            travel_held > 255 || low > 255 || high > 255 || (!present && (travel_held || low || high)))
            throw std::invalid_argument("Observed world file requires 22 valid byte records");
        sample = {{static_cast<std::uint8_t>(held), static_cast<std::uint8_t>(pressed)},
                  static_cast<std::uint8_t>(first), static_cast<std::uint8_t>(second), present != 0,
                  static_cast<std::uint8_t>(travel_held), static_cast<std::uint16_t>(low | (high << 8))};
    }
    input >> std::ws;
    if (input.peek() != std::char_traits<char>::eof() || input.bad())
        throw std::invalid_argument("Trailing or unreadable observed world data");
    return samples;
}

FieldRoundResult advance_observed_control_field_round(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ControlMovementContent& control, const WorldStepContent& world,
    const std::array<ObservedFieldPoll, 22>& inputs) {
    std::array<LogicalButtons, 22> buttons{};
    for (std::size_t i = 0; i < inputs.size(); ++i) {
        if (!inputs[i].travel_observed && (inputs[i].travel_held || inputs[i].travel_speed))
            throw std::invalid_argument("Unobserved travel event must have zero inputs");
        buttons[i] = inputs[i].buttons;
    }
    return run_control_field_round(input, primary, movement, control, world, buttons, &inputs, nullptr, nullptr);
}

static ObservedNmiFieldPoll read_nmi_poll(std::istream& input) {
        unsigned held, pressed, player, sound, tail, carry, present, travel_held, low, high;
        if (!(input >> held >> pressed >> player >> sound >> tail >> carry >> present >> travel_held >> low >> high) ||
            held > 255 || pressed > 255 || player > 255 || sound > 255 || tail > 255 || carry > 1 ||
            present > 1 || travel_held > 255 || low > 255 || high > 255 || (!present && (travel_held || low || high)))
            throw std::invalid_argument("Observed NMI input requires valid records");
        return {{static_cast<std::uint8_t>(held), static_cast<std::uint8_t>(pressed)},
                  {static_cast<std::uint8_t>(player), static_cast<std::uint8_t>(sound), static_cast<std::uint8_t>(tail), carry != 0},
                  present != 0, static_cast<std::uint8_t>(travel_held), static_cast<std::uint16_t>(low | (high << 8))};
}
static void finish_nmi_input(std::istream& input) {
    input >> std::ws;
    if (input.peek() != std::char_traits<char>::eof() || input.bad())
        throw std::invalid_argument("Trailing or unreadable observed NMI world data");
}
ObservedNmiFieldPoll load_observed_nmi_poll(std::istream& input) {
    std::string magic;
    if (!(input >> magic) || magic != "CMJPOLLNMI1")
        throw std::invalid_argument("Invalid observed NMI poll file header");
    auto sample = read_nmi_poll(input);
    finish_nmi_input(input);
    return sample;
}
std::array<ObservedNmiFieldPoll, 22> load_observed_nmi_field_polls(std::istream& input) {
    std::string magic;
    if (!(input >> magic) || magic != "CMJWORLDNMI1")
        throw std::invalid_argument("Invalid observed NMI world file header");
    std::array<ObservedNmiFieldPoll, 22> samples{};
    for (auto& sample : samples) sample = read_nmi_poll(input);
    finish_nmi_input(input);
    return samples;
}

NmiControlPollResult advance_observed_nmi_control_poll(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ControlMovementContent& control, std::uint8_t initial_random_index,
    const ObservedNmiFieldPoll& sample) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::control_ready || !input.control)
        throw std::invalid_argument("Observed NMI poll requires ready control");
    if (!sample.travel_observed && (sample.travel_held || sample.travel_speed))
        throw std::invalid_argument("Unobserved travel event must have zero inputs");
    auto state = input;
    const auto random = advance_reference_random(
        {initial_random_index, state.current.field.random_first, state.current.field.random_second}, sample.random);
    state.current.field.random_first = random.state.first;
    state.current.field.random_second = random.state.second;
    if (sample.travel_observed)
        state = advance_session_control_travel(state, sample.travel_speed, sample.travel_held);
    auto poll = poll_session_control(state, sample.buttons.held, sample.buttons.pressed, primary, movement, control);
    return {std::move(poll), random.state.index};
}

NmiFieldRoundResult advance_observed_nmi_field_round(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    const ControlMovementContent& control, const WorldStepContent& world,
    std::uint8_t initial_random_index, const std::array<ObservedNmiFieldPoll, 22>& inputs) {
    std::array<LogicalButtons, 22> buttons{};
    for (std::size_t i = 0; i < inputs.size(); ++i) {
        if (!inputs[i].travel_observed && (inputs[i].travel_held || inputs[i].travel_speed))
            throw std::invalid_argument("Unobserved travel event must have zero inputs");
        buttons[i] = inputs[i].buttons;
    }
    auto index = initial_random_index;
    auto result = run_control_field_round(input, primary, movement, control, world, buttons, nullptr, &inputs, &index);
    return {std::move(result), index};
}
}

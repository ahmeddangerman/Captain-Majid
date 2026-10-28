#include "captain/phase_session_io.hpp"
#include "captain/field_state_io.hpp"
#include <array>
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace captain {
namespace {
constexpr char magic[] = "CMJSESSION";
std::uint8_t byte(std::istream& input) {
    const auto value = input.get();
    if (value == std::char_traits<char>::eof()) throw std::invalid_argument("Truncated phase session");
    return static_cast<std::uint8_t>(value);
}
void byte(std::ostream& output, std::uint8_t value) { output.put(static_cast<char>(value)); }
}

PhaseSessionState load_phase_session(std::istream& input) {
    for (std::size_t i = 0; i < sizeof(magic) - 1; ++i)
        if (byte(input) != static_cast<std::uint8_t>(magic[i]))
            throw std::invalid_argument("Unsupported phase session format/version");
    const auto version = byte(input);
    if ((version < '1' || version > '9') && version != 'A')
        throw std::invalid_argument("Unsupported phase session format/version");
    std::array<char, 239> field_bytes{};
    input.read(field_bytes.data(), field_bytes.size());
    if (input.gcount() != static_cast<std::streamsize>(field_bytes.size()))
        throw std::invalid_argument("Truncated phase session field section");
    std::istringstream field_input(std::string(field_bytes.data(), field_bytes.size()), std::ios::in | std::ios::binary);
    PhaseSessionState state{};
    state.current.field = load_field_state(field_input);
    const auto low = byte(input);
    state.current.clock.remaining = static_cast<std::uint16_t>(unsigned(low) | (unsigned(byte(input)) << 8));
    state.current.clock.extra_time = byte(input); state.current.clock.flags = byte(input);
    state.current.clock.script_flags = byte(input); state.current.clock.ball_mode = byte(input);
    state.current.shared_recovery_flags = byte(input); state.current.shared_recovery_timer = byte(input);
    state.phase = static_cast<FieldSessionPhase>(byte(input));
    if (version < '8' && static_cast<unsigned>(state.phase) > 20)
        throw std::invalid_argument("New shot phase requires session version8");
    if (version < '9' && static_cast<unsigned>(state.phase) > 22)
        throw std::invalid_argument("Staged shot phase requires session version9");
    if (version != 'A' && static_cast<unsigned>(state.phase) > 27)
        throw std::invalid_argument("Selected handoff phase requires session version10");
    const auto pending = byte(input);
    const PendingTerminalContact contact{byte(input), byte(input), byte(input), byte(input)};
    if (pending > 1) throw std::invalid_argument("Unsupported pending-contact flag");
    if (pending) state.pending = contact;
    else if (contact.result_code || contact.defender_id || contact.defense_action || contact.defense_subtype)
        throw std::invalid_argument("Nonzero unused pending-contact data");
    if (version >= '2') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported experience presence flag");
        ExperienceState experience{};
        experience.campaign = byte(input); experience.opponent_stage = byte(input);
        bool unused = experience.campaign || experience.opponent_stage;
        for (auto& total : experience.totals) {
            const auto lo = byte(input);
            total = static_cast<std::uint16_t>(unsigned(lo) | (unsigned(byte(input)) << 8));
            unused = unused || total != 0;
        }
        if (present) state.experience = experience;
        else if (unused) throw std::invalid_argument("Nonzero unused experience data");
    }
    if (version >= '3') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported recovery presence flag");
        const RecoveryContext recovery{byte(input), byte(input), byte(input), byte(input), byte(input), byte(input)};
        if (present) state.recovery = recovery;
        else if (recovery.opposing_player || recovery.attack_subtype || recovery.special_owner_flags ||
                 recovery.ball_variant || recovery.game_mode || recovery.x_jitter_control)
            throw std::invalid_argument("Nonzero unused recovery data");
    }
    if (version >= '4') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported control presence flag");
        ControlContext control{};
        control.decision_timer = byte(input); control.facing = byte(input);
        control.controlled_player = byte(input); control.last_carrier_cell = byte(input);
        const auto low = byte(input);
        control.travel_accumulator = static_cast<std::uint16_t>(unsigned(low) | (unsigned(byte(input)) << 8));
        control.display_parameter = byte(input);
        if (version >= '5') control.motion_requested = byte(input);
        if (present) state.control = control;
        else if (control.decision_timer || control.facing || control.controlled_player ||
                 control.last_carrier_cell || control.travel_accumulator || control.display_parameter || control.motion_requested)
            throw std::invalid_argument("Nonzero unused control data");
    }
    if (version >= '6') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported scheduling presence flag");
        SchedulingContext schedule{};
        schedule.field_cycle = byte(input); schedule.energy_cycle = byte(input); schedule.special_flags = byte(input);
        for (auto& id : schedule.selected_ids) id = byte(input);
        if (present) state.scheduling = schedule;
        else if (schedule.field_cycle || schedule.energy_cycle || schedule.special_flags ||
                 std::any_of(schedule.selected_ids.begin(), schedule.selected_ids.end(), [](auto id) { return id != 0; }))
            throw std::invalid_argument("Nonzero unused scheduling data");
    }
    if (version >= '7') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported action-context presence flag");
        ActionContext action{};
        action.action = byte(input); action.menu_route = byte(input); action.cursor = byte(input);
        action.flash_counter = byte(input); action.auxiliary = byte(input);
        action.move_effect = byte(input); action.move_flags = byte(input);
        const auto power_low = byte(input);
        action.power = static_cast<std::uint16_t>(unsigned(power_low) | (unsigned(byte(input)) << 8));
        if (present) state.action = action;
        else if (action.action || action.menu_route || action.cursor || action.flash_counter ||
                 action.auxiliary || action.move_effect || action.move_flags || action.power)
            throw std::invalid_argument("Nonzero unused action data");
    }
    if (version >= '9') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported shot progress presence flag");
        const ShotContactProgress progress{byte(input),byte(input),byte(input),byte(input),byte(input)};
        if (present) state.shot_progress = progress;
        else if (progress.cursor || progress.result_code || progress.defender_id ||
                 progress.defense_action || progress.defense_subtype)
            throw std::invalid_argument("Nonzero unused shot progress");
    }
    if (input.peek() != std::char_traits<char>::eof())
        throw std::invalid_argument("Trailing phase session data");
    if (input.bad()) throw std::invalid_argument("Unable to read phase session");
    validate_phase_session(state);
    return state;
}

void write_phase_session(std::ostream& output, const PhaseSessionState& state) {
    validate_phase_session(state);
    output.write(magic, sizeof(magic) - 1);
    byte(output, 'A');
    write_field_state(output, state.current.field);
    byte(output, static_cast<std::uint8_t>(state.current.clock.remaining));
    byte(output, static_cast<std::uint8_t>(state.current.clock.remaining >> 8));
    for (const auto value : {state.current.clock.extra_time, state.current.clock.flags,
         state.current.clock.script_flags, state.current.clock.ball_mode,
         state.current.shared_recovery_flags, state.current.shared_recovery_timer,
         static_cast<std::uint8_t>(state.phase), static_cast<std::uint8_t>(state.pending ? 1 : 0)})
        byte(output, value);
    const auto contact = state.pending.value_or(PendingTerminalContact{0, 0, 0, 0});
    for (const auto value : {contact.result_code, contact.defender_id, contact.defense_action, contact.defense_subtype})
        byte(output, value);
    byte(output, state.experience ? 1 : 0);
    const auto experience = state.experience.value_or(ExperienceState{});
    byte(output, experience.campaign); byte(output, experience.opponent_stage);
    for (const auto total : experience.totals) {
        byte(output, static_cast<std::uint8_t>(total));
        byte(output, static_cast<std::uint8_t>(total >> 8));
    }
    byte(output, state.recovery ? 1 : 0);
    const auto recovery = state.recovery.value_or(RecoveryContext{});
    for (const auto value : {recovery.opposing_player, recovery.attack_subtype, recovery.special_owner_flags,
         recovery.ball_variant, recovery.game_mode, recovery.x_jitter_control}) byte(output, value);
    byte(output, state.control ? 1 : 0);
    const auto control = state.control.value_or(ControlContext{});
    for (const auto value : {control.decision_timer, control.facing, control.controlled_player,
         control.last_carrier_cell, static_cast<std::uint8_t>(control.travel_accumulator),
         static_cast<std::uint8_t>(control.travel_accumulator >> 8), control.display_parameter,
         control.motion_requested}) byte(output, value);
    byte(output, state.scheduling ? 1 : 0);
    const auto schedule = state.scheduling.value_or(SchedulingContext{});
    byte(output, schedule.field_cycle); byte(output, schedule.energy_cycle); byte(output, schedule.special_flags);
    for (const auto id : schedule.selected_ids) byte(output, id);
    byte(output, state.action ? 1 : 0);
    const auto action = state.action.value_or(ActionContext{});
    for (const auto value : {action.action, action.menu_route, action.cursor, action.flash_counter,
         action.auxiliary, action.move_effect, action.move_flags}) byte(output, value);
    byte(output, static_cast<std::uint8_t>(action.power));
    byte(output, static_cast<std::uint8_t>(action.power >> 8));
    byte(output, state.shot_progress ? 1 : 0);
    const auto progress = state.shot_progress.value_or(ShotContactProgress{});
    for (const auto value : {progress.cursor,progress.result_code,progress.defender_id,
                            progress.defense_action,progress.defense_subtype}) byte(output,value);
    if (!output) throw std::invalid_argument("Unable to write phase session");
}
}

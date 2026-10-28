#include "captain/phase_session_io.hpp"
#include "captain/field_state_io.hpp"
#include <array>
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
    if (version != '1' && version != '2' && version != '3')
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
    if (input.peek() != std::char_traits<char>::eof())
        throw std::invalid_argument("Trailing phase session data");
    if (input.bad()) throw std::invalid_argument("Unable to read phase session");
    validate_phase_session(state);
    return state;
}

void write_phase_session(std::ostream& output, const PhaseSessionState& state) {
    validate_phase_session(state);
    output.write(magic, sizeof(magic) - 1);
    byte(output, '3');
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
    if (!output) throw std::invalid_argument("Unable to write phase session");
}
}

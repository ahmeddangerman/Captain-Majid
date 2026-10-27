#include "captain/field_state_io.hpp"
#include <stdexcept>
#include <string>

namespace captain {
namespace {
constexpr char magic[] = "CMJFIELD1";
void validate(const FieldPhaseState& state) {
    if ((state.side != 0 && state.side != 11) || state.ball_owner > 21)
        throw std::invalid_argument("Unsupported field snapshot side/owner");
    for (const auto& team : state.teams)
        if (team.formation >= 4 || team.defense_tactic >= 3)
            throw std::invalid_argument("Unsupported field snapshot formation/tactic");
}
std::uint8_t read_byte(std::istream& input) {
    const auto value = input.get();
    if (value == std::char_traits<char>::eof())
        throw std::invalid_argument("Truncated field snapshot");
    return static_cast<std::uint8_t>(value);
}
std::uint16_t read_word(std::istream& input) {
    const auto low = read_byte(input);
    return static_cast<std::uint16_t>(unsigned(low) | (unsigned(read_byte(input)) << 8));
}
void byte(std::ostream& output, std::uint8_t value) { output.put(static_cast<char>(value)); }
void word(std::ostream& output, std::uint16_t value) {
    byte(output, static_cast<std::uint8_t>(value));
    byte(output, static_cast<std::uint8_t>(value >> 8));
}
}

FieldPhaseState load_field_state(std::istream& input) {
    for (std::size_t i = 0; i < sizeof(magic) - 1; ++i)
        if (read_byte(input) != static_cast<std::uint8_t>(magic[i]))
            throw std::invalid_argument("Unsupported field snapshot format/version");
    FieldPhaseState state{};
    state.side = read_byte(input); state.ball_owner = read_byte(input);
    state.ball_x = read_byte(input); state.ball_y = read_byte(input); state.ball_cell = read_byte(input);
    state.elapsed = read_byte(input); state.random_first = read_byte(input); state.random_second = read_byte(input);
    state.selected_defenders = read_byte(input); state.movement_region = read_byte(input);
    for (auto& team : state.teams) {
        team.formation = read_byte(input); team.defense_tactic = read_byte(input);
    }
    for (auto& player : state.roster) {
        player.type = read_byte(input); player.level = read_byte(input); player.stamina = read_word(input);
    }
    for (auto& player : state.motion) {
        player.x = read_word(input); player.y = read_word(input);
        player.target_cell = read_byte(input); player.status = read_byte(input);
    }
    for (auto& keeper : state.keepers) {
        keeper.timer = read_byte(input); keeper.penalty_duration = read_byte(input);
        keeper.penalty = read_byte(input); keeper.stun = read_byte(input);
    }
    if (input.peek() != std::char_traits<char>::eof())
        throw std::invalid_argument("Trailing data in field snapshot");
    if (input.bad()) throw std::invalid_argument("Unable to read field snapshot");
    validate(state);
    return state;
}

void write_field_state(std::ostream& output, const FieldPhaseState& state) {
    validate(state);
    output.write(magic, sizeof(magic) - 1);
    for (const auto value : {state.side, state.ball_owner, state.ball_x, state.ball_y, state.ball_cell,
         state.elapsed, state.random_first, state.random_second, state.selected_defenders, state.movement_region})
        byte(output, value);
    for (const auto& team : state.teams) { byte(output, team.formation); byte(output, team.defense_tactic); }
    for (const auto& player : state.roster) {
        byte(output, player.type); byte(output, player.level); word(output, player.stamina);
    }
    for (const auto& player : state.motion) {
        word(output, player.x); word(output, player.y);
        byte(output, player.target_cell); byte(output, player.status);
    }
    for (const auto& keeper : state.keepers) {
        byte(output, keeper.timer); byte(output, keeper.penalty_duration);
        byte(output, keeper.penalty); byte(output, keeper.stun);
    }
    if (!output) throw std::invalid_argument("Unable to write field snapshot");
}
}

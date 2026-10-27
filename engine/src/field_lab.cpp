#include "captain/shot_content.hpp"
#include "captain/field_state_io.hpp"
#include "captain/terminal_flight_phase.hpp"
#include <charconv>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
unsigned number(const std::string& token, unsigned maximum = 255) {
    unsigned value = 0;
    const auto parsed = std::from_chars(token.data(), token.data() + token.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != token.data() + token.size() || value > maximum)
        throw std::invalid_argument("Number outside supported range");
    return value;
}
void show(const captain::FieldPhaseState& state) {
    std::cout << "Field phase: side " << unsigned(state.side) << " | owner " << unsigned(state.ball_owner)
              << " | ball " << unsigned(state.ball_x) << ',' << unsigned(state.ball_y)
              << " | cell " << unsigned(state.ball_cell) << " | elapsed " << unsigned(state.elapsed)
              << " | random " << unsigned(state.random_first) << ',' << unsigned(state.random_second)
              << " | selected " << unsigned(state.selected_defenders)
              << " | region " << unsigned(state.movement_region) << '\n';
    for (unsigned slot = 1; slot <= 21; ++slot) {
        if (slot == 11) continue;
        const auto& motion = state.motion[captain::field_motion_index(static_cast<std::uint8_t>(slot))];
        std::cout << "Player " << slot << " | xy " << motion.x << ',' << motion.y
                  << " | target " << unsigned(motion.target_cell) << " | status " << unsigned(motion.status)
                  << " | stamina " << state.roster[slot].stamina << '\n';
    }
}
void save(const std::string& path, const captain::FieldPhaseState& state) {
    if (std::filesystem::path(path).extension() != ".cmjstate")
        throw std::invalid_argument("Use a private .cmjstate output path");
    std::ostringstream output(std::ios::out | std::ios::binary);
    captain::write_field_state(output, state);
    const auto bytes = output.str();
    auto* file = std::fopen(path.c_str(), "wbx");
    if (!file) throw std::invalid_argument("Unable to create new field snapshot; existing files are refused");
    const auto written = std::fwrite(bytes.data(), 1, bytes.size(), file);
    const auto closed = std::fclose(file);
    if (written != bytes.size() || closed != 0)
        throw std::invalid_argument("Unable to finish field snapshot write");
    std::cout << "Saved " << bytes.size() << " bytes to " << path << '\n';
}
void advance(captain::FieldPhaseState& state, const captain::ShotContent& content,
             std::optional<unsigned> elapsed = std::nullopt) {
    auto input = state;
    if (elapsed) input.elapsed = static_cast<std::uint8_t>(*elapsed);
    const auto result = captain::advance_field_phase(input, content.field_players, *content.movement);
    state = result.state;
    std::cout << "Advanced " << result.players.size() << " field players; elapsed consumed.\n";
    for (const auto& player : result.players)
        std::cout << "Update " << unsigned(player.player_id) << " | speed " << player.profile.speed
                  << " | target " << unsigned(player.target.target_cell)
                  << " | direction " << unsigned(player.movement.direction)
                  << " | iterations " << player.movement.iterations << '\n';
}
void terminal(captain::FieldPhaseState& state, const captain::ShotContent& content,
              unsigned code, unsigned defender, unsigned action, unsigned subtype,
              std::optional<unsigned> elapsed = std::nullopt) {
    auto input = state;
    if (elapsed) input.elapsed = static_cast<std::uint8_t>(*elapsed);
    const auto result = captain::resolve_prepared_terminal_flight({input,
        static_cast<std::uint8_t>(code), static_cast<std::uint8_t>(defender),
        static_cast<std::uint8_t>(action), static_cast<std::uint8_t>(subtype)},
        content.field_players, *content.movement);
    state = result.state;
    std::cout << "Terminal handoff: "
              << (result.next_phase == captain::TerminalFlightNextPhase::player_control
                    ? "player control" : "ball recovery")
              << " | advanced players " << (result.movement ? result.movement->players.size() : 0) << '\n';
}
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: captain-field-lab PACK STATE [--advance | --terminal 2|3 --defender ID] "
                     "[--defense-action 0|2] [--defense-subtype 0|128] [--elapsed N] "
                     "[--capture NEW.cmjstate] [--interactive]\n";
        return 2;
    }
    try {
        bool run_advance = false, interactive = false;
        std::optional<unsigned> elapsed;
        std::optional<unsigned> terminal_code, defender;
        unsigned defense_action = 0, defense_subtype = 0;
        bool defense_options = false;
        std::string capture;
        for (int i = 3; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--advance") run_advance = true;
            else if (option == "--interactive") interactive = true;
            else if (option == "--elapsed" || option == "--capture" || option == "--terminal" ||
                     option == "--defender" || option == "--defense-action" || option == "--defense-subtype") {
                if (i + 1 >= argc) throw std::invalid_argument("Missing option value");
                if (option == "--elapsed") elapsed = number(argv[++i]);
                else if (option == "--capture") capture = argv[++i];
                else if (option == "--terminal") terminal_code = number(argv[++i]);
                else if (option == "--defender") defender = number(argv[++i]);
                else if (option == "--defense-action") { defense_action = number(argv[++i]); defense_options = true; }
                else { defense_subtype = number(argv[++i]); defense_options = true; }
            } else throw std::invalid_argument("Unknown field laboratory option");
        }
        if (run_advance && terminal_code)
            throw std::invalid_argument("Choose advance or terminal for one initial operation");
        if (bool(terminal_code) != bool(defender) || (defense_options && !terminal_code))
            throw std::invalid_argument("Terminal operation requires both --terminal and --defender");
        if (elapsed && !run_advance && !terminal_code)
            throw std::invalid_argument("--elapsed requires an operation");
        std::ifstream pack(argv[1], std::ios::binary);
        if (!pack) throw std::invalid_argument("Unable to open private content pack");
        const auto content = captain::load_shot_content(pack);
        if (!content.movement) throw std::invalid_argument("Field laboratory requires version4 movement content");
        std::ifstream snapshot(argv[2], std::ios::binary);
        if (!snapshot) throw std::invalid_argument("Unable to open private field snapshot");
        auto state = captain::load_field_state(snapshot);
        std::cout << "Captain Majid native field-phase laboratory\n"
                     "State persists between commands. Clock/presentation and complete match phases remain separate.\n";
        if (run_advance) advance(state, content, elapsed);
        if (terminal_code) terminal(state, content, *terminal_code, *defender,
                                     defense_action, defense_subtype, elapsed);
        show(state);
        if (!capture.empty()) save(capture, state);
        if (!interactive) return 0;
        std::cout << "Commands: state, advance N, terminal CODE DEFENDER ACTION SUBTYPE, save NEW.cmjstate, quit\n";
        std::string line;
        while (std::cout << "> " && std::getline(std::cin, line)) {
            try {
                std::istringstream input(line);
                std::string command, value, extra;
                if (!(input >> command)) continue;
                if (command == "state" || command == "quit") {
                    if (input >> extra) throw std::invalid_argument("Command takes no arguments");
                    if (command == "quit") break;
                    show(state);
                } else if (command == "advance") {
                    if (!(input >> value) || (input >> extra)) throw std::invalid_argument("Expected advance N");
                    advance(state, content, number(value));
                    show(state);
                } else if (command == "save") {
                    if (!(input >> value) || (input >> extra)) throw std::invalid_argument("Expected save NEW.cmjstate");
                    save(value, state);
                } else if (command == "terminal") {
                    std::string id, action, subtype;
                    if (!(input >> value >> id >> action >> subtype) || (input >> extra))
                        throw std::invalid_argument("Expected terminal CODE DEFENDER ACTION SUBTYPE");
                    terminal(state, content, number(value), number(id), number(action), number(subtype));
                    show(state);
                } else throw std::invalid_argument("Unknown field laboratory command");
            } catch (const std::exception& error) { std::cout << "Error: " << error.what() << '\n'; }
        }
        return std::cin.bad() ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << "Field laboratory stopped: " << error.what() << '\n';
        return 1;
    }
}

#include "captain/phase_session_io.hpp"
#include "captain/shot_content.hpp"
#include <charconv>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
unsigned number(const std::string& token) {
    unsigned value = 0;
    const auto parsed = std::from_chars(token.data(), token.data() + token.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != token.data() + token.size() || value > 255)
        throw std::invalid_argument("Expected a byte from0 through255");
    return value;
}
const char* phase_name(captain::FieldSessionPhase phase) {
    switch (phase) {
    case captain::FieldSessionPhase::field: return "field";
    case captain::FieldSessionPhase::player_control: return "player control";
    case captain::FieldSessionPhase::ball_recovery: return "ball recovery";
    case captain::FieldSessionPhase::added_time: return "added-time presentation pending";
    case captain::FieldSessionPhase::period_end: return "period transition pending";
    }
    return "invalid";
}
void show(const captain::PhaseSessionState& state) {
    const auto& clock = state.current.clock;
    const auto& field = state.current.field;
    std::cout << "Session: " << phase_name(state.phase) << " | clock " << clock.remaining
              << " | extra " << unsigned(clock.extra_time) << " | flags " << unsigned(clock.flags)
              << " | script " << unsigned(clock.script_flags) << " | ball mode " << unsigned(clock.ball_mode)
              << " | shared recovery " << unsigned(state.current.shared_recovery_flags)
              << ',' << unsigned(state.current.shared_recovery_timer) << '\n';
    std::cout << "Field: side " << unsigned(field.side) << " | owner " << unsigned(field.ball_owner)
              << " | ball " << unsigned(field.ball_x) << ',' << unsigned(field.ball_y)
              << " | cell " << unsigned(field.ball_cell) << " | elapsed " << unsigned(field.elapsed)
              << " | selected " << unsigned(field.selected_defenders)
              << " | random " << unsigned(field.random_first) << ',' << unsigned(field.random_second) << '\n';
    if (state.pending) std::cout << "Pending: " << unsigned(state.pending->result_code) << ' '
        << unsigned(state.pending->defender_id) << ' ' << unsigned(state.pending->defense_action)
        << ' ' << unsigned(state.pending->defense_subtype) << '\n';
    for (unsigned slot = 1; slot <= 21; ++slot) {
        if (slot == 11) continue;
        const auto& actor = field.motion[captain::field_motion_index(static_cast<std::uint8_t>(slot))];
        std::cout << "Player " << slot << " | xy " << actor.x << ',' << actor.y
                  << " | target " << unsigned(actor.target_cell) << " | status " << unsigned(actor.status) << '\n';
    }
}
void save(const std::string& path, const captain::PhaseSessionState& state) {
    if (std::filesystem::path(path).extension() != ".cmjsession")
        throw std::invalid_argument("Use a private .cmjsession output path");
    std::ostringstream output(std::ios::out | std::ios::binary);
    captain::write_phase_session(output, state);
    const auto bytes = output.str();
    auto* file = std::fopen(path.c_str(), "wbx");
    if (!file) throw std::invalid_argument("Unable to create new session; existing files are refused");
    const auto written = std::fwrite(bytes.data(), 1, bytes.size(), file);
    const auto closed = std::fclose(file);
    if (written != bytes.size() || closed != 0) throw std::invalid_argument("Unable to finish session write");
    std::cout << "Saved " << bytes.size() << " bytes to " << path << '\n';
}
void contact(captain::PhaseSessionState& state, const captain::ShotContent& content,
             unsigned code, unsigned defender, unsigned action, unsigned subtype) {
    const auto result = captain::resolve_session_terminal(state, {
        static_cast<std::uint8_t>(code), static_cast<std::uint8_t>(defender),
        static_cast<std::uint8_t>(action), static_cast<std::uint8_t>(subtype)},
        content.field_players, *content.movement, *content.contact);
    state = result.state;
    std::cout << "Contact elapsed " << unsigned(result.prelude->elapsed_step)
              << " | clock display request " << result.prelude->clock.request_clock_display
              << " | warning cue request " << result.prelude->clock.request_warning_cue << '\n';
}
void resume(captain::PhaseSessionState& state, const captain::ShotContent& content) {
    const auto result = captain::resume_session_terminal(state, content.field_players, *content.movement);
    state = result.state;
    std::cout << "Resumed terminal tail with frozen development inputs; no NMI/presentation emulated.\n";
}
void advance(captain::PhaseSessionState& state, const captain::ShotContent& content, unsigned elapsed) {
    if (state.phase != captain::FieldSessionPhase::field && state.phase != captain::FieldSessionPhase::player_control)
        throw std::invalid_argument("Movement requires field or player-control phase");
    auto field = state.current.field;
    field.elapsed = static_cast<std::uint8_t>(elapsed);
    const auto result = captain::advance_field_phase(field, content.field_players, *content.movement);
    state.current.field = result.state;
    std::cout << "Advanced " << result.players.size() << " actors; clock unchanged.\n";
}
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: captain-phase-session PACK STATE [--contact CODE --defender ID | --resume | --advance N] "
                     "[--defense-action N] [--defense-subtype N] [--capture NEW.cmjsession] [--interactive]\n";
        return 2;
    }
    try {
        bool interactive = false, run_resume = false, defense_options = false;
        std::optional<unsigned> code, defender, elapsed;
        unsigned action = 0, subtype = 0;
        std::string capture;
        for (int i = 3; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--interactive") interactive = true;
            else if (option == "--resume") run_resume = true;
            else if (option == "--contact" || option == "--defender" || option == "--advance" ||
                     option == "--defense-action" || option == "--defense-subtype" || option == "--capture") {
                if (++i >= argc) throw std::invalid_argument("Missing option value");
                if (option == "--capture") capture = argv[i];
                else if (option == "--contact") code = number(argv[i]);
                else if (option == "--defender") defender = number(argv[i]);
                else if (option == "--advance") elapsed = number(argv[i]);
                else if (option == "--defense-action") { action = number(argv[i]); defense_options = true; }
                else { subtype = number(argv[i]); defense_options = true; }
            } else throw std::invalid_argument("Unknown session option");
        }
        if (unsigned(bool(code)) + unsigned(run_resume) + unsigned(bool(elapsed)) > 1)
            throw std::invalid_argument("Choose one initial operation");
        if (bool(code) != bool(defender) || (defense_options && !code))
            throw std::invalid_argument("Contact needs --contact and --defender");
        std::ifstream pack(argv[1], std::ios::binary), snapshot(argv[2], std::ios::binary);
        if (!pack || !snapshot) throw std::invalid_argument("Unable to open private content/session");
        const auto content = captain::load_shot_content(pack);
        if (!content.movement || !content.contact) throw std::invalid_argument("Session requires version5 contact/movement content");
        auto state = captain::load_phase_session(snapshot);
        std::cout << "Captain Majid native phase session; development checkpoint, incomplete match.\n";
        if (code) contact(state, content, *code, *defender, action, subtype);
        if (run_resume) resume(state, content);
        if (elapsed) advance(state, content, *elapsed);
        show(state);
        if (!capture.empty()) save(capture, state);
        if (!interactive) return 0;
        std::cout << "Commands: state, contact CODE DEFENDER ACTION SUBTYPE, resume, advance N, save NEW.cmjsession, quit\n";
        std::string line;
        while (std::cout << "> " && std::getline(std::cin, line)) {
            try {
                std::istringstream input(line);
                std::string command, value, extra;
                if (!(input >> command)) continue;
                if (command == "state" || command == "quit" || command == "resume") {
                    if (input >> extra) throw std::invalid_argument("Command takes no arguments");
                    if (command == "quit") break;
                    if (command == "resume") resume(state, content);
                    show(state);
                } else if (command == "advance" || command == "save") {
                    if (!(input >> value) || (input >> extra)) throw std::invalid_argument("Expected command and one value");
                    if (command == "save") save(value, state);
                    else { advance(state, content, number(value)); show(state); }
                } else if (command == "contact") {
                    std::string id, defense, kind;
                    if (!(input >> value >> id >> defense >> kind) || (input >> extra))
                        throw std::invalid_argument("Expected contact CODE DEFENDER ACTION SUBTYPE");
                    contact(state, content, number(value), number(id), number(defense), number(kind));
                    show(state);
                } else throw std::invalid_argument("Unknown session command");
            } catch (const std::exception& error) { std::cout << "Error: " << error.what() << '\n'; }
        }
        return std::cin.bad() ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << "Phase session stopped: " << error.what() << '\n';
        return 1;
    }
}

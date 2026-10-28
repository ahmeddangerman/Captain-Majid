#include "captain/phase_session_io.hpp"
#include "captain/shot_content.hpp"
#include "captain/recovery_session.hpp"
#include "captain/control_session.hpp"
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
    case captain::FieldSessionPhase::boundary_restart: return "boundary restart pending";
    case captain::FieldSessionPhase::recovered_owner: return "recovered owner; presentation tail pending";
    case captain::FieldSessionPhase::keeper_interaction: return "keeper-area interaction pending";
    case captain::FieldSessionPhase::story_transition: return "story transition pending";
    case captain::FieldSessionPhase::control_ready: return "player control ready; input loop pending";
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
    if (state.experience) {
        std::cout << "Experience: campaign " << unsigned(state.experience->campaign)
                  << " | stage " << unsigned(state.experience->opponent_stage) << " | totals";
        for (const auto total : state.experience->totals) std::cout << ' ' << total;
        std::cout << '\n';
    } else std::cout << "Experience: context absent\n";
    if (state.recovery) {
        const auto& recovery = *state.recovery;
        std::cout << "Recovery: opponent " << unsigned(recovery.opposing_player)
                  << " | attack subtype " << unsigned(recovery.attack_subtype)
                  << " | owner flags " << unsigned(recovery.special_owner_flags)
                  << " | ball variant " << unsigned(recovery.ball_variant)
                  << " | game mode " << unsigned(recovery.game_mode)
                  << " | jitter control " << unsigned(recovery.x_jitter_control) << '\n';
    } else std::cout << "Recovery: context absent\n";
    if (state.control) {
        const auto& control = *state.control;
        std::cout << "Control: timer " << unsigned(control.decision_timer)
                  << " | facing " << unsigned(control.facing)
                  << " | controlled player " << unsigned(control.controlled_player)
                  << " | last carrier cell " << unsigned(control.last_carrier_cell)
                  << " | travel " << control.travel_accumulator
                  << " | display parameter " << unsigned(control.display_parameter) << '\n';
    } else std::cout << "Control: context absent\n";
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
             unsigned code, unsigned defender, unsigned action, unsigned subtype, bool rewarded = false) {
    if (rewarded && !content.experience) throw std::invalid_argument("Rewarded contact requires version6 experience content");
    const captain::PendingTerminalContact input{
        static_cast<std::uint8_t>(code), static_cast<std::uint8_t>(defender),
        static_cast<std::uint8_t>(action), static_cast<std::uint8_t>(subtype)};
    const auto result = rewarded
        ? captain::resolve_session_rewarded_terminal(state, input, content.field_players,
            *content.movement, *content.contact, *content.experience)
        : captain::resolve_session_terminal(state, input, content.field_players, *content.movement, *content.contact);
    state = result.state;
    for (const auto& reward : result.rewards)
        std::cout << "Reward: " << (reward.slot ? "slot " + std::to_string(*reward.slot) : "ignored type")
                  << " | amount " << unsigned(reward.award) << " | saturated " << reward.saturated << '\n';
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
void recover(captain::PhaseSessionState& state, const captain::ShotContent& content) {
    const auto result = captain::advance_session_recovery(state, content.field_players, *content.movement);
    state = result.state;
    std::cout << "Recovery advanced " << (result.movement ? result.movement->players.size() : 0)
              << " actors | recipient " << (result.recipient ? std::to_string(result.recipient->ball_owner) : "pending")
              << " | owner cleanup " << result.special_owner_cleared
              << " | possession sound request " << result.request_possession_sound << '\n';
}
void continue_recovery(captain::PhaseSessionState& state) {
    state = captain::continue_session_recovery(state);
    std::cout << "Prepared recovery tail: " << phase_name(state.phase)
              << "; frozen development inputs, no presentation/NMI emulation.\n";
}
void setup_control(captain::PhaseSessionState& state) {
    const auto result = captain::prepare_session_control(state);
    state = result.state;
    std::cout << "Prepared control | selected defender "
              << (result.defender ? std::to_string(result.defender->player_id) : "unchanged")
              << " | special cleanup " << result.special_owner_cleared << '\n';
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind)
                  << " | id " << unsigned(request.id) << '\n';
}
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: captain-phase-session PACK STATE [--contact CODE | --rewarded-contact CODE] --defender ID; "
                     "or [--resume | --recover | --continue-recovery | --setup-control | --advance N] "
                     "[--defense-action N] [--defense-subtype N] [--capture NEW.cmjsession] [--interactive]\n";
        return 2;
    }
    try {
        bool interactive = false, run_resume = false, defense_options = false, rewarded = false;
        bool run_recovery = false, run_continue_recovery = false;
        bool run_setup_control = false;
        std::optional<unsigned> code, defender, elapsed;
        unsigned action = 0, subtype = 0;
        std::string capture;
        for (int i = 3; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--interactive") interactive = true;
            else if (option == "--resume") run_resume = true;
            else if (option == "--recover") run_recovery = true;
            else if (option == "--continue-recovery") run_continue_recovery = true;
            else if (option == "--setup-control") run_setup_control = true;
            else if (option == "--contact" || option == "--rewarded-contact" || option == "--defender" || option == "--advance" ||
                     option == "--defense-action" || option == "--defense-subtype" || option == "--capture") {
                if (++i >= argc) throw std::invalid_argument("Missing option value");
                if (option == "--capture") capture = argv[i];
                else if (option == "--contact" || option == "--rewarded-contact") {
                    if (code) throw std::invalid_argument("Choose one contact operation");
                    code = number(argv[i]); rewarded = option == "--rewarded-contact";
                }
                else if (option == "--defender") defender = number(argv[i]);
                else if (option == "--advance") elapsed = number(argv[i]);
                else if (option == "--defense-action") { action = number(argv[i]); defense_options = true; }
                else { subtype = number(argv[i]); defense_options = true; }
            } else throw std::invalid_argument("Unknown session option");
        }
        if (unsigned(bool(code)) + unsigned(run_resume) + unsigned(bool(elapsed)) +
            unsigned(run_recovery) + unsigned(run_continue_recovery) + unsigned(run_setup_control) > 1)
            throw std::invalid_argument("Choose one initial operation");
        if (bool(code) != bool(defender) || (defense_options && !code))
            throw std::invalid_argument("Contact needs --contact and --defender");
        std::ifstream pack(argv[1], std::ios::binary), snapshot(argv[2], std::ios::binary);
        if (!pack || !snapshot) throw std::invalid_argument("Unable to open private content/session");
        const auto content = captain::load_shot_content(pack);
        if (!content.movement || !content.contact) throw std::invalid_argument("Session requires version5 contact/movement content");
        auto state = captain::load_phase_session(snapshot);
        std::cout << "Captain Majid native phase session; development checkpoint, incomplete match.\n";
        if (code) contact(state, content, *code, *defender, action, subtype, rewarded);
        if (run_resume) resume(state, content);
        if (run_recovery) recover(state, content);
        if (run_continue_recovery) continue_recovery(state);
        if (run_setup_control) setup_control(state);
        if (elapsed) advance(state, content, *elapsed);
        show(state);
        if (!capture.empty()) save(capture, state);
        if (!interactive) return 0;
        std::cout << "Commands: state, contact/rewarded-contact CODE DEFENDER ACTION SUBTYPE, resume, recover, continue-recovery, setup-control, advance N, save NEW.cmjsession, quit\n";
        std::string line;
        while (std::cout << "> " && std::getline(std::cin, line)) {
            try {
                std::istringstream input(line);
                std::string command, value, extra;
                if (!(input >> command)) continue;
                if (command == "state" || command == "quit" || command == "resume" ||
                    command == "recover" || command == "continue-recovery" || command == "setup-control") {
                    if (input >> extra) throw std::invalid_argument("Command takes no arguments");
                    if (command == "quit") break;
                    if (command == "resume") resume(state, content);
                    if (command == "recover") recover(state, content);
                    if (command == "continue-recovery") continue_recovery(state);
                    if (command == "setup-control") setup_control(state);
                    show(state);
                } else if (command == "advance" || command == "save") {
                    if (!(input >> value) || (input >> extra)) throw std::invalid_argument("Expected command and one value");
                    if (command == "save") save(value, state);
                    else { advance(state, content, number(value)); show(state); }
                } else if (command == "contact" || command == "rewarded-contact") {
                    std::string id, defense, kind;
                    if (!(input >> value >> id >> defense >> kind) || (input >> extra))
                        throw std::invalid_argument("Expected contact CODE DEFENDER ACTION SUBTYPE");
                    contact(state, content, number(value), number(id), number(defense), number(kind), command == "rewarded-contact");
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

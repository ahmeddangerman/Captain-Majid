#include "captain/phase_session_io.hpp"
#include "captain/shot_content.hpp"
#include "captain/recovery_session.hpp"
#include "captain/control_session.hpp"
#include "captain/session_shot.hpp"
#include "captain/staged_shot.hpp"
#include "captain/selected_dispatch.hpp"
#include "captain/capture_clearance.hpp"
#include "captain/clearance_race.hpp"
#include "captain/clearance_handoff.hpp"
#include "captain/restart_route.hpp"
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
    case captain::FieldSessionPhase::control_clock_pending: return "control clock step pending";
    case captain::FieldSessionPhase::action_menu_pending: return "action menu pending";
    case captain::FieldSessionPhase::ai_cell_decision: return "opposing-carrier cell decision pending";
    case captain::FieldSessionPhase::field_round_pending: return "field round pending";
    case captain::FieldSessionPhase::control_added_time: return "control added-time presentation pending";
    case captain::FieldSessionPhase::field_contact_pending: return "nearby field contact pending";
    case captain::FieldSessionPhase::goal_area_action: return "goal-area action pending";
    case captain::FieldSessionPhase::special_callback: return "special callback pending";
    case captain::FieldSessionPhase::human_action_menu: return "human action menu";
    case captain::FieldSessionPhase::attack_choice_pending: return "confirmed action pending";
    case captain::FieldSessionPhase::attack_prepared: return "ordinary shot prepared";
    case captain::FieldSessionPhase::shot_field_complete: return "shot passed selected defenses; flight pending";
    case captain::FieldSessionPhase::shot_restart_pending: return "shot interruption; restart pending";
    case captain::FieldSessionPhase::shot_defender_next: return "next selected shot defender";
    case captain::FieldSessionPhase::shot_contact_added_time: return "shot contact added-time presentation pending";
    case captain::FieldSessionPhase::shot_dispatch_clock: return "shot final dispatch clock pending";
    case captain::FieldSessionPhase::shot_dispatch_added_time: return "shot dispatch added-time presentation pending";
    case captain::FieldSessionPhase::shot_dispatch_ready: return "shot field result ready for dispatch";
    case captain::FieldSessionPhase::selected_ball_recovery: return "selected shot loose-ball recovery pending";
    case captain::FieldSessionPhase::selected_defense_animation: return "selected defense animation pending";
    case captain::FieldSessionPhase::selected_restart_entry: return "selected shot restart entry";
    case captain::FieldSessionPhase::selected_recovered_owner: return "selected recovered owner; presentation pending";
    case captain::FieldSessionPhase::selected_boundary_restart: return "selected recovery boundary restart pending";
    case captain::FieldSessionPhase::capture_clearance_flight: return "capture clearance prepared; flight pending";
    case captain::FieldSessionPhase::clearance_race_winner: return "clearance won; presentation pending";
    case captain::FieldSessionPhase::clearance_race_timeout: return "clearance race timeout; recovery pending";
    case captain::FieldSessionPhase::clearance_recovery_ready: return "clearance recovery ready; jitter already complete";
    case captain::FieldSessionPhase::clearance_recovered_owner: return "clearance recovered owner; presentation pending";
    case captain::FieldSessionPhase::selected_field_restart: return "selected field restart setup pending";
    case captain::FieldSessionPhase::selected_goal_restart: return "selected goal-area restart setup pending";
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
    if(state.clearance_race) std::cout << "Race: remaining " << unsigned(state.clearance_race->remaining)
        << " | candidates " << unsigned(state.clearance_race->active_candidate) << ','
        << unsigned(state.clearance_race->opposing_candidate) << '\n';
    if (state.clearance_flight) std::cout << "Clearance distance adjustment "
        << unsigned(state.clearance_flight->distance_adjustment) << '\n';
    if (state.shot_progress) std::cout << "Shot progress: cursor " << unsigned(state.shot_progress->cursor)
        << " | result " << unsigned(state.shot_progress->result_code)
        << " | defender " << unsigned(state.shot_progress->defender_id) << '\n';
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
                  << " | display parameter " << unsigned(control.display_parameter)
                  << " | motion requested " << unsigned(control.motion_requested) << '\n';
    } else std::cout << "Control: context absent\n";
    if (state.scheduling) {
        const auto& schedule = *state.scheduling;
        std::cout << "Scheduling: field cycle " << unsigned(schedule.field_cycle)
                  << " | energy cycle " << unsigned(schedule.energy_cycle)
                  << " | special flags " << unsigned(schedule.special_flags) << " | selected";
        for (unsigned i = 0; i < field.selected_defenders; ++i) std::cout << ' ' << unsigned(schedule.selected_ids[i]);
        std::cout << '\n';
    } else std::cout << "Scheduling: context absent\n";
    if (state.action) {
        const auto& action = *state.action;
        std::cout << "Action: " << unsigned(action.action) << " | route " << unsigned(action.menu_route)
                  << " | cursor " << unsigned(action.cursor) << " | flash " << unsigned(action.flash_counter)
                  << " | auxiliary " << unsigned(action.auxiliary) << " | move " << unsigned(action.move_effect)
                  << ',' << unsigned(action.move_flags) << " | power " << action.power << '\n';
    } else std::cout << "Action: context absent\n";
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
void recover_selected_file(captain::PhaseSessionState& state, const captain::ShotContent& content,
                           const std::string& path) {
    captain::validate_phase_session(state);
    if (state.phase != captain::FieldSessionPhase::selected_ball_recovery &&
        state.phase != captain::FieldSessionPhase::clearance_recovery_ready)
        throw std::invalid_argument("Selected recovery requires its saved handoff");
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open selected recovery action file");
    std::vector<captain::RecoveryDefender> choices;
    for (unsigned i=0;i<state.current.field.selected_defenders;++i) {
        unsigned action;
        if (!(input >> action) || action > 6) throw std::invalid_argument("Expected one action0..6 per saved selected player");
        choices.push_back({state.scheduling->selected_ids[i],static_cast<std::uint8_t>(action)});
    }
    input >> std::ws;
    if (input.peek() != std::char_traits<char>::eof() || input.bad())
        throw std::invalid_argument("Trailing or unreadable selected recovery actions");
    const auto result = state.phase == captain::FieldSessionPhase::clearance_recovery_ready
        ? captain::advance_session_clearance_recovery(state,choices,content.field_players,*content.movement)
        : captain::advance_session_selected_recovery(state,choices,content.field_players,*content.movement);
    state = result.state;
    std::cout << "Selected recovery | recipient " << (result.recipient ? std::to_string(result.recipient->ball_owner) : "pending")
              << " | direct selected capture " << (result.recipient && result.recipient->direct_defender_capture)
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
void poll_control(captain::PhaseSessionState& state, const captain::ShotContent& content, unsigned held, unsigned pressed) {
    if (!content.control_movement) throw std::invalid_argument("Control polling requires version7 speed content");
    const auto result = captain::poll_session_control(state, static_cast<std::uint8_t>(held),
        static_cast<std::uint8_t>(pressed), content.field_players, *content.movement, *content.control_movement);
    state = result.state;
    std::cout << "Input poll | decision step " << result.decision_step
              << " | moved player " << (result.moved_player ? std::to_string(*result.moved_player) : "none")
              << " | speed " << result.movement_speed << '\n';
    if (result.menu_requested) std::cout << "Action-menu script68 pending; handler not executed.\n";
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind)
                  << " | id " << unsigned(request.id) << '\n';
}
void advance_control(captain::PhaseSessionState& state, const captain::ShotContent& content, bool resume_time) {
    if (!content.world_step) throw std::invalid_argument("Control cycle requires version8 stamina content");
    const auto result = resume_time
        ? captain::resume_control_added_time(state, content.field_players, content.goalkeepers, *content.world_step)
        : captain::advance_control_tick(state, content.field_players, content.goalkeepers, *content.world_step);
    state = result.state;
    if (result.clock) std::cout << "Control clock | display request " << result.clock->request_clock_display
        << " | warning cue request " << result.clock->request_warning_cue << '\n';
    else std::cout << "Prepared added-time continuation; no repeated clock/recovery.\n";
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << '\n';
}
void advance_world_inputs(captain::PhaseSessionState& state, const captain::ShotContent& content,
                         const std::array<captain::LogicalButtons, 22>& inputs) {
    if (!content.world_step || !content.control_movement)
        throw std::invalid_argument("World round requires version8 speed content");
    const auto result = captain::advance_control_field_round(state, content.field_players,
        *content.movement, *content.control_movement, *content.world_step, inputs);
    state = result.state;
    std::cout << "World round | polls " << result.consumed_polls << " | moved " << result.moved_players
              << " | decayed statuses " << result.decayed_statuses << '\n';
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << '\n';
}
void advance_world(captain::PhaseSessionState& state, const captain::ShotContent& content, unsigned held, unsigned pressed) {
    std::array<captain::LogicalButtons, 22> inputs{};
    inputs.fill({static_cast<std::uint8_t>(held), static_cast<std::uint8_t>(pressed)});
    advance_world_inputs(state, content, inputs);
}
void advance_world_file(captain::PhaseSessionState& state, const captain::ShotContent& content, const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open world input file");
    std::array<captain::LogicalButtons, 22> inputs{};
    for (auto& buttons : inputs) {
        unsigned held, pressed;
        if (!(input >> held >> pressed) || held > 255 || pressed > 255)
            throw std::invalid_argument("World input file requires22held/pressed byte pairs");
        buttons = {static_cast<std::uint8_t>(held), static_cast<std::uint8_t>(pressed)};
    }
    input >> std::ws;
    if (input.peek() != std::char_traits<char>::eof() || input.bad())
        throw std::invalid_argument("Trailing or unreadable world input data");
    advance_world_inputs(state, content, inputs);
}
void open_action_menu(captain::PhaseSessionState& state, const captain::ShotContent& content) {
    if (!content.action_menu) throw std::invalid_argument("Action menu requires version9 menu content");
    const auto result = captain::open_human_action_menu(state, *content.action_menu);
    state = result.state;
    std::cout << "Opened human attacking menu; no game-frame/NMI work performed.\n";
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind)
                  << " | id " << unsigned(request.id) << '\n';
}
void menu_input(captain::PhaseSessionState& state, const captain::ShotContent& content, unsigned pressed) {
    if (!content.action_menu) throw std::invalid_argument("Action menu polling requires version9 menu content");
    const auto result = captain::poll_human_action_menu(state, static_cast<std::uint8_t>(pressed), *content.action_menu);
    state = result.state;
    std::cout << "Menu input | confirmed " << result.confirmed << '\n';
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind)
                  << " | id " << unsigned(request.id) << '\n';
}
void shot_defenders_file(captain::PhaseSessionState& state, const captain::ShotContent& content,
                         const std::string& path) {
    if (!content.experience) throw std::invalid_argument("Shot defenses need experience content");
    captain::validate_phase_session(state);
    if (state.phase != captain::FieldSessionPhase::attack_prepared)
        throw std::invalid_argument("Shot defenses require an already prepared shot");
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open shot defense choices");
    std::vector<captain::FieldDefenseChoice> choices;
    // One row for each saved selected defender: action subtype random1 random2.
    // No caller-supplied roster, player ID, power or comparison coefficients.
    for (unsigned i = 0; i < state.current.field.selected_defenders; ++i) {
        unsigned action, subtype, first, second;
        if (!(input >> action >> subtype >> first >> second) ||
            action > 255 || subtype > 255 || first > 255 || second > 255)
            throw std::invalid_argument("Expected action subtype random1 random2 bytes per selected defender");
        captain::FieldDefenseChoice choice{};
        choice.player_id = state.scheduling->selected_ids[i];
        choice.action = static_cast<std::uint8_t>(action);
        choice.subtype = static_cast<std::uint8_t>(subtype);
        choice.comparison.random_first = static_cast<std::uint8_t>(first);
        choice.comparison.random_second = static_cast<std::uint8_t>(second);
        choices.push_back(choice);
    }
    input >> std::ws;
    if (input.peek() != std::char_traits<char>::eof() || input.bad())
        throw std::invalid_argument("Trailing or unreadable shot defense data");
    const auto result = captain::resolve_session_shot_defenders(state,choices,false,
        content.field_players,content.defense,content.field_comparison,
        *content.movement,*content.contact,*content.experience);
    state = result.state;
    std::cout << "Shot defenses | reached " << result.defenses.contacts.size()
              << " | result " << unsigned(result.defenses.result_code)
              << " | retained power " << state.action->power << '\n';
    if (result.terminal) std::cout << "Terminal rewards " << result.terminal->rewards.size()
        << " | contact elapsed " << unsigned(result.terminal->prelude->elapsed_step) << '\n';
    if (result.restart) std::cout << "Restart route " << static_cast<unsigned>(result.restart->route) << " pending\n";
}
void staged_defense(captain::PhaseSessionState& state, const captain::ShotContent& content,
                    const std::array<unsigned,4>& values) {
    captain::validate_phase_session(state);
    const auto cursor = state.shot_progress ? state.shot_progress->cursor : 0;
    if (!state.scheduling || cursor >= state.current.field.selected_defenders)
        throw std::invalid_argument("No selected defender remains");
    captain::FieldDefenseChoice choice{};
    choice.player_id = state.scheduling->selected_ids[cursor];
    choice.action = static_cast<std::uint8_t>(values[0]);choice.subtype = static_cast<std::uint8_t>(values[1]);
    choice.comparison.random_first = static_cast<std::uint8_t>(values[2]);
    choice.comparison.random_second = static_cast<std::uint8_t>(values[3]);
    const auto result = captain::step_session_shot_defender(state,choice,false,
        content.field_players,content.defense,content.field_comparison,*content.contact);
    state = result.state;
    std::cout << "Selected defender stepped | clock " << state.current.clock.remaining
              << " | result " << unsigned(state.shot_progress->result_code) << '\n';
}
void dispatch_shot(captain::PhaseSessionState& state, unsigned elapsed) {
    const auto result = captain::dispatch_session_shot(state,static_cast<std::uint8_t>(elapsed));
    state = result.state;
    std::cout << "Final shot clock applied; dispatch handlers remain pending.\n";
}
void resolve_shot_result(captain::PhaseSessionState& state, const captain::ShotContent& content) {
    if (!content.experience) throw std::invalid_argument("Selected dispatch needs experience content");
    const auto result = captain::resolve_session_selected_result(state,content.field_players,
        *content.movement,*content.experience);
    state = result.state;
    std::cout << "Selected result dispatched | advanced actors " << result.movement.players.size()
              << " | reward " << (result.reward ? unsigned(result.reward->award) : 0u)
              << " | possession sound request " << result.request_possession_sound << '\n';
}
void prepare_clearance_file(captain::PhaseSessionState& state, const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open private clearance content");
    captain::CaptureClearanceContent content{};
    for (auto& pair : content.destinations) for (auto& axis : pair) {
        unsigned value;
        if (!(input >> value) || value > 255) throw std::invalid_argument("Expected sixteen clearance coordinate bytes");
        axis=static_cast<std::uint8_t>(value);
    }
    unsigned distance;
    if (!(input >> distance) || distance > 255) throw std::invalid_argument("Expected clearance distance byte");
    content.distance_adjustment=static_cast<std::uint8_t>(distance);
    input >> std::ws;
    if (input.peek()!=std::char_traits<char>::eof() || input.bad())
        throw std::invalid_argument("Trailing or unreadable clearance data");
    const auto result=captain::prepare_session_capture_clearance(state,content);
    state=result.state;
    std::cout << "Capture clearance prepared | side music request " << result.request_side_music
              << " | cooperative task request " << unsigned(result.requested_task) << '\n';
}
void clearance_race_file(captain::PhaseSessionState& state,const captain::ShotContent& content,const std::string& path) {
    std::ifstream input(path);
    if(!input) throw std::invalid_argument("Unable to open private race content");
    captain::ClearanceRaceContent race{};
    for(auto& pair:race.speeds) for(auto& speed:pair) {
        unsigned value;
        if(!(input>>value) || value>65535) throw std::invalid_argument("Expected512 unsigned race speed words");
        speed=static_cast<std::uint16_t>(value);
    }
    input>>std::ws;
    if(input.peek()!=std::char_traits<char>::eof() || input.bad()) throw std::invalid_argument("Trailing or unreadable race data");
    const auto result=captain::resolve_session_clearance_race(state,content.field_players,*content.movement,race);
    state=result.state;
    std::cout << "Clearance race | rounds " << result.rounds << " | winner "
        << (result.winner?std::to_string(*result.winner):"none") << " | pending script " << unsigned(result.pending_script)
        << " | side music request " << result.request_side_music << '\n';
}
void prepare_basic_shot(captain::PhaseSessionState& state, const captain::ShotContent& content) {
    if (!content.action_menu) throw std::invalid_argument("Shot preparation requires version9 menu content");
    const auto result = captain::prepare_session_basic_shot(state, content.field_players, content.attack);
    state = result.state;
    std::cout << "Prepared ordinary shot | stamina " << state.current.field.roster[state.current.field.ball_owner].stamina
              << " | raw cost " << result.preparation.preparation.stamina_cost
              << " | power " << result.preparation.action_power.power
              << " | subtype " << unsigned(result.preparation.action_power.subtype)
              << " | move " << unsigned(result.preparation.preparation.move_effect) << '\n';
}
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: captain-phase-session PACK STATE [--contact CODE | --rewarded-contact CODE] --defender ID; "
                     "or [--resume | --recover | --continue-recovery | --setup-control | --poll-control HELD PRESSED | "
                     "--advance-control | --resume-control-time | --advance-world HELD PRESSED | --advance-world-file FILE | "
                     "--open-action-menu | --menu-input PRESSED | --prepare-basic-shot | --shot-defenders-file FILE | --step-shot-defense ACTION SUBTYPE RANDOM1 RANDOM2 | --resume-shot-clock | --dispatch-shot ELAPSED | --resolve-shot-result | --recover-selected-file FILE | --continue-selected-recovery | --prepare-clearance-file FILE | --clearance-race-file FILE | --continue-clearance | --route-selected-restart | --advance N] "
                     "[--defense-action N] [--defense-subtype N] [--capture NEW.cmjsession] [--interactive]\n";
        return 2;
    }
    try {
        bool interactive = false, run_resume = false, defense_options = false, rewarded = false;
        bool run_recovery = false, run_continue_recovery = false;
        bool run_setup_control = false;
        bool run_control_tick = false, resume_control_time = false;
        bool run_open_action_menu = false, run_prepare_basic_shot = false, resume_shot_clock = false, run_selected_result = false, run_continue_selected = false, run_continue_clearance = false, run_restart_route = false;
        std::optional<std::string> selected_recovery_file, clearance_file, race_file;
        std::optional<std::array<unsigned,4>> staged_input;
        std::optional<unsigned> dispatch_elapsed;
        std::optional<unsigned> code, defender, elapsed;
        std::optional<unsigned> menu_pressed;
        std::optional<std::array<unsigned, 2>> poll_input;
        std::optional<std::array<unsigned, 2>> world_input;
        std::optional<std::string> world_file, shot_file;
        unsigned action = 0, subtype = 0;
        std::string capture;
        for (int i = 3; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--interactive") interactive = true;
            else if (option == "--resume") run_resume = true;
            else if (option == "--recover") run_recovery = true;
            else if (option == "--continue-recovery") run_continue_recovery = true;
            else if (option == "--setup-control") run_setup_control = true;
            else if (option == "--advance-control") run_control_tick = true;
            else if (option == "--resume-control-time") resume_control_time = true;
            else if (option == "--open-action-menu") run_open_action_menu = true;
            else if (option == "--prepare-basic-shot") run_prepare_basic_shot = true;
            else if (option == "--resume-shot-clock") resume_shot_clock = true;
            else if (option == "--resolve-shot-result") run_selected_result = true;
            else if (option == "--continue-selected-recovery") run_continue_selected = true;
            else if (option == "--continue-clearance") run_continue_clearance = true;
            else if (option == "--route-selected-restart") run_restart_route = true;
            else if (option == "--clearance-race-file") {
                if(++i>=argc || race_file) throw std::invalid_argument("Expected one race content file");
                race_file=argv[i];
            }
            else if (option == "--prepare-clearance-file") {
                if (++i >= argc || clearance_file) throw std::invalid_argument("Expected one clearance content file");
                clearance_file=argv[i];
            }
            else if (option == "--recover-selected-file") {
                if (++i >= argc || selected_recovery_file) throw std::invalid_argument("Expected one selected recovery file");
                selected_recovery_file = argv[i];
            }
            else if (option == "--dispatch-shot") {
                if (++i >= argc || dispatch_elapsed) throw std::invalid_argument("Expected one dispatch elapsed byte");
                dispatch_elapsed = number(argv[i]);
            }
            else if (option == "--step-shot-defense") {
                if (i + 4 >= argc || staged_input) throw std::invalid_argument("Expected four defense bytes");
                std::array<unsigned,4> values{};
                for (auto& value : values) value = number(argv[++i]);
                staged_input = values;
            }
            else if (option == "--shot-defenders-file") {
                if (++i >= argc || shot_file) throw std::invalid_argument("Expected one shot defense file");
                shot_file = argv[i];
            }
            else if (option == "--menu-input") {
                if (++i >= argc || menu_pressed) throw std::invalid_argument("Expected one --menu-input PRESSED");
                menu_pressed = number(argv[i]);
            }
            else if (option == "--advance-world-file") {
                if (++i >= argc || world_file) throw std::invalid_argument("Expected one world input file");
                world_file = argv[i];
            }
            else if (option == "--advance-world") {
                if (i + 2 >= argc || world_input) throw std::invalid_argument("Expected one --advance-world HELD PRESSED");
                const auto held = number(argv[++i]);
                const auto pressed = number(argv[++i]);
                world_input = std::array<unsigned, 2>{held, pressed};
            }
            else if (option == "--poll-control") {
                if (i + 2 >= argc || poll_input) throw std::invalid_argument("Expected one --poll-control HELD PRESSED");
                const auto held = number(argv[++i]);
                const auto pressed = number(argv[++i]);
                poll_input = std::array<unsigned, 2>{held, pressed};
            }
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
        if (unsigned(bool(code)) + unsigned(run_resume) + unsigned(bool(elapsed)) + unsigned(run_recovery) +
            unsigned(run_continue_recovery) + unsigned(run_setup_control) + unsigned(bool(poll_input)) +
            unsigned(run_control_tick) + unsigned(resume_control_time) + unsigned(bool(world_input)) +
            unsigned(bool(world_file)) + unsigned(run_open_action_menu) + unsigned(bool(menu_pressed)) +
            unsigned(run_prepare_basic_shot) + unsigned(bool(shot_file)) + unsigned(bool(staged_input)) + unsigned(bool(dispatch_elapsed)) + unsigned(resume_shot_clock) + unsigned(run_selected_result) + unsigned(run_continue_selected) + unsigned(bool(selected_recovery_file)) + unsigned(bool(clearance_file)) + unsigned(bool(race_file)) + unsigned(run_continue_clearance) + unsigned(run_restart_route) > 1)
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
        if (poll_input) poll_control(state, content, (*poll_input)[0], (*poll_input)[1]);
        if (run_control_tick || resume_control_time) advance_control(state, content, resume_control_time);
        if (world_input) advance_world(state, content, (*world_input)[0], (*world_input)[1]);
        if (world_file) advance_world_file(state, content, *world_file);
        if (run_open_action_menu) open_action_menu(state, content);
        if (menu_pressed) menu_input(state, content, *menu_pressed);
        if (run_prepare_basic_shot) prepare_basic_shot(state, content);
        if (shot_file) shot_defenders_file(state, content, *shot_file);
        if (staged_input) staged_defense(state,content,*staged_input);
        if (dispatch_elapsed) dispatch_shot(state,*dispatch_elapsed);
        if (resume_shot_clock) state = captain::resume_session_shot_clock(state);
        if (run_selected_result) resolve_shot_result(state,content);
        if (selected_recovery_file) recover_selected_file(state,content,*selected_recovery_file);
        if (run_continue_selected) state = captain::continue_session_selected_recovery(state);
        if (clearance_file) prepare_clearance_file(state,*clearance_file);
        if (race_file) clearance_race_file(state,content,*race_file);
        if (run_continue_clearance) state=captain::continue_session_clearance(state);
        if (run_restart_route) state=captain::route_session_selected_restart(state).state;
        if (elapsed) advance(state, content, *elapsed);
        show(state);
        if (!capture.empty()) save(capture, state);
        if (!interactive) return 0;
        std::cout << "Commands: state, contact/rewarded-contact CODE DEFENDER ACTION SUBTYPE, resume, recover, continue-recovery, setup-control, poll-control HELD PRESSED, advance-control, resume-control-time, advance-world HELD PRESSED, advance-world-file FILE, open-action-menu, menu-input PRESSED, prepare-basic-shot, shot-defenders-file FILE, step-shot-defense ACTION SUBTYPE RANDOM1 RANDOM2, resume-shot-clock, dispatch-shot ELAPSED, resolve-shot-result, recover-selected-file FILE, continue-selected-recovery, prepare-clearance-file FILE, clearance-race-file FILE, continue-clearance, route-selected-restart, advance N, save NEW.cmjsession, quit\n";
        std::string line;
        while (std::cout << "> " && std::getline(std::cin, line)) {
            try {
                std::istringstream input(line);
                std::string command, value, extra;
                if (!(input >> command)) continue;
                if (command == "state" || command == "quit" || command == "resume" ||
                    command == "recover" || command == "continue-recovery" || command == "setup-control" ||
                    command == "advance-control" || command == "resume-control-time" || command == "resume-shot-clock" || command == "resolve-shot-result" || command == "continue-selected-recovery" || command == "continue-clearance" || command == "route-selected-restart") {
                    if (input >> extra) throw std::invalid_argument("Command takes no arguments");
                    if (command == "quit") break;
                    if (command == "route-selected-restart") state=captain::route_session_selected_restart(state).state;
                    if (command == "continue-clearance") state=captain::continue_session_clearance(state);
                    if (command == "continue-selected-recovery") state = captain::continue_session_selected_recovery(state);
                    if (command == "resolve-shot-result") resolve_shot_result(state,content);
                    if (command == "resume-shot-clock") state = captain::resume_session_shot_clock(state);
                    if (command == "resume") resume(state, content);
                    if (command == "recover") recover(state, content);
                    if (command == "continue-recovery") continue_recovery(state);
                    if (command == "setup-control") setup_control(state);
                    if (command == "advance-control" || command == "resume-control-time")
                        advance_control(state, content, command == "resume-control-time");
                    show(state);
                } else if (command == "poll-control" || command == "advance-world") {
                    std::string pressed;
                    if (!(input >> value >> pressed) || (input >> extra))
                        throw std::invalid_argument("Expected poll-control HELD PRESSED");
                    if (command == "poll-control") poll_control(state, content, number(value), number(pressed));
                    else advance_world(state, content, number(value), number(pressed));
                    show(state);
                } else if (command == "open-action-menu" || command == "prepare-basic-shot") {
                    if (input >> extra) throw std::invalid_argument("Command takes no arguments");
                    if (command == "open-action-menu") open_action_menu(state, content);
                    else prepare_basic_shot(state, content);
                    show(state);
                } else if (command == "step-shot-defense") {
                    std::array<unsigned,4> values{};
                    for (auto& entry : values) {
                        if (!(input >> value)) throw std::invalid_argument("Expected four defense bytes");
                        entry = number(value);
                    }
                    if (input >> extra) throw std::invalid_argument("Expected exactly four defense bytes");
                    staged_defense(state,content,values);show(state);
                } else if (command == "dispatch-shot") {
                    if (!(input >> value) || (input >> extra)) throw std::invalid_argument("Expected dispatch-shot ELAPSED");
                    dispatch_shot(state,number(value));show(state);
                } else if (command == "menu-input") {
                    if (!(input >> value) || (input >> extra)) throw std::invalid_argument("Expected menu-input PRESSED");
                    menu_input(state, content, number(value));
                    show(state);
                } else if (command == "advance" || command == "save" || command == "advance-world-file" || command == "shot-defenders-file" || command == "recover-selected-file" || command == "prepare-clearance-file" || command == "clearance-race-file") {
                    if (!(input >> value) || (input >> extra)) throw std::invalid_argument("Expected command and one value");
                    if (command == "save") save(value, state);
                    else if (command == "clearance-race-file") { clearance_race_file(state,content,value); show(state); }
                    else if (command == "prepare-clearance-file") { prepare_clearance_file(state,value); show(state); }
                    else if (command == "recover-selected-file") { recover_selected_file(state,content,value); show(state); }
                    else if (command == "shot-defenders-file") { shot_defenders_file(state, content, value); show(state); }
                    else if (command == "advance-world-file") { advance_world_file(state, content, value); show(state); }
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

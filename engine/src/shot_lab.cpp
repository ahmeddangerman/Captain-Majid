#include "captain/shot_scene.hpp"
#include <charconv>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
using Options = captain::ShotSceneOptions;
unsigned number(const std::string& text, unsigned maximum) {
    unsigned value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || value > maximum)
        throw std::invalid_argument("Number outside supported range");
    return value;
}
void option(Options& options, const std::string& name, const std::string& value) {
    if (name == "--side") {
        const auto side = number(value, 11);
        if (side != 0 && side != 11) throw std::invalid_argument("Side must be0 or11");
        options.side = side;
    }
    else if (name == "--level") options.level = number(value, 255);
    else if (name == "--keeper-level") options.keeper_level = number(value, 255);
    else if (name == "--subtype") options.subtype = number(value, 255);
    else if (name == "--defenders") options.defenders = number(value, 5);
    else if (name == "--flight-defenders") options.flight_defenders = number(value, 5);
    else if (name == "--keeper-action") options.keeper_action = number(value, 3);
    else if (name == "--ball-x") options.ball_x = number(value, 255);
    else if (name == "--ball-y") options.ball_y = number(value, 255);
    else if (name == "--keeper-timer") options.keeper_timer = number(value, 255);
    else if (name == "--keeper-duration") options.keeper_duration = number(value, 255);
    else if (name == "--keeper-penalty") options.keeper_penalty = number(value, 255);
    else if (name == "--keeper-stun") options.keeper_stun = number(value, 255);
    else if (name == "--recover") options.recovery_steps.push_back(number(value, 255));
    else throw std::invalid_argument("Unknown option: " + name);
}
const char* event_name(captain::ShotEvent event) {
    switch (event) {
    case captain::ShotEvent::goal: return "goal";
    case captain::ShotEvent::special_turnover: return "special shot turnover";
    case captain::ShotEvent::rebound: return "rebound";
    case captain::ShotEvent::keeper_catch: return "keeper catch";
    case captain::ShotEvent::keeper_punch: return "keeper punch";
    }
    throw std::logic_error("Unknown shot event");
}
void shoot(const captain::ShotContent& content, const Options& options) {
    const auto result = captain::calculate_shot_scene(content, options);
    const auto state = captain::resolve_shot_scene_keeper_state(options);
    std::cout << "Scene side " << options.side << '\n';
    std::cout << "Pre-shot keeper: timer " << unsigned(state.timer)
              << " | penalty " << unsigned(state.penalty)
              << " | duration " << unsigned(state.penalty_duration)
              << " | stun " << unsigned(state.stun) << '\n';
    std::cout << "Power " << result.attack.action_power.power
              << " | field contacts " << result.flow.field.contacts.size()
              << " | remaining " << result.flow.field.remaining_power << '\n';
    if (result.keeper) {
        std::cout << "Keeper strength " << unsigned(result.keeper->comparison.strength)
                  << " | ratio " << unsigned(result.keeper->comparison.ratio)
                  << " | state adjustment " << unsigned(result.keeper->comparison.state_adjustment)
                  << " | score " << unsigned(result.keeper->comparison.score)
                  << " | raw result " << unsigned(result.keeper->comparison.result_code) << '\n';
    }
    if (result.flight) {
        std::cout << "Flight contacts " << result.flight->contacts.size()
                  << " | remaining " << result.flight->remaining_power
                  << " | result " << unsigned(result.flight->result_code) << '\n';
    }
    if (result.approach) {
        const auto& approach = *result.approach;
        std::cout << "Approach distance " << unsigned(approach.distance_adjustment)
                  << " | target " << unsigned(approach.target_x) << ',' << unsigned(approach.target_y)
                  << " | route " << (approach.loose_ball ? "loose ball" : "toward keeper") << '\n';
    }
    if (result.flow.continuation && result.flow.continuation->outcome) {
        const auto& resolved = *result.flow.continuation->outcome;
        std::cout << "Outcome: " << event_name(resolved.event)
                  << " | score " << unsigned(resolved.left_goals) << ':' << unsigned(resolved.right_goals)
                  << " | possession " << unsigned(resolved.possession_side)
                  << " | owner " << unsigned(resolved.ball_owner)
                  << " | ball " << unsigned(resolved.ball_x) << ',' << unsigned(resolved.ball_y) << '\n';
        std::cout << "Post-shot keeper: timer " << unsigned(resolved.keeper_timer)
                  << " | penalty " << unsigned(resolved.keeper_penalty)
                  << " | duration " << unsigned(resolved.keeper_penalty_duration)
                  << " | stun " << unsigned(resolved.keeper_stun) << '\n';
        std::cout << "Handoff: defenders " << unsigned(resolved.defenders)
                  << " | active " << unsigned(resolved.ball_active)
                  << " | kickoff " << unsigned(resolved.kickoff) << '\n';
    } else if (result.flight && result.flight->stopped) {
        const auto& last = result.flight->contacts.back();
        std::cout << "Pending later-flight handoff: result " << unsigned(result.flight->result_code)
                  << " | defender " << unsigned(last.player_id)
                  << " | action " << unsigned(last.action)
                  << " (terminal handler is outside this calculation)\n";
    } else {
        std::cout << "Field result " << unsigned(result.flow.field.result_code)
                  << " | route " << unsigned(result.flow.field_outcome.route)
                  << " | owner " << unsigned(result.flow.field_outcome.ball_owner)
                  << " (later field phase is outside this scene)\n";
    }
    const auto owner = options.side == 0 ? 3u : 14u;
    const auto keeper_slot = options.side ^ 11u;
    std::cout << "Stamina: attacker " << result.flow.field.roster[owner].stamina
              << " | keeper " << result.flow.field.roster[keeper_slot].stamina << '\n';
}
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: captain-shot-lab PACK [--interactive] [--side 0|11] [--level N] [--keeper-level N] "
                     "[--subtype N] [--defenders 0..5] [--flight-defenders 0..5] [--keeper-action 0..3] [--ball-x N] [--ball-y N]\n"
                     "[--keeper-timer N] [--keeper-duration N] [--keeper-penalty N] [--keeper-stun N] [--recover N ...]\n";
        return 2;
    }
    try {
        Options options;
        bool interactive = false;
        for (int i = 2; i < argc; ++i) {
            const std::string name = argv[i];
            if (name == "--interactive") interactive = true;
            else {
                if (i + 1 >= argc) throw std::invalid_argument("Missing option value");
                option(options, name, argv[++i]);
            }
        }
        std::ifstream file(argv[1], std::ios::binary);
        if (!file) throw std::invalid_argument("Unable to open local content pack");
        const auto content = captain::load_shot_content(file);
        std::cout << "Captain Majid native shot laboratory\n"
                     "Constructed scene with explicit side, staged randomness and local content.\n"
                     "Each shot resets the scene; this is not a match or original outcome-script replay.\n";
        if (!interactive) {
            shoot(content, options);
            return 0;
        }
        std::cout << "Commands: shoot, side 0|11, level N, keeper-level N, subtype N, defenders N, keeper-action N, ball-x N, ball-y N, quit\n";
        std::cout << "Flight burst: flight-defenders N (side0 maximum5, side11 maximum4)\n";
        std::cout << "Keeper state: keeper-timer N, keeper-duration N, keeper-penalty N, keeper-stun N, recover N, clear-recovery\n"
                     "Recover appends a pre-shot elapsed step; each shot reapplies the list from the initial state.\n";
        std::string line;
        while (std::cout << "> " && std::getline(std::cin, line)) {
            std::istringstream command(line);
            std::string name, value, extra;
            if (!(command >> name)) continue;
            try {
                if (name == "shoot" || name == "quit" || name == "clear-recovery") {
                    if (command >> extra) throw std::invalid_argument("Command takes no arguments");
                    if (name == "quit") break;
                    if (name == "clear-recovery") options.recovery_steps.clear();
                    else shoot(content, options);
                } else {
                    if (!(command >> value) || command >> extra)
                        throw std::invalid_argument("Expected one numeric value");
                    option(options, "--" + name, value);
                }
            } catch (const std::exception& error) {
                std::cerr << "Scene request rejected: " << error.what() << '\n';
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Shot laboratory stopped: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

#include "captain/complete_prepared_shot.hpp"
#include "captain/shot_content.hpp"
#include <charconv>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
struct Options {
    unsigned level = 20, keeper_level = 20, subtype = 0, defenders = 2, keeper_action = 1;
};
unsigned number(const std::string& text, unsigned maximum) {
    unsigned value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || value > maximum)
        throw std::invalid_argument("Number outside supported range");
    return value;
}
void option(Options& options, const std::string& name, const std::string& value) {
    if (name == "--level") options.level = number(value, 255);
    else if (name == "--keeper-level") options.keeper_level = number(value, 255);
    else if (name == "--subtype") options.subtype = number(value, 255);
    else if (name == "--defenders") options.defenders = number(value, 5);
    else if (name == "--keeper-action") options.keeper_action = number(value, 3);
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
    const auto index = options.subtype & 63u;
    if (index >= content.attack.moves[0].size())
        throw std::invalid_argument("Shot subtype unavailable in this content pack");
    captain::CompletePreparedShotInput input{};
    for (auto& player : input.field.roster) player = {1, 20, 1000};
    input.field.roster[3].level = static_cast<std::uint8_t>(options.level);
    input.field.roster[11] = {33, static_cast<std::uint8_t>(options.keeper_level), 1000};
    input.field.ball_owner = 3;
    input.field.context = {0, 0, static_cast<std::uint8_t>(options.subtype), 0, 0};
    input.field.context.comparison_content = content.field_comparison;
    input.field.context.interruption = captain::FieldInterruptionContext{160, 100, {}};
    for (unsigned i = 0; i < options.defenders; ++i)
        input.field.choices.push_back({static_cast<std::uint8_t>(14 + i), 2, 0, 0,
            {0, 0, 100, 100, 1, 0, false, false, 0, 0, 0, {0, 0, 0, 0}}});
    input.attack_random_first = 100;
    input.attack_random_second = 100;
    input.keeper_action = static_cast<std::uint8_t>(options.keeper_action);
    auto& keeper = input.field.continuation.keeper;
    keeper.roll.random_first = 0;
    keeper.roll.random_second = 100;
    keeper.roll.range = content.keeper_range;
    keeper.roll.offset = content.keeper_offset;
    keeper.elapsed = 45;
    keeper.thresholds = content.keeper_thresholds;
    auto& outcome = input.field.continuation.outcome;
    outcome.ball_owner = 3;
    outcome.ball_x = 160;
    outcome.ball_y = 100;
    outcome.defenders = static_cast<std::uint8_t>(options.defenders);
    outcome.timer_selector = static_cast<std::uint8_t>(
        (content.attack.moves[0][index].cost_high_flags & 0xF8u) >> 3);
    outcome.punch_intervals = content.punch_intervals;
    const auto result = captain::resolve_complete_prepared_shot(input, content.field_players,
        content.attack, content.defense, content.goalkeepers, content.keeper);
    std::cout << "Power " << result.attack.action_power.power
              << " | field contacts " << result.flow.field.contacts.size()
              << " | remaining " << result.flow.field.remaining_power << '\n';
    if (result.keeper) {
        std::cout << "Keeper strength " << unsigned(result.keeper->comparison.strength)
                  << " | ratio " << unsigned(result.keeper->comparison.ratio)
                  << " | raw result " << unsigned(result.keeper->comparison.result_code) << '\n';
    }
    if (result.flow.continuation && result.flow.continuation->outcome) {
        const auto& resolved = *result.flow.continuation->outcome;
        std::cout << "Outcome: " << event_name(resolved.event)
                  << " | score " << unsigned(resolved.left_goals) << ':' << unsigned(resolved.right_goals)
                  << " | possession " << unsigned(resolved.possession_side)
                  << " | owner " << unsigned(resolved.ball_owner)
                  << " | ball " << unsigned(resolved.ball_x) << ',' << unsigned(resolved.ball_y) << '\n';
    } else {
        std::cout << "Field result " << unsigned(result.flow.field.result_code)
                  << " | route " << unsigned(result.flow.field_outcome.route)
                  << " | owner " << unsigned(result.flow.field_outcome.ball_owner)
                  << " (later field phase is outside this scene)\n";
    }
    std::cout << "Stamina: attacker " << result.flow.field.roster[3].stamina
              << " | keeper " << result.flow.field.roster[11].stamina << '\n';
}
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: captain-shot-lab PACK [--interactive] [--level N] [--keeper-level N] "
                     "[--subtype N] [--defenders 0..5] [--keeper-action 0..3]\n";
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
                     "Constructed side0 scene with explicit staged randomness and distance.\n"
                     "Each shot resets the scene; this is not a match or original outcome-script replay.\n";
        if (!interactive) {
            shoot(content, options);
            return 0;
        }
        std::cout << "Commands: shoot, level N, keeper-level N, subtype N, defenders N, keeper-action N, quit\n";
        std::string line;
        while (std::cout << "> " && std::getline(std::cin, line)) {
            std::istringstream command(line);
            std::string name, value, extra;
            if (!(command >> name)) continue;
            try {
                if (name == "shoot" || name == "quit") {
                    if (command >> extra) throw std::invalid_argument("Command takes no arguments");
                    if (name == "quit") break;
                    shoot(content, options);
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

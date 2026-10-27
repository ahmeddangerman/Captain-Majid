#include "captain/action_strength.hpp"
#include "captain/session.hpp"
#include "research_protocol.hpp"

#include <charconv>
#include <iostream>
#include <sstream>
#include <string>

namespace {
bool number(const std::string& token, std::uint32_t& value) {
    if (token.empty()) return false;
    // from_chars rejects signs/overflow; require consumption of the entire token.
    const auto parsed = std::from_chars(token.data(), token.data() + token.size(), value);
    return parsed.ec == std::errc{} && parsed.ptr == token.data() + token.size();
}

int research_batch(const std::string& command) {
    std::string line;
    while (std::getline(std::cin, line)) {
        const auto output = captain::research::evaluate(command, line);
        if (!output) {
            std::cerr << "Invalid research parameters for " << command
                      << "; see the research interface links in README.md\n";
            return 2;
        }
        std::cout << *output << '\n';
    }
    return std::cin.bad() ? 1 : 0;
}

int strength_batch() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream input(line);
        std::uint32_t power = 0, loss = 0;
        std::string p, l, extra;
        if (!(input >> p >> l) || (input >> extra) || !number(p, power) ||
            !number(l, loss) || power > 65535 || loss > 255) {
            std::cerr << "Expected power 0..65535 and loss 0..255\n";
            return 2;
        }
        std::cout << captain::reduce_action_strength(
            static_cast<std::uint16_t>(power), static_cast<std::uint8_t>(loss)) << '\n';
    }
    return std::cin.bad() ? 1 : 0;
}

int replica() {
    captain::Session session;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream input(line);
        std::string command, participant, sequence, extra;
        input >> command;
        if (command == "defender" || command == "skill" || command == "action" || command == "keeper" ||
            command == "energy" || command == "story" || command == "keeperstate" ||
            command == "random" || command == "outcome" || command == "defenderoutcome" ||
            command == "ballrecovery" || command == "defenderselection" || command == "defenderprep" ||
            command == "fieldaxis" || command == "playermove" || command == "clockstep") {
            std::string fields;
            std::getline(input, fields);
            const auto output = captain::research::evaluate(command, fields);
            std::cout << (output ? *output : "error malformed") << std::endl;
            continue;
        }
        if (command == "quit" && !(input >> extra)) return 0;
        if (command == "state" && !(input >> extra)) {
            std::cout << session.snapshot() << std::endl;
            continue;
        }
        std::uint32_t id = 0, serial = 0;
        if (command != "contact" || !(input >> participant >> sequence) ||
            (input >> extra) || !number(participant, id) || !number(sequence, serial)) {
            std::cout << "error malformed" << std::endl;
            continue;
        }
        const auto result = session.apply_contact(id, serial);
        if (result != captain::ContactReply::accepted) std::cout << "rejected ";
        std::cout << captain::reply_name(result) << std::endl;
    }
    return std::cin.bad() ? 1 : 0;
}
}

int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--strength-batch") return strength_batch();
    if (argc == 2 && std::string(argv[1]) == "--replica") return replica();
    if (argc == 2 && std::string(argv[1]) == "--defender-batch") return research_batch("defender");
    if (argc == 2 && std::string(argv[1]) == "--skill-index-batch") return research_batch("skill");
    if (argc == 2 && std::string(argv[1]) == "--action-power-batch") return research_batch("action");
    if (argc == 2 && std::string(argv[1]) == "--keeper-batch") return research_batch("keeper");
    if (argc == 2 && std::string(argv[1]) == "--energy-batch") return research_batch("energy");
    if (argc == 2 && std::string(argv[1]) == "--keeper-story-batch") return research_batch("story");
    if (argc == 2 && std::string(argv[1]) == "--keeper-state-batch") return research_batch("keeperstate");
    if (argc == 2 && std::string(argv[1]) == "--random-step-batch") return research_batch("random");
    if (argc == 2 && std::string(argv[1]) == "--outcome-batch") return research_batch("outcome");
    if (argc == 2 && std::string(argv[1]) == "--defender-outcome-batch") return research_batch("defenderoutcome");
    if (argc == 2 && std::string(argv[1]) == "--ball-recovery-batch") return research_batch("ballrecovery");
    if (argc == 2 && std::string(argv[1]) == "--defender-selection-batch") return research_batch("defenderselection");
    if (argc == 2 && std::string(argv[1]) == "--field-defender-preparation-batch") return research_batch("defenderprep");
    if (argc == 2 && std::string(argv[1]) == "--field-axis-batch") return research_batch("fieldaxis");
    if (argc == 2 && std::string(argv[1]) == "--player-movement-batch") return research_batch("playermove");
    if (argc == 2 && std::string(argv[1]) == "--match-clock-batch") return research_batch("clockstep");
    std::cerr << "Usage: captain-prototype --replica | --strength-batch | --defender-batch | "
                 "--skill-index-batch | --action-power-batch | --keeper-batch | --energy-batch | "
                 "--keeper-story-batch | --keeper-state-batch | --random-step-batch | --outcome-batch | "
                 "--defender-outcome-batch | --ball-recovery-batch | --defender-selection-batch | "
                 "--field-defender-preparation-batch | --field-axis-batch | --player-movement-batch | --match-clock-batch\n";
    return 2;
}

#include "captain/action_strength.hpp"
#include "captain/session.hpp"
#include "captain/defender_contact.hpp"

#include <charconv>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

namespace {
bool number(const std::string& token, std::uint32_t& value) {
    if (token.empty()) return false;
    // from_chars rejects signs/overflow; require consumption of the entire token.
    const auto parsed = std::from_chars(token.data(), token.data() + token.size(), value);
    return parsed.ec == std::errc{} && parsed.ptr == token.data() + token.size();
}

bool defender_input(std::istringstream& stream, captain::DefenderContactInput& input) {
    std::array<std::uint32_t, 15> values{};
    std::string token;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!(stream >> token) || !number(token, values[i]) || values[i] > (i < 2 ? 65535u : 255u)) {
            return false;
        }
    }
    if ((stream >> token) || values[6] > 1 || values[7] > 1) return false;
    input = {static_cast<std::uint16_t>(values[0]), static_cast<std::uint16_t>(values[1]),
             static_cast<std::uint8_t>(values[2]), static_cast<std::uint8_t>(values[3]),
             static_cast<std::uint8_t>(values[4]), static_cast<std::uint8_t>(values[5]),
             values[6] != 0, values[7] != 0,
             static_cast<std::uint8_t>(values[8]), static_cast<std::uint8_t>(values[9]),
             static_cast<std::uint8_t>(values[10]),
             {static_cast<std::uint8_t>(values[11]), static_cast<std::uint8_t>(values[12]),
              static_cast<std::uint8_t>(values[13]), static_cast<std::uint8_t>(values[14])}};
    return true;
}

bool defender_output(std::istringstream& stream, std::string& output) {
    captain::DefenderContactInput input{};
    if (!defender_input(stream, input)) return false;
    try {
        const auto result = captain::calculate_defender_contact(input);
        const std::array<unsigned, 8> values{result.defender_power, result.strength, result.loss,
            result.score, result.result_code, result.incoming_power, result.subtype,
            result.special ? 1u : 0u};
        output.clear();
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (i) output += ' ';
            output += std::to_string(values[i]);
        }
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    }
}

int defender_batch() {
    std::string line, result;
    while (std::getline(std::cin, line)) {
        std::istringstream input(line);
        if (!defender_output(input, result)) {
            std::cerr << "Invalid defender parameters; see docs/DEFENDER_CONTACT.md\n";
            return 2;
        }
        std::cout << result << '\n';
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
        if (command == "defender") {
            std::string result;
            std::cout << (defender_output(input, result) ? result : "error malformed") << std::endl;
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
    if (argc == 2 && std::string(argv[1]) == "--defender-batch") return defender_batch();
    std::cerr << "Usage: captain-prototype --replica | --strength-batch | --defender-batch\n";
    return 2;
}

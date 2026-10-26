#include "captain/action_strength.hpp"
#include "captain/session.hpp"

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
    std::cerr << "Usage: captain-prototype --replica | --strength-batch\n";
    return 2;
}

#include "captain/defender_sequence.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    const std::array<std::uint8_t, 4> regular{200, 100, 50, 0};
    const std::array<std::uint8_t, 4> terminal{255, 220, 180, 0};
    const std::array<std::uint8_t, 4> code_two{255, 150, 100, 0};
    const std::vector<captain::DefenderContactInput> contacts{
        {256, 256, 100, 100, 100, 0, false, false, 0, 1, 131, regular},
        {0, 1, 100, 100, 100, 0, false, false, 0, 2, 131, regular},
        {0, 256, 100, 100, 100, 0, false, false, 0, 3, 131, terminal},
        {0, 256, 100, 100, 100, 0, false, false, 0, 4, 131, regular}
    };
    const auto result = captain::resolve_field_contacts(contacts, 256, 0);
    if (result.contacts.size() != 3 || !result.stopped || result.result_code != 3 ||
        result.remaining_power != 231 || result.contacts[0].result_code != 1 ||
        result.contacts[0].incoming_power != 231 || result.contacts[1].result_code != 0 ||
        result.contacts[1].incoming_power != 231 || result.contacts[2].result_code != 3 ||
        result.contacts[2].incoming_power != 231 || result.contacts[2].subtype != 131) {
        std::cerr << "Field contact order, power carry, or terminal result is wrong: count="
                  << result.contacts.size() << " stop=" << result.stopped << " code="
                  << unsigned(result.result_code) << " power=" << result.remaining_power;
        for (const auto& contact : result.contacts)
            std::cerr << " [" << unsigned(contact.result_code) << ',' << contact.incoming_power
                      << ',' << unsigned(contact.subtype) << ']';
        std::cerr << '\n';
        return EXIT_FAILURE;
    }

    const auto empty = captain::resolve_field_contacts({}, 400, 7);
    if (!empty.contacts.empty() || empty.stopped || empty.remaining_power != 400 ||
        empty.result_code != 7) {
        std::cerr << "Empty field contact list should leave power for the next phase\n";
        return EXIT_FAILURE;
    }
    const std::vector<captain::DefenderContactInput> code_two_list{
        {160, 256, 100, 100, 100, 0, false, false, 0, 3, 3, code_two},
        {0, 256, 100, 100, 100, 0, false, false, 0, 4, 3, regular}
    };
    const auto stopped_two = captain::resolve_field_contacts(code_two_list, 160, 3);
    if (stopped_two.contacts.size() != 1 || !stopped_two.stopped ||
        stopped_two.result_code != 2) {
        std::cerr << "Field result code two must stop before later defenders\n";
        return EXIT_FAILURE;
    }
    std::cout << "Ordered defender contact checks passed\n";
}

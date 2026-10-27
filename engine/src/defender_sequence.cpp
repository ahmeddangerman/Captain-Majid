#include "captain/defender_sequence.hpp"

namespace captain {
DefenderSequenceResult resolve_field_contacts(
    const std::vector<DefenderContactInput>& inputs,
    std::uint16_t incoming_power, std::uint8_t initial_result_code) {
    DefenderSequenceResult result{{}, incoming_power, initial_result_code, false};
    result.contacts.reserve(inputs.size());
    for (const auto& fixture : inputs) {
        auto contact = fixture;
        contact.incoming_power = result.remaining_power;
        const auto calculated = calculate_defender_contact(contact);
        result.contacts.push_back(calculated);
        result.remaining_power = calculated.incoming_power;
        result.result_code = calculated.result_code;
        if (result.result_code >= 2) {
            result.stopped = true;
            break;
        }
    }
    return result;
}
}

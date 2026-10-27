#pragma once

#include "captain/defender_contact.hpp"

#include <cstdint>
#include <vector>

namespace captain {
struct DefenderSequenceResult {
    std::vector<DefenderContactResult> contacts;
    std::uint16_t remaining_power;
    std::uint8_t result_code;
    bool stopped;
};

// Apply already selected contacts in ROM order. Codes 0/1 continue; 2/3
// leave the contact loop for their caller-specific result handlers.
DefenderSequenceResult resolve_field_contacts(
    const std::vector<DefenderContactInput>& contacts,
    std::uint16_t incoming_power, std::uint8_t initial_result_code);
}

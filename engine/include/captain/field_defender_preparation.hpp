#pragma once

#include "captain/defender_contact.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace captain {
struct FieldDefenseMove {
    std::uint8_t effect;
    std::uint8_t skill_modifier;
    std::uint8_t cost_low;
    std::uint8_t cost_high_flags;
};

struct FieldDefenseContent {
    std::array<std::array<std::uint8_t, 3>, 5> skill_selectors;
    std::array<std::vector<FieldDefenseMove>, 5> moves;
    std::array<std::uint8_t, 192> strength_curve;
};

struct FieldDefenderPreparationInput {
    std::array<std::uint8_t, 31> base_skills;
    std::uint8_t level;
    std::uint8_t defense_action;
    std::uint8_t situation;
    std::uint8_t subtype;
};

struct FieldDefenderPreparationResult {
    std::uint8_t skill_selector;
    std::uint8_t skill_index;
    std::uint8_t base_strength;
    std::uint8_t move_effect;
    std::uint16_t stamina_cost;
};

struct PreparedFieldDefenderResult {
    FieldDefenderPreparationResult preparation;
    DefenderContactResult contact;
};

// Standard non-stamina skills only. Profiles and original/replacement content
// are supplied by the caller. Preparation does not consume stamina.
FieldDefenderPreparationResult prepare_field_defender(
    const FieldDefenderPreparationInput& input, const FieldDefenseContent& content);

// Uses prepared strength and the selected subtype for one contact. Other
// contact parameters, including the post-energy weakened flag, remain explicit.
PreparedFieldDefenderResult calculate_prepared_field_defender(
    const FieldDefenderPreparationInput& input, const FieldDefenseContent& content,
    const DefenderContactInput& contact);
}

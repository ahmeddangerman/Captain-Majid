#include "captain/field_defender_preparation.hpp"
#include "captain/shot_power.hpp"

#include <stdexcept>

namespace captain {
FieldDefenderPreparationResult prepare_field_defender(
    const FieldDefenderPreparationInput& input, const FieldDefenseContent& content) {
    if (input.defense_action >= content.moves.size() || input.situation >= 3)
        throw std::invalid_argument("Unsupported field defense action or situation");
    const auto skill = content.skill_selectors[input.defense_action][input.situation];
    if (skill == 0 || skill >= input.base_skills.size())
        throw std::invalid_argument("Standard field skill selector must be 1..30");
    const auto move_index = input.subtype & 63u;
    const auto& moves = content.moves[input.defense_action];
    if (move_index >= moves.size())
        throw std::invalid_argument("Missing field defense move");
    const auto& move = moves[move_index];
    const auto index = standard_skill_index(input.base_skills[skill], input.level,
                                            move.skill_modifier);
    const auto cost = static_cast<std::uint16_t>(move.cost_low |
        (static_cast<unsigned>(move.cost_high_flags & 3u) << 8));
    return {skill, index, content.strength_curve[index], move.effect, cost};
}

PreparedFieldDefenderResult calculate_prepared_field_defender(
    const FieldDefenderPreparationInput& input, const FieldDefenseContent& content,
    const DefenderContactInput& contact) {
    const auto preparation = prepare_field_defender(input, content);
    auto prepared_contact = contact;
    prepared_contact.defender_power = preparation.base_strength;
    prepared_contact.subtype = input.subtype;
    return {preparation, calculate_defender_contact(prepared_contact)};
}
}

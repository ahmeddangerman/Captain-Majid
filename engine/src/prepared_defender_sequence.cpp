#include "captain/prepared_defender_sequence.hpp"
#include <stdexcept>

namespace captain {
PreparedDefenderSequenceResult resolve_prepared_field_contacts(
    const FieldRoster& roster, const std::vector<FieldDefenseChoice>& choices,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& defense,
    const FieldDefenseContext& context) {
    PreparedDefenderSequenceResult result{roster, {}, context.incoming_power,
                                          context.initial_result_code, false};
    result.contacts.reserve(choices.size());
    for (const auto& choice : choices) {
        if (choice.player_id == 0 || choice.player_id == 11 || choice.player_id > 21)
            throw std::invalid_argument("Expected a field player ID");
        auto& player = result.roster[choice.player_id];
        const FieldPlayerRecord record{choice.player_id, player.type,
            static_cast<std::uint8_t>(player.stamina),
            static_cast<std::uint8_t>(player.stamina >> 8), player.level};
        const auto preparation = prepare_profiled_field_defender(record, profiles,
            choice.action, choice.situation, choice.subtype, defense);
        const auto energy = calculate_energy({player.stamina, preparation.stamina_cost,
            player.type, context.side, context.attack_action, context.attack_subtype,
            true, context.incoming_energy_carry});
        auto comparison = choice.comparison;
        comparison.incoming_power = result.remaining_power;
        comparison.defender_power = preparation.base_strength;
        comparison.defender_slot = choice.player_id;
        comparison.subtype = choice.subtype;
        comparison.weakened = energy.weakened;
        const auto contact = calculate_defender_contact(comparison);
        player.stamina = energy.stamina;
        result.contacts.push_back({choice.player_id, preparation, energy, contact});
        result.remaining_power = contact.incoming_power;
        result.result_code = contact.result_code;
        if (contact.result_code >= 2) {
            result.stopped = true;
            break;
        }
    }
    return result;
}
}

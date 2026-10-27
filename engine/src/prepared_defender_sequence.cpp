#include "captain/prepared_defender_sequence.hpp"
#include <stdexcept>

namespace captain {
PreparedDefenderSequenceResult resolve_prepared_field_contacts(
    const FieldRoster& roster, const std::vector<FieldDefenseChoice>& choices,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& defense,
    const FieldDefenseContext& context) {
    PreparedDefenderSequenceResult result{roster, {}, context.incoming_power,
                                          context.initial_result_code, false};
    if (context.interruption) {
        if (choices.size() > 10) throw std::invalid_argument("Too many selected defenders");
        result.interruption_state = context.interruption->state;
    }
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
        if (context.comparison_content) {
            comparison = configure_field_comparison(comparison, context.attack_action,
                choice.action, choice.player_id, *context.comparison_content);
        }
        const auto contact = compare_defender_contact(comparison);
        player.stamina = energy.stamina;
        std::optional<FieldInterruptionResult> interruption;
        if (context.interruption) {
            interruption = resolve_field_interruption({contact.result_code,
                static_cast<std::uint8_t>(choices.size()),choice.player_id,choice.action,
                context.side,context.interruption->ball_x,context.interruption->ball_y,
                comparison.random_first,*result.interruption_state});
            result.interruption_state = interruption->state;
        }
        result.contacts.push_back({choice.player_id, preparation, energy, contact,interruption});
        result.remaining_power = contact.incoming_power;
        result.result_code = interruption ? interruption->result_code : contact.result_code;
        if (result.result_code >= 2) {
            result.stopped = true;
            break;
        }
    }
    return result;
}
}

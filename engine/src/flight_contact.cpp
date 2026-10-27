#include "captain/flight_contact.hpp"
#include <algorithm>
#include <stdexcept>

namespace captain {
PreparedFlightResult resolve_prepared_flight_contacts(const FieldRoster& roster,
    const std::vector<FlightContactChoice>& choices, const FieldPlayerProfiles& profiles,
    const FieldDefenseContent& defense, const FlightContactContent& content,
    const FlightContactInput& input) {
    if ((input.side != 0 && input.side != 11) ||
        choices.size() > (input.side == 0 ? 5u : 4u))
        throw std::invalid_argument("Unsupported flight side/action/burst size");
    PreparedFlightResult result{{}, input.incoming_power, 0, false};
    result.contacts.reserve(choices.size());
    for (const auto& choice : choices) {
        const auto selected = std::find(content.defense_actions.begin(),
                                        content.defense_actions.end(), choice.action);
        if (choice.player_id == 0 || choice.player_id == 11 || choice.player_id > 21 ||
            (choice.player_id < 11 ? 0 : 11) == input.side || choice.action > 4 ||
            selected == content.defense_actions.end())
            throw std::invalid_argument("Unsupported opposing flight defender/action");
        const auto& player = roster[choice.player_id];
        const FieldPlayerRecord record{choice.player_id, player.type,
            static_cast<std::uint8_t>(player.stamina),
            static_cast<std::uint8_t>(player.stamina >> 8), player.level};
        const auto preparation = prepare_profiled_field_defender(record, profiles,
            choice.action, 0, 0, defense);
        const auto index = static_cast<std::size_t>(selected - content.defense_actions.begin());
        const auto& pair = content.variation[index];
        const auto contact = calculate_defender_contact({result.remaining_power,
            preparation.base_strength, choice.random_first, choice.random_second,
            pair[0], pair[1], false, bool(content.eligibility[index] & 128u),
            0, choice.player_id, 0, content.thresholds[index]});
        result.contacts.push_back({choice.player_id, choice.action, preparation, contact});
        result.remaining_power = contact.incoming_power;
        result.result_code = contact.result_code;
        if (result.result_code >= 2) {
            result.stopped = true;
            break;
        }
    }
    return result;
}
}

#include "captain/defensive_move_inspection.hpp"
#include "captain/defense_affordability.hpp"
#include <stdexcept>
namespace captain {
DefensiveMoveInspection inspect_defensive_move(const FieldPlayerRecord& player, std::uint16_t stamina,
    std::uint8_t action, std::uint8_t variant, std::uint8_t subtype,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& defense) {
    if(player.flags!=static_cast<std::uint8_t>(stamina) || player.alternate_type!=static_cast<std::uint8_t>(stamina>>8))
        throw std::invalid_argument("Player raw bytes must match the inspected stamina");
    const auto preparation=prepare_profiled_field_defender(player,profiles,action,variant,subtype,defense);
    return {preparation.stamina_cost,can_select_defense_move(stamina,preparation.stamina_cost,subtype),preparation.move_effect};
}
}

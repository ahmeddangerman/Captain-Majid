#include "captain/prepared_attack.hpp"
#include <stdexcept>
namespace captain {
PreparedAttackResult prepare_attacking_action(const PreparedAttackInput& input,
    const FieldPlayerProfiles& profiles, const FieldDefenseContent& content) {
    if (input.action >= content.moves.size() || input.situation >= 3 ||
        (input.side != 0 && input.side != 11))
        throw std::invalid_argument("Unsupported attack action, situation or side");
    auto situation = input.situation;
    if (input.action == 0 && (input.subtype & 0x7fu) >= 3) situation = 0;
    auto preparation = prepare_profiled_field_defender(input.player, profiles,
        input.action, situation, input.subtype, content);
    if (input.side == 0 && input.action == 2 && input.selected_defenders == 0)
        preparation.stamina_cost = 0;
    const auto stamina = static_cast<std::uint16_t>(input.player.flags |
        (static_cast<unsigned>(input.player.alternate_type) << 8));
    const auto energy = input.side == 0
        ? calculate_energy({stamina, preparation.stamina_cost, input.player.type,
            input.side, input.action, input.subtype, false, input.incoming_carry})
        : EnergyResult{stamina, preparation.stamina_cost, false, input.incoming_carry};
    const auto power_carry = input.side == 0 ? energy.carry : input.incoming_carry;
    const auto power = calculate_action_power({preparation.base_strength,
        input.random_first, input.random_second, energy.weakened,
        input.subtype, power_carry});
    const auto move_index = input.subtype & 63u;
    const auto move_flags = static_cast<std::uint8_t>(
        (content.moves[input.action][move_index].cost_high_flags & 0xf8u) >> 3);
    return {resolve_field_player_profile(input.player, profiles), preparation,
            energy, move_flags, power};
}
}

#include "captain/prepared_keeper.hpp"
#include "captain/shot_power.hpp"
#include <stdexcept>
namespace captain {
PreparedKeeperResult prepare_goalkeeper(const PreparedKeeperInput& input,
    const GoalkeeperProfiles& profiles, const GoalkeeperMoveContent& content) {
    if ((input.side != 0 && input.side != 11) ||
        input.player.player_id != (input.side ^ 11u))
        throw std::invalid_argument("Expected opposing keeper slot for possession side");
    if (input.defense_action >= content.moves.size())
        throw std::invalid_argument("Unsupported keeper defense action");
    const auto skill = content.skill_selectors[input.defense_action];
    if (skill < 23 || skill > 30)
        throw std::invalid_argument("Keeper standard skill must be 23..30");
    const auto profile = resolve_player_profile_reference(input.player, profiles);
    if (profile.profile_id >= profiles.skills.size())
        throw std::invalid_argument("Missing keeper skill profile");
    const auto index = input.defense_subtype & 63u;
    const auto& moves = content.moves[input.defense_action];
    if (index >= moves.size()) throw std::invalid_argument("Missing keeper move");
    const auto& move = moves[index];
    const auto skill_index = standard_skill_index(
        profiles.skills[profile.profile_id][skill - 23], input.player.level, move.skill_modifier);
    const FieldDefenderPreparationResult preparation{skill, skill_index,
        content.strength_curve[skill_index], move.effect,
        static_cast<std::uint16_t>(move.cost_low | ((unsigned(move.cost_high_flags) & 3u) << 8))};
    const auto stamina = static_cast<std::uint16_t>(input.player.flags |
        (unsigned(input.player.alternate_type) << 8));
    const auto energy = calculate_energy({stamina, preparation.stamina_cost, input.player.type,
        input.side, input.attack_action, input.attack_subtype, true, input.incoming_energy_carry});
    auto comparison = input.comparison;
    comparison.roll.base = preparation.base_strength;
    comparison.roll.subtype = input.defense_subtype;
    comparison.roll.defender_slot = input.player.player_id;
    comparison.roll.weakened = energy.weakened;
    comparison.critical_threshold = input.player.type == 33 || input.player.type == 64 ? 205 : 243;
    return {profile, preparation, energy, calculate_goalkeeper(comparison)};
}
}

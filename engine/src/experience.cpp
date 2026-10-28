#include "captain/experience.hpp"
#include <stdexcept>

namespace captain {
void validate_experience_state(const ExperienceState& state) {
    if (state.campaign >= 3 || state.opponent_stage >= 34)
        throw std::invalid_argument("Unsupported experience campaign/stage");
}
ExperienceRewardResult award_player_experience(const ExperienceState& state,
    std::uint8_t player_type, std::uint8_t selector, const ExperienceContent& content) {
    validate_experience_state(state);
    ExperienceRewardResult result{state};
    if (player_type == 0 || player_type >= 35) return result;
    if (selector >= 8) throw std::invalid_argument("Unsupported experience reward selector");
    const auto mapping_index = unsigned(content.campaign_offsets[state.campaign]) + player_type;
    if (mapping_index >= content.player_mappings.size())
        throw std::invalid_argument("Experience mapping outside content");
    const auto mapping = content.player_mappings[mapping_index];
    const auto slot = unsigned(mapping & 15u);
    const auto factor_index = unsigned(mapping >> 4) * 8u + selector;
    if (slot >= state.totals.size() || factor_index >= content.reward_factors.size())
        throw std::invalid_argument("Unsupported packed experience mapping");
    // AFC2 discards the stage base's high byte before its multiply. Preserve
    // the actual original scaling, including the discontinuity above255.
    const auto base = static_cast<std::uint8_t>(content.stage_bases[state.opponent_stage]) >> 2;
    const auto award = static_cast<std::uint8_t>((base * content.reward_factors[factor_index]) >> 6);
    const auto total = unsigned(state.totals[slot]) + award;
    result.slot = static_cast<std::uint8_t>(slot);
    result.award = award;
    result.saturated = total > 65535u;
    result.state.totals[slot] = static_cast<std::uint16_t>(result.saturated ? 65535u : total);
    return result;
}
}

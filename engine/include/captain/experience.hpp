#pragma once
#include <array>
#include <cstdint>
#include <optional>

namespace captain {
struct ExperienceState {
    std::uint8_t campaign = 0, opponent_stage = 0;
    std::array<std::uint16_t, 11> totals{};
};
struct ExperienceContent {
    std::array<std::uint8_t, 3> campaign_offsets;
    std::array<std::uint8_t, 70> player_mappings;
    std::array<std::uint8_t, 48> reward_factors;
    std::array<std::uint16_t, 34> stage_bases;
};
struct ExperienceRewardResult {
    ExperienceState state;
    std::optional<std::uint8_t> slot{};
    std::uint8_t award = 0;
    bool saturated = false;
};
void validate_experience_state(const ExperienceState& state);
// Original named-type gate and byte scaling; this does not change player levels.
ExperienceRewardResult award_player_experience(const ExperienceState& state,
    std::uint8_t player_type, std::uint8_t selector, const ExperienceContent& content);
}

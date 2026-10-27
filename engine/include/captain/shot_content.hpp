#pragma once
#include "captain/field_comparison_content.hpp"
#include "captain/prepared_keeper.hpp"
#include "captain/shot_approach.hpp"
#include <istream>
#include <optional>

namespace captain {
struct ShotContent {
    std::array<std::uint8_t, 32> source_payload_sha256;
    FieldPlayerProfiles field_players;
    GoalkeeperProfiles goalkeepers;
    FieldDefenseContent attack, defense;
    GoalkeeperMoveContent keeper;
    FieldComparisonContent field_comparison;
    std::uint8_t keeper_range, keeper_offset;
    std::array<std::uint8_t, 4> keeper_thresholds;
    std::array<std::uint8_t, 8> punch_intervals;
    std::optional<ShotApproachContent> approach{};
};

// Development CMJSHOT1/2 content formats. Reject truncated/oversized sections,
// unsupported versions and trailing data; simulation receives ordinary values.
ShotContent load_shot_content(std::istream& input);
}

#pragma once
#include "captain/field_comparison_content.hpp"
#include "captain/prepared_keeper.hpp"
#include "captain/shot_approach.hpp"
#include "captain/flight_contact.hpp"
#include "captain/field_phase.hpp"
#include "captain/contact_prelude.hpp"
#include "captain/experience.hpp"
#include "captain/control_input.hpp"
#include "captain/control_tick.hpp"
#include "captain/action_menu.hpp"
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
    std::optional<FlightContactContent> flight{};
    std::optional<FieldPhaseContent> movement{};
    std::optional<ContactPreludeContent> contact{};
    std::optional<ExperienceContent> experience{};
    std::optional<ControlMovementContent> control_movement{};
    std::optional<WorldStepContent> world_step{};
    std::optional<ActionMenuContent> action_menu{};
};

// Development CMJSHOT1..9 content formats. Reject truncated/oversized sections,
// unsupported versions and trailing data; simulation receives ordinary values.
ShotContent load_shot_content(std::istream& input);
}

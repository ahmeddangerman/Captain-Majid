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
#include "captain/ai_action_candidate.hpp"
#include "captain/clearance_race.hpp"
#include "captain/kickoff_formation.hpp"
#include <istream>
#include <memory>
#include <optional>

namespace captain {
struct RuntimeKickoffContent { KickoffFormationContent formation{}; std::array<std::uint8_t,5> receivers{}; };
struct ShotContent {
    std::shared_ptr<const AiKeeperContent> ai_keeper_choice{};
    std::optional<AiPassDefenseContent> ai_pass_defense{};
    std::optional<FieldComparisonSelection> ordinary_pass_defender_comparison{};
    std::optional<FieldComparisonSelection> ordinary_pass_block_comparison{};
    std::optional<FieldComparisonSelection> ordinary_pass_intercept_comparison{};
    std::optional<std::array<std::uint8_t,11>> first_match_lineup{};
    std::shared_ptr<const RuntimeKickoffContent> kickoff{};
    std::optional<AiAreaChoiceContent> ai_area_choice{};
    std::optional<AiGoalAreaChoiceContent> ai_goal_area_choice{};
    std::optional<AiFieldChoiceContent> ai_field_choice{};
    std::optional<FieldDefenseContent> area_attack6{};
    std::optional<AreaComparisonContent> area_comparison{};
    std::optional<std::array<std::uint8_t,7>> area_prefix_elapsed{};
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
    std::optional<PassFlightComparisonContent> pass_flight_comparison{};
    std::shared_ptr<const ClearanceRaceContent> pass_race{};
    std::optional<FieldPhaseContent> movement{};
    std::optional<ContactPreludeContent> contact{};
    std::optional<ExperienceContent> experience{};
    std::optional<ControlMovementContent> control_movement{};
    std::optional<WorldStepContent> world_step{};
    std::optional<ActionMenuContent> action_menu{};
    std::optional<AreaDefenseMenuContent> area_defense_menu{};
};

// Development CMJSHOT1..9/A..Z/a..b content formats. Reject truncated/oversized sections,
// unsupported versions and trailing data; simulation receives ordinary values.
ShotContent load_shot_content(std::istream& input);
}

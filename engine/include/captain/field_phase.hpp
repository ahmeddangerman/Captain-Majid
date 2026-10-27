#pragma once
#include "captain/prepared_defender_sequence.hpp"
#include "captain/player_movement.hpp"
#include "captain/movement_profile.hpp"
#include "captain/field_targets.hpp"
#include "captain/keeper_state.hpp"

namespace captain {
using FieldMotionRoster = std::array<FieldMotionState, 20>; // IDs1..10,12..21.
struct FieldPhaseContent {
    MovementGeometryContent geometry;
    MovementProfiles profiles;
    FieldTargetContent targets;
};
struct FieldPhaseState {
    FieldRoster roster{};
    FieldMotionRoster motion{};
    std::array<KeeperRecoveryState, 2> keepers{};
    std::array<FieldTeamShape, 2> teams{};
    std::uint8_t side = 0, ball_owner = 3, ball_x = 160, ball_y = 100, ball_cell = 0;
    std::uint8_t elapsed = 0, random_first = 0, random_second = 0;
    std::uint8_t selected_defenders = 0, movement_region = 0;
};
struct FieldAdvancedPlayer {
    std::uint8_t player_id;
    MovementProfileResult profile;
    FieldTargetResult target;
    FieldPlayerMoveResult movement;
};
struct FieldPhaseResult {
    FieldPhaseState state;
    std::vector<FieldAdvancedPlayer> players;
};
std::size_t field_motion_index(std::uint8_t player_id);
// Supported ordinary in-field E074 phase; consumes supplied elapsed, preserves
// keeper roles/stats and does not decide a match clock or presentation schedule.
FieldPhaseResult advance_field_phase(const FieldPhaseState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& content);
}

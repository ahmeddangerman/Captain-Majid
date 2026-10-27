#pragma once
#include <cstdint>
#include <array>

namespace captain {
enum class FieldAxis : std::uint8_t { horizontal = 0, vertical = 1 };
// Raw16-bit coordinate/displacement; wraps the addition, clamps only the high
// byte and preserves the fractional low byte. Does not advance a match tick.
std::uint16_t advance_field_coordinate(std::uint16_t coordinate,
                                      std::uint16_t displacement, FieldAxis axis);

struct FieldMotionState {
    std::uint16_t x = 0, y = 0;
    std::uint8_t target_cell = 0, status = 0;
};
struct MovementGeometryContent {
    std::array<std::uint16_t, 64> direction_thresholds;
    std::array<std::uint8_t, 8> axis_signs;
};
struct FieldPlayerMoveInput {
    FieldMotionState player;
    std::uint16_t speed;
    std::uint8_t elapsed, ball_cell;
};
struct FieldPlayerMoveResult {
    FieldMotionState player;
    std::uint8_t direction = 0;
    std::uint16_t iterations = 0;
    bool direction_computed = false;
};
std::uint8_t calculate_player_direction(const FieldMotionState& player,
    std::uint8_t ball_cell, const MovementGeometryContent& content);
// Supported E854 movement under the E074 caller: zero elapsed is a no-op.
// Speed and target decisions are explicit; no RNG or world clock is advanced.
FieldPlayerMoveResult advance_field_player(const FieldPlayerMoveInput& input,
    const MovementGeometryContent& content);
}

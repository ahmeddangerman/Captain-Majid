#pragma once
#include "captain/ball_velocity.hpp"
namespace captain {
struct BallStepInput {
 std::uint16_t x, y;
 BallVelocity velocity;
 std::uint8_t current_cell, destination;
};
enum class BallStepEvent { already_at_destination, cell_changed, arrived };
struct BallStepResult {
 std::uint16_t x, y;
 std::uint8_t cell;
 std::uint32_t additions;
 BallStepEvent event;
 bool forced_destination;
};
// Prepared originalE778 numeric loop. Caller owns frame/cell/arrival callbacks.
BallStepResult advance_ball_to_next_cell(const BallStepInput& input);
}

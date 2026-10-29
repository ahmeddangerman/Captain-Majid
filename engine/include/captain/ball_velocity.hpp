#pragma once
#include <array>
#include <cstdint>
namespace captain {
struct BallVelocityContent { std::array<std::uint16_t,64> quarter_wave{}; };
struct BallVelocity { std::uint16_t x, y; };
// OriginalCE4A/CE4D projection, raw wrapping two's-complement displacements.
// Does not add coordinates, clamp to field, consume random values or advance time.
BallVelocity calculate_ball_velocity(std::uint8_t direction, const BallVelocityContent& content);
}

#pragma once
#include <cstdint>

namespace captain {
enum class FieldAxis : std::uint8_t { horizontal = 0, vertical = 1 };
// Raw16-bit coordinate/displacement; wraps the addition, clamps only the high
// byte and preserves the fractional low byte. Does not advance a match tick.
std::uint16_t advance_field_coordinate(std::uint16_t coordinate,
                                      std::uint16_t displacement, FieldAxis axis);
}

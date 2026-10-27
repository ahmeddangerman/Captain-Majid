#include "captain/player_movement.hpp"
#include <stdexcept>

namespace captain {
std::uint16_t advance_field_coordinate(std::uint16_t coordinate,
                                      std::uint16_t displacement, FieldAxis axis) {
    if (axis != FieldAxis::horizontal && axis != FieldAxis::vertical)
        throw std::invalid_argument("Unsupported field coordinate axis");
    const auto sum = static_cast<std::uint16_t>(std::uint32_t(coordinate) + displacement);
    auto high = unsigned(sum >> 8);
    const unsigned minimum = axis == FieldAxis::horizontal ? 48 : 80;
    const unsigned maximum = axis == FieldAxis::horizontal ? 208 : 176;
    if (high < minimum) high = minimum;
    else if (high >= maximum) high = maximum - 1;
    return static_cast<std::uint16_t>((high << 8) | (sum & 255u));
}
}

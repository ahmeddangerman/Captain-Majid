#include "captain/player_movement.hpp"
#include "captain/defender_selection.hpp"
#include <algorithm>
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

namespace {
std::uint16_t axis_displacement(std::uint8_t direction, std::uint16_t speed,
                                const MovementGeometryContent& content) {
    const auto sector = static_cast<std::uint8_t>(unsigned(direction) + 16u) >> 5;
    const auto sign = content.axis_signs[sector];
    if (sign > 2) throw std::invalid_argument("Movement axis sign must be0,1 or2");
    if (sign == 0) return 0;
    return sign == 1 ? speed : static_cast<std::uint16_t>(0u - unsigned(speed));
}
}

std::uint8_t calculate_player_direction(const FieldMotionState& player,
    std::uint8_t ball_cell, const MovementGeometryContent& content) {
    const auto x = static_cast<std::uint8_t>(player.x >> 8);
    const auto y = static_cast<std::uint8_t>(player.y >> 8);
    const auto cell = field_cell_at(x, y);
    if (cell == player.target_cell) return cell;
    if (content.direction_thresholds.back() != 65535 ||
        !std::is_sorted(content.direction_thresholds.begin(), content.direction_thresholds.end()))
        throw std::invalid_argument("Movement direction thresholds need sorted full-word coverage");
    const auto target = player.target_cell == 240 ? ball_cell : player.target_cell;
    const unsigned target_x = (target / 12u) * 8u + 52u;
    const unsigned target_y = (target % 12u) * 8u + 84u;
    const unsigned dx = x < target_x ? target_x - x : x - target_x;
    const unsigned dy = y < target_y ? target_y - y : y - target_y;
    const auto ratio = dx == 0 ? 65535u : (dy * 256u) / dx;
    const auto found = std::lower_bound(content.direction_thresholds.begin(),
                                        content.direction_thresholds.end(), ratio);
    auto direction = static_cast<std::uint8_t>(found - content.direction_thresholds.begin());
    if (x >= target_x) direction = static_cast<std::uint8_t>((direction ^ 255u) & 127u);
    if (y >= target_y) direction ^= 255u;
    return direction;
}

FieldPlayerMoveResult advance_field_player(const FieldPlayerMoveInput& input,
    const MovementGeometryContent& content) {
    FieldPlayerMoveResult result{input.player};
    if (input.player.status != 0 || input.elapsed == 0) return result;
    result.direction = calculate_player_direction(input.player, input.ball_cell, content);
    result.direction_computed = true;
    const auto status_delta = static_cast<std::uint8_t>(unsigned(input.player.status) - input.elapsed);
    const bool may_move = (status_delta & 128u) != 0 &&
        static_cast<std::uint8_t>(0u - unsigned(status_delta)) != 0;
    for (unsigned step = 0; step < input.elapsed; ++step) {
        const auto cell = field_cell_at(static_cast<std::uint8_t>(result.player.x >> 8),
                                        static_cast<std::uint8_t>(result.player.y >> 8));
        if (cell == result.player.target_cell ||
            (result.player.target_cell == 240 && cell == input.ball_cell)) break;
        const auto vertical = axis_displacement(result.direction, input.speed, content);
        const auto horizontal = axis_displacement(
            static_cast<std::uint8_t>(unsigned(result.direction) + 64u), input.speed, content);
        if (may_move) {
            result.player.y = advance_field_coordinate(result.player.y, vertical, FieldAxis::vertical);
            result.player.x = advance_field_coordinate(result.player.x, horizontal, FieldAxis::horizontal);
        }
        ++result.iterations;
    }
    return result;
}
}

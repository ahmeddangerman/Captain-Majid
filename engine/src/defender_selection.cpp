#include "captain/defender_selection.hpp"

#include <stdexcept>

namespace captain {
std::vector<std::uint8_t> select_field_defenders(const DefenderSelectionInput& input) {
    if (input.possession_side != 0 && input.possession_side != 11)
        throw std::invalid_argument("possession side must be 0 or 11");

    const std::uint8_t first_id = input.possession_side == 0 ? 12 : 1;
    const std::size_t first_index = input.possession_side == 0 ? 10 : 0;
    const std::size_t capacity = input.possession_side == 0 ? 5 : 4;
    std::vector<std::uint8_t> selected;
    selected.reserve(capacity);

    for (std::size_t offset = 0; offset < 10 && selected.size() < capacity; ++offset) {
        const auto& player = input.players[first_index + offset];
        if (player.status_nonzero) continue;

        const auto dx = player.x > input.ball_x ? player.x - input.ball_x : input.ball_x - player.x;
        const auto dy = player.y > input.ball_y ? player.y - input.ball_y : input.ball_y - player.y;
        if (dx < input.radius && dy < input.radius)
            selected.push_back(static_cast<std::uint8_t>(first_id + offset));
    }
    return selected;
}

std::uint8_t field_cell_at(std::uint8_t x, std::uint8_t y) {
    if (x < 48 || x >= 208 || y < 80 || y >= 176) return 255;
    return static_cast<std::uint8_t>(((x - 48u) >> 3) * 12u + ((y - 80u) >> 3));
}

std::vector<std::uint8_t> select_flight_defenders(const FlightDefenderSelectionInput& input) {
    if (input.possession_side != 0 && input.possession_side != 11)
        throw std::invalid_argument("possession side must be0 or11");
    const std::uint8_t first_id = input.possession_side == 0 ? 12 : 1;
    const std::size_t first_index = input.possession_side == 0 ? 10 : 0;
    const std::size_t capacity = input.possession_side == 0 ? 5 : 4;
    std::vector<std::uint8_t> selected;
    selected.reserve(capacity);
    // Both original count checks leave carry clear before SBC, adding borrow1.
    const auto sample = static_cast<std::uint8_t>(
        unsigned(input.random_first) - input.random_second - 1u);
    for (std::size_t offset = 0; offset < 10 && selected.size() < capacity; ++offset) {
        const auto& player = input.players[first_index + offset];
        if (player.status_nonzero || field_cell_at(player.x, player.y) != input.ball_cell)
            continue;
        if (sample < input.threshold)
            selected.push_back(static_cast<std::uint8_t>(first_id + offset));
    }
    return selected;
}
}

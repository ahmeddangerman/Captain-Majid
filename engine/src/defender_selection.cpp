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
}

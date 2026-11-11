#include "captain/defender_selection.hpp"

#include <stdexcept>
#include <charconv>
#include <string>

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

std::array<std::uint8_t,9> load_goal_area_keeper_cells(std::istream& input) {
    std::string token;
    if(!(input>>token) || token!="CMJGKAREA1")throw std::invalid_argument("Invalid goal-area keeper content header");
    std::array<std::uint8_t,9> cells{};
    for(auto& cell:cells) {
        if(!(input>>token))throw std::invalid_argument("Truncated goal-area keeper content");
        unsigned value=0;const auto parsed=std::from_chars(token.data(),token.data()+token.size(),value);
        if(parsed.ec!=std::errc{} || parsed.ptr!=token.data()+token.size() || value>=240)
            throw std::invalid_argument("Goal-area keeper cell must be an integer from 0 to 239");
        cell=static_cast<std::uint8_t>(value);
    }
    if(input>>token)throw std::invalid_argument("Trailing goal-area keeper content");
    if(input.bad())throw std::invalid_argument("Unable to read goal-area keeper content");
    return cells;
}

bool select_goal_area_keeper(const GoalAreaKeeperSelectionInput& input,const std::array<std::uint8_t,9>& cells) {
    if (input.possession_side!=0 && input.possession_side!=11)
        throw std::invalid_argument("Goal-area keeper selection requires side0/11");
    const auto capacity=input.possession_side==0 ? 5u : 4u;
    if (input.selected_count>capacity)throw std::invalid_argument("Goal-area defender prefix exceeds capacity");
    for(auto cell:cells)if(cell>=240)throw std::invalid_argument("Goal-area keeper cells must be on the field");
    if(input.keeper_unavailable || input.selected_count==capacity)return false;
    const auto mirrored=[](std::uint8_t value) {
        return static_cast<std::uint8_t>((value&128u) ? 0u-unsigned(value) : unsigned(value));
    };
    const auto cell=field_cell_at(mirrored(input.ball_x),mirrored(input.ball_y));
    for(int i=8;i>=0;--i)if(cells[static_cast<unsigned>(i)]==cell)
        return input.random_first<=static_cast<unsigned>(i>=6 ? 85 : 51);
    return false;
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
std::vector<std::uint8_t> accumulate_flight_defenders(const FlightDefenderSelectionInput& input,const std::vector<std::uint8_t>& prior) {
    if(input.possession_side!=0 && input.possession_side!=11)throw std::invalid_argument("Invalid flight side");
    const auto capacity=input.possession_side==0 ? 5u : 4u;
    if(prior.size()>capacity)throw std::invalid_argument("Invalid flight defender prefix");
    for(const auto id:prior)if((input.possession_side==0 && (id<12 || id>21)) || (input.possession_side==11 && (id<1 || id>10)))
        throw std::invalid_argument("Invalid flight defender ID");
    auto result=prior;
    for(const auto id:select_flight_defenders(input)) {if(result.size()==capacity)break;result.push_back(id);}
    return result;
}
}

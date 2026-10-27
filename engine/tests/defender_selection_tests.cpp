#include "captain/defender_selection.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

namespace {
captain::DefenderSelectionInput empty_field() {
    captain::DefenderSelectionInput input{};
    input.ball_x = 120;
    input.ball_y = 100;
    input.radius = 10;
    input.players.fill({200, 200, false});
    return input;
}
}

int main() {
    {
        auto input = empty_field();
        input.possession_side = 0;
        for (std::size_t i = 10; i < input.players.size(); ++i)
            input.players[i] = {120, 100, false};
        assert((captain::select_field_defenders(input) ==
                std::vector<std::uint8_t>{12, 13, 14, 15, 16}));
    }

    {
        auto input = empty_field();
        input.possession_side = 11;
        for (std::size_t i = 0; i < 10; ++i)
            input.players[i] = {120, 100, false};
        assert((captain::select_field_defenders(input) ==
                std::vector<std::uint8_t>{1, 2, 3, 4}));
    }

    {
        auto input = empty_field();
        input.possession_side = 0;
        input.players[10] = {129, 109, false}; // Both deltas are radius - 1.
        input.players[11] = {130, 100, false}; // X is exactly at the radius.
        input.players[12] = {120, 90, false};  // Y is exactly at the radius.
        input.players[13] = {111, 99, false};
        input.players[14] = {120, 100, true};  // Nonzero record status skips it.
        input.players[15] = {120, 100, false};
        assert((captain::select_field_defenders(input) ==
                std::vector<std::uint8_t>{12, 15, 17}));
    }

    {
        auto input = empty_field();
        input.possession_side = 0;
        input.radius = 0;
        input.players.fill({120, 100, false});
        assert(captain::select_field_defenders(input).empty());
    }

    {
        auto input = empty_field();
        input.possession_side = 1;
        bool rejected = false;
        try {
            (void) captain::select_field_defenders(input);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        assert(rejected);
    }

    std::cout << "defender selection checks passed\n";
}

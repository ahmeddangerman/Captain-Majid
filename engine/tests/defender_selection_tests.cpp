#include "captain/defender_selection.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sstream>

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

    {
        std::array<std::uint8_t,9> cells{29,0,1,2,3,4,5,6,7};
        captain::GoalAreaKeeperSelectionInput input{0,188,124,51,0,false};
        assert(captain::select_goal_area_keeper(input,cells));
        input.random_first=52;assert(!captain::select_goal_area_keeper(input,cells));
        cells[0]=0;cells[7]=29;input.random_first=85;assert(captain::select_goal_area_keeper(input,cells));
        input.random_first=86;assert(!captain::select_goal_area_keeper(input,cells));
        input.random_first=0;input.keeper_unavailable=true;assert(!captain::select_goal_area_keeper(input,cells));
        input.keeper_unavailable=false;input.selected_count=5;assert(!captain::select_goal_area_keeper(input,cells));
        input.possession_side=11;input.selected_count=4;assert(!captain::select_goal_area_keeper(input,cells));
        input.selected_count=0;input.ball_y=40;assert(!captain::select_goal_area_keeper(input,cells));
        input.possession_side=1;bool rejected=false;
        try{(void)captain::select_goal_area_keeper(input,cells);}catch(const std::invalid_argument&){rejected=true;}
        assert(rejected);
    }

    {
        std::istringstream valid("CMJGKAREA1 1 2 3 4 5 6 7 8 9");
        assert((captain::load_goal_area_keeper_cells(valid)==std::array<std::uint8_t,9>{1,2,3,4,5,6,7,8,9}));
        for(const char* text:{"CMJGKAREA1 1 2 3", "CMJGKAREA1 1 2 3 4 5 6 7 8 -1", "CMJGKAREA1 1 2 3 4 5 6 7 8 240", "CMJGKAREA1 1 2 3 4 5 6 7 8 9 extra"}) {
            std::istringstream bad(text);bool rejected=false;
            try{(void)captain::load_goal_area_keeper_cells(bad);}catch(const std::invalid_argument&){rejected=true;}
            assert(rejected);
        }
    }

    std::cout << "defender selection checks passed\n";
}

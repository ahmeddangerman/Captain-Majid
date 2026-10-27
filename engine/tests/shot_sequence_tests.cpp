#include "captain/shot_sequence.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    const captain::ShotSequenceInput input{
        {100, 30, 1, 0, 0, 0, false},
        {128, 100, 100, true, 0, false},
        {0, {256, 100, 100, 200, 50, false, false, 4, 0, 0},
         240, 10, 3, 4, {200, 180, 100, 0}},
        {0, 0, 4, 0, 99, 0, 100, 100, 2, 255, 255, 9,
         7, 1, 2, 0, 30, 40, 3, 1, 0, {2, 4, 5, 7, 8, 9, 10, 11}}
    };
    const auto result = captain::resolve_explicit_shot(input);
    if (result.energy.stamina != 70 || result.energy.weakened ||
        result.action.power != 114 || result.keeper.result_code != 2 ||
        result.outcome.event != captain::ShotEvent::rebound ||
        result.outcome.keeper_timer != 21 || result.outcome.keeper_penalty != 4 ||
        result.outcome.defenders != 0 || result.outcome.possession_side != 0 ||
        result.outcome.left_goals != 1 || result.outcome.right_goals != 2) {
        std::cerr << "Explicit shot did not link energy, power, keeper, and rebound state\n";
        return EXIT_FAILURE;
    }
    std::cout << "Explicit shot sequence check passed\n";
}

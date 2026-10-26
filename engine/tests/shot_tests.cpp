#include "captain/shot_power.hpp"
#include "captain/goalkeeper.hpp"

#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
}

int main() {
    using captain::standard_skill_index;
    require(standard_skill_index(10, 5, 3) == 23, "level and move adjustment");
    require(standard_skill_index(250, 5, 0) == 4, "level adjustment wraps to byte");
    require(standard_skill_index(0, 128, 0) == 1, "level shift carry retained");
    require(standard_skill_index(100, 50, 0) == 191, "first index cap");
    require(standard_skill_index(190, 0, 100) == 34, "move adjustment wraps before cap");
    require(standard_skill_index(190, 0, 5) == 191, "second index cap");

    captain::ActionPowerInput attack{256, 100, 100, false, 0, false};
    auto power = captain::calculate_action_power(attack);
    require(power.multiplier == 229 && power.power == 229 && power.subtype == 0,
            "ordinary action rounding");
    attack.random_first = 7; attack.random_second = 100;
    power = captain::calculate_action_power(attack);
    require(power.multiplier == 310 && power.subtype == 128, "rare attack marks critical subtype");
    attack.random_first = 8;
    require(captain::calculate_action_power(attack).subtype == 0, "critical excludes eight");
    attack = {256, 0, 0, true, 0, false};
    power = captain::calculate_action_power(attack);
    require(power.base == 16 && power.multiplier == 128 && power.power == 8 && power.subtype == 0,
            "weakening suppresses new critical flag");
    attack.incoming_carry = true;
    require(captain::calculate_action_power(attack).multiplier == 129, "weak branch retains entry carry");
    attack = {65535, 255, 255, false, 128, false};
    power = captain::calculate_action_power(attack);
    require(power.multiplier == 384 && power.power == 32766, "action word truncates product");

    captain::GoalkeeperInput keeper{300, {256, 100, 100, 200, 50, false, false, 4, 0, 3},
                                   240, 10, 3, 4, {200, 180, 100, 0}};
    auto save = captain::calculate_goalkeeper(keeper);
    require(save.strength == 150 && save.ratio == 255 && save.state_adjustment == 11 && save.score == 255,
            "keeper uses distinct numerator and state adjustment");
    keeper.incoming_power = 100;
    save = captain::calculate_goalkeeper(keeper);
    require(save.ratio == 128 && save.score == 139 && save.result_code == 2,
            "keeper score classification");
    keeper.critical_threshold = 99;
    require(captain::calculate_goalkeeper(keeper).special, "keeper critical above threshold");
    keeper.critical_threshold = 100;
    require(!captain::calculate_goalkeeper(keeper).special, "keeper equality is not critical");
    keeper.roll.weakened = true; keeper.critical_threshold = 0;
    require(!captain::calculate_goalkeeper(keeper).special, "weak keeper suppresses critical");
    keeper.roll.weakened = false; keeper.roll.base = 0;
    require(captain::calculate_goalkeeper(keeper).ratio == 255, "zero keeper strength divider");
    keeper.stored_timer = 2; keeper.elapsed = 3; keeper.stored_penalty = 4;
    require(captain::calculate_goalkeeper(keeper).state_adjustment == 4, "timer subtraction floors at zero");
    keeper.stored_timer = 255; keeper.elapsed = 0; keeper.stored_penalty = 255;
    require(captain::calculate_goalkeeper(keeper).state_adjustment == 255, "state adjustment saturates");
    std::cout << "Shot power and keeper checks passed\n";
}

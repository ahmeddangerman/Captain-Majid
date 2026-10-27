#include "captain/ball_recovery.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
captain::BallRecoveryInput open_field() {
    captain::BallRecoveryInput input{};
    input.ball_x = 120;
    input.ball_y = 100;
    input.ball_owner = 22;
    input.opposing_player = 22;
    input.players.fill({120, 100, false});
    return input;
}
}

int main() {
    {
        auto input = open_field();
        input.players[0] = {130, 100, false}; // player 1, radius 16
        input.players[1] = {124, 100, false}; // player 2, radius 8
        input.players[10] = {125, 100, false}; // player 12, radius 8
        const auto result = captain::recover_ball(input);
        assert(result.ball_owner == 2);
        assert(result.possession_side == 0);
        assert(!result.direct_defender_capture);
    }

    {
        auto input = open_field();
        input.random_e2 = 0x80; // zero jitter; the equal-distance tie favors side 11.
        input.players[0] = {124, 100, false};
        input.players[10] = {125, 100, false};
        const auto result = captain::recover_ball(input);
        assert(result.ball_owner == 12);
        assert(result.possession_side == 11);
    }

    {
        auto input = open_field();
        input.ball_owner = 1;
        input.opposing_player = 12;
        const auto result = captain::recover_ball(input);
        assert(result.ball_owner == 2);
        assert(result.possession_side == 0);
    }

    {
        auto input = open_field();
        input.random_e2 = 0x20; // Below the ROM's $40 direct-capture threshold.
        input.selected_defenders = {{7, 4}, {5, 5}, {8, 5}};
        const auto result = captain::recover_ball(input);
        assert(result.ball_owner == 5);
        assert(result.possession_side == 0);
        assert(result.attack_subtype == 0);
        assert(result.direct_defender_capture);
    }

    {
        auto input = open_field();
        input.random_e2 = 0x20;
        input.selected_defenders = {{11, 5}}; // Goalkeeper is not a field-ball capture.
        const auto result = captain::recover_ball(input);
        assert(result.ball_owner == 1);
        assert(!result.direct_defender_capture);
    }

    {
        auto input = open_field();
        input.ball_y = 120;
        input.random_e2 = 0x83;
        input.random_e3 = 0x02;
        input.players[0] = {140, 100, false};
        input.players[10] = {140, 100, false};
        const auto result = captain::recover_ball(input);
        assert(result.ball_x == 136);
        assert(result.ball_y == 96);
        assert(result.ball_owner == 12);
    }

    {
        auto input = open_field();
        input.random_e2 = 0x20;
        input.selected_defenders = {{22, 5}};
        bool rejected = false;
        try {
            (void) captain::recover_ball(input);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        assert(rejected);
    }

    std::cout << "ball recovery checks passed\n";
}

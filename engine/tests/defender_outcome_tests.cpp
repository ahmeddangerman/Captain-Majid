#include "captain/defender_outcome.hpp"
#include "captain/ball_recovery.hpp"

#include <cstdlib>
#include <iostream>

namespace {
bool same(const captain::DefenderOutcomeResult& a, captain::DefenderRoute route,
          std::uint8_t side, std::uint8_t owner) {
    return a.route == route && a.possession_side == side && a.ball_owner == owner;
}
}

int main() {
    using captain::DefenderRoute;
    if (!same(captain::resolve_defender_outcome({0, 0, 7, 2, 0}),
              DefenderRoute::continue_action, 0, 7) ||
        !same(captain::resolve_defender_outcome({1, 11, 12, 3, 0}),
              DefenderRoute::continue_action, 11, 12)) {
        std::cerr << "Defender results zero and one should preserve possession\n";
        return EXIT_FAILURE;
    }
    if (!same(captain::resolve_defender_outcome({2, 0, 7, 2, 0}),
              DefenderRoute::recover_ball, 0, 7)) {
        std::cerr << "Defender result two should enter ball recovery without immediate transfer\n";
        return EXIT_FAILURE;
    }
    {
        auto recovery = captain::BallRecoveryInput{};
        recovery.ball_x = 120;
        recovery.ball_y = 100;
        recovery.ball_owner = 22;
        recovery.opposing_player = 22;
        recovery.random_e2 = 0x80;
        recovery.players.fill({120, 100, false});
        captain::DefenderOutcomeInput input{2, 0, 7, 2, 0};
        input.recovery = recovery;
        const auto result = captain::resolve_defender_outcome(input);
        if (!same(result, DefenderRoute::recover_ball, 11, 12) || !result.recovery) {
            std::cerr << "Resolved result-two recovery should transfer possession to the selected player\n";
            return EXIT_FAILURE;
        }
    }
    if (!same(captain::resolve_defender_outcome({3, 0, 7, 11, 0}),
              DefenderRoute::keeper_control, 11, 11) ||
        !same(captain::resolve_defender_outcome({3, 11, 7, 0, 1}),
              DefenderRoute::keeper_control, 0, 0)) {
        std::cerr << "Keeper defender should receive possession on result three\n";
        return EXIT_FAILURE;
    }
    if (!same(captain::resolve_defender_outcome({3, 11, 7, 3, 2}),
              DefenderRoute::player_control, 0, 3) ||
        !same(captain::resolve_defender_outcome({3, 0, 7, 12, 2}),
              DefenderRoute::player_control, 11, 12)) {
        std::cerr << "Defense action two should transfer possession to the field defender's side\n";
        return EXIT_FAILURE;
    }
    if (!same(captain::resolve_defender_outcome({3, 11, 7, 3, 1}),
              DefenderRoute::defense_animation, 11, 7)) {
        std::cerr << "Other result-three field defense should defer possession change\n";
        return EXIT_FAILURE;
    }
    std::cout << "Defender result handoff checks passed\n";
}

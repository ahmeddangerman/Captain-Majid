#include "captain/ball_recovery.hpp"

#include <stdexcept>

namespace captain {
namespace {
std::uint8_t add_jitter(std::uint8_t coordinate, std::uint8_t random) {
    const auto sample = static_cast<std::uint8_t>(random & 0x83);
    auto offset = static_cast<std::uint8_t>(sample << 3);
    if ((sample & 0x80) != 0) {
        offset = static_cast<std::uint8_t>(0U - offset);
    }
    return static_cast<std::uint8_t>(coordinate + offset);
}

std::uint8_t player_id_for_index(std::size_t index) {
    return static_cast<std::uint8_t>(index < 10 ? index + 1 : index + 2);
}

struct Candidate {
    std::uint8_t player_id;
    std::uint8_t radius;
};

Candidate closest_available(const BallRecoveryInput& input,
                            std::uint8_t side,
                            std::uint8_t ball_x,
                            std::uint8_t ball_y) {
    for (unsigned radius = 8; radius <= 248; radius += 8) {
        for (std::size_t index = 0; index < input.players.size(); ++index) {
            const auto id = player_id_for_index(index);
            const bool on_side = side == 0 ? id < 11 : id >= 12;
            if (!on_side || id == input.ball_owner || id == input.opposing_player) {
                continue;
            }
            const auto& player = input.players[index];
            if (player.unavailable) continue;

            const auto dx = static_cast<unsigned>(
                player.x > ball_x ? player.x - ball_x : ball_x - player.x);
            if (dx >= radius) continue;
            const auto dy = static_cast<unsigned>(
                player.y > ball_y ? player.y - ball_y : ball_y - player.y);
            if (dy < radius) {
                return {id, static_cast<std::uint8_t>(radius)};
            }
        }
    }
    throw std::invalid_argument("Ball recovery needs an available player on each side");
}
}

BallRecoveryResult recover_ball(const BallRecoveryInput& input) {
    if (input.ball_update_active) {
        throw std::invalid_argument("Active ball movement must be resolved before recovery");
    }
    if (input.selected_defenders.size() > 10) {
        throw std::invalid_argument("Ball recovery accepts at most ten selected field defenders");
    }
    for (const auto& defender : input.selected_defenders) {
        if (defender.player_id > 21) {
            throw std::invalid_argument("Ball recovery defender IDs must be between 0 and 21");
        }
    }

    const auto ball_y = add_jitter(input.ball_y, input.random_e2);
    const auto ball_x = add_jitter(input.ball_x, input.random_e3);
    if (ball_x < 0x30 || ball_x >= 0xd0 || ball_y < 0x50 || ball_y >= 0xb0) {
        throw std::invalid_argument("Ball recovery only covers the ordinary interior field");
    }

    if (input.random_e2 < 0x40) {
        for (const auto& defender : input.selected_defenders) {
            if (defender.player_id != 0 && defender.player_id != 11 &&
                defender.defense_action == 5) {
                const std::uint8_t side = defender.player_id < 11 ? 0 : 11;
                return {ball_x, ball_y, defender.player_id, side, 0, true};
            }
        }
    }

    const auto left = closest_available(input, 0, ball_x, ball_y);
    const auto right = closest_available(input, 11, ball_x, ball_y);
    const auto owner = left.radius < right.radius ||
            (left.radius == right.radius && (input.random_e2 & 0x80) == 0)
        ? left.player_id
        : right.player_id;
    const std::uint8_t side = owner < 11 ? 0 : 11;
    return {ball_x, ball_y, owner, side, 0, false};
}
}

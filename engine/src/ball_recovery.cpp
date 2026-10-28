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

}

NearestFieldPlayerResult find_nearest_field_player(const NearestFieldPlayerInput& input) {
    if (input.side != 0 && input.side != 11)
        throw std::invalid_argument("Nearest-player search requires side0 or11");
    for (unsigned radius = 8; radius <= 248; radius += 8) {
        for (std::size_t index = 0; index < input.players.size(); ++index) {
            const auto id = player_id_for_index(index);
            const bool on_side = input.side == 0 ? id < 11 : id >= 12;
            if (!on_side || id == input.ball_owner || id == input.opposing_player) {
                continue;
            }
            const auto& player = input.players[index];
            if (player.unavailable) continue;

            const auto dx = static_cast<unsigned>(
                player.x > input.ball_x ? player.x - input.ball_x : input.ball_x - player.x);
            if (dx >= radius) continue;
            const auto dy = static_cast<unsigned>(
                player.y > input.ball_y ? player.y - input.ball_y : input.ball_y - player.y);
            if (dy < radius) {
                return {id, static_cast<std::uint8_t>(radius)};
            }
        }
    }
    throw std::invalid_argument("Nearest-player search needs an available player on the requested side");
}

LooseBallPosition jitter_loose_ball(std::uint8_t x, std::uint8_t y,
    std::uint8_t random_e2, std::uint8_t random_e3) {
    const auto ball_x = add_jitter(x, random_e3);
    const auto ball_y = add_jitter(y, random_e2);
    return {ball_x, ball_y, static_cast<std::uint8_t>(random_e3 & 0x83u),
        ball_x >= 48 && ball_x < 208 && ball_y >= 80 && ball_y < 176};
}

BallRecoveryResult recover_stationary_ball(const BallRecoveryInput& input) {
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

    const auto ball_y = input.ball_y;
    const auto ball_x = input.ball_x;
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

    const auto left = find_nearest_field_player({0, ball_x, ball_y, input.ball_owner,
        input.opposing_player, input.players});
    const auto right = find_nearest_field_player({11, ball_x, ball_y, input.ball_owner,
        input.opposing_player, input.players});
    const auto owner = left.radius < right.radius ||
            (left.radius == right.radius && (input.random_e2 & 0x80) == 0)
        ? left.player_id
        : right.player_id;
    const std::uint8_t side = owner < 11 ? 0 : 11;
    return {ball_x, ball_y, owner, side, 0, false};
}

BallRecoveryResult recover_ball(const BallRecoveryInput& input) {
    auto stationary = input;
    const auto position = jitter_loose_ball(input.ball_x, input.ball_y, input.random_e2, input.random_e3);
    stationary.ball_x = position.x;
    stationary.ball_y = position.y;
    return recover_stationary_ball(stationary);
}
}

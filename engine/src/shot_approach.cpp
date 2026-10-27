#include "captain/shot_approach.hpp"
#include <stdexcept>

namespace captain {
ShotApproachResult calculate_shot_approach(const ShotApproachInput& input,
                                         const ShotApproachContent& content) {
    if (input.side != 0 && input.side != 11)
        throw std::invalid_argument("Shot approach needs side0/11");
    auto x = input.side == 0 ? input.ball_x
        : static_cast<std::uint8_t>(0u - unsigned(input.ball_x));
    std::uint8_t coefficient = 0;
    if (x < 160) {
        const auto index = unsigned(static_cast<std::uint8_t>(unsigned(x) - 48u)) >> 3;
        if (index >= content.outer_distance.size())
            throw std::invalid_argument("Unsupported outer shot distance cell");
        coefficient = content.outer_distance[index];
        // The original zero entry falls through with A=0, not the old X byte.
        x = coefficient;
    }
    if (coefficient == 0) {
        const auto column = unsigned(static_cast<std::uint8_t>(unsigned(x) - 160u)) >> 3;
        const auto y = input.ball_y & 128u
            ? static_cast<std::uint8_t>(input.ball_y ^ 255u) : input.ball_y;
        const auto row = (static_cast<std::uint8_t>(unsigned(y) - 80u) & 56u) * 3u / 4u;
        const auto index = row + column;
        if (index >= content.inner_distance.size())
            throw std::invalid_argument("Unsupported inner shot distance cell");
        coefficient = content.inner_distance[index];
    }
    const auto distance = static_cast<std::uint8_t>(
        unsigned(coefficient) * 9u + ((coefficient >> 5) & 1u));
    const auto direction_x = input.side == 0 ? input.ball_x
        : static_cast<std::uint8_t>(input.ball_x ^ 255u);
    const bool loose = input.attack_subtype == 0 && direction_x < 128;
    auto random = input.random_first;
    std::uint8_t cell;
    if (loose) {
        auto target = static_cast<std::uint8_t>(unsigned(direction_x) + 79u);
        if (input.side != 0) target ^= 255u;
        if (target < 48 || target >= 208)
            throw std::invalid_argument("Unsupported shot target coordinate");
        cell = static_cast<std::uint8_t>(((target - 48u) >> 3) * 12u + 5u);
    } else {
        cell = static_cast<std::uint8_t>((input.side == 0 ? 233u : 5u) + (random & 1u));
        random >>= 1;
    }
    return {distance, cell, static_cast<std::uint8_t>((cell / 12u) * 8u + 52u),
            static_cast<std::uint8_t>((cell % 12u) * 8u + 84u), random, loose};
}
}

#include "captain/pass_target.hpp"
#include <iostream>
int main() {
    unsigned x, y;
    while (std::cin >> x >> y) {
        if (x > 65535 || y > 65535) return 2;
        const auto result = captain::initialize_ordinary_pass_target(static_cast<std::uint16_t>(x), static_cast<std::uint16_t>(y));
        std::cout << unsigned(result.ball_x) << ' ' << unsigned(result.ball_y) << ' ' << unsigned(result.cell) << ' '
                  << unsigned(result.cursor) << ' ' << unsigned(result.ball_mode) << ' ' << unsigned(result.attack_subtype) << '\n';
    }
    return std::cin.eof() ? 0 : 1;
}

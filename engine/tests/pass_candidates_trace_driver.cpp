#include "captain/pass_target.hpp"
#include <iostream>
int main() {
    unsigned carrier, cursor;
    while (std::cin >> carrier >> cursor) {
        std::array<std::array<std::uint8_t, 2>, 22> positions{};
        for (auto& position : positions) {
            unsigned x, y;
            if (!(std::cin >> x >> y) || x > 255 || y > 255) return 2;
            position = {static_cast<std::uint8_t>(x), static_cast<std::uint8_t>(y)};
        }
        if (carrier > 255 || cursor > 255) return 2;
        const auto result = captain::select_ordinary_pass_candidates(
            static_cast<std::uint8_t>(carrier), static_cast<std::uint8_t>(cursor), positions);
        std::cout << unsigned(result.count) << ' ' << unsigned(result.allied_count);
        for (auto id : result.ids) std::cout << ' ' << unsigned(id);
        std::cout << '\n';
    }
    return std::cin.eof() ? 0 : 1;
}

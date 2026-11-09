#include "captain/pass_target.hpp"
#include <iostream>
int main() {
    unsigned cursor, held;
    while (std::cin >> cursor >> held) {
        if (cursor > 255 || held > 255) return 2;
        std::cout << unsigned(captain::advance_ordinary_pass_cursor(static_cast<std::uint8_t>(cursor),
                                                                    static_cast<std::uint8_t>(held))) << '\n';
    }
    return std::cin.eof() ? 0 : 1;
}

#include "captain/control_travel.hpp"
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
    using captain::advance_control_travel;
    try {
        if (argc == 2 && std::string(argv[1]) == "--batch") {
            unsigned travel, speed, side, held, facing;
            while (std::cin >> travel >> speed >> side >> held >> facing) {
                if (travel > 65535 || speed > 65535 || side > 255 || held > 255 || facing > 255) return 2;
                std::cout << advance_control_travel(travel, speed, side, held, facing) << '\n';
            }
            return std::cin.eof() ? 0 : 2;
        }
        if (advance_control_travel(100, 928, 0, 0, 64) != 100) return 1;
        if (advance_control_travel(100, 928, 0, 1, 64) != 332) return 1;
        if (advance_control_travel(100, 928, 0, 2, 0) != 65403) return 1;
        if (advance_control_travel(65500, 928, 11, 0, 64) != 196) return 1;
        bool rejected = false;
        try { advance_control_travel(0, 0, 1, 0, 0); }
        catch (const std::invalid_argument&) { rejected = true; }
        return rejected ? 0 : 1;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

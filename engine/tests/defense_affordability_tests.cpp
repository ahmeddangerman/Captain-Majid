#include "captain/defense_affordability.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--batch") {
        unsigned stamina, cost, subtype;
        while (std::cin >> stamina >> cost >> subtype) {
            if (stamina > 65535 || cost > 65535 || subtype > 255) return 2;
            std::cout << captain::can_select_defense_move(stamina, cost, subtype) << '\n';
        }
        return std::cin.eof() ? 0 : 2;
    }
    if (argc != 1) return 2;
    struct Case { unsigned stamina, cost, subtype; bool expected; };
    const Case cases[] = {{100,30,1,true},{80,80,1,true},{79,80,1,false},
        {0,1,0,true},{0,1,255,false},{0,32768,1,false},
        {32768,0,1,false},{65535,0,1,false},{0,65535,1,true},
        {65535,65535,1,true},{32767,65535,1,false}};
    for (const auto& c : cases) if (captain::can_select_defense_move(c.stamina,c.cost,c.subtype)!=c.expected) {
        std::cerr << "Defense affordability mismatch: " << c.stamina << ' ' << c.cost << ' ' << c.subtype << '\n';
        return 1;
    }
    std::cout << "Defense affordability checks passed\n";
}

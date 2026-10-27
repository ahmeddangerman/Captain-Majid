#include "captain/energy.hpp"

#include <cstdlib>
#include <iostream>

namespace {
unsigned checks = 0;

void require_energy(const captain::EnergyInput& input, const captain::EnergyResult& expected,
                    const char* message) {
    const auto actual = captain::calculate_energy(input);
    ++checks;
    if (actual.stamina != expected.stamina || actual.cost != expected.cost ||
        actual.weakened != expected.weakened || actual.carry != expected.carry) {
        std::cerr << "FAIL: " << message << ": got " << actual.stamina << ' ' << actual.cost
                  << ' ' << actual.weakened << ' ' << actual.carry << '\n';
        std::exit(1);
    }
}
}

int main() {
    require_energy({100, 30, 0, 0, 0, 0, false}, {70, 30, false, true},
                   "ordinary subtraction");
    require_energy({80, 80, 0, 0, 0, 0, false}, {0, 80, false, true},
                   "exact exhaustion does not weaken");
    require_energy({79, 80, 0, 0, 0, 0, false}, {0, 80, true, false},
                   "one point short weakens and clears carry");
    require_energy({0, 0, 0, 0, 0, 0, false}, {0, 0, false, true},
                   "zero stamina and zero cost do not weaken");
    require_energy({100, 30, 0, 0, 0, 0, false, true}, {70, 30, false, true},
                   "direct debit ignores incoming carry");
    require_energy({79, 80, 0, 0, 0, 0, false, true}, {0, 80, true, false},
                   "weak direct debit overrides incoming carry");

    require_energy({100, 1, 0x20, 0, 0, 0, false}, {99, 1, false, true},
                   "odd surcharge rounds down");
    require_energy({100, 3, 0x20, 0, 0, 0, false}, {96, 4, false, true},
                   "three point surcharge becomes four");
    require_energy({100, 30, 0x20, 0, 0, 2, false}, {55, 45, false, true},
                   "subtype two retains surcharge");
    require_energy({100, 30, 0x20, 0, 0, 3, false}, {70, 30, false, true},
                   "subtype three is exempt");
    require_energy({100, 30, 0x20, 0, 0, 0x80, false}, {70, 30, false, true},
                   "raw subtype bit seven participates in exemption");
    require_energy({100, 30, 0x20, 0, 0, 255, false}, {70, 30, false, true},
                   "maximum raw subtype is exempt");
    require_energy({100, 30, 0x20, 0, 1, 3, false}, {55, 45, false, true},
                   "different action removes exemption");
    require_energy({100, 30, 0x20, 0, 255, 255, false}, {55, 45, false, true},
                   "all nonzero actions surcharge");
    require_energy({100, 30, 0x20, 11, 0, 3, false}, {55, 45, false, true},
                   "other side removes exemption");
    require_energy({100, 30, 0x20, 1, 0, 3, false}, {55, 45, false, true},
                   "side check accepts any nonzero byte");
    require_energy({100, 30, 0x20, 255, 0, 3, false}, {55, 45, false, true},
                   "maximum side byte surcharges");
    require_energy({100, 30, 0x1f, 0, 1, 0, false}, {70, 30, false, true},
                   "adjacent lower player id has no surcharge");
    require_energy({100, 30, 0x21, 11, 1, 0, false}, {70, 30, false, true},
                   "adjacent higher player id has no surcharge");
    require_energy({44, 30, 0x20, 11, 0, 3, false}, {0, 45, true, false},
                   "increased cost determines weakening");
    require_energy({45, 30, 0x20, 11, 0, 3, false}, {0, 45, false, true},
                   "increased cost exact exhaustion");

    require_energy({79, 80, 0x20, 0, 1, 0, true, false}, {79, 80, false, false},
                   "wrapper home side skips cost and preserves clear carry");
    require_energy({79, 80, 0x20, 0, 1, 0, true, true}, {79, 80, false, true},
                   "wrapper home side preserves set carry");
    require_energy({65535, 65535, 0x20, 0, 1, 0, true}, {65535, 65535, false, false},
                   "skipped debit does not normalize large words");
    require_energy({100, 30, 0x20, 11, 0, 3, true, false}, {55, 45, false, true},
                   "wrapper nonzero side debits");
    require_energy({100, 30, 0x20, 1, 0, 3, true, true}, {55, 45, false, true},
                   "wrapper uses byte nonzero condition");

    require_energy({0, 32768, 0, 0, 0, 0, false}, {0, 32768, true, false},
                   "difference bit fifteen sets weakening");
    require_energy({0, 32769, 0, 0, 0, 0, false}, {32767, 32769, false, false},
                   "large wrapped underflow is retained with clear carry");
    require_energy({0, 65535, 0, 0, 0, 0, false}, {1, 65535, false, false},
                   "maximum cost wraps subtraction to one");
    require_energy({32767, 0, 0, 0, 0, 0, false}, {32767, 0, false, true},
                   "largest nonnegative difference is retained");
    require_energy({32768, 0, 0, 0, 0, 0, false}, {0, 0, true, false},
                   "large positive mathematical difference still weakens");
    require_energy({65535, 0, 0, 0, 0, 0, false}, {0, 0, true, false},
                   "maximum stamina with zero cost weakens");
    require_energy({100, 43690, 0x20, 0, 0, 0, false}, {101, 65535, false, false},
                   "surcharge immediately below word overflow");
    require_energy({100, 43691, 0x20, 0, 0, 0, false}, {100, 0, false, true},
                   "surcharge wraps to zero");
    require_energy({100, 65535, 0x20, 0, 0, 0, false}, {0, 32766, true, false},
                   "maximum cost scales modulo a word");
    require_energy({65535, 65535, 0x20, 0, 0, 3, false}, {0, 65535, false, true},
                   "exempt maximum cost does not scale");
    std::cout << "Energy usage checks passed: " << checks << '\n';
}

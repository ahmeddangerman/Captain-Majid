#include "captain/reference_random.hpp"

#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
}

int main() {
    auto result = captain::advance_reference_random({0, 0, 73}, {0, 0, 0, false});
    require(result.state.index == 1 && result.state.first == 255 && result.state.second == 255 && !result.carry,
            "zero inputs preserve original add/subtract carry chain");
    auto ignored = captain::advance_reference_random({0, 0, 99}, {0, 0, 0, false});
    require(ignored.state.first == result.state.first && ignored.state.second == result.state.second,
            "previous second byte is overwritten, not read");
    result = captain::advance_reference_random({255, 255, 73}, {255, 255, 255, true});
    require(result.state.index == 0 && result.state.first == 0 && result.state.second == 0 && result.carry,
            "byte and index overflow");
    auto carry0 = captain::advance_reference_random({11, 32, 0}, {5, 6, 7, false});
    auto carry1 = captain::advance_reference_random({11, 32, 0}, {5, 6, 7, true});
    require(carry0.state.first != carry1.state.first || carry0.state.second != carry1.state.second ||
            carry0.carry != carry1.carry, "incoming carry affects random step");
    std::array<std::uint8_t, 2048> ram{};
    ram[0x380] = 11; ram[0x780] = 22; ram[0] = 33;
    auto sample = captain::sample_reference_random(ram, 128, true);
    require(sample.player_byte == 11 && sample.sound_byte == 22 && sample.tail_byte == 33 && sample.carry,
            "tail sample at $0800 mirrors RAM $0000");
    ram[0x3FF] = 44; ram[0x7FF] = 55; ram[0x7F] = 66;
    sample = captain::sample_reference_random(ram, 255, false);
    require(sample.player_byte == 44 && sample.sound_byte == 55 && sample.tail_byte == 66 && !sample.carry,
            "highest index samples correct RAM aliases");
    std::cout << "Reference random-step checks passed\n";
}

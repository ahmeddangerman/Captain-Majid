#include "captain/defender_contact.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
}

int main() {
    // Developer-selected content coefficients and fixtures, not bundled game tables.
    captain::DefenderContactInput input{300, 256, 100, 100, 200, 50,
                                       false, false, 0, 1, 3, {160, 100, 50, 0}};
    auto result = captain::calculate_defender_contact(input);
    require(result.strength == 150 && result.loss == 37, "strength and quarter loss");
    require(result.score == 128 && result.result_code == 1 && result.incoming_power == 263,
            "comparison selects loss branch");
    require(result.subtype == 3 && !result.special && result.defender_power == 256,
            "ordinary branch preserves subtype and base");
    input.weakened = true;
    result = captain::calculate_defender_contact(input);
    require(result.defender_power == 16 && result.strength == 9 && result.score == 255,
            "weakened branch changes stored base");
    input.eligible = true; input.context = 4; input.defender_slot = 0;
    require(!captain::calculate_defender_contact(input).special, "weak branch suppresses special");
    input.weakened = false;
    result = captain::calculate_defender_contact(input);
    require(result.special && result.subtype == 131 && result.strength == 255,
            "eligible context and defender select special");
    input.context = 0; input.defender_slot = 1; input.random_second = 248;
    require(captain::calculate_defender_contact(input).special, "rare random special branch");
    input.random_second = 247;
    require(!captain::calculate_defender_contact(input).special, "special threshold excludes 247");
    input.eligible = false; input.random_first = input.random_second = 200;
    require(captain::calculate_defender_contact(input).strength == 250,
            "sample equal to range remains upper bound");
    input.random_first = input.random_second = 201;
    require(captain::calculate_defender_contact(input).strength == 51,
            "sample greater than range wraps by repeated subtraction");
    input.defender_power = 0;
    result = captain::calculate_defender_contact(input);
    require(result.strength == 0 && result.loss == 0 && result.score == 255,
            "zero strength follows original divider");
    input.defender_power = 256; input.random_first = input.random_second = 100;
    input.thresholds = {160, 128, 50, 0};
    require(captain::calculate_defender_contact(input).result_code == 1,
            "score equal to threshold chooses that bucket");
    input.incoming_power = 1024;
    require(captain::calculate_defender_contact(input).score == 0, "16-bit numerator wrap");
    input.defender_power = 65535; input.range = 255; input.offset = 255;
    input.random_first = input.random_second = 2;
    require(captain::calculate_defender_contact(input).strength == 254,
            "product byte 3 is ignored when byte 2 is zero");
    input.range = 0;
    bool invalid = false;
    try { captain::calculate_defender_contact(input); }
    catch (const std::invalid_argument&) { invalid = true; }
    require(invalid, "zero range rejected");
    input.range = 1; input.thresholds = {100, 120, 50, 0}; invalid = false;
    try { captain::calculate_defender_contact(input); }
    catch (const std::invalid_argument&) { invalid = true; }
    require(invalid, "ascending thresholds rejected");
    input.thresholds = {160, 100, 50, 1}; invalid = false;
    try { captain::calculate_defender_contact(input); }
    catch (const std::invalid_argument&) { invalid = true; }
    require(invalid, "thresholds must cover score zero");
    std::cout << "Defender checks passed\n";
}

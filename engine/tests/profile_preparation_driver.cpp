// Private differential-comparison adapter. Reads external content once, then
// emits preparation results for records. No original content is compiled in.
#include "captain/field_player_profile.hpp"
#include <iostream>
#include <stdexcept>

namespace {
std::uint8_t byte() {
    unsigned value;
    if (!(std::cin >> value) || value > 255) throw std::invalid_argument("Expected byte");
    return static_cast<std::uint8_t>(value);
}
std::size_t count(std::size_t limit) {
    unsigned value;
    if (!(std::cin >> value) || value > limit) throw std::invalid_argument("Invalid size");
    return value;
}
}
int main() {
    try {
        captain::FieldPlayerProfiles profiles{};
        for (auto& v : profiles.ordinary) v = byte();
        profiles.alternate.resize(count(221));
        for (auto& v : profiles.alternate) v = byte();
        profiles.skills.resize(count(256));
        for (auto& row : profiles.skills) for (auto& v : row) v = byte();
        for (auto& v : profiles.generic_slot_offsets) v = byte();
        profiles.team_types.resize(count(256));
        for (auto& v : profiles.team_types) v = byte();
        captain::FieldDefenseContent defense{};
        for (auto& row : defense.skill_selectors) for (auto& v : row) v = byte();
        for (auto& moves : defense.moves) {
            moves.resize(count(64));
            for (auto& m : moves) m = {byte(),byte(),byte(),byte()};
        }
        for (auto& v : defense.strength_curve) v = byte();
        for (std::size_t i = count(1000000); i; --i) {
            captain::FieldPlayerRecord player{byte(),byte(),byte(),byte(),byte()};
            const auto action = byte(), situation = byte(), subtype = byte();
            const auto r = captain::prepare_profiled_field_defender(
                player,profiles,action,situation,subtype,defense);
            std::cout << unsigned(r.skill_selector) << ' ' << unsigned(r.skill_index) << ' '
                      << unsigned(r.base_strength) << ' ' << unsigned(r.move_effect) << ' '
                      << r.stamina_cost << '\n';
        }
        std::cin >> std::ws;
        if (!std::cin.eof()) throw std::invalid_argument("Trailing data");
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n'; return 2;
    }
}

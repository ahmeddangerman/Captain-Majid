#include "captain/field_defender_preparation.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
template<typename F> void rejects(F action) {
    try { action(); } catch (const std::invalid_argument&) { return; }
    require(false, "unsupported preparation must throw invalid_argument");
}
captain::FieldDefenseContent content_fixture() {
    captain::FieldDefenseContent content{};
    // Deliberately synthetic selectors: verify replaceable content, not ROM data.
    content.skill_selectors = {{{2,2,2}, {3,3,3}, {7,15,23}, {255,8,16}, {255,9,17}}};
    for (auto& moves : content.moves) moves.push_back({77, 4, 0x34, 0xfe});
    for (unsigned i = 0; i < content.strength_curve.size(); ++i)
        content.strength_curve[i] = static_cast<std::uint8_t>(255 - i);
    return content;
}
}

int main() {
    auto content = content_fixture();
    captain::FieldDefenderPreparationInput input{};
    input.base_skills[7] = 20;
    input.base_skills[15] = 40;
    input.base_skills[23] = 60;
    input.defense_action = 2;
    input.level = 3;
    for (std::uint8_t situation = 0; situation < 3; ++situation) {
        input.situation = situation;
        const auto r = captain::prepare_field_defender(input, content);
        require(r.skill_selector == 7 + 8 * situation && r.skill_index == 30 + 20 * situation,
                "situation selects the appropriate player skill");
        require(r.base_strength == 225 - 20 * situation && r.move_effect == 77 &&
                r.stamina_cost == 564, "curve lookup, move effect and masked cost");
    }
    input.situation = 1;
    const unsigned skills[]{2,3,15,8,9};
    for (std::uint8_t action = 0; action < 5; ++action) {
        input.defense_action = action;
        require(captain::prepare_field_defender(input, content).skill_selector == skills[action],
                "each action chooses its own skill");
    }
    input.defense_action = 2;
    input.situation = 0;
    input.level = 0;
    input.base_skills[7] = 180;
    input.subtype = 0xc1;
    content.moves[2].push_back({88, 130, 255, 255});
    auto r = captain::prepare_field_defender(input, content);
    require(r.skill_index == 54 && r.base_strength == 201 && r.move_effect == 88 &&
            r.stamina_cost == 1023, "subtype offset wrap and modifier byte wrap");
    input.subtype = 0;
    input.level = 128;
    input.base_skills[7] = 1;
    content.moves[2][0].skill_modifier = 0;
    r = captain::prepare_field_defender(input, content);
    require(r.skill_index == 2 && r.base_strength == 253, "level shift carry retained");

    input.level = 3;
    input.base_skills[7] = 20;
    content.moves[2][0].skill_modifier = 4;
    captain::DefenderContactInput contact{200,65535,100,100,100,0,false,false,0,12,99,
                                         {255,150,100,0}};
    const auto linked = captain::calculate_prepared_field_defender(input, content, contact);
    require(linked.preparation.base_strength == 225 && linked.contact.defender_power == 225 &&
            linked.contact.strength == 87 && linked.contact.result_code == 2 &&
            linked.contact.subtype == 0 && linked.contact.incoming_power == 200,
            "loaded strength and selected subtype feed contact calculation");
    require(contact.defender_power == 65535 && contact.subtype == 99,
            "preparation leaves caller inputs unchanged");

    input.defense_action = 5;
    rejects([&] { captain::prepare_field_defender(input, content); });
    input.defense_action = 3;
    rejects([&] { captain::prepare_field_defender(input, content); }); // unavailable selector
    input.defense_action = 2;
    input.situation = 3;
    rejects([&] { captain::prepare_field_defender(input, content); });
    input.situation = 0;
    input.subtype = 63;
    rejects([&] { captain::prepare_field_defender(input, content); });
    input.subtype = 0;
    for (auto selector : {0,31}) {
        content.skill_selectors[2][0] = static_cast<std::uint8_t>(selector);
        rejects([&] { captain::prepare_field_defender(input, content); });
    }
    std::cout << "Field defender preparation checks passed\n";
}

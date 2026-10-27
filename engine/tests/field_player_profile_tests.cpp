#include "captain/field_player_profile.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
namespace {
void require(bool ok, const char* msg) {
    if (!ok) { std::cerr << msg << '\n'; std::exit(1); }
}
template<class F> void rejects(F f) {
    try { f(); } catch (const std::invalid_argument&) { return; }
    require(false, "invalid profile reference accepted");
}
}
int main() {
    captain::FieldPlayerProfiles c{};
    c.skills.resize(3);
    c.skills[0].fill(20); c.skills[1].fill(40); c.skills[2].fill(60);
    c.ordinary[1] = 1; c.ordinary[34] = 2;
    c.alternate = {0,2};
    c.generic_slot_offsets.fill(3);
    c.generic_slot_offsets[1] = 1; c.generic_slot_offsets[10] = 2;
    c.team_types = {0,1,36,34};
    captain::FieldPlayerRecord p{1,1,255,0,3};
    auto r = captain::resolve_field_player_profile(p,c);
    require(r.effective_type == 1 && r.profile_id == 1 && r.skills[22] == 40,
            "ordinary profile ignores alternate flag");
    p = {21,0,0,0,3};
    r = captain::resolve_field_player_profile(p,c);
    require(r.effective_type == 36 && r.profile_id == 2, "last generic slot selects alternate type");
    p.player_id = 12;
    require(captain::resolve_field_player_profile(p,c).profile_id == 1,
            "first generic field slot chooses correct team offset");
    p = {10,35,127,36,3};
    require(captain::resolve_field_player_profile(p,c).profile_id == 0,
            "alternate type uses its own mapping without high flag");
    p.flags = 128;
    require(captain::resolve_field_player_profile(p,c).profile_id == 2,
            "high flag selects replacement alternate type");
    captain::FieldDefenseContent d{};
    for (auto& row : d.skill_selectors) row.fill(4);
    d.moves[0].push_back({7,4,52,254});
    for (unsigned i=0;i<192;++i) d.strength_curve[i]=static_cast<std::uint8_t>(191-i);
    auto prepared = captain::prepare_profiled_field_defender(p,c,0,0,192,d);
    require(prepared.skill_index == 70 && prepared.base_strength == 121 &&
            prepared.move_effect == 7 && prepared.stamina_cost == 564,
            "resolved skill and player level reach move preparation");
    p.alternate_type = 34;
    rejects([&]{ captain::resolve_field_player_profile(p,c); });
    p = {1,0,0,0,0};
    rejects([&]{ captain::resolve_field_player_profile(p,c); });
    for (auto id : {0,11,22,30,31,255}) {
        p = {static_cast<std::uint8_t>(id),1,0,0,0};
        rejects([&]{ captain::resolve_field_player_profile(p,c); });
    }
    p = {12,0,0,0,0}; c.team_types.clear();
    rejects([&]{ captain::resolve_field_player_profile(p,c); });
    p.type = 37;
    rejects([&]{ captain::resolve_field_player_profile(p,c); });
    p.type = 34; c.ordinary[34] = 255;
    rejects([&]{ captain::resolve_field_player_profile(p,c); });
    p.type = 1; c.skills.clear();
    rejects([&]{ captain::resolve_field_player_profile(p,c); });
    std::cout << "Field player profile checks passed\n";
}

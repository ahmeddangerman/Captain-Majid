#include "captain/prepared_defender_sequence.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
namespace {
void require(bool ok,const char* msg) { if(!ok) { std::cerr<<msg<<'\n'; std::exit(1); } }
}
int main() {
    captain::FieldRoster roster{};
    roster[12] = {1,0,79}; roster[13] = {32,0,200};
    captain::FieldPlayerProfiles profiles{};
    profiles.skills.resize(2); profiles.skills[0].fill(10); profiles.skills[1].fill(20);
    profiles.ordinary[1] = 0; profiles.ordinary[32] = 0;
    profiles.alternate = {0,1};
    captain::FieldDefenseContent defense{};
    for(auto& row: defense.skill_selectors) row.fill(4);
    for(auto& moves: defense.moves) moves = {{9,0,80,0},{10,0,0,0}};
    defense.strength_curve.fill(240);
    captain::DefenderContactInput comparison{999,999,100,100,100,0,false,false,0,0,99,{0,0,0,0}};
    const captain::FieldDefenseChoice first{12,0,0,0,comparison};
    const captain::FieldDefenseContext context{11,0,0,256,0};
    auto r = captain::resolve_prepared_field_contacts(roster,{first},profiles,defense,context);
    require(r.roster[12].stamina==0 && r.contacts[0].energy.weakened &&
            r.contacts[0].contact.defender_power==15 && r.contacts[0].contact.strength==5,
            "insufficient stamina weakens loaded strength before contact");
    require(roster[12].stamina==79 && first.comparison.defender_power==999,
            "caller inputs unchanged");
    auto side0 = context; side0.side=0; side0.incoming_energy_carry=true;
    r = captain::resolve_prepared_field_contacts(roster,{first},profiles,defense,side0);
    require(r.roster[12].stamina==79 && !r.contacts[0].energy.weakened &&
            r.contacts[0].energy.carry && r.contacts[0].contact.defender_power==240,
            "side-zero wrapper skips debit and clears weakening");
    auto special=first; special.player_id=13;
    r = captain::resolve_prepared_field_contacts(roster,{special},profiles,defense,context);
    require(r.roster[13].stamina==80 && r.contacts[0].energy.cost==120,
            "surcharge checks record type32 rather than roster slot13");
    roster[12] = {0,0,200}; profiles.team_types = {32};
    r = captain::resolve_prepared_field_contacts(roster,{first},profiles,defense,context);
    require(r.roster[12].stamina==120 && r.contacts[0].energy.cost==80,
            "generic effective type32 does not surcharge primary record type0");
    roster[12] = {1,0,200};
    auto stale = first; stale.comparison.eligible=true; stale.comparison.context=4;
    stale.comparison.weakened=true;
    r = captain::resolve_prepared_field_contacts(roster,{stale},profiles,defense,side0);
    require(!r.contacts[0].contact.special && r.contacts[0].contact.defender_power==240,
            "actual field slot and energy replace stale keeper slot and weakening");
    roster[12].stamina=80;
    r = captain::resolve_prepared_field_contacts(roster,{first,first},profiles,defense,context);
    require(r.contacts.size()==2 && !r.contacts[0].energy.weakened &&
            r.contacts[1].energy.weakened && r.roster[12].stamina==0,
            "repeated defender reads updated stamina, exact exhaustion is not weak");
    // Raw record+1/+2 alias stamina during profile lookup; variant selection
    // occurs BEFORE deduction and sees updated bytes on later visits.
    roster[12] = {35,0,static_cast<std::uint16_t>((36u<<8)|128u)};
    r = captain::resolve_prepared_field_contacts(roster,{first,first},profiles,defense,context);
    require(r.contacts[0].preparation.skill_index==20 &&
            r.contacts[1].preparation.skill_index==10 && r.roster[12].stamina==9184,
            "profile variant reads pre-debit current stamina bytes each time");
    roster[12] = {1,0,500};
    auto loss=first; loss.comparison.thresholds={200,100,50,0};
    auto second=first; second.player_id=13; second.subtype=193;
    r = captain::resolve_prepared_field_contacts(roster,{loss,second},profiles,defense,context);
    require(r.contacts[0].contact.result_code==1 && r.remaining_power==233 &&
            r.contacts[1].contact.incoming_power==233 && r.contacts[1].contact.subtype==193 &&
            r.roster[13].stamina==200,
            "power carries, selected subtype resets and zero-cost move keeps stamina");
    auto terminal=first; terminal.comparison.thresholds={255,220,180,0};
    auto invalid=first; invalid.player_id=255;
    auto lowpower=context; lowpower.incoming_power=1;
    r = captain::resolve_prepared_field_contacts(roster,{terminal,invalid},profiles,defense,lowpower);
    require(r.stopped && r.contacts.size()==1 && r.result_code==3 && r.roster[13].stamina==200,
            "terminal result stops before reading invalid later choice or charging another player");
    terminal.comparison.thresholds={255,220,0,0};
    r = captain::resolve_prepared_field_contacts(roster,{terminal,invalid},profiles,defense,lowpower);
    require(r.stopped && r.result_code==2 && r.contacts.size()==1,"code2 also stops sequence");
    bool rejected=false;
    try { captain::resolve_prepared_field_contacts(roster,{first,invalid},profiles,defense,context); }
    catch(const std::invalid_argument&) { rejected=true; }
    require(rejected && roster[12].stamina==500,"invalid visited choice rejects without input mutation");
    r = captain::resolve_prepared_field_contacts(roster,{},profiles,defense,{11,0,0,1024,7});
    require(r.contacts.empty() && r.remaining_power==1024 && r.result_code==7 && !r.stopped,
            "empty sequence preserves incoming values");
    std::cout<<"Prepared defender sequence checks passed\n";
}

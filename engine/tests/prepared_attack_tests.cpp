#include "captain/prepared_attack.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
namespace { void req(bool b,const char*m){if(!b){std::cerr<<m<<'\n';std::exit(1);}} }
int main(){
 captain::FieldPlayerProfiles p{};p.skills.resize(2);p.skills[0].fill(20);p.skills[1].fill(40);p.ordinary[1]=0;p.ordinary[32]=1;
 captain::FieldDefenseContent c{};c.skill_selectors={{{7,8,9},{2,2,2},{15,16,17},{18,18,18},{19,19,19}}};
 c.moves[0]={{44,4,80,0xf8},{55,0,30,0},{56,0,30,0},{57,0,80,0xf8}};c.moves[2]={{66,0,20,0}};c.strength_curve.fill(128);
 captain::PreparedAttackInput in{{1,1,100,0,3},0,2,3,0,1,100,100,false};
 auto r=captain::prepare_attacking_action(in,p,c);
 req(r.preparation.skill_selector==7 && r.preparation.stamina_cost==80 && r.move_flags==31 && r.energy.stamina==20 && !r.energy.weakened,"attack preparation, flags and side-zero energy");
 in.subtype=2;r=captain::prepare_attacking_action(in,p,c);req(r.preparation.skill_selector==9,"action0 subtype below3 selects situation skill");
 in.subtype=131;r=captain::prepare_attacking_action(in,p,c);req(r.preparation.skill_selector==7,"action0 high bit is masked for selector rule");
 in.action=2;in.situation=1;in.subtype=0;in.selected_defenders=0;in.player={1,1,0,0,3};
 r=captain::prepare_attacking_action(in,p,c);req(r.energy.cost==0&&r.energy.stamina==0&&r.preparation.skill_selector==16,"action2 with no defenders zeroes cost");
 in.selected_defenders=1;r=captain::prepare_attacking_action(in,p,c);req(r.energy.cost==20,"action2 with a defender retains cost");
 in.side=11;in.selected_defenders=0;r=captain::prepare_attacking_action(in,p,c);req(r.energy.cost==20,"side11 skips the action2 zero-cost override");
 in={{13,32,0,0,0},0,0,0,11,1,100,100,false};
 r=captain::prepare_attacking_action(in,p,c);req(r.energy.stamina==0&&r.energy.cost==80&&!r.energy.weakened,"side11 does not spend attacking energy");
 in.side=0;in.player={13,32,0,0,0};r=captain::prepare_attacking_action(in,p,c);req(r.energy.cost==120,"type32 surcharge uses player type");
 in.player={1,1,0,0,0};in.side=0;in.subtype=3;in.incoming_carry=true;r=captain::prepare_attacking_action(in,p,c);req(r.energy.weakened&&r.energy.carry==false&&r.action_power.base==8&&r.action_power.multiplier==228,"energy carry feeds weakened action power");
 bool bad=false;in.action=5;try{(void)captain::prepare_attacking_action(in,p,c);}catch(const std::invalid_argument&){bad=true;}req(bad,"unsupported action rejects");
 std::cout<<"Prepared attack checks passed\n";
}

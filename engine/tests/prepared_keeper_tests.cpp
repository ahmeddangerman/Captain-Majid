#include "captain/prepared_keeper.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
namespace {
void req(bool b,const char*m){if(!b){std::cerr<<m<<'\n';std::exit(1);}}
template<class F> void rejects(F f){bool caught=false;try{f();}catch(const std::invalid_argument&){caught=true;}req(caught,"invalid keeper input accepted");}
}
int main(){
 captain::GoalkeeperProfiles p{};p.skills.resize(2);p.skills[0].fill(20);p.skills[1].fill(40);p.ordinary[33]=1;
 captain::GoalkeeperMoveContent c{};c.skill_selectors={25,26,25,28};for(auto& m:c.moves)m={{77,4,80,0xf8},{88,0,0,0}};c.strength_curve.fill(240);
 captain::PreparedKeeperInput in{{0,33,100,0,3},11,3,0,0,0,false,
  {100,{999,220,100,200,50,true,false,4,13,99},255,10,3,4,{200,180,100,0}}};
 auto r=captain::prepare_goalkeeper(in,p,c);
 req(r.profile.profile_id==1 && r.preparation.skill_selector==28 && r.preparation.skill_index==50 && r.preparation.base_strength==240,"keeper profile and action3 skill");
 req(r.preparation.move_effect==77&&r.energy.stamina==20&&!r.energy.weakened&&r.comparison.special&&r.comparison.base==240,"energy/prepared strength and type33 critical threshold");
 req(in.player.flags==100&&in.comparison.roll.base==999,"caller unchanged");
 in.player.type=1;r=captain::prepare_goalkeeper(in,p,c);req(!r.comparison.special,"ordinary primary type uses higher threshold");
 in.player.type=64;p.alternate.resize(30);in.player.flags=100;r=captain::prepare_goalkeeper(in,p,c);req(r.comparison.special,"type64 uses lower threshold");
 in.player={0,1,79,0,0};r=captain::prepare_goalkeeper(in,p,c);req(r.energy.weakened&&r.comparison.base==15&&!r.comparison.special,"energy weakness reaches keeper comparison");
 in.player={11,0,0,0,0};in.side=0;p.team_types={33};p.generic_slot_offsets[0]=0;
 r=captain::prepare_goalkeeper(in,p,c);req(r.profile.effective_type==33&&!r.comparison.special&&!r.energy.weakened&&r.energy.stamina==0,"generic keeper uses team profile but primary type for critical threshold, side0 skips debit");
 in.defense_subtype=193;r=captain::prepare_goalkeeper(in,p,c);req(r.preparation.move_effect==88&&r.energy.cost==0,"keeper subtype masks to move1");
 in.player={11,35,128,36,0};p.alternate={0,1};r=captain::prepare_goalkeeper(in,p,c);req(r.profile.profile_id==1,"alternate profile uses raw stamina alias bytes");
 in.player={0,0,0,0,0};in.side=11;rejects([&]{captain::prepare_goalkeeper(in,p,c);});
 in.player={11,1,0,0,0};rejects([&]{captain::prepare_goalkeeper(in,p,c);});
 in.side=0;in.defense_action=4;rejects([&]{captain::prepare_goalkeeper(in,p,c);});
 in.defense_action=0;c.skill_selectors[0]=22;rejects([&]{captain::prepare_goalkeeper(in,p,c);});
 c.skill_selectors[0]=31;rejects([&]{captain::prepare_goalkeeper(in,p,c);});
 c.skill_selectors[0]=25;in.defense_subtype=63;rejects([&]{captain::prepare_goalkeeper(in,p,c);});
 in.defense_subtype=0;p.skills.clear();rejects([&]{captain::prepare_goalkeeper(in,p,c);});
 std::cout<<"Prepared keeper checks passed\n";
}

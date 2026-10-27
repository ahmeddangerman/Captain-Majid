// Private trace adapter: reads external content once and emits native outputs.
#include "captain/prepared_attack.hpp"
#include <iostream>
#include <stdexcept>
namespace {
std::uint8_t byte(){unsigned n;if(!(std::cin>>n)||n>255)throw std::invalid_argument("byte");return static_cast<std::uint8_t>(n);}
std::size_t count(unsigned limit){unsigned n;if(!(std::cin>>n)||n>limit)throw std::invalid_argument("count");return n;}
}
int main(){try{
 captain::FieldPlayerProfiles p{};for(auto&v:p.ordinary)v=byte();p.alternate.resize(count(221));for(auto&v:p.alternate)v=byte();p.skills.resize(count(256));for(auto&r:p.skills)for(auto&v:r)v=byte();for(auto&v:p.generic_slot_offsets)v=byte();p.team_types.resize(count(256));for(auto&v:p.team_types)v=byte();
 captain::FieldDefenseContent c{};for(auto&r:c.skill_selectors)for(auto&v:r)v=byte();for(auto&m:c.moves){m.resize(count(64));for(auto&v:m)v={byte(),byte(),byte(),byte()};}for(auto&v:c.strength_curve)v=byte();
 for(auto n=count(100000);n;--n){captain::PreparedAttackInput in{{byte(),byte(),byte(),byte(),byte()},byte(),byte(),byte(),byte(),byte(),byte(),byte(),false};const auto carry=byte();if(carry>1)throw std::invalid_argument("carry");in.incoming_carry=carry;
 const auto r=captain::prepare_attacking_action(in,p,c);
 std::cout<<unsigned(r.preparation.skill_selector)<<' '<<unsigned(r.preparation.skill_index)<<' '<<unsigned(r.preparation.base_strength)<<' '<<unsigned(r.preparation.move_effect)<<' '<<r.preparation.stamina_cost<<' '<<r.energy.stamina<<' '<<r.energy.cost<<' '<<r.energy.weakened<<' '<<r.energy.carry<<' '<<unsigned(r.move_flags)<<' '<<r.action_power.base<<' '<<r.action_power.multiplier<<' '<<r.action_power.power<<' '<<unsigned(r.action_power.subtype)<<'\n';}
 std::cin>>std::ws;if(!std::cin.eof())throw std::invalid_argument("trailing input");
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}}

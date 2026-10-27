#include "captain/field_comparison_content.hpp"
#include <iostream>
#include <stdexcept>
namespace {std::uint8_t byte(){unsigned n;if(!(std::cin>>n)||n>255)throw std::invalid_argument("byte");return static_cast<std::uint8_t>(n);}}
int main(){try{
 captain::FieldComparisonContent c{};for(auto&v:c.attack_groups)v=byte();for(auto&v:c.defense_groups)v=byte();c.eligibility.resize(256);for(auto&v:c.eligibility)v=byte();c.variation.resize(128);for(auto&r:c.variation)for(auto&v:r)v=byte();for(auto&v:c.thresholds)v=byte();
 unsigned n;if(!(std::cin>>n)||n>10000)throw std::invalid_argument("count");
 for(;n;--n){const auto a=byte(),d=byte(),slot=byte();const auto r=captain::select_field_comparison(a,d,slot,c);std::cout<<unsigned(r.table_index)<<' '<<unsigned(r.range)<<' '<<unsigned(r.offset)<<' '<<r.eligible;for(auto t:r.thresholds)std::cout<<' '<<unsigned(t);
 if(r.range){const auto first=static_cast<std::uint8_t>(slot*31+a*17),second=static_cast<std::uint8_t>(d*53+slot*11);const auto result=captain::calculate_defender_contact({static_cast<std::uint16_t>(200+slot*64),240,first,second,r.range,r.offset,bool(slot%2),r.eligible,static_cast<std::uint8_t>(slot==0||slot==11?4:0),slot,3,r.thresholds});std::cout<<" 1 "<<result.defender_power<<' '<<unsigned(result.strength)<<' '<<unsigned(result.loss)<<' '<<unsigned(result.score)<<' '<<unsigned(result.result_code)<<' '<<unsigned(result.subtype)<<' '<<result.special;}
 else std::cout<<" 0 0 0 0 0 0 0 0";
 std::cout<<'\n';}
 std::cin>>std::ws;if(!std::cin.eof())throw std::invalid_argument("trailing input");
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}}

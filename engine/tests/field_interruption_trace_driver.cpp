#include "captain/field_interruption.hpp"
#include <iostream>
#include <stdexcept>
namespace{std::uint8_t byte(){unsigned n;if(!(std::cin>>n)||n>255)throw std::invalid_argument("byte");return static_cast<std::uint8_t>(n);}}
int main(){try{unsigned n;if(!(std::cin>>n)||n>100000)throw std::invalid_argument("count");for(;n;--n){captain::FieldInterruptionInput in{byte(),byte(),byte(),byte(),byte(),byte(),byte(),byte(),{}};for(auto&v:in.state.status)v=byte();for(auto&v:in.state.keeper_penalties)v=byte();const auto r=captain::resolve_field_interruption(in);std::cout<<unsigned(r.result_code)<<' '<<r.triggered;for(auto v:r.state.status)std::cout<<' '<<unsigned(v);for(auto v:r.state.keeper_penalties)std::cout<<' '<<unsigned(v);std::cout<<' '<<(r.restart?unsigned(r.restart->route):2)<<' '<<(r.restart?unsigned(r.restart->situation):77)<<'\n';}std::cin>>std::ws;if(!std::cin.eof())throw std::invalid_argument("trailing input");}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}}

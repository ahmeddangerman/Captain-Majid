#include "captain/pass_distance.hpp"
#include <iostream>

int main() {
 captain::PassDistanceContent content{};unsigned value=0,count=0;
 for(auto& byte:content.values) {if(!(std::cin>>value) || value>255)return 2;byte=static_cast<std::uint8_t>(value);}
 if(!(std::cin>>count))return 2;
 for(unsigned i=0;i<count;++i) {
  unsigned x,y,cell;if(!(std::cin>>x>>y>>cell) || x>255 || y>255 || cell>255)return 2;
  try {std::cout<<unsigned(captain::calculate_pass_distance(static_cast<std::uint8_t>(x),static_cast<std::uint8_t>(y),static_cast<std::uint8_t>(cell),content))<<'\n';}
  catch(...) {return 3;}
 }
 return 0;
}

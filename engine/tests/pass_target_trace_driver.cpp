#include "captain/pass_target.hpp"
#include <iostream>
int main() {
 unsigned count;if(!(std::cin>>count))return 2;
 for(unsigned i=0;i<count;++i) {
  unsigned recipient,previous,x,y;
  if(!(std::cin>>recipient>>previous>>x>>y) || recipient>255 || previous>255 || x>255 || y>255)return 2;
  try {std::cout<<unsigned(captain::resolve_pass_target_cell(static_cast<std::uint8_t>(recipient),static_cast<std::uint8_t>(previous),
       static_cast<std::uint8_t>(x),static_cast<std::uint8_t>(y)))<<'\n';} catch(...) {return 3;}
 }
 return 0;
}

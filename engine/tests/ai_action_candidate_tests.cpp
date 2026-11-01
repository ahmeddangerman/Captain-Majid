#include "captain/ai_action_candidate.hpp"
#include <iostream>
#include <string>
int main(int argc,char** argv) {
 if(argc==2 && std::string(argv[1])=="--batch") {
  unsigned a,b,c,d,r;
  while(std::cin>>a>>b>>c>>d>>r) {
   if(a>255 || b>255 || c>255 || d>255 || r>255)return 2;
   std::cout<<unsigned(captain::choose_ai_action_candidate({{static_cast<std::uint8_t>(a),static_cast<std::uint8_t>(b),static_cast<std::uint8_t>(c),static_cast<std::uint8_t>(d)}},r))<<'\n';
  }
  return std::cin.eof()?0:2;
 }
 if(argc!=1)return 2;
 const std::array<std::uint8_t,4> candidates{{0x01,0x23,0x45,0x67}};
 for(unsigned r=0;r<256;++r)
  if(captain::choose_ai_action_candidate(candidates,r)!=(r&7u))return 1;
 const std::array<std::uint8_t,4> duplicates{{0x99,0x99,0x99,0x92}};
 if(captain::choose_ai_action_candidate(duplicates,6)!=9 || captain::choose_ai_action_candidate(duplicates,7)!=2)return 1;
 std::cout<<"AI candidate selection checks passed\n";
}

#include "captain/ai_action_candidate.hpp"
#include "captain/shot_content.hpp"
#include <fstream>
#include <iostream>
#include <string>
int main(int argc,char** argv) {
 if(argc==3 && std::string(argv[1])=="--area") {
  std::ifstream file(argv[2],std::ios::binary);const auto content=captain::load_shot_content(file);
  if(!content.ai_area_choice)return 2;
  unsigned type,flags,alt,x,y,r;
  while(std::cin>>type>>flags>>alt>>x>>y>>r) {
   if(type>255||flags>255||alt>255||x>255||y>255||r>255)return 2;
   try {auto result=captain::choose_ai_area_candidate(type,flags,alt,x,y,r,*content.ai_area_choice);
    std::cout<<unsigned(result.category)<<' '<<unsigned(result.row)<<' '<<unsigned(result.candidate)<<'\n';
   } catch(const std::invalid_argument&){std::cout<<"unsupported\n";}
  }
  return std::cin.eof()?0:2;
 }
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

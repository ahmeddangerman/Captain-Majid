#include "captain/kickoff_selection.hpp"
#include <array>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
int main(int argc,char**argv){
 const std::array<std::uint8_t,5> candidates{5,6,7,9,10};
 try {
  if(argc==2 && std::string(argv[1])=="--batch"){
   unsigned side,random;while(std::cin>>side>>random){if(side>255 || random>255)return 2;
    std::cout<<unsigned(captain::select_kickoff_receiver(side,random,candidates))<<'\n';}return 0;
  }
  if(argc==2 && std::string(argv[1])=="--side-batch"){
   unsigned team,random;while(std::cin>>team>>random){if(team>255 || random>255)return 2;
    std::cout<<unsigned(captain::select_kickoff_side(team,random))<<'\n';}return 0;
  }
  for(unsigned team=0;team<256;++team)for(unsigned random=0;random<256;++random)
   if(captain::select_kickoff_side(team,random)!=((team!=3 && random>=128)?11:0))return 1;
  const std::array<unsigned,8> expected{5,6,7,9,10,5,6,7};
  for(unsigned n=0;n<256;++n)for(unsigned side:{0u,11u})
   if(captain::select_kickoff_receiver(side,n,candidates)!=side+expected[n&7])return 1;
  bool rejected=false;try{captain::select_kickoff_receiver(1,0,candidates);}catch(const std::invalid_argument&){rejected=true;}
  if(!rejected)return 1;
  auto invalid=candidates;invalid[2]=11;rejected=false;
  try{captain::select_kickoff_receiver(0,0,invalid);}catch(const std::invalid_argument&){rejected=true;}
  if(!rejected)return 1;
  std::cout<<"Kickoff receiver selection checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}

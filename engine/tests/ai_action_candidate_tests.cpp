#include "captain/ai_action_candidate.hpp"
#include "captain/shot_content.hpp"
#include "captain/area_ai_session.hpp"
#include <fstream>
#include <iostream>
#include <string>
int main(int argc,char** argv) {
 if(argc==3 && std::string(argv[1])=="--prepare") {
  std::ifstream file(argv[2],std::ios::binary);const auto content=captain::load_shot_content(file);
  unsigned id,type,flags,alt,level,action,variant,subtype,first,second;
  while(std::cin>>id>>type>>flags>>alt>>level>>action>>variant>>subtype>>first>>second) {
   if(id>255||type>255||flags>255||alt>255||level>255||action>255||variant>255||subtype>255||first>255||second>255)return 2;
   try {auto r=captain::prepare_opponent_area_action({static_cast<std::uint8_t>(id),static_cast<std::uint8_t>(type),static_cast<std::uint8_t>(flags),static_cast<std::uint8_t>(alt),static_cast<std::uint8_t>(level)},action,variant,subtype,first,second,content);
    std::cout<<r.preparation.stamina_cost<<' '<<unsigned(r.preparation.move_effect)<<' '<<unsigned(r.move_flags)<<' '<<r.action_power.power<<' '<<unsigned(r.action_power.subtype)<<' '<<r.energy.stamina<<'\n';
   }catch(const std::invalid_argument&){std::cout<<"unsupported\n";}
  }
  return std::cin.eof()?0:2;
 }
 if(argc==3 && std::string(argv[1])=="--plan") {
  std::ifstream file(argv[2],std::ios::binary);const auto content=captain::load_shot_content(file);
  if(!content.ai_area_choice)return 2;
  unsigned type,flags,alt,carrier,x,y,r,s,variant;
  while(std::cin>>type>>flags>>alt>>carrier>>x>>y>>r>>s>>variant) {
   std::array<std::array<std::uint8_t,2>,10> positions{};
   for(auto& position:positions)for(auto& value:position){unsigned n;if(!(std::cin>>n)||n>255)return 2;value=n;}
   if(type>255||flags>255||alt>255||carrier>255||x>255||y>255||r>255||s>255||variant>255)return 2;
   try {const auto result=captain::plan_ai_area_action(type,flags,alt,carrier,x,y,r,s,variant,positions,*content.ai_area_choice);
    std::cout<<unsigned(result.action)<<' '<<unsigned(result.subtype)<<' '<<unsigned(result.random_first)<<' '<<(result.recipient?int(*result.recipient):-1)<<' '<<(result.cell?int(*result.cell):-1)<<'\n';
   }catch(const std::invalid_argument&){std::cout<<"unsupported\n";}
  }
  return std::cin.eof()?0:2;
 }
 if(argc==3 && std::string(argv[1])=="--recipient-profile") {
  std::ifstream file(argv[2],std::ios::binary);const auto content=captain::load_shot_content(file);
  if(!content.ai_area_choice)return 2;
  unsigned type,flags,alt,carrier,x,r,s;
  while(std::cin>>type>>flags>>alt>>carrier>>x>>r>>s) {
   std::array<std::array<std::uint8_t,2>,10> positions{};
   for(auto& position:positions)for(auto& value:position){unsigned n;if(!(std::cin>>n)||n>255)return 2;value=n;}
   if(type>255||flags>255||alt>255||carrier>255||x>255||r>255||s>255)return 2;
   try {const auto result=captain::choose_profiled_ai_area_recipient(type,flags,alt,carrier,x,r,s,positions,*content.ai_area_choice);
    std::cout<<unsigned(result.action)<<' '<<unsigned(result.random_first)<<' '<<(result.recipient?int(*result.recipient):-1)<<' '<<(result.cell?int(*result.cell):-1)<<'\n';
   }catch(const std::invalid_argument&){std::cout<<"unsupported\n";}
  }
  return std::cin.eof()?0:2;
 }
 if(argc==2 && std::string(argv[1])=="--recipient") {
  unsigned entry,carrier,x,r,s;
  while(std::cin>>entry>>carrier>>x>>r>>s) {
   std::array<std::array<std::uint8_t,2>,10> positions{};
   for(auto& position:positions)for(auto& value:position){unsigned n;if(!(std::cin>>n)||n>255)return 2;value=n;}
   if(entry>255||carrier>255||x>255||r>255||s>255)return 2;
   try {const auto result=captain::choose_ai_area_recipient(entry,carrier,x,r,s,positions);
    std::cout<<unsigned(result.action)<<' '<<unsigned(result.random_first)<<' '<<(result.recipient?int(*result.recipient):-1)<<' '<<(result.cell?int(*result.cell):-1)<<'\n';
   }catch(const std::invalid_argument&){std::cout<<"unsupported\n";}
  }
  return std::cin.eof()?0:2;
 }
 if(argc==3 && (std::string(argv[1])=="--area" || std::string(argv[1])=="--action")) {
  std::ifstream file(argv[2],std::ios::binary);const auto content=captain::load_shot_content(file);
  if(!content.ai_area_choice)return 2;
  unsigned type,flags,alt,x,y,r;
  while(std::cin>>type>>flags>>alt>>x>>y>>r) {
   if(type>255||flags>255||alt>255||x>255||y>255||r>255)return 2;
   try {
    if(std::string(argv[1])=="--action") {
     const auto result=captain::choose_ai_area_action(type,flags,alt,x,y,r,*content.ai_area_choice);
     std::cout<<unsigned(result.action)<<' '<<unsigned(result.subtype)<<'\n';
    } else {
     const auto result=captain::choose_ai_area_candidate(type,flags,alt,x,y,r,*content.ai_area_choice);
     std::cout<<unsigned(result.category)<<' '<<unsigned(result.row)<<' '<<unsigned(result.candidate)<<'\n';
    }
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
 std::array<std::array<std::uint8_t,2>,10> teammates{};
 for(unsigned i=0;i<10;++i)teammates[i]={{static_cast<std::uint8_t>(60+10*i),static_cast<std::uint8_t>(80+8*i)}};
 const auto retry=captain::choose_ai_area_recipient(41,19,120,1,0,teammates);
 if(retry.action!=4 || retry.random_first!=128 || retry.recipient)return 1;
 const std::array<std::uint8_t,4> candidates{{0x01,0x23,0x45,0x67}};
 for(unsigned r=0;r<256;++r)
  if(captain::choose_ai_action_candidate(candidates,r)!=(r&7u))return 1;
 const std::array<std::uint8_t,4> duplicates{{0x99,0x99,0x99,0x92}};
 if(captain::choose_ai_action_candidate(duplicates,6)!=9 || captain::choose_ai_action_candidate(duplicates,7)!=2)return 1;
 captain::AiAreaChoiceContent synthetic{};synthetic.profile_rows={0};synthetic.cells[0]={5,32};synthetic.rows.resize(1);
 synthetic.rows[0][44]=0x01;synthetic.rows[0][45]=0x32;
 const auto first=captain::choose_ai_area_action(35,0,0,51,124,0,synthetic);
 const auto second=captain::choose_ai_area_action(35,0,0,51,124,1,synthetic);
 const auto third=captain::choose_ai_area_action(35,0,0,51,124,2,synthetic);
 if(first.action!=4 || first.subtype || second.action!=6 || second.subtype || third.action!=6 || third.subtype!=1)return 1;
 try {captain::choose_ai_area_action(35,0,0,51,124,3,synthetic);return 1;}catch(const std::invalid_argument&){}
 std::cout<<"AI candidate selection checks passed\n";
}

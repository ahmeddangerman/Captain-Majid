#include "captain/ai_action_candidate.hpp"
#include "captain/shot_content.hpp"
#include "captain/area_ai_session.hpp"
#include <fstream>
#include <iostream>
#include <string>
int main(int argc,char** argv) {
 if(argc==3 && std::string(argv[1])=="--keeper-ai") {
  std::ifstream file(argv[2],std::ios::binary);const auto content=captain::load_shot_content(file);
  if(!content.ai_keeper_choice)return 2;
  unsigned type,flags,alt,route,random;
  while(std::cin>>type>>flags>>alt>>route>>random){
   if(type>255 || flags>255 || alt>255 || route>255 || random>255)return 2;
   try {const auto result=captain::choose_opponent_keeper_action({11,static_cast<std::uint8_t>(type),static_cast<std::uint8_t>(flags),static_cast<std::uint8_t>(alt),0},content.goalkeepers,route,random,*content.ai_keeper_choice);
    std::cout<<unsigned(result.action)<<' '<<unsigned(result.subtype)<<'\n';
   } catch(const std::invalid_argument&){std::cout<<"unsupported\n";}
  }
  return std::cin.eof()?0:2;
 }
 if(argc==3 && std::string(argv[1])=="--field-plan") {
  std::ifstream file(argv[2],std::ios::binary);const auto content=captain::load_shot_content(file);
  if(!content.ai_field_choice || !content.ai_area_choice)return 2;
  unsigned type,flags,alt,carrier,x,y,first,second;
  while(std::cin>>type>>flags>>alt>>carrier>>x>>y>>first>>second) {
   std::array<std::array<std::uint8_t,2>,10> positions{};
   for(auto& p:positions)for(auto& v:p){unsigned n;if(!(std::cin>>n)||n>255)return 2;v=n;}
   if(type>255||flags>255||alt>255||carrier>255||x>255||y>255||first>255||second>255)return 2;
   try {const auto r=captain::plan_ai_field_pass_candidate(type,flags,alt,carrier,x,y,first,second,positions,*content.ai_field_choice,*content.ai_area_choice);
    std::cout<<unsigned(r.action)<<' '<<unsigned(r.subtype)<<' '<<unsigned(r.random_first)<<' '<<(r.recipient?int(*r.recipient):-1)<<' '<<(r.cell?int(*r.cell):-1)<<'\n';
   }catch(const std::invalid_argument&){std::cout<<"unsupported\n";}
  }
  return std::cin.eof()?0:2;
 }
 if(argc==3 && std::string(argv[1])=="--field") {
  std::ifstream file(argv[2],std::ios::binary);const auto content=captain::load_shot_content(file);
  if(!content.ai_field_choice)return 2;
  unsigned type,flags,alt,x,y,r;
  while(std::cin>>type>>flags>>alt>>x>>y>>r) {
   if(type>255 || flags>255 || alt>255 || x>255 || y>255 || r>255)return 2;
   try {const auto result=captain::choose_ai_field_candidate(type,flags,alt,x,y,r,*content.ai_field_choice);
    std::cout<<unsigned(result.category)<<' '<<unsigned(result.row)<<' '<<unsigned(result.candidate)<<'\n';
   }catch(const std::invalid_argument&){std::cout<<"unsupported\n";}
  }
  return std::cin.eof()?0:2;
 }
 if(argc==3 && std::string(argv[1])=="--contact") {
  std::ifstream file(argv[2],std::ios::binary);const auto content=captain::load_shot_content(file);
  unsigned id,type,flags,alt,level,attack,attack_subtype,variant,defense,subtype,incoming,first,second;
  while(std::cin>>id>>type>>flags>>alt>>level>>attack>>attack_subtype>>variant>>defense>>subtype>>incoming>>first>>second) {
   if(id>255||type>255||flags>255||alt>255||level>255||attack>255||attack_subtype>255||variant>255||defense>255||subtype>255||incoming>65535||first>255||second>255)return 2;
   try {const auto r=captain::compare_area_defender({static_cast<std::uint8_t>(id),static_cast<std::uint8_t>(type),static_cast<std::uint8_t>(flags),static_cast<std::uint8_t>(alt),static_cast<std::uint8_t>(level)},attack,attack_subtype,variant,defense,subtype,incoming,first,second,content);
    const auto& c=r.comparison;std::cout<<r.defender.energy.stamina<<' '<<r.defender.energy.cost<<' '<<unsigned(r.defender.preparation.move_effect)<<' '<<c.defender_power<<' '<<unsigned(c.strength)<<' '<<unsigned(c.loss)<<' '<<unsigned(c.score)<<' '<<unsigned(c.result_code)<<' '<<unsigned(c.subtype)<<' '<<c.incoming_power<<'\n';
   }catch(const std::invalid_argument&){std::cout<<"unsupported\n";}
  }
  return std::cin.eof()?0:2;
 }
 if(argc==3 && std::string(argv[1])=="--defender") {
  std::ifstream file(argv[2],std::ios::binary);const auto content=captain::load_shot_content(file);
  unsigned id,type,flags,alt,level,attack,attack_subtype,variant,defense,subtype;
  while(std::cin>>id>>type>>flags>>alt>>level>>attack>>attack_subtype>>variant>>defense>>subtype) {
   if(id>255||type>255||flags>255||alt>255||level>255||attack>255||attack_subtype>255||variant>255||defense>255||subtype>255)return 2;
   try {const auto r=captain::prepare_area_defender({static_cast<std::uint8_t>(id),static_cast<std::uint8_t>(type),static_cast<std::uint8_t>(flags),static_cast<std::uint8_t>(alt),static_cast<std::uint8_t>(level)},attack,attack_subtype,variant,defense,subtype,content);
    std::cout<<unsigned(r.preparation.base_strength)<<' '<<unsigned(r.preparation.move_effect)<<' '<<r.energy.cost<<' '<<r.energy.stamina<<' '<<r.energy.weakened<<' '<<r.energy.carry<<'\n';
   }catch(const std::invalid_argument&){std::cout<<"unsupported\n";}
  }
  return std::cin.eof()?0:2;
 }
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
 captain::AiKeeperContent keeper{};keeper.team_id=3;keeper.rows[0][0]=0x01;keeper.special_options[0]={{2,0}};
 captain::PlayerProfileMappings mapping{};mapping.team_types={35};
 auto keeper_special=captain::choose_opponent_keeper_action({11,0,0,0,0},mapping,0,0,keeper);
 auto keeper_regular=captain::choose_opponent_keeper_action({11,0,0,0,0},mapping,0,1,keeper);
 if(keeper_special.action!=0 || keeper_special.subtype!=2 || keeper_regular.action!=1 || keeper_regular.subtype)return 1;
 try {captain::choose_opponent_keeper_action({11,0,0,0,0},mapping,2,0,keeper);return 1;}catch(const std::invalid_argument&){}
 captain::AiFieldChoiceContent field{};field.profile_rows.fill(255);field.profile_rows[23]=0;field.rows.resize(1);
 for(auto& pair:field.low_x_categories)pair={255,255};
 field.low_x_categories[0]={0,4};
 field.rows[0][16]=0x23;field.rows[0][4]=0x56;
 const auto middle=captain::choose_ai_field_candidate(23,0,0,112,124,0,field);
 const auto left=captain::choose_ai_field_candidate(23,0,0,52,84,1,field);
 if(middle.category!=16 || middle.row!=0 || middle.candidate!=2 || left.category!=4 || left.candidate!=6)return 1;
 try {captain::choose_ai_field_candidate(0,0,0,112,124,0,field);return 1;}catch(const std::invalid_argument&){}
 std::array<std::array<std::uint8_t,2>,10> teammates{};
 for(unsigned i=0;i<10;++i)teammates[i]={{static_cast<std::uint8_t>(60+10*i),static_cast<std::uint8_t>(80+8*i)}};
 captain::AiAreaChoiceContent field_shared{};field_shared.recipient_rows.resize(1);field_shared.recipient_rows[0].fill(41);
 field.recipient_profile_rows.fill(255);field.recipient_profile_rows[23]=0;
 field.rows[0][16]=field.rows[0][20]=0x22;
 const auto field_retry=captain::plan_ai_field_pass_candidate(23,0,0,19,112,124,1,0,teammates,field,field_shared);
 if(field_retry.action!=2 || field_retry.random_first!=128 || field_retry.recipient)return 1;
 try {captain::plan_ai_field_pass_candidate(23,0,0,19,160,124,1,0,teammates,field,field_shared);return 1;}catch(const std::invalid_argument&){}
 field.empty_pass_options=std::array<std::uint8_t,118>{};(*field.empty_pass_options)[23]=1;
 const auto empty_options_pass=captain::plan_ai_field_pass_candidate(23,0,0,19,160,124,1,0,teammates,field,field_shared);
 if(empty_options_pass.action!=1 || empty_options_pass.recipient!=20 || empty_options_pass.subtype)return 1;
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

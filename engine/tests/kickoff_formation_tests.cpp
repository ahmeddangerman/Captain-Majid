#include "captain/kickoff_formation.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>
int main(int argc,char**argv){try{
 using namespace captain;
 if(argc==2 && std::string(argv[1])=="--batch"){
  unsigned side,left,right;while(std::cin>>side>>left>>right){
   if(side>255 || left>255 || right>255)return 2;
   KickoffFormationContent c{};unsigned value;
   for(auto& x:c.on_ball){if(!(std::cin>>value)||value>255)return 2;x=value;}
   for(auto& x:c.off_ball){if(!(std::cin>>value)||value>255)return 2;x=value;}
   std::array<KickoffPosition,22> p{};
   for(auto& x:p){unsigned a,b;if(!(std::cin>>a>>b)||a>65535||b>65535)return 2;x={static_cast<std::uint16_t>(a),static_cast<std::uint16_t>(b)};}
   const auto result=place_kickoff_formation(p,side,left,right,c);
   for(auto v:result)std::cout<<v.x<<' '<<v.y<<' ';
   std::cout<<'\n';
  }return 0;
 }
 KickoffFormationContent c{};c.on_ball.fill(0);c.off_ball.fill(13);
 std::array<KickoffPosition,22> p{};for(auto& v:p)v={0x1267,0x3439};
 const auto x=place_kickoff_formation(p,0,0,1,c);
 if(x[0].x!=0x3467 || x[0].y!=0x5400 || x[1].x!=0x3467 || x[1].y!=0x5439 || x[11].x!=0x3c67 || x[11].y!=0x5c00 || x[12].y!=0x5c39)return 1;
 const auto y=place_kickoff_formation(p,11,0,1,c);
 if(y[0].x!=0xc467 || y[0].y!=0xa400 || y[11].x!=0xcc67 || y[11].y!=0xac00)return 1;
 bool rejected=false;try{place_kickoff_formation(p,1,0,0,c);}catch(const std::invalid_argument&){rejected=true;}if(!rejected)return 1;
 rejected=false;try{place_kickoff_formation(p,0,4,0,c);}catch(const std::invalid_argument&){rejected=true;}if(!rejected)return 1;
 c.on_ball[0]=240;rejected=false;try{place_kickoff_formation(p,0,0,0,c);}catch(const std::invalid_argument&){rejected=true;}if(!rejected)return 1;
 std::cout<<"Kickoff formation checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

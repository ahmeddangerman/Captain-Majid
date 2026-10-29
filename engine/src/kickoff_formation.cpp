#include "captain/kickoff_formation.hpp"
#include <stdexcept>
namespace captain {
std::array<KickoffPosition,22> place_kickoff_formation(const std::array<KickoffPosition,22>& positions,
 std::uint8_t side,std::uint8_t left_formation,std::uint8_t right_formation,const KickoffFormationContent& content) {
 if((side!=0 && side!=11) || left_formation>=4 || right_formation>=4)
  throw std::invalid_argument("Kickoff formation needs valid side and formations0..3");
 auto result=positions;
 for(unsigned slot=0;slot<22;++slot){
  const bool right=slot>=11;const auto formation=right?right_formation:left_formation;
  const bool own=right==(side==11);const auto index=unsigned(formation)*11+slot%11;
  const auto cell=own?content.on_ball[index]:content.off_ball[index];
  if(cell>=240)throw std::invalid_argument("Kickoff formation cell outside field");
  auto x=static_cast<std::uint8_t>(52+8*(cell/12));
  auto y=static_cast<std::uint8_t>(84+8*(cell%12));
  if(side==11){x=static_cast<std::uint8_t>(0u-x);y=static_cast<std::uint8_t>(0u-y);}
  result[slot].x=static_cast<std::uint16_t>((positions[slot].x&255u)|(unsigned(x)<<8));
  result[slot].y=static_cast<std::uint16_t>(((slot==0 || slot==11)?0u:(positions[slot].y&255u))|(unsigned(y)<<8));
 }
 return result;
}
}

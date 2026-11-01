#include "captain/ai_action_candidate.hpp"
#include <stdexcept>
namespace captain {
AiAreaChoice choose_ai_area_candidate(std::uint8_t type,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t x,std::uint8_t y,std::uint8_t random,const AiAreaChoiceContent& content) {
 if(type==0)throw std::invalid_argument("AI generic team resolution is not captured");
 if(flags&128u)type=alternate;
 if(type<35 || unsigned(type-35)>=content.profile_rows.size())throw std::invalid_argument("Unsupported AI type");
 const auto row=content.profile_rows[type-35];
 if(row>=content.rows.size())throw std::invalid_argument("Missing AI candidate row");
 const unsigned fx=(x&128u)?(x^255u):x,fy=(y&128u)?(y^255u):y;
 if(fx<48 || fx>=208 || fy<80 || fy>=176)throw std::invalid_argument("AI position outside cell map");
 const auto cell=((fx-48)/8)*12+(fy-80)/8;
 for(const auto& entry:content.cells)if(entry[0]==cell) {
  const unsigned category=unsigned(entry[1])+12;
  if(category>44)throw std::invalid_argument("AI category outside candidate row");
  std::array<std::uint8_t,4> candidates{};
  for(unsigned i=0;i<4;++i)candidates[i]=content.rows[row][category+i];
  return {static_cast<std::uint8_t>(category),row,choose_ai_action_candidate(candidates,random)};
 }
 throw std::invalid_argument("Missing AI field category");
}
std::uint8_t choose_ai_action_candidate(const std::array<std::uint8_t,4>& candidates,std::uint8_t random) {
    const auto index=static_cast<unsigned>(random&7u);
    const auto packed=candidates[index>>1];
    return static_cast<std::uint8_t>((index&1u) ? packed&15u : packed>>4);
}
}

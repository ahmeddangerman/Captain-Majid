#include "captain/ai_action_candidate.hpp"
#include <stdexcept>
#include "captain/defender_selection.hpp"
namespace captain {
AiKeeperChoice choose_opponent_keeper_action(const FieldPlayerRecord& player,const PlayerProfileMappings& profiles,
 std::uint8_t route,std::uint8_t random,const AiKeeperContent& content) {
 if(player.player_id!=11 || content.team_id<3 || content.team_id>6 || route>4 || route==2)
  throw std::invalid_argument("Unsupported opponent keeper AI context");
 auto type=player.type;
 if(!type){const auto offset=profiles.generic_slot_offsets[0];if(offset>=profiles.team_types.size())throw std::invalid_argument("Missing keeper team profile");type=profiles.team_types[offset];}
 if(player.flags&128u)type=player.alternate_type;
 if(type<35 || unsigned(type-35)>=content.profile_rows.size())throw std::invalid_argument("Unsupported keeper AI type");
 constexpr std::array<unsigned,5> categories{{0,1,0,2,0}};
 const auto category=categories[route];const auto row=content.profile_rows[type-35][category];
 if(row>=content.rows.size())throw std::invalid_argument("Unsupported keeper AI row");
 std::array<std::uint8_t,4> candidates{};for(unsigned i=0;i<4;++i)candidates[i]=content.rows[row][4*category+i];
 const auto action=choose_ai_action_candidate(candidates,random);
 if(action>3)throw std::invalid_argument("Unsupported keeper AI action");
 std::uint8_t subtype=0;
 if(action==0){if(player.type>=content.special_options.size())throw std::invalid_argument("Missing keeper special options");const auto options=content.special_options[player.type];if(options[0] || options[1])subtype=options[0];}
 return {action,subtype};
}
AiAreaChoice choose_ai_field_candidate(std::uint8_t type,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t x,std::uint8_t y,std::uint8_t random,const AiFieldChoiceContent& content) {
 if(!type)throw std::invalid_argument("AI generic team resolution is not captured");
 if(flags&128u)type=alternate;
 const auto row=content.profile_rows[type];
 if(row>=content.rows.size())throw std::invalid_argument("Unsupported field AI profile");
 unsigned category=20;
 if(x<160){category=16;if(x<96){
  const auto mirrored_y=static_cast<std::uint8_t>((y&128u) ? (y^255u) : y);
  const auto cell=field_cell_at(x,mirrored_y);if(cell==255)throw std::invalid_argument("Field AI position outside cell map");
  bool found=false;for(const auto& pair:content.low_x_categories)if(pair[0]==cell){category=pair[1];found=true;break;}
  if(!found)throw std::invalid_argument("Missing field AI cell category");
 }}
 if(category>44)throw std::invalid_argument("Field AI category outside candidate row");
 std::array<std::uint8_t,4> candidates{};for(unsigned i=0;i<4;++i)candidates[i]=content.rows[row][category+i];
 return {static_cast<std::uint8_t>(category),row,choose_ai_action_candidate(candidates,random)};
}
AiAreaPlan plan_ai_field_pass_candidate(std::uint8_t type,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t carrier,std::uint8_t x,std::uint8_t y,std::uint8_t first,std::uint8_t second,
 const std::array<std::array<std::uint8_t,2>,10>& teammates,const AiFieldChoiceContent& field,const AiAreaChoiceContent& shared) {
 if(carrier<12 || carrier>21)throw std::invalid_argument("Field AI requires an opponent carrier");
 const auto raw_type=type;
 const auto choice=choose_ai_field_candidate(type,flags,alternate,x,y,first,field);
 if(choice.candidate!=2)throw std::invalid_argument("Unsupported field AI candidate");
 if(flags&128u)type=alternate;
 const auto row=field.recipient_profile_rows[type];
 if(row>=shared.recipient_rows.size())throw std::invalid_argument("Missing field AI recipient row");
 const auto target=choose_ai_area_recipient(shared.recipient_rows[row][first&7u],carrier,x,first,second,teammates);
 if(target.recipient) {
  if(!field.empty_pass_options || raw_type>=field.empty_pass_options->size() || (*field.empty_pass_options)[raw_type]!=1)
   throw std::invalid_argument("Field AI pass special-option rules are not represented");
  if(!target.cell || *target.cell>=240)throw std::invalid_argument("Field AI recipient is outside the field");
  return {1,0,target.random_first,target.recipient,target.cell};
 }
 return {2,0,target.random_first,std::nullopt,std::nullopt};
}
AiAreaPlan plan_ai_area_action(std::uint8_t type,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t carrier,std::uint8_t ball_x,std::uint8_t ball_y,std::uint8_t first,
 std::uint8_t second,std::uint8_t variant,const std::array<std::array<std::uint8_t,2>,10>& teammates,
 const AiAreaChoiceContent& content) {
 if(carrier<12 || carrier>21 || (variant!=1 && variant!=2))throw std::invalid_argument("Unsupported opponent area context");
 const auto choice=choose_ai_area_candidate(type,flags,alternate,ball_x,ball_y,first,content);
 if(choice.candidate!=2){const auto action=choose_ai_area_action(type,flags,alternate,ball_x,ball_y,first,content);return {action.action,action.subtype,first,std::nullopt,std::nullopt};}
 // Original attack action1 handler8D41 skips special options for nonzero variant.
 // Raw types beyond the captured original option-group table remain unsupported.
 if(type>=118)throw std::invalid_argument("Unrepresented original AI move-option group");
 const auto target=choose_profiled_ai_area_recipient(type,flags,alternate,carrier,ball_x,first,second,teammates,content);
 return {target.action,0,target.random_first,target.recipient,target.cell};
}
AiRecipientResult choose_profiled_ai_area_recipient(std::uint8_t type,std::uint8_t flags,
 std::uint8_t alternate,std::uint8_t carrier,std::uint8_t ball_x,
 std::uint8_t first,std::uint8_t second,
 const std::array<std::array<std::uint8_t,2>,10>& teammates,const AiAreaChoiceContent& content) {
 if(!type)throw std::invalid_argument("Missing generic AI team type");
 if(flags&128u)type=alternate;
 if(type<35 || unsigned(type-35)>=content.recipient_profile_rows.size())throw std::invalid_argument("Missing AI recipient profile");
 const auto row=content.recipient_profile_rows[type-35];
 if(row>=content.recipient_rows.size())throw std::invalid_argument("Missing AI recipient row");
 return choose_ai_area_recipient(content.recipient_rows[row][first&7u],carrier,ball_x,first,second,teammates);
}
AiRecipientResult choose_ai_area_recipient(std::uint8_t entry,std::uint8_t carrier,
 std::uint8_t ball_x,std::uint8_t random_first,std::uint8_t random_second,
 const std::array<std::array<std::uint8_t,2>,10>& teammates) {
 if(carrier<12 || carrier>21)throw std::invalid_argument("AI recipient requires opponent field carrier");
 const auto encoded=unsigned(entry>>2);const auto strategy=unsigned(entry&3u);
 if(encoded!=15 && (encoded<2 || encoded>11))throw std::invalid_argument("Unsupported preferred AI recipient");
 auto random=random_first;
 const auto random_target=[&](unsigned carry) {
  unsigned target=(unsigned(random)+random_second+carry)&15u;
  if(target>=10)target-=10;
  target+=12;
  if(target==carrier){target+=2;if(target>=22)target=12;}
  return target;
 };
 unsigned preferred=encoded+10;
 if(encoded==15 || preferred==carrier)preferred=random_target(1);
 const auto acceptable=[&](unsigned target){return ball_x>=teammates[target-12][0] || ball_x<96;};
 const auto chosen=[&](unsigned target){const auto& p=teammates[target-12];return AiRecipientResult{1,random,static_cast<std::uint8_t>(target),field_cell_at(p[0],p[1])};};
 const auto scan=[&]() -> unsigned {for(unsigned target=12;target<=21;++target)if(target!=carrier && teammates[target-12][0]<96)return target;return 0;};
 if(strategy==3){const auto target=scan();if(target)return chosen(target);}
 if(acceptable(preferred))return chosen(preferred);
 if(strategy==2){const auto target=scan();if(target)return chosen(target);}
 if(strategy!=0) {
  random=static_cast<std::uint8_t>((random>>1)|128u);
  // Original898A calls8A20 but does not store its returned target in3C.
  // The following89B3 retries the unchanged preferred target, which already
  // failed. Preserve the random mutation rather than substituting that target.
 }
 return {4,random,std::nullopt,std::nullopt};
}
AiAreaAction choose_ai_area_action(std::uint8_t type,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t x,std::uint8_t y,std::uint8_t random,const AiAreaChoiceContent& content) {
 const auto choice=choose_ai_area_candidate(type,flags,alternate,x,y,random,content);
 // Original route2 dispatch8903 selects890B/8911/8917/891A. Candidate2
 // enters8933 and needs a roster-based recipient decision, handled separately.
 switch(choice.candidate) {
 case 0:return {choice,4,0};
 case 1:return {choice,6,0};
 case 3:return {choice,6,1};
 default:throw std::invalid_argument("AI recipient selection or unknown action branch is not supported");
 }
}
AiAreaChoice choose_ai_pass_defense_candidate(std::uint8_t type,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t x,std::uint8_t y,std::uint8_t random,const AiPassDefenseContent& content) {
 if(type==0)throw std::invalid_argument("AI generic team resolution is not captured");
 if(flags&128u)type=alternate;
 if(type<35 || unsigned(type-35)>=content.profile_rows.size())throw std::invalid_argument("Unsupported pass-defense AI type");
 const auto row=content.profile_rows[type-35];if(row>=content.rows.size())throw std::invalid_argument("Missing pass-defense AI row");
 const unsigned fx=static_cast<std::uint8_t>(x^255u),fy=(y&128u)?(y^255u):y;
 unsigned category=20;
 if(fx<160){category=16;if(fx<96){
  if(fx<48 || fy<80 || fy>=176)throw std::invalid_argument("Pass-defense AI position outside cell map");
  const auto cell=((fx-48)/8)*12+(fy-80)/8;category=content.cells[0][1];
  for(const auto& entry:content.cells)if(entry[0]==cell){category=entry[1];break;}
 }}
 if(category>44)throw std::invalid_argument("Pass-defense AI category outside row");
 std::array<std::uint8_t,4> choices{};for(unsigned i=0;i<4;++i)choices[i]=content.rows[row][category+i];
 return {static_cast<std::uint8_t>(category),row,choose_ai_action_candidate(choices,random)};
}
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

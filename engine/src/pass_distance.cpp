#include "captain/pass_distance.hpp"
#include <algorithm>
#include <istream>
#include <stdexcept>

namespace captain {
std::uint8_t calculate_pass_distance(std::uint8_t ball_x,std::uint8_t ball_y,
                                     std::uint8_t target_cell,const PassDistanceContent& content) {
 if(ball_x<48 || ball_x>=208 || ball_y<80 || ball_y>=176 || target_cell>=240)
  throw std::invalid_argument("Pass distance requires an in-field ball and target cell");
 const int target_x=52+8*(target_cell/12),target_y=84+8*(target_cell%12);
 const auto dx=static_cast<unsigned>(target_x>=ball_x ? target_x-ball_x : ball_x-target_x);
 const auto dy=static_cast<unsigned>(target_y>=ball_y ? target_y-ball_y : ball_y-target_y);
 return content.values[std::max(dx,dy)/8];
}
PassDistanceContent read_pass_distance_content(std::istream& input) {
 constexpr char magic[]="CMJPDST1";
 for(unsigned i=0;i<8;++i)if(input.get()!=magic[i])throw std::invalid_argument("Invalid pass distance table magic");
 PassDistanceContent content{};
 for(auto& value:content.values) {const auto byte=input.get();if(byte==std::char_traits<char>::eof())throw std::invalid_argument("Truncated pass distance table");value=static_cast<std::uint8_t>(byte);}
 if(input.peek()!=std::char_traits<char>::eof() || input.bad())throw std::invalid_argument("Invalid trailing pass distance table data");
 return content;
}
}

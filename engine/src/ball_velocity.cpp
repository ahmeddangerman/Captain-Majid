#include "captain/ball_velocity.hpp"
#include <charconv>
#include <istream>
#include <stdexcept>
#include <string>
namespace captain {
namespace {
std::uint16_t project(std::uint8_t direction,const BallVelocityContent& content) {
 auto doubled=static_cast<std::uint8_t>(unsigned(direction)*2u);
 if(doubled&128u)doubled^=255u;
 const auto magnitude=content.quarter_wave[(doubled&126u)/2u];
 return direction&128u ? static_cast<std::uint16_t>(0u-unsigned(magnitude)) : magnitude;
}
}
BallVelocityContent read_ball_velocity_content(std::istream& input) {
 std::string magic, token;
 if(!(input>>magic) || magic!="CMJBALL1")throw std::invalid_argument("Expected CMJBALL1 velocity content");
 BallVelocityContent content{};
 for(auto& word:content.quarter_wave){
  if(!(input>>token))throw std::invalid_argument("Expected 64 velocity words");
  unsigned value=0;const auto result=std::from_chars(token.data(),token.data()+token.size(),value);
  if(result.ec!=std::errc{} || result.ptr!=token.data()+token.size() || value>65535)throw std::invalid_argument("Invalid velocity word");
  word=static_cast<std::uint16_t>(value);
 }
 if(input>>token || input.bad())throw std::invalid_argument("Trailing or unreadable velocity content");
 return content;
}
BallVelocity calculate_ball_velocity(std::uint8_t direction,const BallVelocityContent& content) {
 return {project(static_cast<std::uint8_t>(unsigned(direction)+64u),content),project(direction,content)};
}
}

#include "captain/ball_velocity.hpp"
namespace captain {
namespace {
std::uint16_t project(std::uint8_t direction,const BallVelocityContent& content) {
 auto doubled=static_cast<std::uint8_t>(unsigned(direction)*2u);
 if(doubled&128u)doubled^=255u;
 const auto magnitude=content.quarter_wave[(doubled&126u)/2u];
 return direction&128u ? static_cast<std::uint16_t>(0u-unsigned(magnitude)) : magnitude;
}
}
BallVelocity calculate_ball_velocity(std::uint8_t direction,const BallVelocityContent& content) {
 return {project(static_cast<std::uint8_t>(unsigned(direction)+64u),content),project(direction,content)};
}
}

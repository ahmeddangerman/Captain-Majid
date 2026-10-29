#include "captain/ball_step.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>
namespace captain {
BallStepResult advance_ball_to_next_cell(const BallStepInput& input) {
 if(input.current_cell>=240 || input.destination>=240 ||
    field_cell_at(input.x>>8,input.y>>8)!=input.current_cell)
  throw std::invalid_argument("Ball step requires coherent interior coordinates/cells");
 if(input.current_cell==input.destination)
  return {input.x,input.y,input.current_cell,0,BallStepEvent::already_at_destination,false};
 if(!input.velocity.x && !input.velocity.y)
  throw std::invalid_argument("Stationary ball cannot advance to a different cell");
 auto x=input.x,y=input.y;
 for(std::uint32_t n=1;n<=65536;++n) {
  x=static_cast<std::uint16_t>(unsigned(x)+input.velocity.x);
  y=static_cast<std::uint16_t>(unsigned(y)+input.velocity.y);
  auto cell=field_cell_at(x>>8,y>>8);const bool outside=cell==255;
  if(!outside && cell==input.current_cell)continue;
  if(outside)cell=input.destination;
  if(cell==input.destination) {
   x=static_cast<std::uint16_t>((((cell/12u)*8u+52u)<<8)|(x&255u));
   y=static_cast<std::uint16_t>((((cell%12u)*8u+84u)<<8)|(y&255u));
   return {x,y,cell,n,BallStepEvent::arrived,outside};
  }
  return {x,y,cell,n,BallStepEvent::cell_changed,false};
 }
 throw std::invalid_argument("Ball trajectory did not leave its cell within one coordinate cycle");
}
}

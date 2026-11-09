#include "captain/pass_target.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>

namespace captain {
OrdinaryPassTargetInitialization initialize_ordinary_pass_target(std::uint16_t carrier_x, std::uint16_t carrier_y) {
 const auto x=static_cast<std::uint8_t>(carrier_x >> 8);
 const auto y=static_cast<std::uint8_t>(carrier_y >> 8);
 const auto cell=field_cell_at(x,y);
 if(cell==255)throw std::invalid_argument("Ordinary pass target requires an interior carrier");
 return {x,y,cell,cell,129,0};
}
std::uint8_t resolve_pass_target_cell(std::uint8_t recipient,std::uint8_t previous_cell,
                                      std::uint8_t recipient_x,std::uint8_t recipient_y) {
 if(recipient==255)return previous_cell;
 if(recipient==0 || recipient==11 || recipient>21)
  throw std::invalid_argument("Pass target needs a field recipient or sentinel255");
 return field_cell_at(recipient_x,recipient_y);
}
}

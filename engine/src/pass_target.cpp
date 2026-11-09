#include "captain/pass_target.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>

namespace captain {
std::uint8_t resolve_pass_target_cell(std::uint8_t recipient,std::uint8_t previous_cell,
                                      std::uint8_t recipient_x,std::uint8_t recipient_y) {
 if(recipient==255)return previous_cell;
 if(recipient==0 || recipient==11 || recipient>21)
  throw std::invalid_argument("Pass target needs a field recipient or sentinel255");
 return field_cell_at(recipient_x,recipient_y);
}
}

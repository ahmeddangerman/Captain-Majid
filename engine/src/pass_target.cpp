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
std::uint8_t advance_ordinary_pass_cursor(std::uint8_t cursor, std::uint8_t held) {
 if (cursor >= 240) throw std::invalid_argument("Ordinary pass cursor requires field cell 0..239");
 const auto directions = static_cast<std::uint8_t>(held & 15u);
 if (!directions) return cursor;
 unsigned direction = 0;
 while (!(directions & (1u << direction))) ++direction;
 constexpr int deltas[] = {12, -12, 1, -1};
 const int target = static_cast<int>(cursor) + deltas[direction];
 return target < 0 || target >= 240 ? cursor : static_cast<std::uint8_t>(target);
}
std::uint8_t resolve_pass_target_cell(std::uint8_t recipient,std::uint8_t previous_cell,
                                      std::uint8_t recipient_x,std::uint8_t recipient_y) {
 if(recipient==255)return previous_cell;
 if(recipient==0 || recipient==11 || recipient>21)
  throw std::invalid_argument("Pass target needs a field recipient or sentinel255");
 return field_cell_at(recipient_x,recipient_y);
}
}

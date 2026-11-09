#include "captain/pass_target.hpp"
#include <iostream>
#include <stdexcept>
namespace {
void check(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
template<class F> void rejects(F operation) {try {operation();} catch(const std::invalid_argument&) {return;} throw std::runtime_error("Unsupported pass recipient accepted");}
}
int main() {try {
 using captain::resolve_pass_target_cell;
 using captain::initialize_ordinary_pass_target;
 const auto initial=initialize_ordinary_pass_target(0x7f32,0x8467);
 check(initial.ball_x==127 && initial.ball_y==132 && initial.cell==114 && initial.cursor==114 &&
       initial.ball_mode==129 && initial.attack_subtype==0,
       "Ordinary pass target starts at the carrier's field cell with the ordinary pass mode");
 rejects([&]{initialize_ordinary_pass_target(0x2fff,0x5000);});
 using captain::advance_ordinary_pass_cursor;
 check(advance_ordinary_pass_cursor(11,4)==12 && advance_ordinary_pass_cursor(12,8)==11,
       "Single-cell movement crosses row boundaries");
 check(advance_ordinary_pass_cursor(228,1)==228 && advance_ordinary_pass_cursor(0,2)==0 &&
       advance_ordinary_pass_cursor(239,4)==239 && advance_ordinary_pass_cursor(0,8)==0,
       "Movement remains inside the original linear field range");
 check(advance_ordinary_pass_cursor(87,15)==99 && advance_ordinary_pass_cursor(87,0xf4)==88,
       "Lowest direction bit wins and non-direction held bits do not move the cursor");
 rejects([&]{advance_ordinary_pass_cursor(240,0);});
 check(resolve_pass_target_cell(7,42,52,84)==0,"First field cell");
 check(resolve_pass_target_cell(21,42,204,172)==239,"Last field cell");
 check(resolve_pass_target_cell(1,42,47,84)==255,"Out-of-field recipient");
 check(resolve_pass_target_cell(255,240,0,255)==240,"No-recipient target retains previous byte");
 check(resolve_pass_target_cell(255,255,52,84)==255,"No-recipient target retains sentinel");
 for(auto id:{0,11,22,254})rejects([&]{resolve_pass_target_cell(static_cast<std::uint8_t>(id),0,52,84);});
 std::cout<<"Pass target checks passed\n";return 0;
 } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;} }

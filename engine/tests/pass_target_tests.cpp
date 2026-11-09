#include "captain/pass_target.hpp"
#include <iostream>
#include <stdexcept>
namespace {
void check(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
template<class F> void rejects(F operation) {try {operation();} catch(const std::invalid_argument&) {return;} throw std::runtime_error("Unsupported pass recipient accepted");}
}
int main() {try {
 using captain::resolve_pass_target_cell;
 check(resolve_pass_target_cell(7,42,52,84)==0,"First field cell");
 check(resolve_pass_target_cell(21,42,204,172)==239,"Last field cell");
 check(resolve_pass_target_cell(1,42,47,84)==255,"Out-of-field recipient");
 check(resolve_pass_target_cell(255,240,0,255)==240,"No-recipient target retains previous byte");
 check(resolve_pass_target_cell(255,255,52,84)==255,"No-recipient target retains sentinel");
 for(auto id:{0,11,22,254})rejects([&]{resolve_pass_target_cell(static_cast<std::uint8_t>(id),0,52,84);});
 std::cout<<"Pass target checks passed\n";return 0;
 } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;} }

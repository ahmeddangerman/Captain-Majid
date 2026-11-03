#include "captain/area_ai_session.hpp"
#include "captain/phase_session_io.hpp"
#include <fstream>
#include <iostream>
#include <string>
int main(int argc,char** argv) {
 try {
  if(argc!=4)return 2;
  const unsigned elapsed=std::stoul(argv[2]);if(elapsed>255)return 2;
  std::ifstream input(argv[1],std::ios::binary);
  auto state=captain::load_phase_session(input);
  const auto calculated=captain::advance_session_area_prefix_clock(state,static_cast<std::uint8_t>(elapsed));
  state=calculated.state;
  std::ofstream output(argv[3],std::ios::binary);captain::write_phase_session(output,state);
  std::cout<<static_cast<unsigned>(calculated.prelude->clock.continuation)<<'\n';
 } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

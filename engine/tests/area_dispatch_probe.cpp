#include "captain/area_ai_session.hpp"
#include "captain/phase_session_io.hpp"
#include "captain/field_state_io.hpp"
#include <fstream>
#include <iostream>
int main(int argc,char** argv) {
 try {
  if(argc!=4)return 2;
  std::ifstream pack(argv[1],std::ios::binary),source(argv[2],std::ios::binary);
  auto content=captain::load_shot_content(pack);auto state=captain::load_phase_session(source);
  const auto result=captain::preview_area_result_dispatch(state,content);
  std::ofstream out(argv[3],std::ios::binary);captain::write_field_state(out,result.movement.state);
  std::cout<<unsigned(result.route)<<'\n';
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

#include "captain/match_runtime.hpp"
#include "captain/phase_session_io.hpp"
#include <fstream>
#include <iostream>
int main(int argc,char** argv) {
 if(argc!=3){std::cerr<<"Usage: captain-match-lab CONTENT_PACK DEVELOPMENT_SESSION\n";return 2;}
 try {
  std::ifstream pack(argv[1],std::ios::binary),snapshot(argv[2],std::ios::binary);
  if(!pack || !snapshot)throw std::runtime_error("Cannot open content pack or session snapshot");
  captain::MatchContent content{captain::load_shot_content(pack)};
  captain::MatchConfig config{};config.initial=captain::load_phase_session(snapshot);
  auto start=captain::create_match(config,content);
  if(start.unsupported){std::cerr<<"Unsupported: "<<captain::capability_name(*start.unsupported)<<'\n';return 1;}
  auto step=captain::step_match(start.state,{captain::MatchInputKind::tick,1},content);
  if(step.unsupported){std::cout<<"Runtime stopped: "<<captain::capability_name(*step.unsupported)<<'\n';return 0;}
  std::cout<<"Control prepared; next step requires live timing and random scheduling.\n";
  return 0;
 } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

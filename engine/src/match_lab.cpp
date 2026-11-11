#include "captain/match_runtime.hpp"
#include "captain/phase_session_io.hpp"
#include <charconv>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
namespace {
unsigned parse_number(const char* text,unsigned max,const char* message){
 const std::string value(text);unsigned result=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);
 if(value.empty() || parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size() || result>max)throw std::invalid_argument(message);
 return result;
}
}
int main(int argc,char** argv) {
 if(argc<3){std::cerr<<"Usage: captain-match-lab CONTENT_PACK DEVELOPMENT_SESSION [RANDOM_INDEX | --demo SEED TICKS [--distance FILE] [--velocity FILE] [--ball-fractions X Y]]\n";return 2;}
 try {
  std::ifstream pack(argv[1],std::ios::binary),snapshot(argv[2],std::ios::binary);
  if(!pack || !snapshot)throw std::runtime_error("Cannot open content pack or session snapshot");
  captain::MatchContent content{captain::load_shot_content(pack)};
  captain::MatchConfig config{};config.initial=captain::load_phase_session(snapshot);
  unsigned ticks=1;
  if(argc==4 && std::string(argv[3])!="--demo")config.random_index=static_cast<std::uint8_t>(parse_number(argv[3],255,"Random index must be 0..255"));
  else if(argc>3){
   if(argc<6 || std::string(argv[3])!="--demo")throw std::invalid_argument("Expected --demo SEED TICKS");
   config.mode=captain::MatchTimingMode::provisional_demo;config.seed=parse_number(argv[4],0xffffffffu,"Seed must be 1..4294967295");
   ticks=parse_number(argv[5],10000,"Ticks must be 1..10000");if(!config.seed || !ticks)throw std::invalid_argument("Demo seed and ticks must be nonzero");
   for(int i=6;i<argc;){const std::string option(argv[i++]);
    if(option=="--distance" && i<argc && !content.pass_distance){std::ifstream source(argv[i++],std::ios::binary);if(!source)throw std::invalid_argument("Cannot open pass distance file");content.pass_distance=captain::read_pass_distance_content(source);}
    else if(option=="--velocity" && i<argc && !content.ball_velocity){std::ifstream source(argv[i++]);if(!source)throw std::invalid_argument("Cannot open velocity file");content.ball_velocity=captain::read_ball_velocity_content(source);}
    else if(option=="--ball-fractions" && i+1<argc && !config.ball_fraction){auto x=parse_number(argv[i++],255,"Ball fraction must be 0..255");auto y=parse_number(argv[i++],255,"Ball fraction must be 0..255");config.ball_fraction=captain::MatchBallFraction{static_cast<std::uint8_t>(x),static_cast<std::uint8_t>(y)};}
    else throw std::invalid_argument("Invalid, incomplete or repeated demo option");
   }
   std::cout<<"PROVISIONAL DEMO: simplified timing and random values; not original parity.\n";
  }
  auto start=captain::create_match(config,content);
  if(start.unsupported){std::cerr<<"Unsupported: "<<captain::capability_name(*start.unsupported)<<'\n';return 1;}
  auto state=start.state;
  for(unsigned i=1;i<=ticks;++i){
   auto step=captain::step_match(state,{captain::MatchInputKind::tick,i},content);
   if(step.unsupported){std::cout<<"Runtime stopped after "<<i-1<<" ticks: "<<captain::capability_name(*step.unsupported)<<'\n';return 0;}
   state=std::move(step.state);
   std::cout<<"tick "<<i<<" phase "<<unsigned(state.session.phase)<<" random "<<unsigned(state.session.current.field.random_first)<<','<<unsigned(state.session.current.field.random_second)<<'\n';
  }
  return 0;
 } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

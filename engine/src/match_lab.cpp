#include "captain/match_runtime.hpp"
#include "captain/match_save.hpp"
#include "captain/phase_session_io.hpp"
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <stdexcept>
namespace {
unsigned parse_number(const char* text,unsigned max,const char* message){
 const std::string value(text);unsigned result=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);
 if(value.empty() || parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size() || result>max)throw std::invalid_argument(message);
 return result;
}
std::string read_file(const std::string& path) {
 std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("Cannot open input file: "+path);
 std::string bytes{std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};
 if(in.bad())throw std::runtime_error("Cannot read input file: "+path);
 return bytes;
}
void save_new_file(const std::string& path,const captain::MatchRuntimeState& state,const captain::MatchContentIdentity& identity) {
 namespace fs=std::filesystem;
 const fs::path destination(path),temporary=fs::path(path+".tmp");
 if(fs::exists(destination) || fs::exists(temporary))throw std::runtime_error("Save destination or temporary file already exists");
 try {
  std::ofstream output(temporary,std::ios::binary|std::ios::trunc);if(!output)throw std::runtime_error("Cannot create temporary save");
  captain::write_match_save(output,state,identity);output.close();if(!output)throw std::runtime_error("Cannot finish temporary save");
  fs::rename(temporary,destination);
 } catch(...) {std::error_code ignored;fs::remove(temporary,ignored);throw;}
}
}
int main(int argc,char** argv) {
 if(argc<3){std::cerr<<"Usage: captain-match-lab CONTENT_PACK DEVELOPMENT_SESSION [RANDOM_INDEX | --demo SEED TICKS [options]]\n"
  <<"       captain-match-lab CONTENT_PACK --resume SAVE TICKS [options]\n"
  <<"Options: --distance FILE --velocity FILE --save NEW_FILE; initial demo also: --ball-fractions X Y\n";return 2;}
 try {
  const bool resume=std::string(argv[2])=="--resume";captain::MatchConfig config{};
  std::string session_path,save_path,resume_path;std::optional<std::string> distance_path,velocity_path;
  unsigned ticks=1;int option_start=argc;
  if(resume){
   if(argc<5)throw std::invalid_argument("Expected --resume SAVE TICKS");
   resume_path=argv[3];
   ticks=parse_number(argv[4],10000,"Ticks must be 1..10000");option_start=5;
  } else {
   session_path=argv[2];
   if(argc==4 && std::string(argv[3])!="--demo")config.random_index=static_cast<std::uint8_t>(parse_number(argv[3],255,"Random index must be 0..255"));
   else if(argc>3){
    if(argc<6 || std::string(argv[3])!="--demo")throw std::invalid_argument("Expected --demo SEED TICKS");
    config.mode=captain::MatchTimingMode::provisional_demo;config.seed=parse_number(argv[4],0xffffffffu,"Seed must be 1..4294967295");
    ticks=parse_number(argv[5],10000,"Ticks must be 1..10000");if(!config.seed)throw std::invalid_argument("Demo seed must be nonzero");option_start=6;
   }
  }
  if(!ticks)throw std::invalid_argument("Ticks must be nonzero");
  for(int i=option_start;i<argc;){const std::string option(argv[i++]);
   if(option=="--distance" && i<argc && !distance_path)distance_path=argv[i++];
   else if(option=="--velocity" && i<argc && !velocity_path)velocity_path=argv[i++];
   else if(option=="--save" && i<argc && save_path.empty())save_path=argv[i++];
   else if(option=="--ball-fractions" && !resume && config.mode==captain::MatchTimingMode::provisional_demo && i+1<argc && !config.ball_fraction){auto x=parse_number(argv[i++],255,"Ball fraction must be 0..255");auto y=parse_number(argv[i++],255,"Ball fraction must be 0..255");config.ball_fraction=captain::MatchBallFraction{static_cast<std::uint8_t>(x),static_cast<std::uint8_t>(y)};}
   else throw std::invalid_argument("Invalid, incomplete or repeated option");
  }
  const auto pack_bytes=read_file(argv[1]);std::optional<std::string> distance_bytes,velocity_bytes;
  if(distance_path)distance_bytes=read_file(*distance_path);
  if(velocity_path)velocity_bytes=read_file(*velocity_path);
  const auto identity=captain::match_content_identity(pack_bytes,distance_bytes,velocity_bytes);
  std::istringstream pack(pack_bytes,std::ios::binary);captain::MatchContent content{captain::load_shot_content(pack)};
  if(distance_bytes){std::istringstream source(*distance_bytes,std::ios::binary);content.pass_distance=captain::read_pass_distance_content(source);}
  if(velocity_bytes){std::istringstream source(*velocity_bytes);content.ball_velocity=captain::read_ball_velocity_content(source);}
  captain::MatchRuntimeState state{};
  if(resume){std::ifstream saved(resume_path,std::ios::binary);if(!saved)throw std::runtime_error("Cannot open match save");state=captain::load_match_save(saved,identity);}
  else {
   std::ifstream snapshot(session_path,std::ios::binary);if(!snapshot)throw std::runtime_error("Cannot open session snapshot");config.initial=captain::load_phase_session(snapshot);
   auto start=captain::create_match(config,content);if(start.unsupported){std::cerr<<"Unsupported: "<<captain::capability_name(*start.unsupported)<<'\n';return 1;}state=start.state;
  }
  if(state.mode==captain::MatchTimingMode::provisional_demo)std::cout<<"PROVISIONAL DEMO: simplified timing and random values; not original parity.\n";
  unsigned completed=0;
  for(unsigned i=0;i<ticks;++i){
   if(state.last_sequence==std::numeric_limits<std::uint64_t>::max())throw std::runtime_error("Input sequence exhausted");
   auto step=captain::step_match(state,{captain::MatchInputKind::tick,state.last_sequence+1},content);
   if(step.unsupported){std::cout<<"Runtime stopped after "<<completed<<" ticks: "<<captain::capability_name(*step.unsupported)<<'\n';break;}
   state=std::move(step.state);++completed;
   std::cout<<"sequence "<<state.last_sequence<<" phase "<<unsigned(state.session.phase)<<" random "<<unsigned(state.session.current.field.random_first)<<','<<unsigned(state.session.current.field.random_second)<<'\n';
  }
  if(!save_path.empty()){save_new_file(save_path,state,identity);std::cout<<"Saved runtime checkpoint: "<<save_path<<'\n';}
  return 0;
 } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}

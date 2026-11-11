#include "captain/runtime_random.hpp"
#include "captain/phase_session_io.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
using namespace captain;
static void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static std::array<std::uint8_t,2048> read_ram(const char* path){
 std::ifstream input(path,std::ios::binary);std::array<std::uint8_t,2048> ram{};
 if(!input.read(reinterpret_cast<char*>(ram.data()),ram.size()) || input.peek()!=EOF)throw std::runtime_error("Invalid RAM capture");return ram;
}
int main(int argc,char** argv){try{
 if(argc==4 && std::string(argv[1])=="--projection"){
  std::ifstream snapshot(argv[2],std::ios::binary);const auto state=load_phase_session(snapshot);const auto original=read_ram(argv[3]);
  RuntimeRandomMemory projected{};project_field_random_memory(state.current.field,projected);
  unsigned count=0;for(unsigned address=0x300;address<0x400;++address)if(projected.known[address]){++count;if(projected.bytes[address]!=original[address])throw std::runtime_error("Projected original byte mismatch at "+std::to_string(address));}
  require(count>150,"Too few represented bytes");return 0;
 }
 if(argc==5 && std::string(argv[1])=="--original"){
  const auto before=read_ram(argv[2]),after=read_ram(argv[3]);
  RuntimeRandomMemory memory{};memory.bytes=before;memory.known.fill(true);
  const auto result=advance_runtime_random({before[0xe1],before[0xe2],before[0xe3]},memory,std::string(argv[4])=="1");
  require(bool(result),"Original random sample unexpectedly unavailable");
  require(result->state.index==after[0xe1] && result->state.first==after[0xe2] && result->state.second==after[0xe3],"Original random state mismatch");return 0;
 }
 RuntimeRandomMemory projection{};projection.known.fill(true);projection.bytes.fill(42);
 FieldPhaseState field{};field.roster[1]={7,8,0x1234};field.motion[0]={0x5678,0x9abc,10,11};
 project_field_random_memory(field,projection);
 require(projection.known[0x30c] && projection.bytes[0x30c]==7 && projection.bytes[0x30d]==0x34 && projection.bytes[0x30e]==0x12 && projection.bytes[0x30f]==8,"Roster projection differs");
 require(projection.bytes[0x311]==0x78 && projection.bytes[0x312]==0x56 && projection.bytes[0x313]==0xbc && projection.bytes[0x314]==0x9a && projection.bytes[0x315]==10 && projection.bytes[0x316]==11,"Motion projection differs");
 require(!projection.known[0x310] && !projection.known[0x317] && projection.known[0x700] && projection.bytes[0x700]==42,"Unknown or external bytes were mishandled");
 RuntimeRandomMemory memory{}; ReferenceRandomState state{255,42,91};
 require(!advance_runtime_random(state,memory,false),"Unknown memory must reject");
 for(auto address:{0x3ffu,0x7ffu,0x7fu}){memory.bytes[address]=static_cast<std::uint8_t>(address);memory.known[address]=true;}
 const auto expected=advance_reference_random(state,{255,255,127,false});
 const auto actual=advance_runtime_random(state,memory,false);
 require(bool(actual),"Known memory rejected");
 require(actual->state.index==0 && actual->state.first==expected.state.first && actual->state.second==expected.state.second && actual->carry==expected.carry,"Random arithmetic or index wrap differs");
 for(auto address:{0x3ffu,0x7ffu,0x7fu}) {
  memory.known[address]=false;
  require(!advance_runtime_random(state,memory,false),"Unknown input accepted");
  memory.known[address]=true;
 }
 const auto carry_expected=advance_reference_random(state,{255,255,127,true});
 const auto carry_actual=advance_runtime_random(state,memory,true);
 require(carry_actual && carry_actual->state.first==carry_expected.state.first && carry_actual->state.second==carry_expected.state.second && carry_actual->carry==carry_expected.carry,"Incoming carry lost");
 require(state.index==255 && state.first==42 && state.second==91,"Input state mutated");
 return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

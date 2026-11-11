#include "captain/runtime_random.hpp"
namespace captain {
void project_field_random_memory(const FieldPhaseState& field,RuntimeRandomMemory& memory) {
 for(unsigned address=0x300;address<0x400;++address)memory.known[address]=false;
 const auto set=[&](unsigned address,std::uint8_t value){if(address<0x400){memory.bytes[address]=value;memory.known[address]=true;}};
 for(unsigned id=0;id<22;++id){
  const auto base=0x300u+id*12;const auto& player=field.roster[id];
  set(base,player.type);set(base+1,static_cast<std::uint8_t>(player.stamina));set(base+2,static_cast<std::uint8_t>(player.stamina>>8));set(base+3,player.level);
  if(id==0 || id==11){const auto& keeper=field.keepers[id==0?0:1];set(base+5,keeper.timer);set(base+6,keeper.penalty_duration);set(base+7,keeper.penalty);set(base+10,keeper.stun);}
  else {const auto& motion=field.motion[field_motion_index(static_cast<std::uint8_t>(id))];set(base+5,static_cast<std::uint8_t>(motion.x));set(base+6,static_cast<std::uint8_t>(motion.x>>8));set(base+7,static_cast<std::uint8_t>(motion.y));set(base+8,static_cast<std::uint8_t>(motion.y>>8));set(base+9,motion.target_cell);set(base+10,motion.status);}
 }
}
std::optional<ReferenceRandomResult> advance_runtime_random(
 ReferenceRandomState state,const RuntimeRandomMemory& memory,bool incoming_carry) {
 const auto player=0x300u+state.index, sound=0x700u+state.index, tail=(0x780u+state.index)&0x7ffu;
 if(!memory.known[player] || !memory.known[sound] || !memory.known[tail])return std::nullopt;
 return advance_reference_random(state,sample_reference_random(memory.bytes,state.index,incoming_carry));
}
}

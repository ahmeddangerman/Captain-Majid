#include "captain/kickoff_session.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState prepare_session_kickoff_formation(const PhaseSessionState& input,std::uint8_t selected_side,const KickoffFormationContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::goal_kickoff_pending || (selected_side!=0 && selected_side!=11))
  throw std::invalid_argument("Formation requires prepared kickoff and selected side0/11");
 const auto& field=input.current.field;std::array<KickoffPosition,22> positions{};
 for(unsigned slot=0;slot<22;++slot){
  if(slot==0 || slot==11){const auto& k=field.keepers[slot==0?0:1];
   // Original overwrites keeper Y high and clears its low byte; old Y high is immaterial.
   positions[slot]={static_cast<std::uint16_t>(k.timer|(unsigned(k.penalty_duration)<<8)),k.penalty};
  }else{const auto& m=field.motion[field_motion_index(slot)];positions[slot]={m.x,m.y};}
 }
 const auto placed=place_kickoff_formation(positions,selected_side,field.teams[0].formation,field.teams[1].formation,content);
 auto state=input;state.current.field.side=selected_side;state.current.field.ball_owner=selected_side+8;
 KickoffFormationContext captured{};
 for(unsigned slot=0;slot<22;++slot){
  if(slot==0 || slot==11){const auto i=slot==0?0u:1u;auto& k=state.current.field.keepers[i];
   k.timer=placed[slot].x&255;k.penalty_duration=placed[slot].x>>8;k.penalty=placed[slot].y&255;
   captured.keeper_y[i]=placed[slot].y>>8;
  }else{auto& m=state.current.field.motion[field_motion_index(slot)];m.x=placed[slot].x;m.y=placed[slot].y;}
 }
 state.kickoff_formation=captured;state.phase=FieldSessionPhase::kickoff_formation_placed;
 validate_phase_session(state);return state;
}
}

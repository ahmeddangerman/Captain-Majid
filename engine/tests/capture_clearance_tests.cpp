#include "captain/capture_clearance.hpp"
#include "captain/phase_session_io.hpp"
#include "captain/defender_selection.hpp"
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
namespace {
void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid clearance accepted");}
std::string bytes(const captain::PhaseSessionState&s){std::ostringstream o;captain::write_phase_session(o,s);return o.str();}
}
int main(){
 using namespace captain;
 PhaseSessionState s{};s.phase=FieldSessionPhase::selected_defense_animation;
 s.experience=ExperienceState{};s.recovery=RecoveryContext{};s.control=ControlContext{};
 s.scheduling=SchedulingContext{};s.action=ActionContext{};s.shot_progress=ShotContactProgress{1,3,12,0,0};
 s.current.field.ball_owner=2;s.current.field.ball_x=95;s.current.field.ball_y=100;
 s.current.field.selected_defenders=1;s.scheduling->selected_ids={12,0,0,0,0};
 s.current.clock.remaining=449;s.experience->totals[0]=7;s.current.field.roster[2].stamina=123;
 CaptureClearanceContent content{};
 for(unsigned i=0;i<8;++i)content.destinations[i]={static_cast<std::uint8_t>(60+i*8),static_cast<std::uint8_t>(90+i*4)};
 content.distance_adjustment=4;
 auto run=[&]{return prepare_session_capture_clearance(s,content);};
 const auto before=bytes(s);auto r=run();
 check(static_cast<unsigned>(r.state.phase)==33 && r.state.current.field.side==11 &&
  r.state.current.field.ball_owner==2 && r.state.current.field.ball_x==60 && r.state.current.field.ball_y==90 &&
  r.state.current.field.ball_cell==field_cell_at(60,90),"Clearance switches side and stores destination without ownership transfer");
 check(r.state.current.clock.remaining==449 && r.state.experience->totals[0]==7 &&
  r.state.current.field.roster[2].stamina==123 && r.state.current.field.selected_defenders==1 &&
  r.request_side_music && r.requested_task==2 && bytes(s)==before,"Inputs/costs/prefix preserved with explicit presentation requests");
 auto encoded=bytes(r.state);check(encoded.size()==370 && encoded[10]=='W',"Session32 format");
 std::istringstream saved(encoded);check(bytes(load_phase_session(saved))==encoded,"Clearance side/owner mismatch snapshot roundtrip");
 encoded[10]='B';rejects([&]{std::istringstream old(encoded);load_phase_session(old);});
 auto legacy=before;legacy.resize(330);legacy[10]='B';std::istringstream old_entry(legacy);
 check(bytes(load_phase_session(old_entry))==before,"Version11 animation entry rewrites with absent flight context");
 auto initial=s;s=r.state;rejects(run);s=initial;
 for(unsigned mask=0;mask<4;++mask)for(unsigned sample=0;sample<8;++sample){
  s.current.field.ball_x=mask&1?160:95;s.current.field.ball_y=mask&2?150:100;
  s.current.field.random_first=static_cast<std::uint8_t>(sample+248);r=run();
  const auto x=static_cast<std::uint8_t>(content.destinations[sample][0]^(mask&1?255:0));
  const auto y=static_cast<std::uint8_t>(content.destinations[sample][1]^(mask&2?255:0));
  check(r.state.current.field.ball_x==x && r.state.current.field.ball_y==y,"Sample and ones-complement reflections");
 }
 s=initial;content.destinations[0]={0,0};rejects(run);check(bytes(s)==before,"Reached invalid target is atomic");
 std::cout<<"Capture clearance checks passed\n";
}

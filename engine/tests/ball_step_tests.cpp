#include "captain/ball_step.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
namespace {void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool v=false;try{f();}catch(const std::invalid_argument&){v=true;}check(v,"Invalid ball step accepted");}}
int main(){try{
 using namespace captain;BallStepInput s{(120<<8)|64,(100<<8)|192,{256,0},110,150};
 auto r=advance_ball_to_next_cell(s);
 check(r.x==((128<<8)|64) && r.y==s.y && r.cell==122 && r.additions==8 &&
  r.event==BallStepEvent::cell_changed && !r.forced_destination,"Advance repeatedly within same frame until new cell");
 s.destination=122;r=advance_ball_to_next_cell(s);
 check(r.x==((132<<8)|64) && r.y==s.y && r.additions==8 && r.event==BallStepEvent::arrived,
  "Arrival snaps high coordinates to center and preserves fractional bytes");
 s.destination=150;s.velocity.x=65535;r=advance_ball_to_next_cell(s);
 check(r.additions==65 && r.x==((119<<8)|255) && r.cell==98,"Negative fractional velocity preserves borrow and long subcell loop");
 s.x=(207<<8)|64;s.current_cell=230;s.destination=5;s.velocity.x=256;r=advance_ball_to_next_cell(s);
 check(r.forced_destination && r.cell==5 && r.x==((52<<8)|64) && r.y==((124<<8)|192) &&
  r.event==BallStepEvent::arrived,"Outside field follows original forced destination, not player clamp");
 s.destination=s.current_cell;r=advance_ball_to_next_cell(s);
 check(r.additions==0 && r.x==s.x && r.event==BallStepEvent::already_at_destination,"Already at destination is untouched");
 s.destination=5;s.velocity={0,0};rejects([&]{advance_ball_to_next_cell(s);});
 s.velocity.x=256;s.destination=240;rejects([&]{advance_ball_to_next_cell(s);});
 std::cout<<"Ball coordinate step checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

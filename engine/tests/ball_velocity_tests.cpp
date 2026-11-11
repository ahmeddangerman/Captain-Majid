#include "captain/ball_velocity.hpp"
#include <iostream>
#include <cstdlib>
#include <exception>
#include <sstream>
#include <stdexcept>
namespace {void check(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}}
int main(){try{
 using namespace captain;BallVelocityContent c{};
 for(unsigned i=0;i<64;++i)c.quarter_wave[i]=i*257+3;
 auto at=[&](unsigned a,unsigned x,unsigned y){auto v=calculate_ball_velocity(a,c);check(v.x==x && v.y==y,"Signed ball projection/quadrant boundary");};
 at(0,16194,3);at(63,3,16194);at(64,65533,16194);at(127,49342,3);
 at(128,49342,65533);at(191,65533,49342);at(192,3,49342);at(255,16194,65533);
 c.quarter_wave.fill(32768);auto v=calculate_ball_velocity(128,c);
 check(v.x==32768 && v.y==32768,"Projection negation wraps at signed minimum");
 c.quarter_wave.fill(0);for(unsigned d=0;d<256;++d){v=calculate_ball_velocity(d,c);check(v.x==0 && v.y==0,"Zero projection in every quadrant");}
 std::ostringstream encoded;encoded<<"CMJBALL1";for(unsigned i=0;i<64;++i)encoded<<' '<<i;
 std::istringstream valid(encoded.str());auto loaded=read_ball_velocity_content(valid);check(loaded.quarter_wave[63]==63,"Velocity table roundtrip");
 auto rejects=[](const std::string& input){try{std::istringstream source(input);read_ball_velocity_content(source);return false;}catch(const std::invalid_argument&){return true;}};
 check(rejects("bad") && rejects("CMJBALL1 1") && rejects(encoded.str()+" 2"),"Malformed velocity tables rejected");
 std::ostringstream invalid;invalid<<"CMJBALL1 -1";for(unsigned i=1;i<64;++i)invalid<<" 0";
 check(rejects(invalid.str()),"Negative velocity word rejected");
 std::ostringstream oversized;oversized<<"CMJBALL1 65536";for(unsigned i=1;i<64;++i)oversized<<" 0";
 check(rejects(oversized.str()),"Oversized velocity word rejected");
 std::cout<<"Ball velocity checks passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

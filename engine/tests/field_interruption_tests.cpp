#include "captain/field_interruption.hpp"
#include "captain/defender_outcome.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
namespace{void req(bool b,const char*m){if(!b){std::cerr<<m<<'\n';std::exit(1);}}}
int main(){
 captain::FieldInterruptionInput in{0,1,3,2,0,120,100,63,{}};in.state.status.fill(7);in.state.keeper_penalties={8,9};
 auto r=captain::resolve_field_interruption(in);req(r.triggered&&r.result_code==4&&r.state.status[21]==0&&r.state.keeper_penalties[0]==0&&r.restart->route==captain::FieldRestartRoute::field_restart&&r.restart->situation==0,"ordinary threshold and resets");
 req(in.state.status[21]==7&&in.state.keeper_penalties[0]==8,"input unchanged");
 in.random_first=64;r=captain::resolve_field_interruption(in);req(!r.triggered&&r.state.status==in.state.status&&!r.restart,"outside inclusive threshold unchanged");
 in.ball_x=0xb0;in.ball_y=0x60;in.random_first=15;r=captain::resolve_field_interruption(in);req(r.triggered&&r.restart->route==captain::FieldRestartRoute::goal_area_restart,"opposing goal-area threshold and route");
 in.random_first=16;req(!captain::resolve_field_interruption(in).triggered,"goal-area threshold upper boundary");
 in.possession_side=11;in.random_first=15;r=captain::resolve_field_interruption(in);req(r.restart->route==captain::FieldRestartRoute::field_restart,"own goal area uses field route");
 for(auto y:{0x5f,0xa0}){in.ball_y=y;in.random_first=63;req(captain::resolve_field_interruption(in).triggered,"outside vertical band uses ordinary threshold");}
 in.ball_y=100;in.ball_x=120;in.random_first=0;
 for(auto code:{1,2,3}){in.result_code=code;r=captain::resolve_field_interruption(in);req(!r.triggered&&r.result_code==code&&r.state.status==in.state.status,"existing nonzero result unchanged");}
 in.result_code=0;in.selected_defenders=0;req(!captain::resolve_field_interruption(in).triggered,"empty selected list bypasses override");in.selected_defenders=1;
 for(auto action:{5,6}){in.defense_action=action;req(!captain::resolve_field_interruption(in).triggered,"field actions5/6 bypass");}
 in.defender_id=0;in.defense_action=4;req(!captain::resolve_field_interruption(in).triggered,"keeper action4 bypass");in.defense_action=5;req(captain::resolve_field_interruption(in).triggered,"keeper action5 is not field action5 bypass");
 in.result_code=4;r=captain::resolve_field_interruption(in);req(!r.triggered&&r.restart&&r.state.status==in.state.status,"existing result4 routes without another reset");
 const auto outcome=captain::resolve_defender_outcome({4,11,15,3,2,{}});req(outcome.route==captain::DefenderRoute::restart_decision&&outcome.ball_owner==15&&outcome.possession_side==11,"result4 does not award possession as result3");
 bool bad=false;in.possession_side=1;try{captain::resolve_field_interruption(in);}catch(const std::invalid_argument&){bad=true;}req(bad,"invalid side rejected");
 std::cout<<"Field interruption checks passed\n";
}

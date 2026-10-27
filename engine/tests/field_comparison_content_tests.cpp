#include "captain/field_comparison_content.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
namespace {void req(bool b,const char*m){if(!b){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool b=false;try{f();}catch(const std::invalid_argument&){b=true;}req(b,"invalid selection accepted");}}
int main(){
 captain::FieldComparisonContent c{{1,2,0,3,0},{0,1,2,3,0},std::vector<std::uint8_t>(16),std::vector<std::array<std::uint8_t,2>>(16),{210,120,60,0}};
 c.eligibility[7]=128;c.variation[7]={77,33};c.variation[6]={90,20};
 auto zero=captain::select_field_comparison(2,0,12,c);req(zero.range==0&&zero.offset==0,"zero-width content is preserved by selection, not silently corrected");
 auto r=captain::select_field_comparison(0,3,12,c);
 req(r.table_index==7&&r.range==77&&r.offset==33&&r.eligible&&r.thresholds[0]==210,"action groups select content");
 r=captain::select_field_comparison(0,3,11,c);req(r.table_index==6&&r.range==90&&!r.eligible,"keeper uses fixed defense group2");
 c.eligibility[7]=127;r=captain::select_field_comparison(0,3,1,c);req(!r.eligible,"only eligibility sign bit matters");
 captain::DefenderContactInput in{256,240,100,100,1,1,true,false,4,0,193,{0,0,0,0}};
 auto linked=captain::configure_field_comparison(in,0,3,12,c);
 req(linked.range==77&&linked.offset==33&&linked.defender_slot==12&&linked.thresholds==c.thresholds&&linked.incoming_power==256&&linked.defender_power==240&&linked.weakened&&linked.subtype==193&&in.range==1,"configuration preserves calculation inputs and caller");
 c.attack_groups[0]=255;c.defense_groups[3]=7;c.eligibility.resize(256);c.variation.resize(128);c.variation[3]={5,6};
 r=captain::select_field_comparison(0,3,12,c);req(r.table_index==3&&r.range==5,"byte arithmetic wraps group addition");
 c.attack_groups[0]=32;c.defense_groups[3]=3;c.variation[3]={9,10};
 r=captain::select_field_comparison(0,3,12,c);req(r.table_index==131&&r.range==9,"coefficient offset wraps while eligibility index remains full byte");
 rejects([&]{captain::select_field_comparison(5,0,12,c);});rejects([&]{captain::select_field_comparison(0,5,12,c);});rejects([&]{captain::select_field_comparison(0,0,22,c);});
 c.eligibility.clear();rejects([&]{captain::select_field_comparison(0,3,12,c);});c.eligibility.resize(256);c.variation.clear();rejects([&]{captain::select_field_comparison(0,3,12,c);});
 std::cout<<"Field comparison selection checks passed\n";
}

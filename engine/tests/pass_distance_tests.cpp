#include "captain/pass_distance.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
void check(bool value,const char* message) { if(!value)throw std::runtime_error(message); }
template<class F> void rejects(F operation) { try { operation(); } catch(const std::invalid_argument&) {return;} throw std::runtime_error("Invalid pass distance input accepted"); }
}
int main() { try {
 using namespace captain;
 PassDistanceContent content{};for(unsigned i=0;i<content.values.size();++i)content.values[i]=static_cast<std::uint8_t>(i);
 check(calculate_pass_distance(48,80,0,content)==0,"Nearby target index");
 check(calculate_pass_distance(60,84,0,content)==1,"Eight-pixel boundary");
 check(calculate_pass_distance(180,84,0,content)==16,"Positive 128-byte difference");
 check(calculate_pass_distance(48,80,239,content)==19,"Opposite field corner");
 check(calculate_pass_distance(207,175,0,content)==19,"Reverse field corner");
 rejects([&]{calculate_pass_distance(47,80,0,content);});
 rejects([&]{calculate_pass_distance(208,80,0,content);});
 rejects([&]{calculate_pass_distance(48,79,0,content);});
 rejects([&]{calculate_pass_distance(48,176,0,content);});
 rejects([&]{calculate_pass_distance(48,80,240,content);});
 const std::string magic="CMJPDST1";std::string bytes=magic;for(unsigned i=0;i<20;++i)bytes.push_back(static_cast<char>(i));
 std::istringstream input(bytes);check(read_pass_distance_content(input).values==content.values,"Table roundtrip");
 auto high_bytes=bytes;high_bytes[8]=static_cast<char>(255);high_bytes[9]=static_cast<char>(128);
 std::istringstream high_input(high_bytes);const auto high_table=read_pass_distance_content(high_input);
 check(high_table.values[0]==255 && high_table.values[1]==128 && calculate_pass_distance(48,80,0,high_table)==255,
       "Replaceable table preserves all byte values");
 auto corrupt=bytes;corrupt[0]='X';rejects([&]{std::istringstream in(corrupt);read_pass_distance_content(in);});
 corrupt=bytes.substr(0,bytes.size()-1);rejects([&]{std::istringstream in(corrupt);read_pass_distance_content(in);});
 corrupt=bytes+'x';rejects([&]{std::istringstream in(corrupt);read_pass_distance_content(in);});
 std::cout<<"Pass distance checks passed\n";return 0;
 } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;} }

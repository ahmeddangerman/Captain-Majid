#include "captain/match_save.hpp"
#include "captain/phase_session_io.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <limits>

namespace captain {
namespace {
constexpr char magic[]="CMJMATCH";
constexpr std::uint8_t current_version=13;
constexpr std::uint32_t max_payload=1024*1024;
void put(std::string& out,std::uint64_t n,unsigned bytes) {
 for(unsigned i=0;i<bytes;++i)out.push_back(static_cast<char>((n>>(8*i))&255));
}
struct Reader {
 const std::string& data;std::size_t pos=0;
 std::uint64_t get(unsigned count) {
  if(count>data.size()-pos)throw std::invalid_argument("Truncated match save");
  std::uint64_t result=0;for(unsigned i=0;i<count;++i)result|=std::uint64_t(static_cast<unsigned char>(data[pos++]))<<(8*i);
  return result;
 }
 bool flag() {auto n=get(1);if(n>1)throw std::invalid_argument("Invalid match save flag");return n!=0;}
};
std::uint32_t checksum(const std::string& data) {
 std::uint32_t crc=0xffffffffu;
 for(unsigned char c:data){crc^=c;for(unsigned i=0;i<8;++i)crc=(crc>>1)^(0xedb88320u&static_cast<std::uint32_t>(-static_cast<std::int32_t>(crc&1)));}
 return ~crc;
}
void validate_identity(const MatchContentIdentity& identity) {
 if(std::all_of(identity.begin(),identity.end(),[](std::uint8_t n){return n==0;}))
  throw std::invalid_argument("Match save requires a content identity");
}
void validate_runtime(const MatchRuntimeState& s) {
 validate_phase_session(s.session);
 if((s.mode!=MatchTimingMode::original_backed && s.mode!=MatchTimingMode::provisional_demo) ||
    (s.mode==MatchTimingMode::original_backed && s.demo_rng) ||
    (s.mode==MatchTimingMode::provisional_demo && !s.demo_rng) ||
    static_cast<unsigned>(s.pass_stage)>static_cast<unsigned>(MatchPassStage::flight_arrived))
  throw std::invalid_argument("Invalid match runtime mode or pass stage");
 const bool target=static_cast<unsigned>(s.pass_stage)>=static_cast<unsigned>(MatchPassStage::target_ready);
 const bool flight=static_cast<unsigned>(s.pass_stage)>=static_cast<unsigned>(MatchPassStage::flight_ready);
 const bool ai_flight=s.defense_menu && static_cast<unsigned>(s.defense_menu->stage)>=static_cast<unsigned>(MatchDefenseStage::ai_flight_ready);
 const auto side=s.session.current.field.side;const auto capacity=side==0 ? 5u : 4u;
 if(!s.flight_defenders.empty() && (s.mode!=MatchTimingMode::provisional_demo || (!flight && !ai_flight) || (side!=0 && side!=11) || s.flight_defenders.size()>capacity))
  throw std::invalid_argument("Invalid flight defender prefix");
 for(const auto id:s.flight_defenders)if((side==0 && (id<12 || id>21)) || (side==11 && (id<1 || id>10)))
  throw std::invalid_argument("Invalid flight defender ID");
 const bool arrived=s.pass_stage==MatchPassStage::flight_arrived || (s.defense_menu && s.defense_menu->stage==MatchDefenseStage::ai_flight_arrived);
 if(!s.processed_flight_defenders.empty() && (s.mode!=MatchTimingMode::provisional_demo || !arrived || (side!=0 && side!=11) || s.flight_defenders.empty() || s.flight_defenders.size()+s.processed_flight_defenders.size()>capacity || s.session.current.field.elapsed!=s.processed_flight_defenders.size()))
  throw std::invalid_argument("Invalid processed flight defender prefix");
 for(const auto id:s.processed_flight_defenders)if((side==0 && (id<12 || id>21)) || (side==11 && (id<1 || id>10)))
  throw std::invalid_argument("Invalid processed flight defender ID");
 if(bool(s.pass_target)!=target || bool(s.pass_trajectory)!=flight || bool(s.pass_cell)!=flight ||
    (s.pass_stage!=MatchPassStage::none && (s.mode!=MatchTimingMode::provisional_demo || s.session.phase!=FieldSessionPhase::ordinary_pass_prepared)))
  throw std::invalid_argument("Inconsistent match pass state");
 if(target && (!s.session.ordinary_pass_recipient || s.pass_target->recipient!=*s.session.ordinary_pass_recipient ||
    s.pass_target->recipient<1 || s.pass_target->recipient>10 || s.pass_target->destination>=240 || s.pass_target->counter!=255))
  throw std::invalid_argument("Invalid match pass target");
 if(flight && (*s.pass_cell>=240 || s.pass_trajectory->flags!=1 || !s.session.saved_ball_flight_flag))
  throw std::invalid_argument("Invalid match flight state");
 if(s.target_chooser && (s.mode!=MatchTimingMode::provisional_demo || s.session.phase!=FieldSessionPhase::ordinary_human_pass_target ||
    s.pass_stage!=MatchPassStage::none || s.defense_menu || !s.session.ordinary_pass_candidates || !s.session.ordinary_pass_candidates->scanned ||
    s.session.ordinary_pass_candidates->candidates.allied_count<2 || s.target_chooser->index>=s.session.ordinary_pass_candidates->candidates.allied_count))
  throw std::invalid_argument("Invalid match target chooser");
 if(s.defense_menu) {
  const auto& m=*s.defense_menu;const auto count=m.defender_count;
  const bool flight=static_cast<unsigned>(m.stage)>=static_cast<unsigned>(MatchDefenseStage::ai_flight_ready);
  if(s.mode!=MatchTimingMode::provisional_demo || (flight ? s.session.phase!=FieldSessionPhase::field : s.session.phase!=FieldSessionPhase::field_contact_pending) ||
     s.session.current.field.side!=11 || s.pass_stage!=MatchPassStage::none || !s.session.scheduling || !count || count>4 ||
     (flight ? s.session.current.field.selected_defenders!=0 : s.session.current.field.selected_defenders!=count) ||
     static_cast<unsigned>(m.stage)>static_cast<unsigned>(MatchDefenseStage::ai_flight_arrived) || m.poll.cursor>count ||
     (static_cast<unsigned>(m.stage)>=static_cast<unsigned>(MatchDefenseStage::complete) && m.poll.cursor!=count) ||
     (m.stage==MatchDefenseStage::ai_chosen && (!s.session.action || !s.session.recovery || s.session.recovery->attack_subtype!=0)) ||
     (static_cast<unsigned>(m.stage)>=static_cast<unsigned>(MatchDefenseStage::ai_chosen) && (!s.session.action || !s.session.recovery || (s.session.action->action!=1 && s.session.action->action!=2))) ||
     (bool(m.ai_pass_target)!=(static_cast<unsigned>(m.stage)>=static_cast<unsigned>(MatchDefenseStage::ai_chosen) && s.session.action && s.session.action->action==1)) ||
     (m.ai_pass_target && (m.ai_pass_target->recipient<12 || m.ai_pass_target->recipient>21 || m.ai_pass_target->cell>=240)) ||
     (static_cast<unsigned>(m.stage)>=static_cast<unsigned>(MatchDefenseStage::ai_pass_handoff) && (!s.session.action || s.session.action->action!=1)) ||
     (bool(m.ai_pass_distance)!=(static_cast<unsigned>(m.stage)>=static_cast<unsigned>(MatchDefenseStage::ai_pass_target_ready))) ||
     (bool(m.ai_trajectory)!=flight) || (bool(m.ai_ball_cell)!=flight) ||
     (m.ai_trajectory && (m.ai_trajectory->flags!=1 || *m.ai_ball_cell>=240 || !s.session.saved_ball_flight_flag)) ||
     (m.stage==MatchDefenseStage::ai_flight_arrived && *m.ai_ball_cell!=m.ai_pass_target->cell) ||
     (bool(m.attack_cost)!=(static_cast<unsigned>(m.stage)>=static_cast<unsigned>(MatchDefenseStage::attack_prepared))) ||
     (m.stage==MatchDefenseStage::choice_confirmed && (m.poll.cursor>=count || m.poll.defense.actions[m.poll.cursor]==255 || !(m.poll.defense.pressed&128u))) ||
     (m.stage!=MatchDefenseStage::choice_confirmed && m.poll.defense.pressed))
   throw std::invalid_argument("Invalid match defensive menu state");
  for(unsigned slot=0;slot<count;++slot)
   if(m.poll.defense.actions[slot]!=255 && m.poll.defense.actions[slot]>6)throw std::invalid_argument("Invalid saved defensive action");
  for(unsigned slot=0;slot<m.poll.cursor;++slot)
   if(m.poll.defense.actions[slot]!=6)throw std::invalid_argument("Unregistered saved defensive choice");
 }
}
std::uint32_t rotate(std::uint32_t value,unsigned bits) {return (value>>bits)|(value<<(32-bits));}
MatchContentIdentity sha256(std::string data) {
 static constexpr std::uint32_t k[64]={
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
 if(data.size()>std::numeric_limits<std::uint64_t>::max()/8)throw std::invalid_argument("Content is too large to identify");
 const auto bits=static_cast<std::uint64_t>(data.size())*8;
 data.push_back(static_cast<char>(0x80));while(data.size()%64!=56)data.push_back(0);
 for(int i=7;i>=0;--i)data.push_back(static_cast<char>(bits>>(8*i)));
 std::uint32_t h[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
 for(std::size_t block=0;block<data.size();block+=64){
  std::uint32_t w[64]{};
  for(unsigned i=0;i<16;++i)for(unsigned j=0;j<4;++j)w[i]=(w[i]<<8)|static_cast<unsigned char>(data[block+i*4+j]);
  for(unsigned i=16;i<64;++i){auto a=rotate(w[i-15],7)^rotate(w[i-15],18)^(w[i-15]>>3);auto b=rotate(w[i-2],17)^rotate(w[i-2],19)^(w[i-2]>>10);w[i]=w[i-16]+a+w[i-7]+b;}
  auto a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],z=h[7];
  for(unsigned i=0;i<64;++i){auto s1=rotate(e,6)^rotate(e,11)^rotate(e,25);auto choice=(e&f)^(~e&g);auto t1=z+s1+choice+k[i]+w[i];auto s0=rotate(a,2)^rotate(a,13)^rotate(a,22);auto majority=(a&b)^(a&c)^(b&c);auto t2=s0+majority;z=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;}
  h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=z;
 }
 MatchContentIdentity result{};for(unsigned i=0;i<8;++i)for(unsigned j=0;j<4;++j)result[i*4+j]=static_cast<std::uint8_t>(h[i]>>(24-8*j));return result;
}
}
MatchContentIdentity match_content_identity(const std::string& pack,const std::optional<std::string>& distance,const std::optional<std::string>& velocity) {
 std::string bytes="CMJCONTENT1";
 const auto append=[&](const std::optional<std::string>& part){put(bytes,part.has_value(),1);if(part){put(bytes,part->size(),8);bytes+=*part;}};
 append(pack);append(distance);append(velocity);return sha256(std::move(bytes));
}
void write_match_save(std::ostream& output,const MatchRuntimeState& s,const MatchContentIdentity& identity) {
 validate_identity(identity);validate_runtime(s);
 std::ostringstream session(std::ios::out|std::ios::binary);write_phase_session(session,s.session);
 const auto session_bytes=session.str();std::string payload;
 payload.append(reinterpret_cast<const char*>(identity.data()),identity.size());
 put(payload,session_bytes.size(),4);payload+=session_bytes;
 put(payload,s.last_sequence,8);put(payload,s.random_index.has_value(),1);put(payload,s.random_index.value_or(0),1);
 for(auto byte:s.random_memory.bytes)put(payload,byte,1);
 for(bool known:s.random_memory.known)put(payload,known,1);
 put(payload,static_cast<unsigned>(s.mode),1);put(payload,s.demo_rng,4);put(payload,static_cast<unsigned>(s.pass_stage),1);
 put(payload,s.pass_target.has_value(),1);if(s.pass_target)for(auto n:{s.pass_target->recipient,s.pass_target->destination,s.pass_target->counter,s.pass_target->distance})put(payload,n,1);
 put(payload,s.ball_fraction.has_value(),1);if(s.ball_fraction){put(payload,s.ball_fraction->x,1);put(payload,s.ball_fraction->y,1);}
 put(payload,s.pass_trajectory.has_value(),1);if(s.pass_trajectory){const auto& t=*s.pass_trajectory;for(auto n:{t.x,t.y,t.vx,t.vy})put(payload,n,2);put(payload,t.direction,1);put(payload,t.flags,1);}
 put(payload,s.pass_cell.has_value(),1);if(s.pass_cell)put(payload,*s.pass_cell,1);
 put(payload,s.defense_menu.has_value(),1);if(s.defense_menu){const auto& m=*s.defense_menu;put(payload,static_cast<unsigned>(m.stage),1);put(payload,m.poll.cursor,1);put(payload,m.poll.flash_counter,1);for(auto n:m.poll.defense.actions)put(payload,n,1);for(auto n:m.poll.defense.subtypes)put(payload,n,1);put(payload,m.poll.defense.blink,1);put(payload,m.poll.defense.pressed,1);put(payload,m.attack_cost.has_value(),1);if(m.attack_cost)put(payload,*m.attack_cost,2);put(payload,m.ai_pass_target.has_value(),1);if(m.ai_pass_target){put(payload,m.ai_pass_target->recipient,1);put(payload,m.ai_pass_target->cell,1);}put(payload,m.ai_pass_distance.has_value(),1);if(m.ai_pass_distance)put(payload,*m.ai_pass_distance,1);put(payload,m.defender_count,1);put(payload,m.ai_trajectory.has_value(),1);if(m.ai_trajectory){const auto& t=*m.ai_trajectory;for(auto n:{t.x,t.y,t.vx,t.vy})put(payload,n,2);put(payload,t.direction,1);put(payload,t.flags,1);}put(payload,m.ai_ball_cell.has_value(),1);if(m.ai_ball_cell)put(payload,*m.ai_ball_cell,1);}
 put(payload,s.target_chooser.has_value(),1);if(s.target_chooser)put(payload,s.target_chooser->index,1);
 put(payload,s.flight_defenders.size(),1);for(const auto id:s.flight_defenders)put(payload,id,1);
 put(payload,s.processed_flight_defenders.size(),1);for(const auto id:s.processed_flight_defenders)put(payload,id,1);
 if(payload.size()>max_payload)throw std::invalid_argument("Match save is too large");
 std::string header(magic,sizeof(magic)-1);put(header,current_version,1);put(header,payload.size(),4);put(header,checksum(payload),4);
 output.write(header.data(),header.size());output.write(payload.data(),payload.size());
 if(!output)throw std::invalid_argument("Unable to write match save");
}
MatchRuntimeState load_match_save(std::istream& input,const MatchContentIdentity& identity) {
 validate_identity(identity);
 std::string header(sizeof(magic)+8,'\0');input.read(&header[0],header.size());
 if(input.gcount()!=static_cast<std::streamsize>(header.size()) || header.compare(0,sizeof(magic)-1,magic,sizeof(magic)-1))
  throw std::invalid_argument("Unsupported or truncated match save");
 Reader head{header,sizeof(magic)-1};const auto version=head.get(1);if(version<1 || version>current_version)throw std::invalid_argument("Unsupported match save version");
 auto size=head.get(4);auto expected=head.get(4);
 if(size>max_payload)throw std::invalid_argument("Match save is too large");
 std::string payload(size,'\0');input.read(payload.data(),size);
 if(input.gcount()!=static_cast<std::streamsize>(size) || input.peek()!=std::char_traits<char>::eof() || checksum(payload)!=expected)
  throw std::invalid_argument("Corrupt or truncated match save");
 Reader r{payload};for(auto n:identity)if(r.get(1)!=n)throw std::invalid_argument("Match save content identity mismatch");
 auto session_size=r.get(4);if(session_size>payload.size()-r.pos)throw std::invalid_argument("Truncated match session");
 std::istringstream session(payload.substr(r.pos,session_size),std::ios::in|std::ios::binary);r.pos+=session_size;
 MatchRuntimeState s{};s.session=load_phase_session(session);
 if(session.peek()!=std::char_traits<char>::eof())throw std::invalid_argument("Unexpected match session data");
 s.last_sequence=r.get(8);const bool random_index=r.flag();auto index=r.get(1);if(random_index)s.random_index=static_cast<std::uint8_t>(index);else if(index)throw std::invalid_argument("Invalid absent random index");
 for(auto& byte:s.random_memory.bytes)byte=static_cast<std::uint8_t>(r.get(1));
 for(auto& known:s.random_memory.known)known=r.flag();
 s.mode=static_cast<MatchTimingMode>(r.get(1));s.demo_rng=static_cast<std::uint32_t>(r.get(4));s.pass_stage=static_cast<MatchPassStage>(r.get(1));
 if(r.flag()){MatchPassTarget t{};t.recipient=r.get(1);t.destination=r.get(1);t.counter=r.get(1);t.distance=r.get(1);s.pass_target=t;}
 if(r.flag()){MatchBallFraction f{};f.x=r.get(1);f.y=r.get(1);s.ball_fraction=f;}
 if(r.flag()){BallTrajectoryContext t{};t.x=r.get(2);t.y=r.get(2);t.vx=r.get(2);t.vy=r.get(2);t.direction=r.get(1);t.flags=r.get(1);s.pass_trajectory=t;}
 if(r.flag())s.pass_cell=static_cast<std::uint8_t>(r.get(1));
 if(version>=2 && r.flag()){MatchDefenseMenu m{};m.stage=static_cast<MatchDefenseStage>(r.get(1));m.poll.cursor=r.get(1);m.poll.flash_counter=r.get(1);for(auto& n:m.poll.defense.actions)n=r.get(1);for(auto& n:m.poll.defense.subtypes)n=r.get(1);m.poll.defense.blink=r.get(1);m.poll.defense.pressed=r.get(1);if(version>=4 && r.flag())m.attack_cost=static_cast<std::uint16_t>(r.get(2));if(version>=6 && r.flag()){MatchAiPassTarget t{};t.recipient=r.get(1);t.cell=r.get(1);m.ai_pass_target=t;}if(version>=8 && r.flag())m.ai_pass_distance=static_cast<std::uint8_t>(r.get(1));if(version>=9){m.defender_count=r.get(1);if(r.flag()){BallTrajectoryContext t{};t.x=r.get(2);t.y=r.get(2);t.vx=r.get(2);t.vy=r.get(2);t.direction=r.get(1);t.flags=r.get(1);m.ai_trajectory=t;}if(r.flag())m.ai_ball_cell=static_cast<std::uint8_t>(r.get(1));}else m.defender_count=s.session.current.field.selected_defenders;s.defense_menu=m;}
 if(version<3 && s.defense_menu && s.defense_menu->stage==MatchDefenseStage::ai_chosen)throw std::invalid_argument("Unsupported saved defense stage");
 if(version<4 && s.defense_menu && s.defense_menu->stage==MatchDefenseStage::attack_prepared)throw std::invalid_argument("Unsupported saved attack stage");
 if(version<5 && s.defense_menu && static_cast<unsigned>(s.defense_menu->stage)>static_cast<unsigned>(MatchDefenseStage::attack_prepared))throw std::invalid_argument("Unsupported saved clock stage");
 if(version<7 && s.defense_menu && static_cast<unsigned>(s.defense_menu->stage)>static_cast<unsigned>(MatchDefenseStage::dispatch_pending))throw std::invalid_argument("Unsupported saved opponent pass stage");
 if(version<8 && s.defense_menu && static_cast<unsigned>(s.defense_menu->stage)>static_cast<unsigned>(MatchDefenseStage::ai_pass_handoff))throw std::invalid_argument("Unsupported saved opponent target stage");
 if(version<9 && s.defense_menu && static_cast<unsigned>(s.defense_menu->stage)>static_cast<unsigned>(MatchDefenseStage::ai_pass_target_ready))throw std::invalid_argument("Unsupported saved opponent flight stage");
 if(version<10 && s.defense_menu && static_cast<unsigned>(s.defense_menu->stage)>static_cast<unsigned>(MatchDefenseStage::ai_flight_ready))throw std::invalid_argument("Unsupported saved opponent flight progression");
 if(version>=11 && r.flag())s.target_chooser=MatchTargetChooser{static_cast<std::uint8_t>(r.get(1))};
 if(version>=12){const auto count=r.get(1);if(count>5)throw std::invalid_argument("Invalid flight defender count");for(unsigned i=0;i<count;++i)s.flight_defenders.push_back(static_cast<std::uint8_t>(r.get(1)));}
 if(version>=13){const auto count=r.get(1);if(count>5)throw std::invalid_argument("Invalid processed flight defender count");for(unsigned i=0;i<count;++i)s.processed_flight_defenders.push_back(static_cast<std::uint8_t>(r.get(1)));}
 if(r.pos!=payload.size())throw std::invalid_argument("Unexpected match save data");
 validate_runtime(s);return s;
}
}

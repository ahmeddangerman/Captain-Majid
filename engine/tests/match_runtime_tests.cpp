#include "captain/match_runtime.hpp"
#include "captain/match_save.hpp"
#include "captain/phase_session_io.hpp"
#include "captain/field_state_io.hpp"
#include "captain/defender_selection.hpp"
#include <cassert>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>
static int comparisons(int argc,char** argv) {
 using namespace captain;
 if(argc==6 && std::string(argv[1])=="--compare-pass-cancel") {
  std::ifstream source_file(argv[2],std::ios::binary),before_file(argv[3],std::ios::binary),expected_file(argv[4],std::ios::binary);
  auto source=load_phase_session(source_file);auto expected=load_phase_session(expected_file);std::array<unsigned char,2048> before{};
  if(!before_file.read(reinterpret_cast<char*>(before.data()),before.size()))throw std::runtime_error("Missing original target RAM");
  auto target=begin_ordinary_human_pass_target(source);target.human_pass_cursor=before[0x624];
  MatchRuntimeState state{};state.session=target;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;
  MatchContent content{};content.data=ShotContent{};MatchInput input{MatchInputKind::tick,1};input.pressed=static_cast<std::uint8_t>(std::stoul(argv[5]));
  auto result=step_match(state,input,content);
  if(result.unsupported || result.state.session.phase!=FieldSessionPhase::human_action_menu)throw std::runtime_error("Runtime pass cancellation unsupported");
  result.state.session.current.field.random_first=expected.current.field.random_first;
  result.state.session.current.field.random_second=expected.current.field.random_second;
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);
  write_phase_session(actual_bytes,result.state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str())throw std::runtime_error("Runtime pass cancellation differs from original represented session");
  return 0;
 }
 if((argc==6 || argc==7 || argc==8 || argc==9 || argc==10 || argc==13) && (std::string(argv[1])=="--compare-field-ai-preparation" || std::string(argv[1])=="--compare-field-ai-clock" || std::string(argv[1])=="--compare-field-ai-handoff" || std::string(argv[1])=="--compare-field-ai-target" || std::string(argv[1])=="--compare-field-ai-flight" || std::string(argv[1])=="--compare-field-ai-crossing" || std::string(argv[1])=="--compare-field-ai-reception" || std::string(argv[1])=="--compare-field-ai-pass-control" || std::string(argv[1])=="--compare-field-ai-return" || std::string(argv[1])=="--compare-field-ai-control")) {
  const bool pass_control=std::string(argv[1])=="--compare-field-ai-pass-control";
  const bool through_crossing=std::string(argv[1])=="--compare-field-ai-crossing";
  const bool through_reception=pass_control || std::string(argv[1])=="--compare-field-ai-reception";
  const bool through_flight=through_crossing || through_reception || std::string(argv[1])=="--compare-field-ai-flight";
  const bool through_target=through_flight || std::string(argv[1])=="--compare-field-ai-target";
  if(through_target && argc!=((through_reception || through_crossing) ? 9 : through_flight ? 8 : 7) && !(pass_control && argc==13))throw std::invalid_argument("Expected pass distance, velocity and setup data");
  const bool through_handoff=through_target || std::string(argv[1])=="--compare-field-ai-handoff";
  const bool through_control=pass_control || std::string(argv[1])=="--compare-field-ai-control";
  const bool through_return=through_control || std::string(argv[1])=="--compare-field-ai-return";
  const bool through_clock=through_handoff || through_return || std::string(argv[1])=="--compare-field-ai-clock";
  std::ifstream source_file(argv[2],std::ios::binary),original_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto source=load_phase_session(source_file);std::array<unsigned char,2048> original{};
  if(!original_file.read(reinterpret_cast<char*>(original.data()),original.size()))throw std::runtime_error("Missing original field attack RAM");
  std::array<unsigned char,2048> setup=original;
  if(through_reception || through_crossing){std::ifstream setup_file(argv[8],std::ios::binary);if(!setup_file.read(reinterpret_cast<char*>(setup.data()),setup.size()))throw std::runtime_error("Missing original pass setup RAM");}
  MatchContent content{};content.data=load_shot_content(pack_file);
  if(through_target){std::ifstream table(argv[6],std::ios::binary);content.pass_distance=read_pass_distance_content(table);}
  if(through_flight){std::ifstream table(argv[7]);content.ball_velocity=read_ball_velocity_content(table);}
  if(!source.action || !source.area_defense || !source.scheduling || !source.recovery)throw std::runtime_error("Missing field attack context");
  if(argc==10 || argc==13){auto& player=source.current.field.roster[source.current.field.ball_owner];const auto offset=argc==13 ? 9 : 6;
   player.type=static_cast<std::uint8_t>(std::stoul(argv[offset]));player.stamina=static_cast<std::uint16_t>(std::stoul(argv[offset+1])|(std::stoul(argv[offset+2])<<8));player.level=static_cast<std::uint8_t>(std::stoul(argv[offset+3]));}
  MatchRuntimeState state{};state.session=source;state.session.phase=FieldSessionPhase::field_contact_pending;state.session.area_defense.reset();
  state.mode=MatchTimingMode::provisional_demo;state.demo_rng=static_cast<std::uint32_t>(std::stoul(argv[5]));state.defense_menu=MatchDefenseMenu{};
  state.defense_menu->stage=MatchDefenseStage::complete;state.defense_menu->poll={source.action->cursor,source.action->flash_counter,*source.area_defense};
  const auto chosen=step_match(state,{MatchInputKind::tick,1},content);
  if(chosen.unsupported)throw std::runtime_error("Runtime field AI choice unsupported");
  if(original[0x43b]==1 && (!chosen.state.defense_menu->ai_pass_target ||
     chosen.state.defense_menu->ai_pass_target->recipient!=original[0x5fc] || (!through_handoff && chosen.state.defense_menu->ai_pass_target->cell!=original[0x638])))
   throw std::runtime_error("Runtime field AI pass target differs from original");
  auto result=step_match(chosen.state,{MatchInputKind::tick,2},content);
  if(result.unsupported || result.state.defense_menu->stage!=MatchDefenseStage::attack_prepared)throw std::runtime_error("Runtime field attack preparation unsupported");
  const auto attack_cost=*result.state.defense_menu->attack_cost;
  if(through_clock){const auto presented=step_match(result.state,{MatchInputKind::presentation_complete,3},content);
   if(presented.unsupported)throw std::runtime_error("Runtime field attack presentation unsupported");
   result=step_match(presented.state,{MatchInputKind::tick,4},content);
   if(result.unsupported || result.state.defense_menu->stage!=MatchDefenseStage::dispatch_pending)throw std::runtime_error("Runtime field attack clock unsupported");}
  if(through_return && !through_reception){result=step_match(result.state,{MatchInputKind::presentation_complete,5},content);
   if(result.unsupported || result.state.session.phase!=FieldSessionPhase::player_control || result.state.defense_menu)throw std::runtime_error("Runtime field attack return unsupported");}
  if(through_handoff){result=step_match(result.state,{MatchInputKind::presentation_complete,5},content);
   if(result.unsupported || !result.state.defense_menu || result.state.defense_menu->stage!=MatchDefenseStage::ai_pass_handoff ||
      result.state.defense_menu->ai_pass_target->cell!=original[0x638])throw std::runtime_error("Runtime opponent pass handoff differs from original");}
  if(through_target){result=step_match(result.state,{MatchInputKind::tick,6},content);
   if(result.unsupported || !result.state.defense_menu->ai_pass_distance || *result.state.defense_menu->ai_pass_distance!=original[0x62b] || original[0x61a]!=255)
    throw std::runtime_error("Runtime opponent pass distance differs from original");}
  if(through_flight){result.state.ball_fraction=MatchBallFraction{setup[0x634],setup[0x636]};result=step_match(result.state,{MatchInputKind::tick,7},content);
   if(result.unsupported || result.state.defense_menu->stage!=MatchDefenseStage::ai_flight_ready)throw std::runtime_error("Runtime opponent pass setup unsupported");
   const auto& trajectory=*result.state.defense_menu->ai_trajectory;
   const auto word_at=[&](unsigned address){return unsigned(setup[address])|(unsigned(setup[address+1])<<8);};
   if(trajectory.x!=word_at(0x634) || trajectory.y!=word_at(0x636) || trajectory.vx!=word_at(0x639) || trajectory.vy!=word_at(0x63b) ||
      trajectory.direction!=setup[0x62c] || trajectory.flags!=setup[0x61b] || *result.state.defense_menu->ai_ball_cell!=setup[0x5fe])
    throw std::runtime_error("Runtime opponent pass trajectory differs from original");}
  if(through_crossing){result=step_match(result.state,{MatchInputKind::tick,8},content);
   if(result.unsupported || result.state.defense_menu->stage!=MatchDefenseStage::ai_flight_cell)throw std::runtime_error("Runtime opponent pass first crossing unsupported");
   result=step_match(result.state,{MatchInputKind::tick,9},content);
   if(result.unsupported || result.state.defense_menu->stage!=MatchDefenseStage::ai_flight_ready)throw std::runtime_error("Runtime opponent pass first callback unsupported");
   const auto& t=*result.state.defense_menu->ai_trajectory;
   const auto original_word=[&](unsigned address){return unsigned(original[address])|(unsigned(original[address+1])<<8);};
   if(t.x!=original_word(0x634) || t.y!=original_word(0x636) || *result.state.defense_menu->ai_ball_cell!=original[0x5fe])
    throw std::runtime_error("Runtime opponent pass first crossing differs from original");}
  std::uint64_t sequence=8;
  if(through_reception){
   while(result.state.defense_menu->stage!=MatchDefenseStage::ai_flight_arrived && sequence<512){result=step_match(result.state,{MatchInputKind::tick,sequence++},content);
    if(result.unsupported)throw std::runtime_error(std::string("Runtime opponent pass flight interrupted: ")+capability_name(*result.unsupported));}
   if(result.state.defense_menu->stage!=MatchDefenseStage::ai_flight_arrived)throw std::runtime_error("Runtime opponent pass did not arrive");
   result=step_match(result.state,{MatchInputKind::presentation_complete,sequence++},content);
   if(result.unsupported || result.state.session.phase!=FieldSessionPhase::player_control || result.state.defense_menu || !result.state.ball_fraction ||
      result.state.ball_fraction->x!=original[0x634] || result.state.ball_fraction->y!=original[0x636])throw std::runtime_error("Runtime opponent reception differs from original");
  }
  if(through_control){result=step_match(result.state,{MatchInputKind::tick,pass_control ? sequence : 6},content);
   if(result.unsupported || result.state.session.phase!=FieldSessionPhase::control_ready)throw std::runtime_error("Runtime opponent control setup unsupported");}
  const auto word=[&](unsigned address){return unsigned(original[address])|(unsigned(original[address+1])<<8);};
  const auto& s=result.state.session;const auto id=s.current.field.ball_owner;
  if(s.action->action!=original[0x43b] || s.recovery->attack_subtype!=original[0x43c] || s.recovery->ball_variant!=original[0x44e] || s.action->power!=word(0x61c) ||
     s.action->move_effect!=original[0x444] || s.action->move_flags!=original[0x443] || attack_cost!=word(0x43f) ||
     s.current.field.random_first!=original[0xe2] || s.current.field.random_second!=original[0xe3] ||
     s.current.field.roster[id].stamina!=word(0x301+12*id))
   throw std::runtime_error("Runtime opponent attack preparation differs from original");
  if(through_clock){const auto& current=s.current;const auto& f=current.field;
   if(current.clock.remaining!=word(0x5f7) || current.clock.extra_time!=original[0x5f9] || current.clock.flags!=original[0x63e] ||
      current.clock.script_flags!=original[0x615] || current.clock.ball_mode!=original[0x62d] || f.elapsed!=original[0x5ff] ||
      current.shared_recovery_flags!=original[0x449] || current.shared_recovery_timer!=original[0x44a])
    throw std::runtime_error("Runtime opponent attack clock differs from original");
   RuntimeRandomMemory projected{};project_field_random_memory(f,projected);
   for(unsigned address=0x300;address<0x400;++address)
    if(projected.known[address] && projected.bytes[address]!=original[address])throw std::runtime_error("Runtime field roster differs after attack clock");
   if(through_handoff || through_return){
    const auto visible=match_ball_view(result.state);
    if(f.side!=original[0x5fb] || f.ball_owner!=original[0x441] || visible.x!=original[0x635] || visible.y!=original[0x637] ||
       visible.cell!=original[0x5fe] || f.selected_defenders!=original[0x600] || f.movement_region!=original[0x62a] || s.saved_ball_flight_flag!=original[0x61b])
     throw std::runtime_error("Runtime post-action field differs from original");
    for(unsigned player=1;player<=21;++player){if(player==11)continue;const auto base=0x300+12*player;
     const auto& motion=f.motion[field_motion_index(static_cast<std::uint8_t>(player))];
     if(motion.x!=word(base+5) || motion.y!=word(base+7) || motion.target_cell!=original[base+9] || motion.status!=original[base+10])
      throw std::runtime_error("Runtime post-action movement differs from original");
    }
    if(through_control && (!s.control || s.control->decision_timer!=original[0x614] || s.control->facing!=original[0x517] ||
       s.control->controlled_player!=original[0x5fd] || s.control->last_carrier_cell!=original[0x617] ||
       s.control->travel_accumulator!=word(0x642) || s.control->display_parameter!=original[0x8e] || s.control->motion_requested!=original[0x532] ||
       s.recovery->opposing_player!=original[0x442] || s.recovery->ball_variant!=original[0x44e]))
     throw std::runtime_error("Runtime opponent control setup differs from original");
   }
  }
  return 0;
 }
 if(argc==6 && std::string(argv[1])=="--compare-field-ai-runtime") {
  std::ifstream source_file(argv[2],std::ios::binary),original_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto source=load_phase_session(source_file);std::array<unsigned char,2048> original{};
  if(!original_file.read(reinterpret_cast<char*>(original.data()),original.size()))throw std::runtime_error("Missing original field AI RAM");
  MatchContent content{};content.data=load_shot_content(pack_file);
  if(!source.action || !source.area_defense || !source.scheduling || !source.recovery)throw std::runtime_error("Missing original field AI context");
  MatchRuntimeState state{};state.session=source;state.session.phase=FieldSessionPhase::field_contact_pending;state.session.area_defense.reset();
  state.mode=MatchTimingMode::provisional_demo;state.demo_rng=static_cast<std::uint32_t>(std::stoul(argv[5]));state.defense_menu=MatchDefenseMenu{};
  state.defense_menu->stage=MatchDefenseStage::complete;
  state.defense_menu->poll={source.action->cursor,source.action->flash_counter,*source.area_defense};
  const auto result=step_match(state,{MatchInputKind::tick,1},content);
  if(result.unsupported || result.state.defense_menu->stage!=MatchDefenseStage::ai_chosen ||
     result.state.session.action->action!=original[0x43b] || result.state.session.recovery->attack_subtype!=original[0x43c] ||
     result.state.session.current.field.random_first!=original[0xe2] || result.state.session.current.field.random_second!=original[0xe3])
   throw std::runtime_error("Runtime field AI fallback differs from original");
  auto expected_field=source.current.field;expected_field.random_first=original[0xe2];expected_field.random_second=original[0xe3];
  std::ostringstream actual(std::ios::binary),expected(std::ios::binary);
  write_field_state(actual,result.state.session.current.field);write_field_state(expected,expected_field);
  if(actual.str()!=expected.str())throw std::runtime_error("Runtime field AI changed unrelated field state");
  return 0;
 }
 if((argc==5 || argc==7) && std::string(argv[1])=="--compare-defense-register") {
  std::ifstream source_file(argv[2],std::ios::binary),original_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto source=load_phase_session(source_file);std::array<unsigned char,2048> original{};
  if(!original_file.read(reinterpret_cast<char*>(original.data()),original.size()))throw std::runtime_error("Missing original registration RAM");
  MatchContent content{};content.data=load_shot_content(pack_file);
  if(!source.action || !source.area_defense || !source.scheduling || !content.data->area_defense_menu)throw std::runtime_error("Missing defense registration context");
  if(argc==7){const auto count=std::stoul(argv[5]),slot=std::stoul(argv[6]);
   if((count!=1 && count!=4) || slot>=count || (slot!=0 && slot!=3))throw std::runtime_error("Unsupported constructed defense comparison");
   source.current.field.selected_defenders=static_cast<std::uint8_t>(count);
   if(count==1)for(unsigned i=1;i<5;++i)source.scheduling->selected_ids[i]=0;
   if(count==4)source.scheduling->selected_ids[3]=5;
   source.action->cursor=static_cast<std::uint8_t>(slot);source.area_defense->actions[slot]=6;
  }
  MatchRuntimeState state{};state.session=source;state.session.phase=FieldSessionPhase::field_contact_pending;state.session.area_defense.reset();
  state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;state.defense_menu=MatchDefenseMenu{};
  state.defense_menu->stage=MatchDefenseStage::choice_confirmed;
  state.defense_menu->poll={source.action->cursor,source.action->flash_counter,*source.area_defense};
  const auto result=step_match(state,{MatchInputKind::tick,1},content);
  if(result.unsupported || !result.state.defense_menu)throw std::runtime_error("Runtime defensive registration unsupported");
  const auto& m=result.state.defense_menu->poll;
  if(m.cursor!=original[0x61e] || m.flash_counter!=original[0x61f] || m.defense.blink!=original[0x620])throw std::runtime_error("Original registration menu differs");
  for(unsigned slot=0;slot<4;++slot)if(m.defense.actions[slot]!=original[0x60b+slot])throw std::runtime_error("Original registered defensive actions differ");
  return 0;
 }
 if(argc==6 && std::string(argv[1])=="--compare-defense-poll") {
  std::ifstream source_file(argv[2],std::ios::binary),expected_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  const auto source=load_phase_session(source_file),expected=load_phase_session(expected_file);const auto pack=load_shot_content(pack_file);
  if(!source.action || !source.area_defense || !source.scheduling || !expected.action || !expected.area_defense || !pack.area_defense_menu)
   throw std::runtime_error("Missing defensive comparison context");
  const auto pressed=static_cast<std::uint8_t>(std::stoul(argv[5]));
  const DefenseMenuPollState state{source.action->cursor,source.action->flash_counter,*source.area_defense};
  const auto actual=poll_defense_menu(state,source.current.field.selected_defenders,source.scheduling->selected_ids,true,pressed,*pack.area_defense_menu);
  const auto& a=actual.state.defense;const auto& e=*expected.area_defense;
  if(actual.state.cursor!=expected.action->cursor || actual.state.flash_counter!=expected.action->flash_counter ||
     a.actions!=e.actions || a.subtypes!=e.subtypes || a.blink!=e.blink || a.pressed!=e.pressed)
   throw std::runtime_error("Original field defense poll differs");
  const bool confirmed=expected.phase==FieldSessionPhase::sampled_field_defense_preparation;
  if((actual.boundary==DefenseMenuBoundary::choice_confirmed)!=confirmed || (confirmed && actual.opposing_player!=expected.recovery->opposing_player))
   throw std::runtime_error("Original field defense boundary differs");
  return 0;
 }
 if(argc==6 && std::string(argv[1])=="--compare-callback") {
  std::ifstream session_file(argv[2],std::ios::binary),before_file(argv[3],std::ios::binary),original_file(argv[4],std::ios::binary),pack_file(argv[5],std::ios::binary);
  auto source=load_phase_session(session_file);std::array<unsigned char,2048> before{},original{};
  if(!before_file.read(reinterpret_cast<char*>(before.data()),before.size()) || !original_file.read(reinterpret_cast<char*>(original.data()),original.size()))throw std::runtime_error("Missing original RAM");
  MatchContent content{};content.data=load_shot_content(pack_file);const auto recipient=before[0x5fc];
  source.phase=FieldSessionPhase::ordinary_pass_prepared;source.human_pass_steps=0;source.ordinary_pass_candidates=OrdinaryPassCandidateContext{};
  source.ordinary_pass_candidates->scanned=true;source.ordinary_pass_candidates->candidates.count=1;source.ordinary_pass_candidates->candidates.allied_count=1;
  source.ordinary_pass_candidates->candidates.ids[0]=recipient;source.ordinary_pass_recipient=recipient;source.ordinary_pass_defenses=OrdinaryPassDefenseContext{};
  source.recovery->x_jitter_control=before[0x62c];source.saved_ball_flight_flag=before[0x61b];
  source.current.field.random_first=before[0xe2];source.current.field.random_second=before[0xe3];
  const auto word=[&](const auto& b,unsigned at){return static_cast<std::uint16_t>(unsigned(b[at])|(unsigned(b[at+1])<<8));};
  MatchRuntimeState state{};state.session=source;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;state.pass_stage=MatchPassStage::flight_cell;
  state.pass_target=MatchPassTarget{recipient,before[0x638],before[0x61a],before[0x62b]};state.pass_cell=before[0x5fe];
  state.pass_trajectory=BallTrajectoryContext{word(before,0x634),word(before,0x636),word(before,0x639),word(before,0x63b),before[0x62c],before[0x61b]};
  FlightDefenderSelectionInput selection{};const auto& field=state.session.current.field;
  selection.possession_side=field.side;selection.ball_cell=*state.pass_cell;selection.random_first=field.random_first;selection.random_second=field.random_second;selection.threshold=state.pass_target->counter;
  for(unsigned i=0;i<field.motion.size();++i){const auto& m=field.motion[i];selection.players[i]={static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8),m.status!=0};}
  const auto selected=select_flight_defenders(selection);if(selected.size()!=original[0x600])throw std::runtime_error("Original defender count differs");
  for(unsigned i=0;i<selected.size();++i)if(selected[i]!=original[0x601+i])throw std::runtime_error("Original defender IDs differ");
  const auto result=step_match(state,{MatchInputKind::tick,1},content);
  if(selected.empty() ? (result.unsupported || result.state.pass_stage!=MatchPassStage::flight_ready) : (result.unsupported!=UnsupportedCapability::defender_contact))
   throw std::runtime_error("Flight callback handling differs");
  return 0;
 }
 if(argc==6 && (std::string(argv[1])=="--compare-cross" || std::string(argv[1])=="--compare-arrival" || std::string(argv[1])=="--compare-reception")) {
  std::ifstream session_file(argv[2],std::ios::binary),before_file(argv[3],std::ios::binary),original_file(argv[4],std::ios::binary),pack_file(argv[5],std::ios::binary);
  auto source=load_phase_session(session_file);std::array<unsigned char,2048> before{},original{};
  if(!before_file.read(reinterpret_cast<char*>(before.data()),before.size()) || !original_file.read(reinterpret_cast<char*>(original.data()),original.size()))throw std::runtime_error("Missing original RAM");
  MatchContent content{};content.data=load_shot_content(pack_file);const auto recipient=before[0x5fc];
  source.phase=FieldSessionPhase::ordinary_pass_prepared;source.human_pass_steps=0;source.ordinary_pass_candidates=OrdinaryPassCandidateContext{};
  source.ordinary_pass_candidates->scanned=true;source.ordinary_pass_candidates->candidates.count=1;source.ordinary_pass_candidates->candidates.allied_count=1;
  source.ordinary_pass_candidates->candidates.ids[0]=recipient;source.ordinary_pass_recipient=recipient;source.ordinary_pass_defenses=OrdinaryPassDefenseContext{};
  source.recovery->x_jitter_control=before[0x62c];source.saved_ball_flight_flag=before[0x61b];
  const auto word=[&](const auto& b,unsigned at){return static_cast<std::uint16_t>(unsigned(b[at])|(unsigned(b[at+1])<<8));};
  MatchRuntimeState state{};state.session=source;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;state.pass_stage=MatchPassStage::flight_ready;
  state.pass_target=MatchPassTarget{recipient,before[0x638],before[0x61a],before[0x62b]};state.pass_cell=before[0x5fe];
  state.pass_trajectory=BallTrajectoryContext{word(before,0x634),word(before,0x636),word(before,0x639),word(before,0x63b),before[0x62c],before[0x61b]};
  auto result=step_match(state,{MatchInputKind::tick,1},content);
  if(std::string(argv[1])=="--compare-arrival" || std::string(argv[1])=="--compare-reception"){
   for(std::uint64_t sequence=2;!result.unsupported && result.state.pass_stage!=MatchPassStage::flight_arrived && sequence<200;++sequence)
    result=step_match(result.state,{MatchInputKind::tick,sequence},content);
   if(result.unsupported || result.state.pass_stage!=MatchPassStage::flight_arrived)throw std::runtime_error("Flight did not arrive unopposed");
  }
  if(std::string(argv[1])=="--compare-reception"){
   result=step_match(result.state,{MatchInputKind::presentation_complete,result.state.last_sequence+1},content);
   if(result.unsupported || result.state.session.phase!=FieldSessionPhase::player_control || !result.state.ball_fraction)throw std::runtime_error("Reception unsupported");
   const auto& f=result.state.session.current.field;
   if(f.side!=original[0x5fb] || f.ball_owner!=original[0x441] || f.ball_x!=original[0x635] || f.ball_y!=original[0x637] || f.ball_cell!=original[0x5fe] || f.selected_defenders!=original[0x600] || result.state.ball_fraction->x!=original[0x634] || result.state.ball_fraction->y!=original[0x636] || result.state.session.recovery->ball_variant!=original[0x44e] || result.state.session.saved_ball_flight_flag!=original[0x61b])throw std::runtime_error("Reception state differs from original");
   if(f.elapsed!=original[0x5ff] || f.random_first!=original[0xe2] || f.random_second!=original[0xe3] || f.movement_region!=original[0x62a])throw std::runtime_error("Reception field control differs");
   for(unsigned team=0;team<2;++team){if(f.teams[team].formation!=original[0x2c+2*team] || f.teams[team].defense_tactic!=original[0x2d+2*team])throw std::runtime_error("Reception team shape differs");}
   for(unsigned keeper=0;keeper<2;++keeper){const auto base=0x300u+12u*(keeper?11u:0u);const auto& k=f.keepers[keeper];if(k.timer!=original[base+5] || k.penalty_duration!=original[base+6] || k.penalty!=original[base+7] || k.stun!=original[base+10])throw std::runtime_error("Reception keeper differs");}
   for(unsigned id=0;id<22;++id){const auto base=0x300u+12u*id;const auto& p=f.roster[id];
    if(p.type!=original[base] || p.stamina!=word(original,base+1) || p.level!=original[base+3])throw std::runtime_error("Reception roster differs");
    if(id!=0 && id!=11){const auto& m=f.motion[field_motion_index(static_cast<std::uint8_t>(id))];if(m.x!=word(original,base+5) || m.y!=word(original,base+7) || m.target_cell!=original[base+9] || m.status!=original[base+10])throw std::runtime_error("Reception motion differs");}
   }
   return 0;
  }
  if(result.unsupported || !result.state.pass_trajectory || !result.state.pass_cell || *result.state.pass_cell!=original[0x5fe] ||
     result.state.pass_trajectory->x!=word(original,0x634) || result.state.pass_trajectory->y!=word(original,0x636))
   throw std::runtime_error("Flight crossing differs from original");
  return 0;
 }
 if(argc==7 && std::string(argv[1])=="--compare-flight") {
  std::ifstream session_file(argv[2],std::ios::binary),before_file(argv[3],std::ios::binary),original_file(argv[4],std::ios::binary),pack_file(argv[5],std::ios::binary),velocity_file(argv[6]);
  auto source=load_phase_session(session_file);std::array<unsigned char,2048> before{},original{};
  if(!before_file.read(reinterpret_cast<char*>(before.data()),before.size()) || !original_file.read(reinterpret_cast<char*>(original.data()),original.size()))throw std::runtime_error("Missing original RAM");
  auto pack=load_shot_content(pack_file);MatchContent content{};content.data=pack;content.ball_velocity=read_ball_velocity_content(velocity_file);
  const auto recipient=original[0x5fc];source.phase=FieldSessionPhase::ordinary_pass_prepared;source.human_pass_steps=0;
  source.ordinary_pass_candidates=OrdinaryPassCandidateContext{};source.ordinary_pass_candidates->scanned=true;
  source.ordinary_pass_candidates->candidates.count=1;source.ordinary_pass_candidates->candidates.allied_count=1;
  source.ordinary_pass_candidates->candidates.ids[0]=recipient;source.ordinary_pass_recipient=recipient;source.ordinary_pass_defenses=OrdinaryPassDefenseContext{};
  MatchRuntimeState state{};state.session=source;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;state.pass_stage=MatchPassStage::target_ready;
  state.pass_target=MatchPassTarget{recipient,original[0x638],original[0x61a],original[0x62b]};state.ball_fraction=MatchBallFraction{before[0x634],before[0x636]};
  const auto result=step_match(state,{MatchInputKind::tick,1},content);
  if(result.unsupported || !result.state.pass_trajectory)throw std::runtime_error("Flight setup unsupported");
  const auto& t=*result.state.pass_trajectory;
  const auto word=[&](unsigned at){return unsigned(original[at])|(unsigned(original[at+1])<<8);};
  if(t.x!=word(0x634) || t.y!=word(0x636) || t.vx!=word(0x639) || t.vy!=word(0x63b) || t.direction!=original[0x62c] || t.flags!=original[0x61b])throw std::runtime_error("Flight setup differs from original");
  return 0;
 }
 if(argc==5 && std::string(argv[1])=="--compare-target") {
  std::ifstream session_file(argv[2],std::ios::binary),original_file(argv[3],std::ios::binary),distance_file(argv[4],std::ios::binary);
  auto source=load_phase_session(session_file);std::array<unsigned char,2048> original{};
  if(!original_file.read(reinterpret_cast<char*>(original.data()),original.size()))throw std::runtime_error("Missing original RAM");
  MatchContent target_content{};target_content.data=ShotContent{};target_content.pass_distance=read_pass_distance_content(distance_file);
  const auto recipient=original[0x5fc];source.phase=FieldSessionPhase::ordinary_pass_prepared;source.human_pass_steps=0;
  source.ordinary_pass_candidates=OrdinaryPassCandidateContext{};source.ordinary_pass_candidates->scanned=true;
  source.ordinary_pass_candidates->candidates.count=1;source.ordinary_pass_candidates->candidates.allied_count=1;
  source.ordinary_pass_candidates->candidates.ids[0]=recipient;source.ordinary_pass_recipient=recipient;
  source.ordinary_pass_defenses=OrdinaryPassDefenseContext{};
  MatchRuntimeState state{};state.session=source;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;state.pass_stage=MatchPassStage::handoff;
  const auto result=step_match(state,{MatchInputKind::tick,1},target_content);
  if(result.unsupported || !result.state.pass_target || result.state.pass_target->recipient!=recipient || result.state.pass_target->destination!=original[0x638] ||
     result.state.pass_target->distance!=original[0x62b] || result.state.pass_target->counter!=original[0x61a])
    throw std::runtime_error("Unopposed target differs from original");
  return 0;
 }
 if(argc==5 && (std::string(argv[1])=="--demo-clock" || std::string(argv[1])=="--demo-round")) {
  std::ifstream source_file(argv[2],std::ios::binary),pack_file(argv[3],std::ios::binary);
  MatchRuntimeState state{};state.session=load_phase_session(source_file);state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;
  MatchContent content{};content.data=load_shot_content(pack_file);
  auto result=step_match(state,{MatchInputKind::tick,1},content);
  if(result.unsupported)throw std::runtime_error("Demo clock unsupported");
  if(std::string(argv[1])=="--demo-round"){
   result=step_match(result.state,{MatchInputKind::tick,2},content);
   if(result.unsupported)throw std::runtime_error("Demo round unsupported");
  }
  std::ofstream output(argv[4],std::ios::binary);if(!output)throw std::runtime_error("Cannot open clock output");write_phase_session(output,result.state.session);return 0;
 }
 if(argc==4 && std::string(argv[1])=="--compare-unopposed") {
  std::ifstream source_file(argv[2],std::ios::binary),pack_file(argv[3],std::ios::binary);
  const auto legacy=load_phase_session(source_file);const auto pack=load_shot_content(pack_file);
  const auto reference=prepare_sampled_human_empty_pass(legacy,pack.field_players,pack.attack);
  auto input=legacy;input.phase=FieldSessionPhase::ordinary_human_pass_confirmed;input.human_pass_steps=0;
  input.ordinary_pass_candidates=OrdinaryPassCandidateContext{};input.ordinary_pass_recipient=255;
  const auto actual=prepare_ordinary_unopposed_pass(input,pack.field_players,pack.attack);
  auto teammate=input;teammate.ordinary_pass_candidates->scanned=true;
  teammate.ordinary_pass_candidates->candidates.count=1;teammate.ordinary_pass_candidates->candidates.allied_count=1;
  teammate.ordinary_pass_candidates->candidates.ids[0]=6;teammate.ordinary_pass_recipient=6;
  const auto teammate_prepared=prepare_ordinary_unopposed_pass(teammate,pack.field_players,pack.attack);
  if(teammate_prepared.ordinary_pass_recipient!=6 || teammate_prepared.action->power!=actual.action->power)
   throw std::runtime_error("Teammate target changed independent pass preparation");
  const auto id=input.current.field.ball_owner;
  if(actual.current.field.roster[id].stamina!=reference.current.field.roster[id].stamina || actual.action->power!=reference.action->power ||
     actual.action->move_effect!=reference.action->move_effect || actual.action->move_flags!=reference.action->move_flags ||
     actual.recovery->attack_subtype!=reference.recovery->attack_subtype || actual.current.clock.ball_mode!=reference.current.clock.ball_mode)
   throw std::runtime_error("Unopposed pass differs from original-compared preparation");
  const auto expected_handoff=acknowledge_sampled_human_empty_pass_handoff(reference);
  MatchContent handoff_content{};handoff_content.data=pack;
  MatchRuntimeState handoff_input{};handoff_input.session=actual;handoff_input.mode=MatchTimingMode::provisional_demo;
  handoff_input.demo_rng=1;handoff_input.pass_stage=MatchPassStage::awaiting_presentation;
  const auto handoff_result=step_match(handoff_input,{MatchInputKind::presentation_complete,1},handoff_content);
  if(handoff_result.unsupported || handoff_result.state.session.recovery->ball_variant!=expected_handoff.recovery->ball_variant)
   throw std::runtime_error("Unopposed handoff differs from original-compared case");
  std::ostringstream actual_field,expected_field;write_field_state(actual_field,handoff_result.state.session.current.field);
  write_field_state(expected_field,expected_handoff.current.field);
  if(actual_field.str()!=expected_field.str())throw std::runtime_error("Unopposed handoff field differs");
  std::ostringstream saved;write_phase_session(saved,actual);std::istringstream read(saved.str());auto restored=load_phase_session(read);
  if(!restored.ordinary_pass_defenses || restored.ordinary_pass_defenses->count)throw std::runtime_error("Empty defense snapshot mismatch");
  return 0;
 }
 if(argc==4 && (std::string(argv[1])=="--compare-menu" || std::string(argv[1])=="--compare-confirm")) {
  std::ifstream before_file(argv[2],std::ios::binary), expected_file(argv[3],std::ios::binary);
  auto before=load_phase_session(before_file);auto expected=load_phase_session(expected_file);
  MatchContent captured{};captured.data=ShotContent{};captured.data->action_menu=ActionMenuContent{};
  MatchRuntimeState current{before,0};current.mode=MatchTimingMode::provisional_demo;current.demo_rng=1;
  MatchInput request{MatchInputKind::tick,1};if(std::string(argv[1])=="--compare-confirm")request.pressed=128;
  auto result=step_match(current,request,captured);
  assert(!result.unsupported);result.state.session.current.field.random_first=expected.current.field.random_first;
  result.state.session.current.field.random_second=expected.current.field.random_second;
  std::ostringstream actual_bytes,expected_bytes;write_phase_session(actual_bytes,result.state.session);write_phase_session(expected_bytes,expected);
  assert(actual_bytes.str()==expected_bytes.str());return 0;
 }
 return 2;
}

static int regular_tests() {
 using namespace captain;
 MatchConfig cfg{}; MatchContent content{};
 auto missing=create_match(cfg,content);
 assert(missing.unsupported==UnsupportedCapability::content);
 cfg.initial=PhaseSessionState{}; content.data=ShotContent{};
 cfg.seed=1; assert(create_match(cfg,content).unsupported==UnsupportedCapability::initialization); cfg.seed=0;
 auto start=create_match(cfg,content);
 assert(!start.unsupported && start.state.session.phase==FieldSessionPhase::field);
 assert(start.state.random_memory.known[0x300] && !start.state.random_memory.known[0x304] && !start.state.random_memory.known[0x700]);
 auto tick=step_match(start.state,MatchInput{MatchInputKind::tick,1},content);
 assert(tick.unsupported==UnsupportedCapability::initialization);
 assert(tick.state.last_sequence==start.state.last_sequence);
 auto pre=PhaseSessionState{}; pre.phase=FieldSessionPhase::player_control;
 pre.experience=ExperienceState{};pre.recovery=RecoveryContext{};pre.control=ControlContext{};pre.scheduling=SchedulingContext{};pre.action=ActionContext{};
 pre.current.field.roster[7].type=1;pre.current.field.roster[7].stamina=100;pre.current.field.side=0;pre.current.field.ball_owner=7;pre.current.field.ball_x=124;pre.current.field.ball_y=124;
 pre.current.field.motion[6].x=124u<<8;pre.current.field.motion[6].y=124u<<8;
 MatchRuntimeState prepared_input{pre,0};prepared_input.random_memory.known[0x304]=true;
 prepared_input.random_memory.known[0x700]=true;prepared_input.random_memory.bytes[0x700]=23;
 auto ready=step_match(prepared_input,MatchInput{MatchInputKind::tick,1},content);
 assert(!ready.unsupported && ready.state.session.phase==FieldSessionPhase::control_ready);
 assert(pre.phase==FieldSessionPhase::player_control && ready.state.last_sequence==1);
 assert(!ready.events.empty() && ready.events[0].presentation.kind==ControlPresentationKind::display);
 assert(!ready.state.random_memory.known[0x304] && ready.state.random_memory.known[0x700] && ready.state.random_memory.bytes[0x700]==23);
 assert(ready.state.random_memory.known[0x300+12*7] && ready.state.random_memory.bytes[0x300+12*7]==pre.current.field.roster[7].type);
 auto action=step_match(ready.state,MatchInput{MatchInputKind::action,2},content);
 assert(action.unsupported==UnsupportedCapability::action && action.state.last_sequence==1);
 auto wait=step_match(ready.state,MatchInput{MatchInputKind::tick,2},content);
 assert(wait.unsupported==UnsupportedCapability::random_state && wait.state.last_sequence==1);
 MatchConfig with_index{};with_index.initial=ready.state.session;with_index.random_index=0;
 auto resumed=create_match(with_index,content);
 assert(resumed.state.random_index && *resumed.state.random_index==0);
 auto scheduled=step_match(resumed.state,MatchInput{MatchInputKind::tick,1},content);
 assert(scheduled.unsupported==UnsupportedCapability::timing && *scheduled.state.random_index==0);
MatchConfig demo{};demo.initial=pre;demo.mode=MatchTimingMode::provisional_demo;demo.seed=1;
 content.data->movement=FieldPhaseContent{};content.data->control_movement=ControlMovementContent{};
 auto first=create_match(demo,content);auto second=create_match(demo,content);
 assert(!first.unsupported && first.state.mode==MatchTimingMode::provisional_demo);
 auto c1=step_match(first.state,{MatchInputKind::tick,1},content);auto c2=step_match(second.state,{MatchInputKind::tick,1},content);
 auto p1=step_match(c1.state,{MatchInputKind::tick,2},content);auto p2=step_match(c2.state,{MatchInputKind::tick,2},content);
 assert(!p1.unsupported && p1.state.last_sequence==2 && p1.state.demo_rng==p2.state.demo_rng);
 assert(p1.state.session.current.field.random_first==p2.state.session.current.field.random_first && p1.state.session.control->decision_timer==9);
 assert(c1.state.session.control->decision_timer==10);
 auto moving=c1.state;moving.session.control->decision_timer=0;moving.ball_fraction=MatchBallFraction{5,9};
 content.data->movement->profiles.speeds.resize(1);content.data->control_movement->carrier_speeds[0]=1;
 MatchInput move{MatchInputKind::tick,2};move.held=1;
 auto moved=step_match(moving,move,content);
 assert(!moved.unsupported && moved.state.session.current.field.ball_x==moving.session.current.field.ball_x && !moved.state.ball_fraction);
 MatchInput menu{MatchInputKind::tick,2};menu.held=64;
 auto menu_result=step_match(c1.state,menu,content);
 assert(!menu_result.unsupported && menu_result.state.session.phase==FieldSessionPhase::action_menu_pending);
 content.data->action_menu=ActionMenuContent{};content.data->action_menu->directions[0][0]=1;
 auto open=step_match(menu_result.state,{MatchInputKind::tick,3},content);
 assert(!open.unsupported && open.state.session.phase==FieldSessionPhase::human_action_menu && !open.events.empty());
 MatchInput select{MatchInputKind::tick,4};select.pressed=1;
 auto selected=step_match(open.state,select,content);
 assert(!selected.unsupported && selected.state.session.action->action==1);
 MatchInput confirm{MatchInputKind::tick,5};confirm.pressed=128;
 auto confirmed=step_match(selected.state,confirm,content);
 assert(!confirmed.unsupported && confirmed.state.session.phase==FieldSessionPhase::attack_choice_pending);
 MatchInput ordinary{MatchInputKind::action,6};ordinary.action=MatchAction::pass;
 auto target_mode=step_match(confirmed.state,ordinary,content);
 assert(!target_mode.unsupported && target_mode.state.session.phase==FieldSessionPhase::ordinary_human_pass_target);
 MatchInput aim{MatchInputKind::tick,7};aim.held=1;
 auto aimed=step_match(target_mode.state,aim,content);
 assert(!aimed.unsupported && aimed.state.session.human_pass_cursor!=target_mode.state.session.human_pass_cursor);
 MatchInput choose{MatchInputKind::tick,8};choose.pressed=128;
 auto chosen=step_match(aimed.state,choose,content);
 assert(!chosen.unsupported && chosen.state.session.phase==FieldSessionPhase::ordinary_human_pass_confirmed);
 content.data->field_players.skills.resize(1);content.data->field_players.skills[0].fill(20);
 content.data->attack.skill_selectors[1][0]=1;content.data->attack.moves[1].push_back({2,0,20,0});content.data->attack.strength_curve.fill(20);
 auto prepared=step_match(chosen.state,{MatchInputKind::tick,9},content);
 assert(!prepared.unsupported && prepared.state.session.phase==FieldSessionPhase::ordinary_pass_prepared);
 assert(prepared.state.session.current.field.roster[7].stamina==80 && prepared.state.session.action->power>0);
 assert(prepared.state.pass_stage==MatchPassStage::awaiting_presentation);
 auto early_flight=step_match(prepared.state,{MatchInputKind::tick,10},content);
 assert(early_flight.unsupported==UnsupportedCapability::presentation && early_flight.state.last_sequence==9);
 auto handoff=step_match(prepared.state,{MatchInputKind::presentation_complete,10},content);
 assert(!handoff.unsupported && handoff.state.pass_stage==MatchPassStage::handoff && handoff.state.session.recovery->ball_variant==1);
 auto recipient_state=handoff.state;recipient_state.session.ordinary_pass_candidates->scanned=true;
 recipient_state.session.ordinary_pass_candidates->candidates.count=1;recipient_state.session.ordinary_pass_candidates->candidates.allied_count=1;
 recipient_state.session.ordinary_pass_candidates->candidates.ids[0]=6;recipient_state.session.ordinary_pass_recipient=6;
 recipient_state.session.current.field.motion[field_motion_index(6)].x=140u<<8;
 recipient_state.session.current.field.motion[field_motion_index(6)].y=124u<<8;
 auto missing_distance=step_match(recipient_state,{MatchInputKind::tick,11},content);
 assert(missing_distance.unsupported==UnsupportedCapability::content && missing_distance.state.last_sequence==10);
 content.pass_distance=PassDistanceContent{};content.pass_distance->values.fill(4);
 auto target_result=step_match(recipient_state,{MatchInputKind::tick,11},content);
 assert(!target_result.unsupported && target_result.state.pass_stage==MatchPassStage::target_ready && target_result.state.pass_target);
 assert(target_result.state.pass_target->recipient==6 && target_result.state.pass_target->destination==field_cell_at(140,124) && target_result.state.pass_target->distance==4 && target_result.state.pass_target->counter==255);
 assert(target_result.state.session.current.field.ball_owner==7 && recipient_state.pass_stage==MatchPassStage::handoff);
 auto missing_flight=step_match(target_result.state,{MatchInputKind::tick,12},content);
 assert(missing_flight.unsupported==UnsupportedCapability::content && missing_flight.state.last_sequence==11);
 content.ball_velocity=BallVelocityContent{};content.ball_velocity->quarter_wave.fill(128);
 content.data->movement->geometry.direction_thresholds.fill(65535);
 auto missing_fraction=step_match(target_result.state,{MatchInputKind::tick,12},content);
 assert(missing_fraction.unsupported==UnsupportedCapability::ball_fraction && missing_fraction.state.last_sequence==11);
 auto with_fraction=target_result.state;with_fraction.ball_fraction=MatchBallFraction{5,9};
 auto invalid_target=with_fraction;invalid_target.pass_target->destination=255;
 auto invalid_flight=step_match(invalid_target,{MatchInputKind::tick,12},content);
 assert(invalid_flight.unsupported==UnsupportedCapability::phase && invalid_flight.state.last_sequence==11);
 auto flight=step_match(with_fraction,{MatchInputKind::tick,12},content);
 assert(!flight.unsupported && flight.state.pass_stage==MatchPassStage::flight_ready && flight.state.pass_trajectory);
 assert((flight.state.pass_trajectory->x&255)==5 && (flight.state.pass_trajectory->y&255)==9 && flight.state.pass_trajectory->flags==1);
 assert(with_fraction.pass_stage==MatchPassStage::target_ready);
 auto crossing=step_match(flight.state,{MatchInputKind::tick,13},content);
 assert(!crossing.unsupported && crossing.state.pass_cell && *crossing.state.pass_cell!=flight.state.session.current.field.ball_cell);
 assert(crossing.state.pass_stage==MatchPassStage::flight_cell || crossing.state.pass_stage==MatchPassStage::flight_arrived);
 assert(flight.state.pass_stage==MatchPassStage::flight_ready && flight.state.last_sequence==12);
 MatchContentIdentity identity{};identity[0]=42;
 const MatchContentIdentity known_identity{{0x68,0x76,0x27,0x7d,0xbe,0xf7,0x26,0xc2,0xcb,0xd8,0x41,0x99,0xda,0xb2,0x7b,0xf8,0x02,0xe6,0x03,0x55,0x6d,0xd6,0x26,0xee,0xd2,0x0c,0x04,0x54,0xeb,0xd5,0x00,0x14}};
 assert(match_content_identity("abc",{}, {})==known_identity);
 assert(match_content_identity("abc",std::string{}, {})!=known_identity);
 const MatchContentIdentity long_identity{{0xf2,0xb3,0x8a,0x7e,0x55,0x5b,0x29,0xea,0x1a,0xc6,0xcc,0x08,0x30,0x9e,0xb6,0x72,0xf0,0x3d,0xa0,0x2c,0x7e,0x5f,0x54,0x71,0xad,0x88,0xb2,0xe2,0x06,0xed,0xfb,0x49}};
 assert(match_content_identity(std::string(1000,'a'),std::string("xy"),std::string("z"))==long_identity);
 auto verify_checkpoint=[&](const MatchRuntimeState& state) {
  std::ostringstream before(std::ios::binary);write_match_save(before,state,identity);
  std::istringstream in(before.str(),std::ios::binary);auto restored=load_match_save(in,identity);
  std::ostringstream after(std::ios::binary);write_match_save(after,restored,identity);
  assert(before.str()==after.str());return restored;
 };
 verify_checkpoint(c1.state);verify_checkpoint(prepared.state);verify_checkpoint(handoff.state);
 verify_checkpoint(target_result.state);verify_checkpoint(flight.state);
 crossing.state.random_memory.bytes[17]=77;crossing.state.random_memory.known[17]=true;
 std::ostringstream checkpoint(std::ios::binary);write_match_save(checkpoint,crossing.state,identity);
 std::istringstream resume_stream(checkpoint.str(),std::ios::binary);auto restored_match=load_match_save(resume_stream,identity);
 assert(restored_match.last_sequence==crossing.state.last_sequence && restored_match.demo_rng==crossing.state.demo_rng);
 assert(restored_match.pass_stage==crossing.state.pass_stage && restored_match.pass_target->recipient==6);
 assert(restored_match.pass_trajectory->x==crossing.state.pass_trajectory->x && restored_match.pass_cell==crossing.state.pass_cell);
 assert(restored_match.random_memory.known[17] && restored_match.random_memory.bytes[17]==77);
 auto uninterrupted=step_match(crossing.state,{MatchInputKind::tick,14},content);
 auto continued=step_match(restored_match,{MatchInputKind::tick,14},content);
 assert(uninterrupted.unsupported==continued.unsupported && uninterrupted.state.pass_stage==continued.state.pass_stage);
 assert(uninterrupted.state.pass_trajectory->x==continued.state.pass_trajectory->x);
 assert(step_match(restored_match,{MatchInputKind::tick,13},content).unsupported==UnsupportedCapability::input_sequence);
 auto damaged=checkpoint.str();damaged[damaged.size()/2]^=1;
 bool reject_corruption=false;try {std::istringstream bad(damaged,std::ios::binary);load_match_save(bad,identity);}catch(const std::invalid_argument&){reject_corruption=true;}assert(reject_corruption);
 bool reject_truncated=false;try {std::istringstream bad(checkpoint.str().substr(0,checkpoint.str().size()-1),std::ios::binary);load_match_save(bad,identity);}catch(const std::invalid_argument&){reject_truncated=true;}assert(reject_truncated);
 auto wrong_identity=identity;wrong_identity[0]++;
 bool reject_content=false;try {std::istringstream bad(checkpoint.str(),std::ios::binary);load_match_save(bad,wrong_identity);}catch(const std::invalid_argument&){reject_content=true;}assert(reject_content);
 bool reject_extra=false;try {std::istringstream bad(checkpoint.str()+"x",std::ios::binary);load_match_save(bad,identity);}catch(const std::invalid_argument&){reject_extra=true;}assert(reject_extra);
 auto invalid_saved=crossing.state;invalid_saved.pass_target.reset();
 bool reject_inconsistent=false;try {std::ostringstream bad(std::ios::binary);write_match_save(bad,invalid_saved,identity);}catch(const std::invalid_argument&){reject_inconsistent=true;}assert(reject_inconsistent);
 auto visible=match_ball_view(crossing.state);
 assert(visible.in_flight && visible.x==(crossing.state.pass_trajectory->x>>8) && visible.y==(crossing.state.pass_trajectory->y>>8) && visible.cell==*crossing.state.pass_cell);
 auto stationary=match_ball_view(c1.state);assert(!stationary.in_flight && stationary.x==c1.state.session.current.field.ball_x);
 if(crossing.state.pass_stage==MatchPassStage::flight_cell){
  auto callback=step_match(crossing.state,{MatchInputKind::tick,14},content);
  assert(!callback.unsupported && callback.state.pass_stage==MatchPassStage::flight_ready);
  auto contested=crossing.state;auto& defender=contested.session.current.field.motion[field_motion_index(12)];
  defender.x=contested.pass_trajectory->x;defender.y=contested.pass_trajectory->y;defender.status=0;
  contested.session.current.field.random_first=1;contested.session.current.field.random_second=0;
  auto blocked=step_match(contested,{MatchInputKind::tick,14},content);
  assert(blocked.unsupported==UnsupportedCapability::defender_contact && blocked.state.last_sequence==13);
 }
 auto flight_state=crossing.state;std::uint64_t seq=14;
 auto resumed_flight=restored_match;
 for(;flight_state.pass_stage!=MatchPassStage::flight_arrived && seq<150;++seq){
  auto advance=step_match(flight_state,{MatchInputKind::tick,seq},content);
  auto resume_advance=step_match(resumed_flight,{MatchInputKind::tick,seq},content);
  assert(!advance.unsupported && !resume_advance.unsupported);flight_state=advance.state;resumed_flight=resume_advance.state;
  std::ostringstream left(std::ios::binary),right(std::ios::binary);
  write_match_save(left,flight_state,identity);write_match_save(right,resumed_flight,identity);assert(left.str()==right.str());
 }
 assert(flight_state.pass_stage==MatchPassStage::flight_arrived);
 verify_checkpoint(flight_state);
 auto before_reception=step_match(flight_state,{MatchInputKind::tick,seq},content);
 assert(before_reception.unsupported==UnsupportedCapability::presentation && before_reception.state.last_sequence==flight_state.last_sequence);
 auto wrong_target=flight_state;wrong_target.pass_target->recipient=5;
 auto rejected_target=step_match(wrong_target,{MatchInputKind::presentation_complete,seq},content);
 assert(rejected_target.unsupported==UnsupportedCapability::phase && rejected_target.state.last_sequence==flight_state.last_sequence);
 auto moved_recipient=flight_state;
 moved_recipient.session.current.field.motion[field_motion_index(6)].x+=256;
 auto unsupported_reception=step_match(moved_recipient,{MatchInputKind::presentation_complete,seq},content);
 assert(unsupported_reception.unsupported==UnsupportedCapability::phase && unsupported_reception.state.last_sequence==flight_state.last_sequence);
 auto received=step_match(flight_state,{MatchInputKind::presentation_complete,seq},content);
 auto resumed_received=step_match(resumed_flight,{MatchInputKind::presentation_complete,seq},content);
 assert(!received.unsupported && received.state.session.phase==FieldSessionPhase::player_control && received.state.session.current.field.ball_owner==6);
 assert(!resumed_received.unsupported);verify_checkpoint(received.state);
 std::ostringstream final_left(std::ios::binary),final_right(std::ios::binary);
 write_match_save(final_left,received.state,identity);write_match_save(final_right,resumed_received.state,identity);assert(final_left.str()==final_right.str());
 assert(!received.state.session.ordinary_pass_recipient && !received.state.pass_trajectory && received.state.ball_fraction);
 auto stale_input=pre;stale_input.current.field.ball_x=116;
 MatchRuntimeState stale{stale_input,0};stale.ball_fraction=MatchBallFraction{5,9};
 auto refreshed=step_match(stale,{MatchInputKind::tick,1},content);
 assert(!refreshed.unsupported && refreshed.state.session.current.field.ball_x==124 && !refreshed.state.ball_fraction);
 auto repeated_handoff=step_match(handoff.state,{MatchInputKind::presentation_complete,11},content);
 assert(repeated_handoff.unsupported==UnsupportedCapability::presentation && repeated_handoff.state.last_sequence==10);
 std::ostringstream pass_bytes;write_phase_session(pass_bytes,prepared.state.session);
 std::istringstream pass_stream(pass_bytes.str());auto pass_loaded=load_phase_session(pass_stream);
 assert(pass_loaded.ordinary_pass_defenses && pass_loaded.ordinary_pass_defenses->count==0);
 auto guarded=chosen.state.session;guarded.current.field.selected_defenders=1;guarded.scheduling->selected_ids[0]=12;
 bool reject_defender=false;try{prepare_ordinary_unopposed_pass(guarded,content.data->field_players,content.data->attack);}catch(const std::invalid_argument&){reject_defender=true;}
 assert(reject_defender && guarded.current.field.roster[7].stamina==100);
 assert(chosen.state.session.current.field.roster[7].stamina==100);
 auto repeated=step_match(prepared.state,{MatchInputKind::tick,9},content);
 assert(repeated.unsupported==UnsupportedCapability::input_sequence);
 MatchInput cancel{MatchInputKind::tick,8};cancel.pressed=64;
 auto cancelled=step_match(aimed.state,cancel,content);
 assert(!cancelled.unsupported && cancelled.state.last_sequence==8 && cancelled.state.session.phase==FieldSessionPhase::human_action_menu);
 assert(!cancelled.state.session.human_pass_cursor && !cancelled.state.session.ordinary_pass_candidates && cancelled.state.session.action->cursor==0);
 cancel.pressed=192;auto cancel_priority=step_match(aimed.state,cancel,content);
 assert(!cancel_priority.unsupported && cancel_priority.state.session.phase==FieldSessionPhase::human_action_menu);
 assert(aimed.state.session.human_pass_cursor && aimed.state.last_sequence==7);
 auto direct=ordinary;direct.target=7;
 auto direct_rejected=step_match(confirmed.state,direct,content);
 assert(direct_rejected.unsupported==UnsupportedCapability::action && direct_rejected.state.last_sequence==5);
 auto no_menu_content=content;no_menu_content.data->action_menu.reset();
 auto blocked_menu=step_match(menu_result.state,{MatchInputKind::tick,3},no_menu_content);
 assert(blocked_menu.unsupported==UnsupportedCapability::content && blocked_menu.state.last_sequence==2);
 auto clock_input=c1.state;clock_input.session.phase=FieldSessionPhase::control_clock_pending;clock_input.session.current.clock.remaining=200;
 for(auto& player:clock_input.session.current.field.roster){player.type=1;player.stamina=100;}
 content.data->world_step=WorldStepContent{};content.data->world_step->stamina_curve.fill(100);
 content.data->field_players.skills.resize(1);content.data->goalkeepers.skills.resize(1);
 auto clock_result=step_match(clock_input,{MatchInputKind::tick,2},content);
 assert(!clock_result.unsupported && clock_result.state.session.phase==FieldSessionPhase::field_round_pending);
 assert(clock_result.state.session.current.clock.remaining==199 && clock_input.session.current.clock.remaining==200);
 auto round_input=clock_result.state;
 for(unsigned id=1;id<=21;++id)if(id!=11 && id!=7){auto& m=round_input.session.current.field.motion[field_motion_index(static_cast<std::uint8_t>(id))];m.x=180u<<8;m.y=140u<<8;}
 content.data->movement->profiles.speeds.resize(1);content.data->movement->geometry.direction_thresholds.fill(65535);
 auto round_result=step_match(round_input,{MatchInputKind::tick,3},content);
 assert(!round_result.unsupported && (round_result.state.session.phase==FieldSessionPhase::control_ready || round_result.state.session.phase==FieldSessionPhase::field_contact_pending));
 assert(round_result.state.last_sequence==3 && !round_result.state.ball_fraction);
 auto no_clock_content=content;no_clock_content.data->world_step.reset();
 auto blocked_clock=step_match(clock_input,{MatchInputKind::tick,2},no_clock_content);
 assert(blocked_clock.unsupported==UnsupportedCapability::content && blocked_clock.state.last_sequence==1 && blocked_clock.state.demo_rng==clock_input.demo_rng);
 auto opponent_contact=c1.state;opponent_contact.session.phase=FieldSessionPhase::field_contact_pending;
 opponent_contact.session.current.field.side=11;opponent_contact.session.current.field.ball_owner=15;
 opponent_contact.session.current.field.selected_defenders=1;opponent_contact.session.scheduling->selected_ids[0]=1;
 opponent_contact.session.control->controlled_player=1;
 auto opponent_prompt=step_match(opponent_contact,{MatchInputKind::tick,2},content);
 assert(opponent_prompt.unsupported==UnsupportedCapability::defensive_prompt && opponent_prompt.state.last_sequence==1);
 AreaDefenseMenuContent defense_content{};defense_content.directions[0].fill(255);defense_content.directions[0][0]=6;defense_content.directions[0][1]=0;
 defense_content.field_icons=std::array<std::uint8_t,7>{10,11,12,13,14,15,16};
 DefenseMenuPollState defense_input{};const std::array<std::uint8_t,5> selected_defenders{{1,0,0,0,0}};
 auto defense_selection=poll_defense_menu(defense_input,1,selected_defenders,true,1,defense_content);
 assert(defense_selection.boundary==DefenseMenuBoundary::polling && defense_selection.state.defense.actions[0]==6);
 assert(defense_selection.state.flash_counter==192 && defense_selection.state.defense.blink==13);
 auto defense_confirmation=poll_defense_menu(defense_selection.state,1,selected_defenders,true,128,defense_content);
 assert(defense_confirmation.boundary==DefenseMenuBoundary::choice_confirmed && defense_confirmation.opposing_player==1);
 assert(defense_confirmation.state.defense.pressed==128 && defense_input.defense.actions[0]==255);
 bool reject_defense_count=false;try {poll_defense_menu(defense_input,5,selected_defenders,true,0,defense_content);}catch(const std::invalid_argument&){reject_defense_count=true;}assert(reject_defense_count);
 content.data->area_defense_menu=defense_content;
 auto defense_open=step_match(opponent_contact,{MatchInputKind::presentation_complete,2},content);
 assert(!defense_open.unsupported && defense_open.state.defense_menu && defense_open.state.defense_menu->stage==MatchDefenseStage::polling);
 assert(verify_checkpoint(defense_open.state).defense_menu->stage==MatchDefenseStage::polling);
 MatchInput defense_choose{MatchInputKind::tick,3};defense_choose.pressed=1;
 auto defense_choice=step_match(defense_open.state,defense_choose,content);
 assert(!defense_choice.unsupported && defense_choice.state.defense_menu->poll.defense.actions[0]==6);
 MatchInput defense_confirm{MatchInputKind::tick,4};defense_confirm.pressed=128;
 auto defense_pending=step_match(defense_choice.state,defense_confirm,content);
 assert(!defense_pending.unsupported && defense_pending.state.defense_menu->stage==MatchDefenseStage::choice_confirmed);
 assert(verify_checkpoint(defense_pending.state).defense_menu->stage==MatchDefenseStage::choice_confirmed);
 auto defense_registered=step_match(defense_pending.state,{MatchInputKind::tick,5},content);
 assert(!defense_registered.unsupported && defense_registered.state.defense_menu->poll.cursor==1);
 MatchInput full_direction{MatchInputKind::tick,6};full_direction.pressed=1;
 auto full_rejected=step_match(defense_registered.state,full_direction,content);
 assert(full_rejected.unsupported==UnsupportedCapability::action && full_rejected.state.last_sequence==5 && full_rejected.state.demo_rng==defense_registered.state.demo_rng);
 defense_confirm.sequence=6;auto defense_complete=step_match(defense_registered.state,defense_confirm,content);
 assert(!defense_complete.unsupported && defense_complete.state.defense_menu->stage==MatchDefenseStage::complete);
 auto defense_ai=step_match(defense_complete.state,{MatchInputKind::tick,7},content);
 assert(defense_ai.unsupported==UnsupportedCapability::content && defense_ai.state.last_sequence==6);
 assert(verify_checkpoint(defense_complete.state).defense_menu->stage==MatchDefenseStage::complete);
 auto legacy_menu_save=[](std::string bytes,std::uint8_t version) {
  assert(bytes.back()==0);bytes.pop_back();
  assert(bytes.back()==0);bytes.pop_back();
  bytes.pop_back(); // Version9 defender count; older saves derive it from the session.
  if(version<8){assert(bytes.back()==0);bytes.pop_back();}
  if(version<6){assert(bytes.back()==0);bytes.pop_back();}
  if(version<4){assert(bytes.back()==0);bytes.pop_back();}
  bytes[8]=static_cast<char>(version);
  const auto length=static_cast<std::uint32_t>(bytes.size()-17);std::uint32_t crc=0xffffffffu;
  for(std::size_t i=17;i<bytes.size();++i){crc^=static_cast<unsigned char>(bytes[i]);for(unsigned j=0;j<8;++j)crc=(crc>>1)^(0xedb88320u&static_cast<std::uint32_t>(-static_cast<std::int32_t>(crc&1)));}
  crc=~crc;for(unsigned i=0;i<4;++i){bytes[9+i]=static_cast<char>(length>>(8*i));bytes[13+i]=static_cast<char>(crc>>(8*i));}return bytes;
 };
 std::ostringstream saved_v2_source(std::ios::binary);write_match_save(saved_v2_source,defense_complete.state,identity);
 auto saved_v2=legacy_menu_save(saved_v2_source.str(),2);std::istringstream saved_v2_stream(saved_v2,std::ios::binary);
 assert(load_match_save(saved_v2_stream,identity).defense_menu->stage==MatchDefenseStage::complete);
 auto ai_input=defense_complete.state;for(unsigned id=1;id<=21;++id)if(id!=11)ai_input.session.current.field.roster[id].type=1;
 ai_input.session.current.field.roster[15].type=23;
 ai_input.session.current.clock.remaining=200;
 for(unsigned id=12;id<=21;++id)ai_input.session.current.field.motion[field_motion_index(static_cast<std::uint8_t>(id))].x=200u<<8;
 ai_input.session.current.field.motion[field_motion_index(15)].x=124u<<8;
 ai_input.session.current.field.motion[field_motion_index(15)].y=124u<<8;
 content.data->ai_field_choice=AiFieldChoiceContent{};auto& field_ai=*content.data->ai_field_choice;
 field_ai.profile_rows.fill(255);field_ai.profile_rows[23]=0;field_ai.recipient_profile_rows.fill(255);field_ai.recipient_profile_rows[23]=0;
 field_ai.rows.resize(1);for(auto& n:field_ai.rows[0])n=0x22;
 content.data->ai_area_choice=AiAreaChoiceContent{};content.data->ai_area_choice->recipient_rows.resize(1);
 content.data->ai_area_choice->recipient_rows[0].fill(41);
 auto ai_selected=step_match(ai_input,{MatchInputKind::tick,7},content);
 assert(!ai_selected.unsupported && ai_selected.state.session.action->action==2 && ai_selected.state.defense_menu->stage==MatchDefenseStage::ai_chosen);
 assert(verify_checkpoint(ai_selected.state).defense_menu->stage==MatchDefenseStage::ai_chosen);
 auto pass_ai_input=ai_input;pass_ai_input.session.current.field.motion[field_motion_index(20)].x=100u<<8;
 pass_ai_input.session.current.field.motion[field_motion_index(20)].y=124u<<8;
 for(unsigned id=1;id<=10;++id){auto& player=pass_ai_input.session.current.field.motion[field_motion_index(static_cast<std::uint8_t>(id))];player.x=52u<<8;player.y=84u<<8;player.status=0;}
 field_ai.empty_pass_options=std::array<std::uint8_t,118>{};(*field_ai.empty_pass_options)[23]=1;
 auto ai_pass=step_match(pass_ai_input,{MatchInputKind::tick,7},content);
 assert(!ai_pass.unsupported && ai_pass.state.session.action->action==1 && ai_pass.state.defense_menu->ai_pass_target->recipient==20);
 assert(verify_checkpoint(ai_pass.state).defense_menu->ai_pass_target->recipient==20);
 auto ai_pass_prepared=step_match(ai_pass.state,{MatchInputKind::tick,8},content);
 assert(!ai_pass_prepared.unsupported && ai_pass_prepared.state.defense_menu->stage==MatchDefenseStage::attack_prepared);
 assert(ai_pass_prepared.state.defense_menu->ai_pass_target->recipient==20);
 auto pass_presented=step_match(ai_pass_prepared.state,{MatchInputKind::presentation_complete,9},content);
 assert(!pass_presented.unsupported && pass_presented.state.session.recovery->ball_variant==1 && pass_presented.state.session.control->last_carrier_cell==128);
 content.data->area_prefix_elapsed=std::array<std::uint8_t,7>{};(*content.data->area_prefix_elapsed)[1]=1;
 auto pass_clock=step_match(pass_presented.state,{MatchInputKind::tick,10},content);assert(!pass_clock.unsupported);
 auto pass_handoff=step_match(pass_clock.state,{MatchInputKind::presentation_complete,11},content);
 assert(!pass_handoff.unsupported && pass_handoff.state.defense_menu->stage==MatchDefenseStage::ai_pass_handoff);
 const auto& moving_recipient=pass_handoff.state.session.current.field.motion[field_motion_index(20)];
 assert(pass_handoff.state.defense_menu->ai_pass_target->cell==field_cell_at(moving_recipient.x>>8,moving_recipient.y>>8));
 assert(verify_checkpoint(pass_handoff.state).defense_menu->stage==MatchDefenseStage::ai_pass_handoff);
 auto no_distance=content;no_distance.pass_distance.reset();
 auto pass_needs_distance=step_match(pass_handoff.state,{MatchInputKind::tick,12},no_distance);
 assert(pass_needs_distance.unsupported==UnsupportedCapability::content && pass_needs_distance.state.last_sequence==11);
 auto ai_target_ready=step_match(pass_handoff.state,{MatchInputKind::tick,12},content);
 assert(!ai_target_ready.unsupported && ai_target_ready.state.defense_menu->stage==MatchDefenseStage::ai_pass_target_ready && ai_target_ready.state.defense_menu->ai_pass_distance);
 assert(verify_checkpoint(ai_target_ready.state).defense_menu->ai_pass_distance==ai_target_ready.state.defense_menu->ai_pass_distance);
 std::ostringstream save_v8_source(std::ios::binary);write_match_save(save_v8_source,ai_target_ready.state,identity);
 auto save_v8=legacy_menu_save(save_v8_source.str(),8);std::istringstream save_v8_stream(save_v8,std::ios::binary);
 assert(load_match_save(save_v8_stream,identity).defense_menu->stage==MatchDefenseStage::ai_pass_target_ready);
 auto ai_flight_without_fraction=step_match(ai_target_ready.state,{MatchInputKind::tick,13},content);
 assert(ai_flight_without_fraction.unsupported==UnsupportedCapability::ball_fraction && ai_flight_without_fraction.state.last_sequence==12);
 auto ai_fraction=ai_target_ready.state;ai_fraction.ball_fraction=MatchBallFraction{5,9};
 auto ai_flight=step_match(ai_fraction,{MatchInputKind::tick,13},content);
 assert(!ai_flight.unsupported && ai_flight.state.defense_menu->stage==MatchDefenseStage::ai_flight_ready);
 assert(ai_flight.state.session.current.field.selected_defenders==0 && ai_flight.state.defense_menu->ai_trajectory);
 assert(match_ball_view(ai_flight.state).in_flight);
 auto ai_flight_restored=verify_checkpoint(ai_flight.state);
 assert(ai_flight_restored.defense_menu->ai_trajectory->x==ai_flight.state.defense_menu->ai_trajectory->x);
 std::ostringstream save_v9_source(std::ios::binary);write_match_save(save_v9_source,ai_flight.state,identity);
 auto save_v9=save_v9_source.str();save_v9[8]=9;std::istringstream save_v9_stream(save_v9,std::ios::binary);
 assert(load_match_save(save_v9_stream,identity).defense_menu->stage==MatchDefenseStage::ai_flight_ready);
 auto ai_first_crossing=step_match(ai_flight.state,{MatchInputKind::tick,14},content);
 if(!ai_first_crossing.unsupported && ai_first_crossing.state.defense_menu->stage==MatchDefenseStage::ai_flight_cell){
  auto contested=ai_first_crossing.state;auto& defender=contested.session.current.field.motion[field_motion_index(1)];
  defender.x=contested.defense_menu->ai_trajectory->x;defender.y=contested.defense_menu->ai_trajectory->y;defender.status=0;
  contested.session.current.field.random_first=1;contested.session.current.field.random_second=0;
  auto denied_flight=step_match(contested,{MatchInputKind::tick,15},content);
  assert(denied_flight.unsupported==UnsupportedCapability::defender_contact && denied_flight.state.last_sequence==14);
 }
 auto ai_flight_state=ai_flight.state;std::uint64_t ai_seq=14;
 for(;ai_flight_state.defense_menu->stage!=MatchDefenseStage::ai_flight_arrived && ai_seq<150;++ai_seq){
  auto next=step_match(ai_flight_state,{MatchInputKind::tick,ai_seq},content);
  auto resumed_next=step_match(ai_flight_restored,{MatchInputKind::tick,ai_seq},content);
  assert(!next.unsupported && !resumed_next.unsupported);ai_flight_state=next.state;ai_flight_restored=resumed_next.state;
  std::ostringstream live_save(std::ios::binary),resumed_save(std::ios::binary);
  write_match_save(live_save,ai_flight_state,identity);write_match_save(resumed_save,ai_flight_restored,identity);assert(live_save.str()==resumed_save.str());
 }
 assert(ai_flight_state.defense_menu->stage==MatchDefenseStage::ai_flight_arrived);
 auto ai_received=step_match(ai_flight_state,{MatchInputKind::presentation_complete,ai_seq},content);
 assert(!ai_received.unsupported && !ai_received.state.defense_menu && ai_received.state.session.current.field.ball_owner==20);
 assert(ai_received.state.session.phase==FieldSessionPhase::player_control && ai_received.state.ball_fraction);
 auto ai_receiver_control=step_match(ai_received.state,{MatchInputKind::tick,ai_seq+1},content);
 assert(!ai_receiver_control.unsupported && ai_receiver_control.state.session.phase==FieldSessionPhase::control_ready);
 content.data->area_prefix_elapsed.reset();
 std::ostringstream future_save(std::ios::binary);write_match_save(future_save,ai_selected.state,identity);
 auto saved_v3=legacy_menu_save(future_save.str(),3);std::istringstream saved_v3_stream(saved_v3,std::ios::binary);
 assert(load_match_save(saved_v3_stream,identity).defense_menu->stage==MatchDefenseStage::ai_chosen);
 auto invalid_v2=legacy_menu_save(future_save.str(),2);bool reject_v2_stage=false;
 try {std::istringstream old_stage(invalid_v2,std::ios::binary);load_match_save(old_stage,identity);}catch(const std::invalid_argument&){reject_v2_stage=true;}assert(reject_v2_stage);
 auto missing_attack=step_match(ai_selected.state,{MatchInputKind::tick,8},content);
 assert(missing_attack.unsupported==UnsupportedCapability::content && missing_attack.state.last_sequence==7);
 content.data->attack.skill_selectors[2][0]=1;content.data->attack.moves[2].push_back({2,0,20,0});
 auto ai_prepared=step_match(ai_selected.state,{MatchInputKind::tick,8},content);
 assert(!ai_prepared.unsupported && ai_prepared.state.defense_menu->stage==MatchDefenseStage::attack_prepared && ai_prepared.state.defense_menu->attack_cost);
 assert(ai_prepared.state.demo_rng==ai_selected.state.demo_rng && ai_prepared.state.session.current.field.roster[15].stamina==ai_selected.state.session.current.field.roster[15].stamina);
 assert(verify_checkpoint(ai_prepared.state).defense_menu->stage==MatchDefenseStage::attack_prepared);
 std::ostringstream save_v4_source(std::ios::binary);write_match_save(save_v4_source,ai_prepared.state,identity);
 auto save_v4=legacy_menu_save(save_v4_source.str(),4);std::istringstream save_v4_stream(save_v4,std::ios::binary);
 assert(load_match_save(save_v4_stream,identity).defense_menu->stage==MatchDefenseStage::attack_prepared);
 auto early_ai_clock=step_match(ai_prepared.state,{MatchInputKind::tick,9},content);
 assert(early_ai_clock.unsupported==UnsupportedCapability::presentation && early_ai_clock.state.last_sequence==8);
 auto ai_presented=step_match(ai_prepared.state,{MatchInputKind::presentation_complete,9},content);
 assert(!ai_presented.unsupported && ai_presented.state.defense_menu->stage==MatchDefenseStage::clock_pending);
 std::ostringstream save_future(std::ios::binary);write_match_save(save_future,ai_presented.state,identity);
 auto future_v4=legacy_menu_save(save_future.str(),4);bool reject_v4_stage=false;
 try {std::istringstream invalid(future_v4,std::ios::binary);load_match_save(invalid,identity);}catch(const std::invalid_argument&){reject_v4_stage=true;}assert(reject_v4_stage);
 auto no_elapsed=step_match(ai_presented.state,{MatchInputKind::tick,10},content);
 assert(no_elapsed.unsupported==UnsupportedCapability::content && no_elapsed.state.last_sequence==9);
 content.data->area_prefix_elapsed=std::array<std::uint8_t,7>{};(*content.data->area_prefix_elapsed)[2]=1;
 auto ai_clock=step_match(ai_presented.state,{MatchInputKind::tick,10},content);
 assert(!ai_clock.unsupported && ai_clock.state.defense_menu->stage==MatchDefenseStage::dispatch_pending);
 assert(ai_clock.state.session.current.clock.remaining==199 && ai_clock.state.session.current.field.elapsed==ai_presented.state.session.current.field.elapsed+1);
 assert(verify_checkpoint(ai_clock.state).defense_menu->stage==MatchDefenseStage::dispatch_pending);
 auto ai_execution=step_match(ai_clock.state,{MatchInputKind::tick,11},content);
 assert(ai_execution.unsupported==UnsupportedCapability::presentation && ai_execution.state.last_sequence==10);
 auto ai_resumed=step_match(ai_clock.state,{MatchInputKind::presentation_complete,11},content);
 assert(!ai_resumed.unsupported && !ai_resumed.state.defense_menu && ai_resumed.state.session.phase==FieldSessionPhase::player_control);
 assert(ai_resumed.state.session.current.field.side==11 && ai_resumed.state.session.current.field.ball_owner==15);
 assert(verify_checkpoint(ai_resumed.state).session.phase==FieldSessionPhase::player_control);
 auto ai_control=step_match(ai_resumed.state,{MatchInputKind::tick,12},content);
 assert(!ai_control.unsupported && ai_control.state.session.phase==FieldSessionPhase::control_ready);
 auto ended=ai_presented.state;ended.session.current.clock.remaining=0;ended.session.current.clock.extra_time=0;
 auto period_blocked=step_match(ended,{MatchInputKind::tick,10},content);
 assert(period_blocked.unsupported==UnsupportedCapability::period_transition && period_blocked.state.last_sequence==9);
 field_ai.rows[0].fill(0);auto unsupported_ai=step_match(ai_input,{MatchInputKind::tick,7},content);
 assert(unsupported_ai.unsupported==UnsupportedCapability::opponent_action && unsupported_ai.state.last_sequence==6 && unsupported_ai.state.demo_rng==ai_input.demo_rng);
 MatchInput unsupported_choice{MatchInputKind::tick,3};unsupported_choice.pressed=2;
 auto other_choice=step_match(defense_open.state,unsupported_choice,content);
 auto other_pending=step_match(other_choice.state,defense_confirm,content);
 auto unsupported_register=step_match(other_pending.state,{MatchInputKind::tick,7},content);
 assert(unsupported_register.unsupported==UnsupportedCapability::action && unsupported_register.state.last_sequence==6);
 auto three_defenders=opponent_contact;three_defenders.session.current.field.selected_defenders=3;
 three_defenders.session.scheduling->selected_ids={1,2,3,0,0};
 auto three_open=step_match(three_defenders,{MatchInputKind::presentation_complete,2},content);
 assert(!three_open.unsupported);auto defense_state=three_open.state;std::uint64_t defense_sequence=3;
 for(unsigned slot=0;slot<3;++slot) {
  MatchInput direction{MatchInputKind::tick,defense_sequence++};direction.pressed=1;
  auto pick=step_match(defense_state,direction,content);assert(!pick.unsupported);
  MatchInput confirm_skip{MatchInputKind::tick,defense_sequence++};confirm_skip.pressed=128;
  auto pending=step_match(pick.state,confirm_skip,content);assert(!pending.unsupported);
  auto register_skip=step_match(pending.state,{MatchInputKind::tick,defense_sequence++},content);
  assert(!register_skip.unsupported && register_skip.state.defense_menu->poll.cursor==slot+1);
  defense_state=verify_checkpoint(register_skip.state);
 }
 MatchInput final_confirm{MatchInputKind::tick,defense_sequence++};final_confirm.pressed=128;
 auto all_defended=step_match(defense_state,final_confirm,content);
 assert(!all_defended.unsupported && all_defended.state.defense_menu->stage==MatchDefenseStage::complete);
 auto incomplete=content;incomplete.data->movement.reset();
 auto denied=step_match(c1.state,{MatchInputKind::tick,2},incomplete);
 assert(denied.unsupported==UnsupportedCapability::content && denied.state.last_sequence==1 && denied.state.demo_rng==c1.state.demo_rng);
 demo.seed=0;assert(create_match(demo,content).unsupported==UnsupportedCapability::initialization);
 demo.mode=static_cast<MatchTimingMode>(99);demo.seed=1;assert(create_match(demo,content).unsupported==UnsupportedCapability::initialization);
 auto invalid_demo=c1.state;invalid_demo.demo_rng=0;
 auto invalid_step=step_match(invalid_demo,{MatchInputKind::tick,2},content);
 assert(invalid_step.unsupported==UnsupportedCapability::random_state && invalid_step.state.last_sequence==1);
 auto duplicate=step_match(start.state,MatchInput{MatchInputKind::tick,0},content);
 assert(duplicate.unsupported==UnsupportedCapability::input_sequence);
 assert(duplicate.state.last_sequence==start.state.last_sequence);
 return 0;
}

int main(int argc,char** argv) {
 return argc==1 ? regular_tests() : comparisons(argc,argv);
}

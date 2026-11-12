#include "captain/match_runtime.hpp"
#include "captain/match_client.hpp"
#include "captain/keeper_save_session.hpp"
#include "captain/match_save.hpp"
#include "captain/phase_session_io.hpp"
#include "captain/field_state_io.hpp"
#include "captain/defender_selection.hpp"
#include "captain/field_player_profile.hpp"
#include "captain/staged_shot.hpp"
#include <cassert>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
static int shot_comparisons(int argc,char** argv) {
 using namespace captain;
 if((argc==6 || argc==8) && std::string(argv[1])=="--compare-goal-kickoff") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  auto state=std::make_unique<MatchRuntimeState>();state->session=before;state->mode=MatchTimingMode::provisional_demo;state->demo_rng=1;
  std::array<unsigned char,2048> initial_raw{},expected_raw{};
  if(argc==8){std::ifstream initial_file(argv[6],std::ios::binary),expected_raw_file(argv[7],std::ios::binary);
   if(!initial_file.read(reinterpret_cast<char*>(initial_raw.data()),initial_raw.size()) || !expected_raw_file.read(reinterpret_cast<char*>(expected_raw.data()),expected_raw.size()))throw std::runtime_error("Missing original kickoff fractions");
   state->ball_fraction=MatchBallFraction{initial_raw[0x634],initial_raw[0x636]};
  }
  auto result=std::make_unique<MatchStepResult>();const auto steps=std::stoul(argv[5]);
  for(unsigned i=0;i<steps;++i){
   const auto kind=state->session.phase==FieldSessionPhase::kickoff_ball_placed ? MatchInputKind::presentation_complete : MatchInputKind::tick;
   *result=step_match(*state,{kind,state->last_sequence+1},content);
   if(result->unsupported)throw std::runtime_error(std::string("Goal kickoff unsupported: ")+capability_name(*result->unsupported)+" phase "+std::to_string(static_cast<unsigned>(state->session.phase)));
   *state=result->state;
   if(argc==8){MatchContentIdentity identity{};identity[0]=42;std::ostringstream out(std::ios::binary);write_match_save(out,*state,identity);std::istringstream in(out.str(),std::ios::binary);*state=load_match_save(in,identity);}
  }
  if(argc==8 && (!state->ball_fraction || state->ball_fraction->x!=expected_raw[0x634] || state->ball_fraction->y!=expected_raw[0x636]))throw std::runtime_error("Original kickoff fractions differ");
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);write_phase_session(actual_bytes,state->session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Goal kickoff differs at byte "+std::to_string(i)+": "+std::to_string(static_cast<unsigned char>(actual_bytes.str()[i]))+" vs "+std::to_string(static_cast<unsigned char>(expected_bytes.str()[i])));throw std::runtime_error("Goal kickoff session lengths differ");}
  return 0;
 }
 if(argc==6 && std::string(argv[1])=="--compare-human-shot-keeper-recovery") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  auto state=std::make_unique<MatchRuntimeState>();state->session=before;state->mode=MatchTimingMode::provisional_demo;state->demo_rng=1;
  auto result=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::presentation_complete,1},content));if(result->unsupported)throw std::runtime_error("Recovery fixture keeper AI unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::presentation_complete,2},content);if(result->unsupported)throw std::runtime_error("Recovery fixture save unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::presentation_complete,3},content);if(result->unsupported)throw std::runtime_error(std::string("Keeper recovery unsupported: ")+capability_name(*result->unsupported));
  if(std::string(argv[5])=="tail" || std::string(argv[5])=="control" || std::string(argv[5])=="area"){*state=result->state;*result=step_match(*state,{MatchInputKind::presentation_complete,4},content);if(result->unsupported)throw std::runtime_error("Keeper recovery tail unsupported");}
  if(std::string(argv[5])=="control"){*state=result->state;*result=step_match(*state,{MatchInputKind::tick,5},content);if(result->unsupported)throw std::runtime_error("Keeper recovery control setup unsupported");}
  if(std::string(argv[5])=="area"){*state=result->state;*result=step_match(*state,{MatchInputKind::tick,5},content);if(result->unsupported)throw std::runtime_error("Keeper area setup unsupported");}
  if(result->state.session.phase!=expected.phase)throw std::runtime_error("Keeper recovery phase differs");
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);write_phase_session(actual_bytes,result->state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Keeper recovery differs at byte "+std::to_string(i)+": "+std::to_string(static_cast<unsigned char>(actual_bytes.str()[i]))+" vs "+std::to_string(static_cast<unsigned char>(expected_bytes.str()[i])));throw std::runtime_error("Keeper recovery session lengths differ");}
  return 0;
 }
 if((argc==11 || argc==13 || argc==15 || argc==16 || argc==17 || argc==18) && std::string(argv[1])=="--compare-human-shot-end-to-end") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary),raw_file(argv[6],std::ios::binary),velocity_file(argv[7]);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);content.ball_velocity=read_ball_velocity_content(velocity_file);
  if(argc>=17){std::ifstream distance_file(argv[16],std::ios::binary);content.pass_distance=read_pass_distance_content(distance_file);}
  const bool kickoff=argc>=13 && std::string(argv[11])=="control";
  if(!kickoff)expected.kickoff_formation.reset(); // Before kickoff the starting session has no formation metadata.
  std::array<unsigned char,2048> raw{};if(!raw_file.read(reinterpret_cast<char*>(raw.data()),raw.size()))throw std::runtime_error("Missing original flight metadata");
  before.score=MatchScoreContext{static_cast<std::uint8_t>(std::stoul(argv[8])),static_cast<std::uint8_t>(std::stoul(argv[9])),static_cast<std::uint8_t>(std::stoul(argv[10]))};
  MatchConfig config{};config.initial=before;config.mode=MatchTimingMode::provisional_demo;config.seed=1;config.retained_defense_action=static_cast<std::uint8_t>(std::stoul(argv[5]));config.ball_fraction=MatchBallFraction{raw[0x634],raw[0x636]};
  auto state=std::make_unique<MatchRuntimeState>(create_match(config,content).state);auto result=std::make_unique<MatchStepResult>();
  const auto final_phase=kickoff ? FieldSessionPhase::control_ready : FieldSessionPhase::goal_kickoff_pending;
  for(unsigned turn=0;turn<200 && state->session.phase!=final_phase;++turn){
   const auto phase=state->session.phase;
   if(argc>=15 && (phase==FieldSessionPhase::keeper_decided || phase==FieldSessionPhase::goal_celebration_pending)){
    state->ball_fraction=MatchBallFraction{0,0};MatchContentIdentity identity{};identity[0]=42;std::ostringstream old(std::ios::binary);write_match_save(old,*state,identity);std::istringstream in(old.str(),std::ios::binary);*state=load_match_save(in,identity);
   }
   const auto input=client_input(*state,0,0);
   *result=step_match(*state,input,content);
   if(result->unsupported)throw std::runtime_error(std::string("End-to-end shot unsupported: ")+capability_name(*result->unsupported)+" phase "+std::to_string(static_cast<unsigned>(phase)));
   if(argc>=15 && result->state.session.ball_trajectory){const auto& trajectory=*result->state.session.ball_trajectory;
    if(!result->state.ball_fraction || result->state.ball_fraction->x!=static_cast<std::uint8_t>(trajectory.x) || result->state.ball_fraction->y!=static_cast<std::uint8_t>(trajectory.y))throw std::runtime_error("Shot trajectory fractions are not tracked");}
   if(result->state.retained_defense_action){MatchContentIdentity identity{};identity[0]=42;std::ostringstream saved(std::ios::binary);write_match_save(saved,result->state,identity);std::istringstream in(saved.str(),std::ios::binary);auto restored=std::make_unique<MatchRuntimeState>(load_match_save(in,identity));if(restored->retained_defense_action!=result->state.retained_defense_action)throw std::runtime_error("Retained action lost across runtime save");}
   *state=result->state;
  }
  if(state->session.phase!=final_phase)throw std::runtime_error("End-to-end shot did not reach requested continuation");
  if(kickoff && (!state->retained_defense_action || *state->retained_defense_action!=std::stoul(argv[12])))throw std::runtime_error("Goalkeeper action was not retained through kickoff");
  if(argc>=15 && (!state->ball_fraction || state->ball_fraction->x!=std::stoul(argv[13]) || state->ball_fraction->y!=std::stoul(argv[14])))throw std::runtime_error("Goal kickoff lost the original flight fractions");
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);write_phase_session(actual_bytes,state->session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("End-to-end shot differs at byte "+std::to_string(i));throw std::runtime_error("End-to-end shot session lengths differ");}
  if(argc>=16 && std::string(argv[15])=="post"){
   unsigned decisions=0,received=0,prompts=0;bool passed=false,home_restart=false;
   for(unsigned turn=0;turn<5000;++turn){
    const auto phase=state->session.phase;std::uint8_t pressed=0;
    if(argc>=18 && state->defense_menu && state->defense_menu->stage==MatchDefenseStage::polling){
     const auto& menu=*state->defense_menu;
     if(menu.poll.cursor==menu.defender_count || menu.poll.defense.actions[menu.poll.cursor]==6)pressed=128;
     else if(content.data->area_defense_menu){for(unsigned i=0;i<4;++i)if(content.data->area_defense_menu->directions[0][i]==6){pressed=static_cast<std::uint8_t>(1u<<i);break;}}
     if(!pressed)throw std::runtime_error("Cannot select skip in the defense prompt");
    }
    if(argc>=18 && phase==FieldSessionPhase::keeper_presentation_pending && state->session.current.field.side==11 && state->session.action && state->session.action->menu_route==4)pressed=(state->session.action->cursor&128u) ? 128 : 1;
    if(argc>=18 && phase==FieldSessionPhase::field_contact_pending && state->session.current.field.side==11 && !state->defense_menu)++prompts;
    *result=step_match(*state,client_input(*state,0,pressed),content);
    if(result->unsupported){
     std::cout<<"post: unsupported "<<capability_name(*result->unsupported)<<" phase "<<static_cast<unsigned>(phase)<<" after "<<decisions<<" fallbacks, "<<received<<" passes and "<<prompts<<" defense prompts";
     if(phase==FieldSessionPhase::ai_cell_decision && content.data->ai_field_choice){
      auto random=state->demo_rng;random^=random<<13;random^=random>>17;random^=random<<5;
      const auto& field=state->session.current.field;const auto& actor=field.roster[field.ball_owner];
      try{const auto choice=choose_ai_field_candidate(actor.type,static_cast<std::uint8_t>(actor.stamina),static_cast<std::uint8_t>(actor.stamina>>8),field.ball_x,field.ball_y,static_cast<std::uint8_t>(random),*content.data->ai_field_choice);std::cout<<" candidate "<<static_cast<unsigned>(choice.candidate)<<" actor "<<static_cast<unsigned>(actor.type)<<" variant "<<static_cast<unsigned>(state->session.recovery->ball_variant)<<" random "<<static_cast<unsigned>(static_cast<std::uint8_t>(random))<<" position "<<static_cast<unsigned>(field.ball_x)<<","<<static_cast<unsigned>(field.ball_y);}catch(const std::invalid_argument&){std::cout<<" candidate unavailable";}
     }
     if(phase==FieldSessionPhase::ball_arrival_callback && state->session.current.field.side==11){const auto& f=state->session.current.field;std::cout<<" defenders "<<static_cast<unsigned>(f.selected_defenders)<<" elapsed "<<static_cast<unsigned>(f.elapsed)<<" flag "<<static_cast<unsigned>(state->session.saved_ball_flight_flag);if(f.selected_defenders && state->session.scheduling)std::cout<<" first "<<static_cast<unsigned>(state->session.scheduling->selected_ids[0]);}
     std::cout<<'\n';return 0;
    }
    if(phase==FieldSessionPhase::ai_cell_decision && result->state.defense_menu && !result->state.defense_menu->defender_count)passed=true;
    *state=result->state;
    if(phase==FieldSessionPhase::goal_celebration_pending && state->session.phase==FieldSessionPhase::goal_kickoff_pending && state->session.current.field.side==0)home_restart=true;
    if(argc>=17){MatchContentIdentity identity{};identity[0]=42;std::ostringstream saved(std::ios::binary);write_match_save(saved,*state,identity);std::istringstream in(saved.str(),std::ios::binary);*state=load_match_save(in,identity);}
    if(passed && phase==FieldSessionPhase::player_control && state->session.phase==FieldSessionPhase::control_ready){++received;passed=false;if(argc<18){std::cout<<"post: pass received after "<<decisions<<" fallbacks\n";return 0;}}
    if(home_restart && state->session.phase==FieldSessionPhase::control_ready && state->session.current.field.side==0){std::cout<<"post: home kickoff control after "<<decisions<<" fallbacks, "<<received<<" passes and "<<prompts<<" defense prompts\n";return 0;}
    if(phase==FieldSessionPhase::ai_cell_decision && state->session.phase==FieldSessionPhase::control_clock_pending && ++decisions==3 && argc<18){std::cout<<"post: three fallbacks\n";return 0;}
   }
   const auto& field=state->session.current.field;
   std::cout<<"post: bound phase "<<static_cast<unsigned>(state->session.phase)<<" owner "<<static_cast<unsigned>(field.ball_owner)<<" position "<<static_cast<unsigned>(field.ball_x)<<","<<static_cast<unsigned>(field.ball_y)<<" cell "<<static_cast<unsigned>(field.ball_cell)<<" clock "<<state->session.current.clock.remaining<<" after "<<decisions<<" fallbacks, "<<received<<" passes and "<<prompts<<" defense prompts";
   if(state->defense_menu)std::cout<<" defense stage "<<static_cast<unsigned>(state->defense_menu->stage)<<" count "<<static_cast<unsigned>(state->defense_menu->defender_count);
   std::cout<<'\n';
  }
  return 0;
 }
 if(argc==8 && std::string(argv[1])=="--compare-human-shot-goal-restart") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  expected.kickoff_formation.reset(); // Development input did not include the separately captured formation context.
  before.score=MatchScoreContext{static_cast<std::uint8_t>(std::stoul(argv[5])),static_cast<std::uint8_t>(std::stoul(argv[6])),static_cast<std::uint8_t>(std::stoul(argv[7]))};
  auto state=std::make_unique<MatchRuntimeState>();state->session=before;state->mode=MatchTimingMode::provisional_demo;state->demo_rng=1;
  auto result=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::presentation_complete,1},content));if(result->unsupported)throw std::runtime_error("Restart fixture keeper AI unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::presentation_complete,2},content);if(result->unsupported)throw std::runtime_error("Restart fixture goal unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::presentation_complete,3},content);
  if(result->unsupported || result->state.session.phase!=FieldSessionPhase::goal_kickoff_pending)throw std::runtime_error("Human goal restart unsupported");
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);write_phase_session(actual_bytes,result->state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Human goal restart differs at byte "+std::to_string(i)+": "+std::to_string(static_cast<unsigned char>(actual_bytes.str()[i]))+" vs "+std::to_string(static_cast<unsigned char>(expected_bytes.str()[i])));throw std::runtime_error("Human goal restart session lengths differ");}
  return 0;
 }
 if(argc==5 && std::string(argv[1])=="--compare-human-shot-keeper-save") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  auto state=std::make_unique<MatchRuntimeState>();state->session=before;state->mode=MatchTimingMode::provisional_demo;state->demo_rng=1;
  auto result=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::presentation_complete,1},content));
  if(result->unsupported || result->state.session.phase!=FieldSessionPhase::keeper_decided)throw std::runtime_error("Keeper-save fixture AI unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::presentation_complete,2},content);
  if(result->unsupported || result->state.session.phase!=expected.phase)throw std::runtime_error(std::string("Human shot keeper save unsupported: ")+(result->unsupported ? capability_name(*result->unsupported) : "different phase")+" expected="+std::to_string(static_cast<unsigned>(expected.phase))+" actual="+std::to_string(static_cast<unsigned>(result->state.session.phase)));
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);write_phase_session(actual_bytes,result->state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Human shot keeper save differs at byte "+std::to_string(i));throw std::runtime_error("Human shot keeper save session lengths differ");}
  return 0;
 }
 if(argc==8 && std::string(argv[1])=="--compare-human-shot-goal") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  before.score=MatchScoreContext{static_cast<std::uint8_t>(std::stoul(argv[5])),static_cast<std::uint8_t>(std::stoul(argv[6])),static_cast<std::uint8_t>(std::stoul(argv[7]))};
  auto state=std::make_unique<MatchRuntimeState>();state->session=before;state->mode=MatchTimingMode::provisional_demo;state->demo_rng=1;
  auto result=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::presentation_complete,1},content));
  if(result->unsupported || result->state.session.phase!=FieldSessionPhase::keeper_decided)throw std::runtime_error("Goal fixture keeper AI unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::presentation_complete,2},content);
  if(result->unsupported || result->state.session.phase!=FieldSessionPhase::goal_celebration_pending)throw std::runtime_error("Human goal registration unsupported");
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);
  write_phase_session(actual_bytes,result->state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Human goal differs at byte "+std::to_string(i)+": "+std::to_string(static_cast<unsigned char>(actual_bytes.str()[i]))+" vs "+std::to_string(static_cast<unsigned char>(expected_bytes.str()[i])));throw std::runtime_error("Human goal session lengths differ");}
  return 0;
 }
 if(argc==5 && std::string(argv[1])=="--compare-human-shot-keeper") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  auto state=std::make_unique<MatchRuntimeState>();state->session=before;state->mode=MatchTimingMode::provisional_demo;state->demo_rng=1;
  auto result=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::presentation_complete,1},content));
  if(result->unsupported || result->state.session.phase!=FieldSessionPhase::keeper_decided)throw std::runtime_error("Human shot keeper AI resolution unsupported");
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);
  write_phase_session(actual_bytes,result->state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Keeper AI result differs at byte "+std::to_string(i)+": "+std::to_string(static_cast<unsigned char>(actual_bytes.str()[i]))+" vs "+std::to_string(static_cast<unsigned char>(expected_bytes.str()[i])));throw std::runtime_error("Keeper AI result session lengths differ");}
  return 0;
 }
 if(argc==6 && std::string(argv[1])=="--compare-human-shot-flight-chain") {
  std::ifstream before_file(argv[2],std::ios::binary),raw_file(argv[3],std::ios::binary),records(argv[4],std::ios::binary),pack_file(argv[5],std::ios::binary);
  auto source=load_phase_session(before_file);MatchContent content{};content.data=load_shot_content(pack_file);std::array<unsigned char,2048> raw{};
  if(!raw_file.read(reinterpret_cast<char*>(raw.data()),raw.size()))throw std::runtime_error("Missing original shot flight state");
  const auto word=[&](unsigned at){return static_cast<std::uint16_t>(raw[at]|(unsigned(raw[at+1])<<8));};
  source.phase=FieldSessionPhase::ball_advance_ready;source.shot_flight=ShotFlightContext{raw[0x62b],raw[0x638],raw[0x61a],false};
  source.ball_trajectory=BallTrajectoryContext{word(0x634),word(0x636),word(0x639),word(0x63b),raw[0x62c],raw[0x61b]};source.saved_ball_flight_flag=raw[0x61b];
  auto state=std::make_unique<MatchRuntimeState>();state->session=source;state->mode=MatchTimingMode::provisional_demo;state->demo_rng=1;
  auto result=std::make_unique<MatchStepResult>();unsigned count=0;
  while(records.peek()!=std::char_traits<char>::eof()){
   const auto event=records.get();if(event<0 || !records.read(reinterpret_cast<char*>(raw.data()),raw.size()))throw std::runtime_error("Truncated original flight record");
   *result=step_match(*state,{event==3 ? MatchInputKind::presentation_complete : MatchInputKind::tick,state->last_sequence+1},content);
   if(result->unsupported)throw std::runtime_error(std::string("Human shot flight unsupported: ")+capability_name(*result->unsupported));
   const auto expected_phase=event==0 ? FieldSessionPhase::ball_cell_callback : event==1 ? FieldSessionPhase::ball_arrival_callback : event==2 ? FieldSessionPhase::ball_advance_ready : event==3 ? FieldSessionPhase::shot_keeper_entry : FieldSessionPhase::keeper_presentation_pending;
   if(event>4 || result->state.session.phase!=expected_phase)throw std::runtime_error("Shot flight boundary differs");
   const auto& field=result->state.session.current.field;const auto& trajectory=*result->state.session.ball_trajectory;
   if(field.ball_x!=raw[0x635] || field.ball_y!=raw[0x637] || field.ball_cell!=raw[0x5fe] || field.selected_defenders!=raw[0x600] ||
      trajectory.x!=word(0x634) || trajectory.y!=word(0x636) || trajectory.vx!=word(0x639) || trajectory.vy!=word(0x63b))throw std::runtime_error("Shot flight position or defender count differs");
   for(unsigned i=0;i<field.selected_defenders;++i)if(result->state.session.scheduling->selected_ids[i]!=raw[0x601+i])throw std::runtime_error("Shot flight defender order differs");
   if(result->state.session.recovery->opposing_player!=raw[0x442])throw std::runtime_error("Shot keeper selection differs");
   if(event<=2){const auto view=match_ball_view(result->state);if(!view.in_flight || view.x!=raw[0x635] || view.y!=raw[0x637] || view.cell!=raw[0x5fe])throw std::runtime_error("Shot ball view differs");}
   *state=result->state;++count;
  }
  if(!count || (state->session.phase!=FieldSessionPhase::ball_arrival_callback && state->session.phase!=FieldSessionPhase::keeper_presentation_pending))throw std::runtime_error("Shot flight did not arrive");
  return 0;
 }
 if(argc==9 && std::string(argv[1])=="--compare-human-shot-flight-setup") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary),raw_file(argv[6],std::ios::binary),velocity_file(argv[8]);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);content.ball_velocity=read_ball_velocity_content(velocity_file);std::array<unsigned char,2048> raw{};
  if(!raw_file.read(reinterpret_cast<char*>(raw.data()),raw.size()))throw std::runtime_error("Missing original shot flight setup");
  MatchConfig config{};config.initial=before;config.mode=MatchTimingMode::provisional_demo;config.seed=1;config.retained_defense_action=static_cast<std::uint8_t>(std::stoul(argv[5]));config.ball_fraction=MatchBallFraction{raw[0x634],raw[0x636]};
  auto state=std::make_unique<MatchRuntimeState>(create_match(config,content).state);
  auto result=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::presentation_complete,1},content));if(result->unsupported)throw std::runtime_error("Shot dispatch unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::tick,2},content);if(result->unsupported)throw std::runtime_error("Shot movement unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::presentation_complete,3},content);if(result->unsupported)throw std::runtime_error("Shot tail unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::tick,4},content);if(result->unsupported)throw std::runtime_error("Shot approach unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::tick,5},content);
  if(result->unsupported || result->state.session.phase!=FieldSessionPhase::ball_advance_ready)throw std::runtime_error("Human shot flight setup unsupported");
  const auto word=[&](unsigned at){return static_cast<std::uint16_t>(raw[at]|(unsigned(raw[at+1])<<8));};
  expected.phase=FieldSessionPhase::ball_advance_ready;expected.shot_flight=ShotFlightContext{raw[0x62b],raw[0x638],raw[0x61a],std::stoul(argv[7])!=0};
  expected.ball_trajectory=BallTrajectoryContext{word(0x634),word(0x636),word(0x639),word(0x63b),raw[0x62c],raw[0x61b]};expected.saved_ball_flight_flag=raw[0x61b];
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);
  write_phase_session(actual_bytes,result->state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Human shot flight setup differs at byte "+std::to_string(i));throw std::runtime_error("Human shot flight setup session lengths differ");}
  return 0;
 }
 if(argc==8 && std::string(argv[1])=="--compare-human-shot-approach") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary),raw_file(argv[6],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);std::array<unsigned char,2048> raw{};
  if(!raw_file.read(reinterpret_cast<char*>(raw.data()),raw.size()))throw std::runtime_error("Missing original shot approach");
  MatchConfig config{};config.initial=before;config.mode=MatchTimingMode::provisional_demo;config.seed=1;config.retained_defense_action=static_cast<std::uint8_t>(std::stoul(argv[5]));
  auto state=std::make_unique<MatchRuntimeState>(create_match(config,content).state);
  auto result=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::presentation_complete,1},content));if(result->unsupported)throw std::runtime_error("Shot dispatch unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::tick,2},content);if(result->unsupported)throw std::runtime_error("Shot movement unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::presentation_complete,3},content);if(result->unsupported)throw std::runtime_error("Shot tail unsupported");
  *state=result->state;*result=step_match(*state,{MatchInputKind::tick,4},content);
  if(result->unsupported || result->state.session.phase!=FieldSessionPhase::shot_flight_pending)throw std::runtime_error("Human shot approach unsupported");
  expected.phase=FieldSessionPhase::shot_flight_pending;expected.shot_progress=ShotContactProgress{};
  expected.shot_flight=ShotFlightContext{raw[0x62b],raw[0x638],raw[0x61a],std::stoul(argv[7])!=0};
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);
  write_phase_session(actual_bytes,result->state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Human shot approach differs at byte "+std::to_string(i));throw std::runtime_error("Human shot approach session lengths differ");}
  return 0;
 }
 if(argc==6 && std::string(argv[1])=="--compare-human-shot-tail") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  MatchConfig config{};config.initial=before;config.mode=MatchTimingMode::provisional_demo;config.seed=1;config.retained_defense_action=static_cast<std::uint8_t>(std::stoul(argv[5]));
  auto created=create_match(config,content);if(created.unsupported)throw std::runtime_error("Human shot retained action unavailable");auto state=std::move(created.state);
  auto clock=step_match(state,{MatchInputKind::presentation_complete,1},content);if(clock.unsupported)throw std::runtime_error("Human shot dispatch unsupported");
  auto movement=step_match(clock.state,{MatchInputKind::tick,2},content);if(movement.unsupported)throw std::runtime_error("Human shot movement unsupported");
  auto result=step_match(movement.state,{MatchInputKind::presentation_complete,3},content);
  if(result.unsupported || result.state.session.phase!=FieldSessionPhase::shot_approach_ready)throw std::runtime_error("Human shot attack tail unsupported");
  expected.phase=FieldSessionPhase::shot_approach_ready;expected.shot_progress=ShotContactProgress{};
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);
  write_phase_session(actual_bytes,result.state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Human shot tail differs at byte "+std::to_string(i)+": "+std::to_string(static_cast<unsigned char>(actual_bytes.str()[i]))+" vs "+std::to_string(static_cast<unsigned char>(expected_bytes.str()[i])));throw std::runtime_error("Human shot tail session lengths differ");}
  return 0;
 }
 if(argc==5 && std::string(argv[1])=="--compare-human-shot-movement") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  MatchRuntimeState state{};state.session=before;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;
  auto clock=step_match(state,{MatchInputKind::presentation_complete,1},content);
  if(clock.unsupported)throw std::runtime_error("Human shot dispatch unsupported");
  auto result=step_match(clock.state,{MatchInputKind::tick,2},content);
  if(result.unsupported || result.state.session.phase!=FieldSessionPhase::selected_attack_presentation)throw std::runtime_error("Human shot movement unsupported");
  expected.phase=FieldSessionPhase::selected_attack_presentation;expected.shot_progress=ShotContactProgress{};
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);
  write_phase_session(actual_bytes,result.state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Human shot movement differs at byte "+std::to_string(i)+": "+std::to_string(static_cast<unsigned char>(actual_bytes.str()[i]))+" vs "+std::to_string(static_cast<unsigned char>(expected_bytes.str()[i])));throw std::runtime_error("Human shot movement session lengths differ");}
  return 0;
 }
 if(argc==5 && std::string(argv[1])=="--compare-human-shot-clock") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  MatchRuntimeState state{};state.session=before;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;
  auto result=step_match(state,{MatchInputKind::presentation_complete,1},content);
  if(result.unsupported || result.state.session.phase!=FieldSessionPhase::shot_dispatch_ready)throw std::runtime_error("Human shot dispatch unsupported");
  expected.phase=FieldSessionPhase::shot_dispatch_ready;expected.shot_progress=ShotContactProgress{};
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);
  write_phase_session(actual_bytes,result.state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Human shot dispatch differs at byte "+std::to_string(i)+": "+std::to_string(static_cast<unsigned char>(actual_bytes.str()[i]))+" vs "+std::to_string(static_cast<unsigned char>(expected_bytes.str()[i])));throw std::runtime_error("Human shot dispatch session lengths differ");}
  return 0;
 }
 if(argc==5 && std::string(argv[1])=="--compare-human-shot") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  MatchRuntimeState state{};state.session=before;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;
  MatchInput shot{MatchInputKind::action,1};shot.action=MatchAction::shot;
  auto result=step_match(state,shot,content);
  if(result.unsupported)throw std::runtime_error(std::string("Human shot preparation unsupported: ")+capability_name(*result.unsupported));
  std::ostringstream actual_bytes(std::ios::binary),expected_bytes(std::ios::binary);
  write_phase_session(actual_bytes,result.state.session);write_phase_session(expected_bytes,expected);
  if(actual_bytes.str()!=expected_bytes.str()){for(std::size_t i=0;i<actual_bytes.str().size() && i<expected_bytes.str().size();++i)if(actual_bytes.str()[i]!=expected_bytes.str()[i])throw std::runtime_error("Human shot preparation differs at byte "+std::to_string(i));throw std::runtime_error("Human shot session lengths differ");}
  return 0;
 }
 return 2;
}
static int human_keeper_menu_comparison(int argc,char** argv) {
 using namespace captain;if(argc!=8)return 2;
 std::ifstream source_file(argv[2],std::ios::binary),records_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
 auto state=std::make_unique<MatchRuntimeState>();state->session=load_phase_session(source_file);auto content=std::make_unique<MatchContent>();content->data=load_shot_content(pack_file);
 std::array<std::array<unsigned char,2048>,26> raw{};for(auto& item:raw)if(!records_file.read(reinterpret_cast<char*>(item.data()),item.size()))throw std::runtime_error("Missing home keeper menu checkpoint");
 const auto word=[&](unsigned at){return static_cast<std::uint16_t>(raw[0][at]|(unsigned(raw[0][at+1])<<8));};
 state->session.phase=FieldSessionPhase::shot_keeper_entry;state->session.saved_ball_flight_flag=raw[0][0x61b];
 state->session.shot_flight=ShotFlightContext{raw[0][0x62b],raw[0][0x638],raw[0][0x61a],false};
 state->session.ball_trajectory=BallTrajectoryContext{word(0x634),word(0x636),word(0x639),word(0x63b),raw[0][0x62c],raw[0][0x61b]};
 state->session.score=MatchScoreContext{raw[0][0x28],raw[0][0x29],raw[0][0x629]};
 state->mode=MatchTimingMode::provisional_demo;state->demo_rng=std::stoul(argv[6]);state->ball_fraction=MatchBallFraction{raw[0][0x634],raw[0][0x636]};state->home_keeper_y=raw[0][0x308];
 {auto config=std::make_unique<MatchConfig>();config->initial=state->session;config->mode=state->mode;config->seed=state->demo_rng;config->home_keeper_y=state->home_keeper_y;
  auto created=std::make_unique<MatchStepResult>(create_match(*config,*content));if(created->unsupported || created->state.home_keeper_y!=state->home_keeper_y)throw std::runtime_error("Explicit keeper position initialization differs");
  config->mode=MatchTimingMode::original_backed;config->seed=0;*created=create_match(*config,*content);if(!created->unsupported)throw std::runtime_error("Original mode accepted provisional keeper metadata");}
 MatchContentIdentity identity{};identity[0]=42;auto result=std::make_unique<MatchStepResult>();const std::string names[]={"entry","open","selected","confirmed","compared","goal","restart","formation","receiver","ball","handoff","control","recovered","tail","caught_ready","caught_poll","caught_menu","caught_open","caught_selected","caught_confirmed","caught_target","caught_moved1","caught_moved2","caught_moved4","caught_moved8","caught_moved0"};
 const auto advance=[&](std::uint8_t pressed,std::uint8_t held=0){*result=step_match(*state,client_input(*state,held,pressed),*content);if(result->unsupported){if(state->session.phase==FieldSessionPhase::keeper_decided && state->session.keeper_decision->result_code>=2)resolve_session_keeper_save(state->session,*content->data);throw std::runtime_error(std::string("Home goalkeeper stopped: ")+capability_name(*result->unsupported)+" phase "+std::to_string(static_cast<unsigned>(state->session.phase)));}*state=result->state;std::ostringstream out(std::ios::binary);write_match_save(out,*state,identity);std::istringstream in(out.str(),std::ios::binary);*state=load_match_save(in,identity);};
 const auto compare=[&](unsigned index){std::ifstream in(std::string(argv[7])+"/"+names[index]+".cmjsession",std::ios::binary);auto expected=load_phase_session(in),actual=state->session;expected.saved_ball_flight_flag=raw[index][0x61b];
  if(!actual.score || actual.score->left_goals!=raw[index][0x28] || actual.score->right_goals!=raw[index][0x29] || actual.score->kickoff!=raw[index][0x629])throw std::runtime_error("Home keeper score differs");
  actual.phase=FieldSessionPhase::field;actual.shot_flight.reset();actual.ball_trajectory.reset();actual.keeper_decision.reset();actual.score.reset();actual.kickoff_formation.reset();actual.kickoff_receiver.reset();actual.human_pass_cursor.reset();actual.human_pass_steps=0;actual.ordinary_pass_candidates.reset();actual.current.field.selected_defenders=0;actual.scheduling->selected_ids.fill(0);
  if(index>=1 && expected.control && actual.control)expected.control->facing=actual.control->facing;
  std::ostringstream a(std::ios::binary),b(std::ios::binary);write_phase_session(a,actual);write_phase_session(b,expected);
  if(a.str()!=b.str()){for(std::size_t i=0;i<a.str().size() && i<b.str().size();++i)if(a.str()[i]!=b.str()[i])throw std::runtime_error("Home keeper "+names[index]+" differs at byte "+std::to_string(i)+" actual "+std::to_string(static_cast<unsigned char>(a.str()[i]))+" expected "+std::to_string(static_cast<unsigned char>(b.str()[i])));throw std::runtime_error("Home keeper session lengths differ");}
 };
 advance(0);compare(0);advance(0);compare(1);advance(static_cast<std::uint8_t>(std::stoul(argv[5])));compare(2);
 if(!state->retained_defense_action || *state->retained_defense_action!=raw[2][0x43d])throw std::runtime_error("Home keeper selection differs: actual "+std::to_string(state->retained_defense_action.value_or(255))+" expected "+std::to_string(raw[2][0x43d]));
 advance(128);compare(4);
 if(!state->session.keeper_decision || state->session.keeper_decision->result_code!=raw[4][0x612] || state->session.keeper_decision->defense_action!=raw[4][0x43d] || state->session.keeper_decision->defense_subtype!=raw[4][0x43e] || state->session.keeper_decision->effect!=raw[4][0x445])throw std::runtime_error("Home keeper comparison differs");
 if(raw[4][0x612]==0){advance(0);if(state->session.phase!=FieldSessionPhase::goal_celebration_pending)throw std::runtime_error("Opponent goal phase differs");compare(5);advance(0);if(state->session.phase!=FieldSessionPhase::goal_kickoff_pending)throw std::runtime_error("Opponent goal restart phase differs");compare(6);
  const FieldSessionPhase phases[]={FieldSessionPhase::kickoff_formation_placed,FieldSessionPhase::kickoff_receiver_selected,FieldSessionPhase::kickoff_ball_placed,FieldSessionPhase::kickoff_control_pending,FieldSessionPhase::control_ready};
  for(unsigned i=0;i<5;++i){advance(0);if(state->session.phase!=phases[i])throw std::runtime_error("Opponent goal kickoff phase differs");compare(i+7);if(!state->ball_fraction || state->ball_fraction->x!=raw[i+7][0x634] || state->ball_fraction->y!=raw[i+7][0x636] || !state->retained_defense_action || *state->retained_defense_action!=raw[i+7][0x43d])throw std::runtime_error("Opponent goal kickoff lost original metadata");}
 }
 else if(raw[4][0x612]==2 || raw[4][0x612]==3){advance(0);const auto phase=raw[4][0x612]==2 ? FieldSessionPhase::ball_recovery : raw[4][0x43d]==1 ? FieldSessionPhase::keeper_punch_recovery : FieldSessionPhase::keeper_possession;if(state->session.phase!=phase)throw std::runtime_error("Home keeper save phase differs");compare(5);if(phase==FieldSessionPhase::keeper_punch_recovery){advance(0);if(state->session.phase!=FieldSessionPhase::recovered_owner)throw std::runtime_error("Home keeper punch recovery phase differs");compare(12);advance(0);const auto x=raw[13][0x635],y=raw[13][0x637];const auto expected=y>=96 && y<160 && (x<80 || x>=176) ? FieldSessionPhase::keeper_interaction : FieldSessionPhase::player_control;if(state->session.phase!=expected)throw std::runtime_error("Home keeper punch continuation phase differs");compare(13);}else if(phase==FieldSessionPhase::keeper_possession){{auto unknown=std::make_unique<MatchRuntimeState>(*state);unknown->home_keeper_y.reset();*result=step_match(*unknown,client_input(*unknown,0,0),*content);if(result->unsupported!=UnsupportedCapability::keeper_position || result->state.last_sequence!=unknown->last_sequence)throw std::runtime_error("Unknown caught keeper position did not stop atomically");}advance(0);if(state->session.phase!=FieldSessionPhase::control_ready)throw std::runtime_error("Caught home keeper control phase differs");compare(14);advance(0);compare(15);advance(64,64);if(state->session.phase!=FieldSessionPhase::action_menu_pending)throw std::runtime_error("Caught keeper menu entry differs");compare(16);advance(0);if(state->session.phase!=FieldSessionPhase::human_action_menu)throw std::runtime_error("Caught keeper menu open differs");compare(17);advance(2);compare(18);advance(128);if(state->session.phase!=FieldSessionPhase::attack_choice_pending)throw std::runtime_error("Caught keeper pass confirmation differs");compare(19);{auto unknown=std::make_unique<MatchRuntimeState>(*state);unknown->home_keeper_y.reset();*result=step_match(*unknown,client_input(*unknown,0,0),*content);if(result->unsupported!=UnsupportedCapability::keeper_position || result->state.last_sequence!=unknown->last_sequence)throw std::runtime_error("Unknown keeper position did not stop atomically");}if(!state->home_keeper_y || *state->home_keeper_y!=raw[0][0x308])throw std::runtime_error("Keeper position did not survive save/reload");advance(0);if(state->session.phase!=FieldSessionPhase::ordinary_human_pass_target || !state->session.human_pass_cursor || *state->session.human_pass_cursor!=raw[20][0x624])throw std::runtime_error("Caught keeper pass target differs");compare(20);{auto start=std::make_unique<MatchRuntimeState>(*state);const std::uint8_t directions[]={1,2,4,8,0};for(unsigned k=0;k<5;++k){*state=*start;advance(0,directions[k]);const auto index=21+k;if(!state->session.human_pass_cursor || *state->session.human_pass_cursor!=raw[index][0x624] || !state->session.ordinary_pass_candidates)throw std::runtime_error("Caught keeper cursor move differs");const bool moved=raw[index][0x624]!=raw[20][0x624];if(state->session.ordinary_pass_candidates->scanned!=moved)throw std::runtime_error("Caught keeper candidate scan state differs");if(moved){const auto& candidates=state->session.ordinary_pass_candidates->candidates;if(candidates.count!=raw[index][0x430] || candidates.allied_count!=raw[index][0x625])throw std::runtime_error("Caught keeper candidate counts differ");for(unsigned j=0;j<candidates.count;++j)if(candidates.ids[j]!=raw[index][0x431+j])throw std::runtime_error("Caught keeper candidate differs");}compare(index);}}}}
 return 0;
}
static int opponent_shot_defender_comparison(int argc,char** argv) {
 using namespace captain;if(argc!=6)return 2;
 std::ifstream source_file(argv[2],std::ios::binary),expected_file(argv[3],std::ios::binary),records_file(argv[4],std::ios::binary),pack_file(argv[5],std::ios::binary);
 auto state=std::make_unique<MatchRuntimeState>();state->session=load_phase_session(source_file);auto expected=load_phase_session(expected_file);auto content=std::make_unique<MatchContent>();content->data=load_shot_content(pack_file);
 std::array<std::array<unsigned char,2048>,4> raw{};for(auto& item:raw)if(!records_file.read(reinterpret_cast<char*>(item.data()),item.size()))throw std::runtime_error("Missing opponent shot defender checkpoints");
 const auto word=[&](unsigned index,unsigned at){return static_cast<std::uint16_t>(raw[index][at]|(unsigned(raw[index][at+1])<<8));};
 state->session.phase=FieldSessionPhase::ball_arrival_callback;state->session.saved_ball_flight_flag=raw[0][0x61b];
 state->session.shot_flight=ShotFlightContext{raw[0][0x62b],raw[0][0x638],raw[0][0x61a],false};
 state->session.ball_trajectory=BallTrajectoryContext{word(0,0x634),word(0,0x636),word(0,0x639),word(0,0x63b),raw[0][0x62c],raw[0][0x61b]};
 state->mode=MatchTimingMode::provisional_demo;state->demo_rng=1;state->ball_fraction=MatchBallFraction{raw[0][0x634],raw[0][0x636]};
 MatchContentIdentity identity{};identity[0]=42;std::ostringstream before(std::ios::binary);write_match_save(before,*state,identity);std::istringstream saved(before.str(),std::ios::binary);*state=load_match_save(saved,identity);
 auto missing=std::make_unique<MatchContent>(*content);missing->data->flight.reset();auto stopped=std::make_unique<MatchStepResult>(step_match(*state,client_input(*state,0,0),*missing));
 std::ostringstream missing_state(std::ios::binary);write_match_save(missing_state,stopped->state,identity);
 if(stopped->unsupported!=UnsupportedCapability::content || missing_state.str()!=before.str())throw std::runtime_error("Missing opponent shot defender content was not atomic");
 auto result=std::make_unique<MatchStepResult>(step_match(*state,client_input(*state,0,0),*content));
 if(std::string(argv[1])=="--compare-opponent-shot-defender-terminal"){
  std::ostringstream unchanged(std::ios::binary);write_match_save(unchanged,result->state,identity);
  if(result->unsupported!=UnsupportedCapability::defender_contact || unchanged.str()!=before.str())throw std::runtime_error("Terminal opponent shot defender was not rejected atomically");
  return 0;
 }
 if(result->unsupported || result->state.session.phase!=FieldSessionPhase::shot_keeper_entry)throw std::runtime_error(std::string("Opponent shot defender stopped: ")+(result->unsupported ? capability_name(*result->unsupported) : "incorrect phase"));
 auto actual=result->state.session;actual.phase=FieldSessionPhase::field;actual.shot_flight.reset();actual.ball_trajectory.reset();expected.saved_ball_flight_flag=raw[3][0x61b];
 if(actual.control && expected.control){expected.control->facing=actual.control->facing;expected.control->motion_requested=actual.control->motion_requested;expected.control->display_parameter=actual.control->display_parameter;}
 std::ostringstream a(std::ios::binary),b(std::ios::binary);write_phase_session(a,actual);write_phase_session(b,expected);
 if(a.str()!=b.str()){for(std::size_t i=0;i<a.str().size() && i<b.str().size();++i)if(a.str()[i]!=b.str()[i])throw std::runtime_error("Opponent shot defender differs at byte "+std::to_string(i)+" actual "+std::to_string(static_cast<unsigned char>(a.str()[i]))+" expected "+std::to_string(static_cast<unsigned char>(b.str()[i])));throw std::runtime_error("Opponent shot defender session lengths differ");}
 if(!result->state.ball_fraction || result->state.ball_fraction->x!=raw[3][0x634] || result->state.ball_fraction->y!=raw[3][0x636] || !result->state.retained_defense_action || *result->state.retained_defense_action!=raw[3][0x43d])throw std::runtime_error("Opponent shot defender lost original metadata");
 std::ostringstream out(std::ios::binary);write_match_save(out,result->state,identity);std::istringstream in(out.str(),std::ios::binary);load_match_save(in,identity);
 return 0;
}
static int defense_skip_comparison(int argc,char** argv) {
 using namespace captain;if(argc!=7)return 2;
 std::ifstream source_file(argv[2],std::ios::binary),expected_file(argv[3],std::ios::binary),raw_file(argv[4],std::ios::binary),pack_file(argv[5],std::ios::binary);
 auto state=std::make_unique<MatchRuntimeState>();state->session=load_phase_session(source_file);auto expected=load_phase_session(expected_file);
 std::array<unsigned char,2048> raw{};if(!raw_file.read(reinterpret_cast<char*>(raw.data()),raw.size()))throw std::runtime_error("Missing skip metadata");
 auto content=std::make_unique<MatchContent>();content->data=load_shot_content(pack_file);state->mode=MatchTimingMode::provisional_demo;state->demo_rng=std::stoul(argv[6]);
 state->session.saved_ball_flight_flag=expected.saved_ball_flight_flag=raw[0x61b];state->ball_fraction=MatchBallFraction{raw[0x634],raw[0x636]};
 state->defense_menu=MatchDefenseMenu{};auto& menu=*state->defense_menu;menu.defender_count=state->session.current.field.selected_defenders;menu.poll.cursor=raw[0x61e];menu.poll.defense.actions={6,6,6,255};
 auto result=std::make_unique<MatchStepResult>(step_match(*state,client_input(*state,0,128),*content));
 if(result->unsupported || !result->state.retained_defense_action || *result->state.retained_defense_action!=6 || !result->state.ball_fraction || result->state.ball_fraction->x!=raw[0x634] || result->state.ball_fraction->y!=raw[0x636])throw std::runtime_error("Skip confirmation did not preserve original metadata");
 std::ostringstream a(std::ios::binary),b(std::ios::binary);write_phase_session(a,result->state.session);write_phase_session(b,expected);
 if(a.str()!=b.str()){for(std::size_t i=0;i<a.str().size() && i<b.str().size();++i)if(a.str()[i]!=b.str()[i])throw std::runtime_error("Skip confirmation differs at byte "+std::to_string(i));throw std::runtime_error("Skip confirmation session lengths differ");}
 MatchContentIdentity identity{};identity[0]=42;std::ostringstream saved(std::ios::binary);write_match_save(saved,result->state,identity);std::istringstream in(saved.str(),std::ios::binary);if(load_match_save(in,identity).retained_defense_action!=6)throw std::runtime_error("Skip action not saved");
 return 0;
}
static int unopposed_shot_comparison(int argc,char** argv) {
 using namespace captain;
 if(argc!=8)return 2;
  std::ifstream source_file(argv[2],std::ios::binary),records_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary),velocity_file(argv[5]);
  auto state=std::make_unique<MatchRuntimeState>();state->session=load_phase_session(source_file);MatchContent content{};content.data=load_shot_content(pack_file);content.ball_velocity=read_ball_velocity_content(velocity_file);
  std::array<std::array<unsigned char,2048>,6> raw{};for(auto& item:raw)if(!records_file.read(reinterpret_cast<char*>(item.data()),item.size()))throw std::runtime_error("Missing original unopposed AI shot checkpoints");
  state->session.saved_ball_flight_flag=raw[0][0x61b];state->mode=MatchTimingMode::provisional_demo;state->demo_rng=std::stoul(argv[6]);state->ball_fraction=MatchBallFraction{raw[0][0x634],raw[0][0x636]};state->retained_defense_action=raw[0][0x43d];
  const std::string names[]={"choice","prepared","clocked","approach","target","flight"};MatchContentIdentity identity{};identity[0]=42;auto result=std::make_unique<MatchStepResult>();
  const auto advance=[&](){*result=step_match(*state,client_input(*state,0,0),content);if(result->unsupported){std::string detail;if(state->session.phase==FieldSessionPhase::attack_prepared){try{dispatch_session_shot(state->session,1);}catch(const std::invalid_argument& e){detail=std::string(" ")+e.what();}}throw std::runtime_error(std::string("Unopposed AI shot stopped: ")+capability_name(*result->unsupported)+" phase "+std::to_string(static_cast<unsigned>(state->session.phase))+detail);}*state=result->state;std::ostringstream out(std::ios::binary);write_match_save(out,*state,identity);std::istringstream in(out.str(),std::ios::binary);*state=load_match_save(in,identity);};
  const auto compare=[&](unsigned index){std::ifstream in(std::string(argv[7])+"/"+names[index]+".cmjsession",std::ios::binary);auto expected=load_phase_session(in),actual=state->session;expected.saved_ball_flight_flag=raw[index][0x61b];
   actual.phase=FieldSessionPhase::field;actual.shot_progress.reset();actual.shot_flight.reset();actual.ball_trajectory.reset();
   if(index>=2 && actual.control && expected.control){expected.control->facing=actual.control->facing;expected.control->motion_requested=actual.control->motion_requested;expected.control->display_parameter=actual.control->display_parameter;}
   std::ostringstream a(std::ios::binary),b(std::ios::binary);write_phase_session(a,actual);write_phase_session(b,expected);
   if(a.str()!=b.str()){for(std::size_t i=0;i<a.str().size() && i<b.str().size();++i)if(a.str()[i]!=b.str()[i])throw std::runtime_error("Unopposed AI shot "+names[index]+" differs at byte "+std::to_string(i)+" actual "+std::to_string(static_cast<unsigned char>(a.str()[i]))+" expected "+std::to_string(static_cast<unsigned char>(b.str()[i])));throw std::runtime_error("Unopposed AI shot session lengths differ");}
  };
  advance();compare(0);advance();compare(1);advance();compare(2);advance();advance();compare(3);advance();compare(4);advance();compare(5);
  const auto word=[&](unsigned at){return static_cast<std::uint16_t>(raw[5][at]|(unsigned(raw[5][at+1])<<8));};
  if(!state->session.ball_trajectory || state->session.ball_trajectory->x!=word(0x634) || state->session.ball_trajectory->y!=word(0x636) || state->session.ball_trajectory->vx!=word(0x639) || state->session.ball_trajectory->vy!=word(0x63b))throw std::runtime_error("Unopposed AI shot trajectory differs");
  std::ifstream flight_records(std::string(argv[7])+"/flight-records.bin",std::ios::binary);std::array<unsigned char,2048> flight{};unsigned boundaries=0;
  while(flight_records.peek()!=std::char_traits<char>::eof()){
   const auto event=flight_records.get();if(event<0 || !flight_records.read(reinterpret_cast<char*>(flight.data()),flight.size()))throw std::runtime_error("Truncated opponent shot flight record");
   advance();const auto phase=event==0 ? FieldSessionPhase::ball_cell_callback : event==1 ? FieldSessionPhase::ball_arrival_callback : event==2 ? FieldSessionPhase::ball_advance_ready : event==3 ? FieldSessionPhase::shot_keeper_entry : FieldSessionPhase::keeper_presentation_pending;
   if(event>4 || state->session.phase!=phase || !state->session.ball_trajectory)throw std::runtime_error("Opponent shot flight phase differs");
   const auto value=[&](unsigned at){return static_cast<std::uint16_t>(flight[at]|(unsigned(flight[at+1])<<8));};const auto& trajectory=*state->session.ball_trajectory;const auto& field=state->session.current.field;
   if(trajectory.x!=value(0x634) || trajectory.y!=value(0x636) || field.ball_cell!=flight[0x5fe] || field.selected_defenders!=flight[0x600])throw std::runtime_error("Opponent shot flight position or defender count differs");
   for(unsigned i=0;i<field.selected_defenders;++i)if(state->session.scheduling->selected_ids[i]!=flight[0x601+i])throw std::runtime_error("Opponent shot flight defender order differs");
   if(event>=3 && state->session.recovery->opposing_player!=flight[0x442])throw std::runtime_error("Opponent shot keeper selection differs");
   ++boundaries;
  }
  if(!boundaries || (state->session.phase!=FieldSessionPhase::ball_arrival_callback && state->session.phase!=FieldSessionPhase::keeper_presentation_pending))throw std::runtime_error("Opponent shot did not arrive");
  return 0;
}
static int comparisons(int argc,char** argv) {
 using namespace captain;

 if(argc==9 && std::string(argv[1])=="--compare-unopposed-ai-pass") {
  std::ifstream source_file(argv[2],std::ios::binary),records_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary),distance_file(argv[5],std::ios::binary),velocity_file(argv[6]);
  auto source=load_phase_session(source_file);MatchContent content{};content.data=load_shot_content(pack_file);content.pass_distance=read_pass_distance_content(distance_file);content.ball_velocity=read_ball_velocity_content(velocity_file);
  std::array<std::array<unsigned char,2048>,9> raw{};for(auto& item:raw)if(!records_file.read(reinterpret_cast<char*>(item.data()),item.size()))throw std::runtime_error("Missing original unopposed AI pass checkpoints");
  auto state=std::make_unique<MatchRuntimeState>();state->session=source;state->session.saved_ball_flight_flag=raw[0][0x61b];state->mode=MatchTimingMode::provisional_demo;state->demo_rng=std::stoul(argv[7]);
  state->ball_fraction=MatchBallFraction{raw[0][0x634],raw[0][0x636]};state->retained_defense_action=raw[0][0x43d];
  const std::string names[]={"choice","prepared","preclock","handoff","target","flight","arrival","control","ready"};
  auto result=std::make_unique<MatchStepResult>();MatchContentIdentity identity{};identity[0]=42;
  const auto advance=[&](){
   *result=step_match(*state,client_input(*state,0,0),content);
   if(result->unsupported)throw std::runtime_error(std::string("Unopposed AI pass stopped: ")+capability_name(*result->unsupported)+" phase "+std::to_string(static_cast<unsigned>(state->session.phase)));
   *state=result->state;std::ostringstream out(std::ios::binary);write_match_save(out,*state,identity);std::istringstream in(out.str(),std::ios::binary);*state=load_match_save(in,identity);
  };
  const auto compare=[&](unsigned index,bool flight_arrival=false){
   std::ifstream expected_file(std::string(argv[8])+"/"+names[index]+".cmjsession",std::ios::binary);auto wanted=load_phase_session(expected_file);auto actual=state->session;
   wanted.saved_ball_flight_flag=raw[index][0x61b];
   // The script parser writes facing during visual acknowledgements. The demo
   // does not execute that presentation engine; control-ready resets it.
   if(index>=3 && index<=7 && wanted.control && actual.control)wanted.control->facing=actual.control->facing;
   if(flight_arrival){actual.current.field.ball_x=raw[index][0x635];actual.current.field.ball_y=raw[index][0x637];actual.current.field.ball_cell=raw[index][0x5fe];}
   std::ostringstream a(std::ios::binary),b(std::ios::binary);write_phase_session(a,actual);write_phase_session(b,wanted);
   if(a.str()!=b.str()){for(std::size_t i=0;i<a.str().size() && i<b.str().size();++i)if(a.str()[i]!=b.str()[i])throw std::runtime_error("Unopposed AI "+names[index]+" differs at byte "+std::to_string(i)+" actual "+std::to_string(static_cast<unsigned char>(a.str()[i]))+" expected "+std::to_string(static_cast<unsigned char>(b.str()[i])));throw std::runtime_error("Unopposed AI session lengths differ");}
   if(state->retained_defense_action!=raw[index][0x43d])throw std::runtime_error("Unopposed AI pass lost retained action");
  };
  advance();compare(0);if(!state->defense_menu || !state->defense_menu->ai_pass_target || state->defense_menu->ai_pass_target->recipient!=raw[0][0x5fc] || state->defense_menu->ai_pass_target->cell!=raw[0][0x638])throw std::runtime_error("Unopposed AI chose wrong target");
  advance();compare(1);advance();compare(2);advance();advance();compare(3);
  advance();compare(4);if(state->defense_menu->ai_pass_distance!=raw[4][0x62b])throw std::runtime_error("Unopposed AI pass distance differs");
  advance();compare(5);const auto word=[&](unsigned at){return static_cast<std::uint16_t>(raw[5][at]|(unsigned(raw[5][at+1])<<8));};
  if(!state->defense_menu->ai_trajectory || state->defense_menu->ai_trajectory->x!=word(0x634) || state->defense_menu->ai_trajectory->y!=word(0x636) || state->defense_menu->ai_trajectory->vx!=word(0x639) || state->defense_menu->ai_trajectory->vy!=word(0x63b))throw std::runtime_error("Unopposed AI flight setup differs");
  for(unsigned turn=0;turn<300 && state->defense_menu->stage!=MatchDefenseStage::ai_flight_arrived;++turn)advance();
  if(state->defense_menu->stage!=MatchDefenseStage::ai_flight_arrived || !state->flight_defenders.empty())throw std::runtime_error("Unopposed AI pass did not arrive unopposed");
  const auto arrival_word=[&](unsigned at){return static_cast<std::uint16_t>(raw[6][at]|(unsigned(raw[6][at+1])<<8));};
  if(!state->defense_menu->ai_trajectory || !state->defense_menu->ai_ball_cell || state->defense_menu->ai_trajectory->x!=arrival_word(0x634) || state->defense_menu->ai_trajectory->y!=arrival_word(0x636) || *state->defense_menu->ai_ball_cell!=raw[6][0x5fe])throw std::runtime_error("Unopposed AI arrival trajectory differs");
  compare(6,true);advance();compare(7);advance();compare(8);
  if(!state->ball_fraction || state->ball_fraction->x!=raw[8][0x634] || state->ball_fraction->y!=raw[8][0x636])throw std::runtime_error("Unopposed AI reception fractions differ");
  return 0;
 }
 if(argc==7 && (std::string(argv[1])=="--compare-ai-cell" || std::string(argv[1])=="--compare-ai-cell-pass" || std::string(argv[1])=="--compare-ai-cell-shot" || std::string(argv[1])=="--compare-ai-cell-unsupported" || std::string(argv[1])=="--compare-ai-cell-poll")) {
  const bool poll=std::string(argv[1])=="--compare-ai-cell-poll";
  const bool pass=std::string(argv[1])=="--compare-ai-cell-pass";
  const bool shot=std::string(argv[1])=="--compare-ai-cell-shot";
  std::ifstream source_file(argv[2],std::ios::binary),expected_file(argv[3],std::ios::binary),raw_file(argv[4],std::ios::binary),pack_file(argv[5],std::ios::binary);
  auto source=load_phase_session(source_file),expected=load_phase_session(expected_file);MatchContent content{};content.data=load_shot_content(pack_file);
  std::array<unsigned char,2048> raw{};if(!raw_file.read(reinterpret_cast<char*>(raw.data()),raw.size()))throw std::runtime_error("Missing AI cell metadata");
  auto state=std::make_unique<MatchRuntimeState>();state->session=source;state->mode=MatchTimingMode::provisional_demo;state->demo_rng=std::stoul(argv[6]);
  state->ball_fraction=MatchBallFraction{raw[0x634],raw[0x636]};state->retained_defense_action=raw[0x43d];
  MatchContentIdentity identity{};identity[0]=42;std::ostringstream before(std::ios::binary);write_match_save(before,*state,identity);std::istringstream saved(before.str(),std::ios::binary);*state=load_match_save(saved,identity);
  auto result=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::tick,1},content));
  if(std::string(argv[1])=="--compare-ai-cell-unsupported"){
   if(result->unsupported!=UnsupportedCapability::opponent_action || result->state.last_sequence!=state->last_sequence || result->state.demo_rng!=state->demo_rng)throw std::runtime_error("Unsupported AI cell choice was not atomic");
   std::ostringstream unchanged(std::ios::binary);write_match_save(unchanged,result->state,identity);if(unchanged.str()!=before.str())throw std::runtime_error("Unsupported AI cell changed state");
   return 0;
  }
  if(result->unsupported || result->state.session.phase!=(poll?FieldSessionPhase::ai_cell_decision:(pass || shot)?FieldSessionPhase::field:FieldSessionPhase::control_clock_pending))throw std::runtime_error("AI cell fallback, pass, shot, or poll unsupported");
  if(pass && (!result->state.defense_menu || result->state.defense_menu->defender_count || result->state.defense_menu->stage!=MatchDefenseStage::ai_chosen || !result->state.defense_menu->ai_pass_target || result->state.defense_menu->ai_pass_target->recipient!=raw[0x5fc] || result->state.defense_menu->ai_pass_target->cell!=raw[0x638]))throw std::runtime_error("AI cell chose incorrect pass recipient");
  if(shot && (!result->state.defense_menu || result->state.defense_menu->defender_count || result->state.defense_menu->stage!=MatchDefenseStage::ai_chosen || result->state.defense_menu->ai_pass_target || result->state.session.action->action!=0))throw std::runtime_error("AI cell did not choose the ordinary shot");
  if(!result->state.ball_fraction || result->state.ball_fraction->x!=raw[0x634] || result->state.ball_fraction->y!=raw[0x636] || result->state.retained_defense_action!=raw[0x43d])throw std::runtime_error("AI cell fallback lost known metadata");
  if(!poll){
   auto unknown=std::make_unique<MatchRuntimeState>(*state);unknown->ball_fraction.reset();unknown->retained_defense_action.reset();
   auto next=std::make_unique<MatchStepResult>(step_match(*unknown,{MatchInputKind::tick,1},content));
   if(next->unsupported || next->state.ball_fraction || next->state.retained_defense_action)throw std::runtime_error("AI fallback invented unknown metadata");
   auto missing=std::make_unique<MatchContent>(content);missing->data->ai_field_choice.reset();
   *next=step_match(*state,{MatchInputKind::tick,1},*missing);
   if(next->unsupported!=UnsupportedCapability::content || next->state.last_sequence!=state->last_sequence)throw std::runtime_error("Missing AI content was not atomic");
  }
  std::ostringstream actual(std::ios::binary),wanted(std::ios::binary);write_phase_session(actual,result->state.session);write_phase_session(wanted,expected);
  if(actual.str()!=wanted.str()){for(std::size_t i=0;i<actual.str().size() && i<wanted.str().size();++i)if(actual.str()[i]!=wanted.str()[i])throw std::runtime_error("Original AI cell fallback differs at byte "+std::to_string(i));throw std::runtime_error("Original AI cell session lengths differ");}
  return 0;
 }
 if(argc==6 && std::string(argv[1])=="--compare-pass-field-handoff") {
  std::ifstream source_file(argv[2],std::ios::binary),expected_file(argv[3],std::ios::binary),raw_file(argv[4],std::ios::binary),pack_file(argv[5],std::ios::binary);
  auto source=load_phase_session(source_file),expected=load_phase_session(expected_file);MatchContent content{};content.data=load_shot_content(pack_file);
  std::array<unsigned char,2048> raw{};if(!raw_file.read(reinterpret_cast<char*>(raw.data()),raw.size()))throw std::runtime_error("Missing original pass movement input");
  source.phase=FieldSessionPhase::ordinary_pass_prepared;source.human_pass_cursor=raw[0x624];source.ordinary_pass_candidates=OrdinaryPassCandidateContext{};
  source.ordinary_pass_recipient=255;source.ordinary_pass_defenses=OrdinaryPassDefenseContext{};source.recovery->ball_variant=0;
  auto state=std::make_unique<MatchRuntimeState>();state->session=source;state->mode=MatchTimingMode::provisional_demo;state->demo_rng=1;
  state->pass_stage=MatchPassStage::awaiting_presentation;state->ball_fraction=MatchBallFraction{raw[0x634],raw[0x636]};state->retained_defense_action=raw[0x43d];
  auto result=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::presentation_complete,1},content));
  if(result->unsupported || result->state.pass_stage!=MatchPassStage::handoff)throw std::runtime_error("Pass field handoff unsupported");
  std::ostringstream actual(std::ios::binary),wanted(std::ios::binary);write_field_state(actual,result->state.session.current.field);write_field_state(wanted,expected.current.field);
  if(actual.str()!=wanted.str()){for(std::size_t i=0;i<actual.str().size();++i)if(actual.str()[i]!=wanted.str()[i])throw std::runtime_error("Original pass movement differs at byte "+std::to_string(i));}
  const auto& a=result->state.session.current.clock;const auto& e=expected.current.clock;
  if(a.remaining!=e.remaining || a.extra_time!=e.extra_time || a.flags!=e.flags || a.script_flags!=e.script_flags || a.ball_mode!=e.ball_mode || result->state.session.recovery->ball_variant!=1 || !result->state.ball_fraction || result->state.ball_fraction->x!=raw[0x634] || result->state.ball_fraction->y!=raw[0x636])throw std::runtime_error("Original pass clock, variant or fractions differ");
  return 0;
 }
 if(argc==7 && (std::string(argv[1])=="--drive-control-shot" || std::string(argv[1])=="--drive-control-pass")) {
  const bool pass=std::string(argv[1])=="--drive-control-pass";
  std::ifstream source_file(argv[2],std::ios::binary),pack_file(argv[3],std::ios::binary),velocity_file(argv[4]),distance_file(argv[5],std::ios::binary),raw_file(argv[6],std::ios::binary);
  auto source=load_phase_session(source_file);MatchContent content{};content.data=load_shot_content(pack_file);content.ball_velocity=read_ball_velocity_content(velocity_file);content.pass_distance=read_pass_distance_content(distance_file);
  std::array<unsigned char,2048> raw{};if(!raw_file.read(reinterpret_cast<char*>(raw.data()),raw.size()))throw std::runtime_error("Missing original control metadata");
  source.score=MatchScoreContext{raw[0x28],raw[0x29],raw[0x629]};
  MatchConfig config{};config.initial=source;config.mode=MatchTimingMode::provisional_demo;config.seed=1;config.ball_fraction=MatchBallFraction{raw[0x634],raw[0x636]};config.retained_defense_action=raw[0x43d];
  auto result=std::make_unique<MatchStepResult>(create_match(config,content));if(result->unsupported)throw std::runtime_error("Move-to-shot setup unsupported");
  auto state=std::make_unique<MatchRuntimeState>(result->state);const auto start_x=state->session.current.field.ball_x;
  const auto advance=[&](std::uint8_t held,std::uint8_t pressed){
   *result=step_match(*state,client_input(*state,held,pressed),content);
   if(result->unsupported)throw std::runtime_error(std::string("Move-to-action stopped at phase ")+std::to_string(static_cast<unsigned>(state->session.phase))+", pass stage "+std::to_string(static_cast<unsigned>(state->pass_stage))+": "+capability_name(*result->unsupported));
   *state=result->state;
  };
  advance(1,1);if(state->session.current.field.ball_x==start_x || state->session.phase!=FieldSessionPhase::control_clock_pending)throw std::runtime_error("Carrier did not move");
  advance(1,0);advance(1,0);if(state->session.phase!=FieldSessionPhase::control_ready)throw std::runtime_error("Moved control did not resume");
  advance(64,64);advance(0,0);advance(pass?2:1,pass?2:1);advance(128,128);
  if(state->session.phase!=FieldSessionPhase::attack_choice_pending || state->session.action->action!=(pass?1:0))throw std::runtime_error("Action menu did not confirm");
  if(pass){
   advance(0,0);if(state->session.phase!=FieldSessionPhase::ordinary_human_pass_target)throw std::runtime_error("Pass targeting did not open");
   advance(128,128);
   for(unsigned turn=0;turn<20 && state->pass_stage!=MatchPassStage::flight_arrived;++turn)advance(0,0);
   if(state->pass_stage!=MatchPassStage::flight_arrived || !state->pass_trajectory)throw std::runtime_error("Moved same-cell pass did not reach arrival");
   if(!state->ball_fraction || state->ball_fraction->x!=raw[0x634] || state->ball_fraction->y!=raw[0x636])throw std::runtime_error("Moved pass lost known initial fractions");
   advance(0,0);if(state->session.phase!=FieldSessionPhase::player_control)throw std::runtime_error("Moved pass did not reach player control");
   advance(0,0);if(state->session.phase!=FieldSessionPhase::control_ready)throw std::runtime_error("Moved pass did not resume control");
  } else {
   for(unsigned turn=0;turn<20 && state->session.phase!=FieldSessionPhase::ball_advance_ready;++turn)advance(0,0);
   if(state->session.phase!=FieldSessionPhase::ball_advance_ready || !state->session.ball_trajectory)throw std::runtime_error("Moved shot did not enter flight");
  }
  if(!state->ball_fraction || state->ball_fraction->x!=raw[0x634] || state->ball_fraction->y!=raw[0x636])throw std::runtime_error("Moved action lost known initial fractions");
  return 0;
 }
 if(argc==8 && std::string(argv[1])=="--compare-control-fractions") {
  std::ifstream source_file(argv[2],std::ios::binary),poll_file(argv[3],std::ios::binary),clock_file(argv[4],std::ios::binary),field_file(argv[5],std::ios::binary),pack_file(argv[6],std::ios::binary);
  const auto source=load_phase_session(source_file);MatchContent content{};content.data=load_shot_content(pack_file);
  std::array<std::array<unsigned char,2048>,3> raw{};
  if(!poll_file.read(reinterpret_cast<char*>(raw[0].data()),2048) || !clock_file.read(reinterpret_cast<char*>(raw[1].data()),2048) || !field_file.read(reinterpret_cast<char*>(raw[2].data()),2048))throw std::runtime_error("Missing original control fraction boundaries");
  MatchConfig config{};config.initial=source;config.mode=MatchTimingMode::provisional_demo;config.seed=1;config.ball_fraction=MatchBallFraction{raw[0][0x634],raw[0][0x636]};
  auto created=std::make_unique<MatchStepResult>(create_match(config,content));if(created->unsupported)throw std::runtime_error("Control fixture unsupported");
  auto state=std::make_unique<MatchRuntimeState>(created->state);const auto held=static_cast<std::uint8_t>(std::stoul(argv[7]));
  const FieldSessionPhase expected[]={FieldSessionPhase::control_clock_pending,FieldSessionPhase::field_round_pending,FieldSessionPhase::control_ready};
  for(unsigned i=0;i<3;++i){
   MatchInput input{MatchInputKind::tick,state->last_sequence+1};input.held=held;
   *created=step_match(*state,input,content);
   if(created->unsupported || created->state.session.phase!=expected[i])throw std::runtime_error("Control fraction path unsupported or changed phase");
   if(!created->state.ball_fraction || created->state.ball_fraction->x!=raw[i][0x634] || created->state.ball_fraction->y!=raw[i][0x636])throw std::runtime_error("Original ball fractions differ");
   const auto& field=created->state.session.current.field;
   if(i==0){const auto owner=field.ball_owner;const auto& player=field.motion[field_motion_index(owner)];const auto at=0x300u+12u*owner;
    if(field.ball_x!=raw[i][0x635] || field.ball_y!=raw[i][0x637] || player.x!=(raw[i][at+5]|unsigned(raw[i][at+6])<<8) || player.y!=(raw[i][at+7]|unsigned(raw[i][at+8])<<8))throw std::runtime_error("Original control movement differs");
   }
   MatchContentIdentity identity{};identity[0]=42;std::ostringstream saved(std::ios::binary);write_match_save(saved,created->state,identity);std::istringstream in(saved.str(),std::ios::binary);*state=load_match_save(in,identity);
  }
  return 0;
 }
 if((argc==6 || (argc==7 && std::string(argv[6])=="--retained")) && (std::string(argv[1])=="--compare-free-arrival" || std::string(argv[1])=="--compare-same-arrival")) {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),raw_file(argv[4],std::ios::binary),pack_file(argv[5],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);std::array<unsigned char,2048> raw{};
  if(!raw_file.read(reinterpret_cast<char*>(raw.data()),raw.size()))throw std::runtime_error("Missing original free-pass arrival");
  MatchContent content{};content.data=load_shot_content(pack_file);const auto owner=before.current.field.ball_owner;
  PhaseSessionState session{};session.current=before.current;session.experience=before.experience;session.recovery=before.recovery;session.control=before.control;session.scheduling=before.scheduling;session.action=before.action;
  const auto& carrier=session.current.field.motion[field_motion_index(owner)];session.current.field.ball_x=carrier.x>>8;session.current.field.ball_y=carrier.y>>8;
  session.current.field.ball_cell=field_cell_at(session.current.field.ball_x,session.current.field.ball_y);session.current.field.selected_defenders=0;
  session.phase=FieldSessionPhase::ordinary_pass_prepared;session.human_pass_cursor=raw[0x638];session.ordinary_pass_candidates=OrdinaryPassCandidateContext{};
  session.ordinary_pass_candidates->scanned=true;session.ordinary_pass_recipient=255;session.ordinary_pass_defenses=OrdinaryPassDefenseContext{};session.saved_ball_flight_flag=raw[0x61b];
  const auto word=[&](unsigned at){return static_cast<std::uint16_t>(raw[at]|(unsigned(raw[at+1])<<8));};
  MatchRuntimeState state{};state.session=session;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;state.pass_stage=MatchPassStage::flight_arrived;
  if(argc==7){if(raw[0x43d]>6)throw std::runtime_error("Invalid original retained action");state.retained_defense_action=raw[0x43d];}
  state.pass_target=MatchPassTarget{255,raw[0x638],raw[0x61a],raw[0x62b]};state.pass_cell=raw[0x5fe];
  state.pass_trajectory=BallTrajectoryContext{word(0x634),word(0x636),word(0x639),word(0x63b),raw[0x62c],raw[0x61b]};
  std::uint64_t sequence=1;
  if(std::string(argv[1])=="--compare-same-arrival") {
   state.pass_stage=MatchPassStage::target_ready;state.pass_trajectory.reset();state.pass_cell.reset();
   state.session.saved_ball_flight_flag=0;
   state.ball_fraction=MatchBallFraction{raw[0x634],raw[0x636]};
   auto setup=step_match(state,{MatchInputKind::tick,sequence++},content);
   if(setup.unsupported || setup.state.pass_stage!=MatchPassStage::flight_arrived || !setup.state.pass_trajectory || !setup.state.pass_cell)
    throw std::runtime_error("Same-cell pass setup unsupported");
   if(setup.state.pass_trajectory->x!=word(0x634) || setup.state.pass_trajectory->y!=word(0x636) ||
      *setup.state.pass_cell!=raw[0x5fe] || setup.state.session.saved_ball_flight_flag!=raw[0x61b] ||
      setup.state.session.recovery->x_jitter_control!=before.recovery->x_jitter_control)
    throw std::runtime_error("Same-cell pass setup differs from original arrival");
   state=std::move(setup.state);
  }
  auto result=step_match(state,{MatchInputKind::presentation_complete,sequence},content);
  if(result.unsupported)throw std::runtime_error(std::string("Original free-pass contest unsupported: ")+capability_name(*result.unsupported));
  if(result.state.session.phase!=FieldSessionPhase::player_control)throw std::runtime_error("Original free-pass contest did not return control");
  if(argc==7){
   if(result.state.retained_defense_action!=state.retained_defense_action)throw std::runtime_error("Free-pass contest lost retained action");
   MatchContentIdentity identity{};identity[0]=42;std::ostringstream out(std::ios::binary);write_match_save(out,result.state,identity);std::istringstream in(out.str(),std::ios::binary);
   auto restored=load_match_save(in,identity);auto ready=step_match(restored,{MatchInputKind::tick,sequence+1},content);
   if(ready.unsupported || ready.state.session.phase!=FieldSessionPhase::control_ready || ready.state.retained_defense_action!=state.retained_defense_action)throw std::runtime_error("Free-pass control lost retained action");
  }
  std::ostringstream actual_field(std::ios::binary),expected_field(std::ios::binary);write_field_state(actual_field,result.state.session.current.field);write_field_state(expected_field,expected.current.field);
  if(actual_field.str()!=expected_field.str()){for(std::size_t i=0;i<actual_field.str().size();++i)if(actual_field.str()[i]!=expected_field.str()[i])throw std::runtime_error("Original free-pass contest field differs at byte "+std::to_string(i));}
  const auto& a=result.state.session.current;const auto& e=expected.current;
  if(a.clock.remaining!=e.clock.remaining || a.clock.extra_time!=e.clock.extra_time || a.clock.flags!=e.clock.flags || a.clock.script_flags!=e.clock.script_flags || a.clock.ball_mode!=e.clock.ball_mode || a.shared_recovery_flags!=e.shared_recovery_flags || a.shared_recovery_timer!=e.shared_recovery_timer || result.state.session.action->power!=expected.action->power || result.state.session.experience->totals!=expected.experience->totals)throw std::runtime_error("Original free-pass contest clock or experience differs");
  return 0;
 }
 if(argc==5 && std::string(argv[1])=="--compare-free-race") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto before=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  if(!before.shot_flight || !before.ball_trajectory || !before.area_ai_target || before.area_ai_target->recipient!=255 || !content.data->pass_race)throw std::runtime_error("Missing original free-pass flight context");
  PhaseSessionState session{};session.current=before.current;session.experience=before.experience;session.recovery=before.recovery;session.control=before.control;session.scheduling=before.scheduling;session.action=before.action;
  const auto owner=session.current.field.ball_owner;const auto& player=session.current.field.motion[field_motion_index(owner)];
  session.current.field.ball_x=player.x>>8;session.current.field.ball_y=player.y>>8;session.current.field.ball_cell=field_cell_at(session.current.field.ball_x,session.current.field.ball_y);session.current.field.selected_defenders=0;
  session.phase=FieldSessionPhase::ordinary_pass_prepared;session.human_pass_cursor=before.shot_flight->destination;session.ordinary_pass_candidates=OrdinaryPassCandidateContext{};
  session.ordinary_pass_recipient=255;session.ordinary_pass_defenses=OrdinaryPassDefenseContext{};session.saved_ball_flight_flag=before.saved_ball_flight_flag;
  MatchRuntimeState state{};state.session=session;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;state.pass_stage=MatchPassStage::flight_ready;
  state.pass_target=MatchPassTarget{255,before.shot_flight->destination,before.shot_flight->counter,before.shot_flight->distance};state.pass_trajectory=*before.ball_trajectory;state.pass_cell=before.current.field.ball_cell;
  for(std::uint64_t seq=1;state.pass_stage!=MatchPassStage::flight_arrived && seq<150;++seq){auto next=step_match(state,{MatchInputKind::tick,seq},content);if(next.unsupported)throw std::runtime_error("Original free pass flight unsupported");state=next.state;}
  if(state.pass_stage!=MatchPassStage::flight_arrived)throw std::runtime_error("Original free pass did not arrive");
  auto result=step_match(state,{MatchInputKind::presentation_complete,state.last_sequence+1},content);
  if(result.unsupported || result.state.session.phase!=FieldSessionPhase::player_control)throw std::runtime_error("Original free race unsupported");
  std::ostringstream actual_field(std::ios::binary),expected_field(std::ios::binary);write_field_state(actual_field,result.state.session.current.field);write_field_state(expected_field,expected.current.field);
  if(actual_field.str()!=expected_field.str()){for(std::size_t i=0;i<actual_field.str().size();++i)if(actual_field.str()[i]!=expected_field.str()[i])throw std::runtime_error("Original free race field differs at byte "+std::to_string(i));}
  if(result.state.session.current.clock.remaining!=expected.current.clock.remaining || result.state.session.experience->totals!=expected.experience->totals || result.state.session.action->power!=expected.action->power)
   throw std::runtime_error("Original free race clock, experience or power differs");
  return 0;
 }
 if(argc==4 && std::string(argv[1])=="--compare-race-pack") {
  std::ifstream pack_file(argv[2],std::ios::binary),table(argv[3]);auto pack=load_shot_content(pack_file);
  if(!pack.pass_race)throw std::runtime_error("Missing pass race content");
  for(const auto& pair:pack.pass_race->speeds)for(const auto speed:pair){unsigned expected;if(!(table>>expected) || expected!=speed)throw std::runtime_error("Pass race speed differs from original exporter");}
  std::string extra;if(table>>extra)throw std::runtime_error("Unexpected race table content");return 0;
 }
 if(argc==7 && std::string(argv[1])=="--compare-human-flight-contact") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary),raw_file(argv[5],std::ios::binary);
  auto source=load_phase_session(before_file),expected=load_phase_session(after_file);std::array<unsigned char,2048> raw{};
  if(!raw_file.read(reinterpret_cast<char*>(raw.data()),raw.size()))throw std::runtime_error("Missing original human-flight RAM");
  MatchContent content{};content.data=load_shot_content(pack_file);const auto owner=source.current.field.ball_owner,recipient=raw[0x5fc],count=source.current.field.selected_defenders;
  MatchRuntimeState state{};state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;
  for(unsigned i=0;i<count;++i)state.flight_defenders.push_back(source.scheduling->selected_ids[i]);
  const auto& carrier=source.current.field.motion[field_motion_index(owner)];source.current.field.ball_x=carrier.x>>8;source.current.field.ball_y=carrier.y>>8;
  source.current.field.ball_cell=field_cell_at(source.current.field.ball_x,source.current.field.ball_y);source.current.field.selected_defenders=0;
  source.phase=FieldSessionPhase::ordinary_pass_prepared;source.human_pass_cursor=raw[0x638];source.human_pass_steps=0;
  source.ordinary_pass_candidates=OrdinaryPassCandidateContext{};source.ordinary_pass_candidates->scanned=true;
  source.ordinary_pass_candidates->candidates.count=1;source.ordinary_pass_candidates->candidates.allied_count=1;source.ordinary_pass_candidates->candidates.ids[0]=recipient;
  source.ordinary_pass_recipient=recipient;source.ordinary_pass_defenses=OrdinaryPassDefenseContext{};source.saved_ball_flight_flag=raw[0x61b];state.session=source;
  const auto word=[&](unsigned at){return static_cast<std::uint16_t>(raw[at]|(unsigned(raw[at+1])<<8));};
  state.pass_stage=MatchPassStage::flight_arrived;state.pass_target=MatchPassTarget{recipient,raw[0x638],raw[0x61a],raw[0x62b]};state.pass_cell=raw[0x5fe];
  state.pass_trajectory=BallTrajectoryContext{word(0x634),word(0x636),word(0x639),word(0x63b),raw[0x62c],raw[0x61b]};
  MatchStepResult result{};for(unsigned i=0;i<count;++i){result=step_match(state,{MatchInputKind::presentation_complete,i+1},content);if(result.unsupported)throw std::runtime_error("Human flight contact unsupported");state=result.state;}
  const auto code=std::stoul(argv[6]);if(!result.state.flight_defenders.empty())throw std::runtime_error("Human flight defender prefix not cleared");
  if(code<2){
   if(result.state.pass_stage!=MatchPassStage::flight_arrived)throw std::runtime_error("Human pass did not retain arrival");
   expected.current.field.ball_x=source.current.field.ball_x;expected.current.field.ball_y=source.current.field.ball_y;expected.current.field.ball_cell=source.current.field.ball_cell;
  }else if(result.state.session.phase!=FieldSessionPhase::player_control || result.state.pass_stage!=MatchPassStage::none)throw std::runtime_error("Human defended pass did not return to control");
  if(code==3)for(unsigned i=0;i<expected.current.field.selected_defenders;++i)if(result.state.session.scheduling->selected_ids[i]!=expected.scheduling->selected_ids[i])throw std::runtime_error("Human original defender prefix differs");
  std::ostringstream actual_field(std::ios::binary),expected_field(std::ios::binary);write_field_state(actual_field,result.state.session.current.field);write_field_state(expected_field,expected.current.field);
  if(actual_field.str()!=expected_field.str()){for(std::size_t i=0;i<actual_field.str().size();++i)if(actual_field.str()[i]!=expected_field.str()[i])throw std::runtime_error("Human original field differs at byte "+std::to_string(i));}
  const auto& a=result.state.session.current;const auto& e=expected.current;const auto& ar=*result.state.session.recovery;const auto& er=*expected.recovery;
  if(a.clock.remaining!=e.clock.remaining)throw std::runtime_error("Human original remaining clock differs");
  if(a.clock.extra_time!=e.clock.extra_time || a.clock.flags!=e.clock.flags || a.clock.script_flags!=e.clock.script_flags || a.clock.ball_mode!=e.clock.ball_mode)throw std::runtime_error("Human original clock flags differ");
  if(a.shared_recovery_flags!=e.shared_recovery_flags || a.shared_recovery_timer!=e.shared_recovery_timer)throw std::runtime_error("Human original shared recovery differs");
  if(result.state.session.action->power!=expected.action->power)throw std::runtime_error("Human original power differs");
  if(result.state.session.experience->totals!=expected.experience->totals){
   std::string message="Human original experience differs; awards";
   for(auto id:{unsigned(owner),unsigned(raw[0x601])})for(unsigned selector=0;selector<8;++selector){auto award=award_player_experience(*source.experience,source.current.field.roster[id].type,selector,*content.data->experience);message+=" "+std::to_string(id)+"/"+std::to_string(selector)+"="+std::to_string(award.award);}
   throw std::runtime_error(message);
  }
  if(ar.opposing_player!=er.opposing_player || ar.attack_subtype!=er.attack_subtype || ar.special_owner_flags!=er.special_owner_flags || ar.ball_variant!=er.ball_variant || ar.game_mode!=er.game_mode || ar.x_jitter_control!=er.x_jitter_control)
   throw std::runtime_error("Human original recovery context differs");
  return 0;
 }
 if(argc==5 && (std::string(argv[1])=="--compare-flight-contact-clock" || std::string(argv[1])=="--compare-flight-interception" || std::string(argv[1])=="--compare-flight-deflection")) {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary),pack_file(argv[4],std::ios::binary);
  auto source=load_phase_session(before_file),expected=load_phase_session(after_file);MatchContent content{};content.data=load_shot_content(pack_file);
  const auto count=source.current.field.selected_defenders;
  MatchRuntimeState state{};state.session=source;state.session.current.field.selected_defenders=0;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;
  for(unsigned i=0;i<count;++i)state.flight_defenders.push_back(source.scheduling->selected_ids[i]);
  state.defense_menu=MatchDefenseMenu{};auto& menu=*state.defense_menu;menu.stage=MatchDefenseStage::ai_flight_arrived;menu.defender_count=1;
  menu.ai_ball_cell=source.current.field.ball_cell;menu.ai_trajectory=BallTrajectoryContext{static_cast<std::uint16_t>(unsigned(source.current.field.ball_x)<<8),static_cast<std::uint16_t>(unsigned(source.current.field.ball_y)<<8),0,0,0,1};
  menu.ai_pass_target=MatchAiPassTarget{20,*menu.ai_ball_cell};menu.ai_pass_distance=1;menu.attack_cost=1;
  MatchStepResult result{};
  for(unsigned i=0;i<count;++i){result=step_match(state,{MatchInputKind::presentation_complete,i+1},content);if(result.unsupported)throw std::runtime_error("Opponent flight contact unsupported");state=result.state;}
  if(!result.state.flight_defenders.empty())throw std::runtime_error("Opponent flight contact prefix was not cleared");
  if(std::string(argv[1])!="--compare-flight-contact-clock" && (result.state.session.phase!=FieldSessionPhase::player_control || result.state.defense_menu))throw std::runtime_error("Defender outcome did not return player control");
  if(std::string(argv[1])=="--compare-flight-interception")for(unsigned i=0;i<expected.current.field.selected_defenders;++i)
   if(result.state.session.scheduling->selected_ids[i]!=expected.scheduling->selected_ids[i])throw std::runtime_error("Original intercepted defender prefix differs");
  std::ostringstream actual_field(std::ios::binary),expected_field(std::ios::binary);
  write_field_state(actual_field,result.state.session.current.field);write_field_state(expected_field,expected.current.field);
  if(actual_field.str()!=expected_field.str()) {for(std::size_t i=0;i<actual_field.str().size();++i)if(actual_field.str()[i]!=expected_field.str()[i])throw std::runtime_error("Original field differs at byte "+std::to_string(i));}
  const auto& a=result.state.session.current;const auto& e=expected.current;
  if(a.clock.remaining!=e.clock.remaining || a.clock.extra_time!=e.clock.extra_time || a.clock.flags!=e.clock.flags || a.clock.script_flags!=e.clock.script_flags || a.clock.ball_mode!=e.clock.ball_mode || a.shared_recovery_flags!=e.shared_recovery_flags || a.shared_recovery_timer!=e.shared_recovery_timer || result.state.session.action->power!=expected.action->power || result.state.session.experience->totals!=expected.experience->totals || result.state.session.recovery->opposing_player!=expected.recovery->opposing_player)
   throw std::runtime_error("Original clock, power, recovery or experience differs");
  const auto& ar=*result.state.session.recovery;const auto& er=*expected.recovery;
  if(ar.attack_subtype!=er.attack_subtype || ar.special_owner_flags!=er.special_owner_flags || ar.ball_variant!=er.ball_variant || ar.game_mode!=er.game_mode || ar.x_jitter_control!=er.x_jitter_control)throw std::runtime_error("Original recovery context differs");
  return 0;
 }
 if(argc==9 && std::string(argv[1])=="--compare-flight-contact") {
  std::array<unsigned char,2048> before{},prepared{},after{};
  std::ifstream a(argv[2],std::ios::binary),b(argv[3],std::ios::binary),c(argv[4],std::ios::binary),p(argv[5],std::ios::binary),rules(argv[8]);
  if(!a.read(reinterpret_cast<char*>(before.data()),before.size()) || !b.read(reinterpret_cast<char*>(prepared.data()),prepared.size()) || !c.read(reinterpret_cast<char*>(after.data()),after.size()))throw std::runtime_error("Missing original contact RAM");
  auto content=load_shot_content(p);std::string tag;unsigned range,offset,eligible,t0,t1,t2,t3;
  if(!(rules>>tag>>range>>offset>>eligible>>t0>>t1>>t2>>t3) || tag!="CMJPASS1")throw std::runtime_error("Missing pass comparison content");
  const auto id=before[0x601];const auto at=0x300+12*id;const FieldPlayerRecord player{id,before[at],before[at+1],before[at+2],before[at+3]};
  const auto prep=prepare_profiled_field_defender(player,content.field_players,2,0,0,content.defense);
  if(prep.base_strength!=(prepared[0x32]|(unsigned(prepared[0x33])<<8)) || prep.stamina_cost!=(prepared[0x43f]|(unsigned(prepared[0x440])<<8)) || prep.move_effect!=prepared[0x444])throw std::runtime_error("Flight defender preparation differs");
  auto contact=compare_defender_contact({static_cast<std::uint16_t>(before[0x61c]|(unsigned(before[0x61d])<<8)),prep.base_strength,before[0xe2],before[0xe3],static_cast<std::uint8_t>(range),static_cast<std::uint8_t>(offset),false,bool(eligible&128),0,id,0,{static_cast<std::uint8_t>(t0),static_cast<std::uint8_t>(t1),static_cast<std::uint8_t>(t2),static_cast<std::uint8_t>(t3)}});
  if(contact.strength!=after[0x71] || contact.loss!=after[0x619] || contact.score!=std::stoul(argv[6]) || contact.result_code!=std::stoul(argv[7]) || contact.subtype!=after[0x43e])throw std::runtime_error("Flight defender comparison differs");
  return 0;
 }
 if(argc==4 && std::string(argv[1])=="--compare-flight-prefix") {
  std::ifstream before_file(argv[2],std::ios::binary),after_file(argv[3],std::ios::binary);std::array<unsigned char,2048> before{},after{};
  if(!before_file.read(reinterpret_cast<char*>(before.data()),before.size()) || !after_file.read(reinterpret_cast<char*>(after.data()),after.size()))throw std::runtime_error("Missing original callback RAM");
  FlightDefenderSelectionInput selection{};selection.possession_side=before[0x5fb];selection.ball_cell=before[0x5fe];selection.random_first=before[0xe2];selection.random_second=before[0xe3];selection.threshold=before[0x61a];
  for(unsigned id=1;id<=21;++id)if(id!=11){const unsigned record=0x300+12*id;selection.players[field_motion_index(static_cast<std::uint8_t>(id))]={before[record+6],before[record+8],before[record+10]!=0};}
  std::vector<std::uint8_t> prior(before.begin()+0x601,before.begin()+0x601+before[0x600]);const auto selected=accumulate_flight_defenders(selection,prior);
  if(selected.size()!=after[0x600])throw std::runtime_error("Original flight defender count differs");
  for(unsigned i=0;i<selected.size();++i)if(selected[i]!=after[0x601+i])throw std::runtime_error("Original flight defender prefix differs");
  return 0;
 }
 if(argc==8 && std::string(argv[1])=="--compare-multi-chooser") {
  std::ifstream source_file(argv[2],std::ios::binary),before_file(argv[3],std::ios::binary),after_file(argv[4],std::ios::binary),expected_file(argv[5],std::ios::binary);
  auto source=load_phase_session(source_file);auto expected=load_phase_session(expected_file);std::array<unsigned char,2048> before{},after{};
  if(!before_file.read(reinterpret_cast<char*>(before.data()),before.size()) || !after_file.read(reinterpret_cast<char*>(after.data()),after.size()))throw std::runtime_error("Missing original chooser RAM");
  auto target=begin_ordinary_human_pass_target(source);target.human_pass_cursor=before[0x624];
  auto& candidates=*target.ordinary_pass_candidates;candidates.scanned=true;candidates.candidates.count=before[0x430];candidates.candidates.allied_count=before[0x430];
  if(candidates.candidates.count<2 || candidates.candidates.count>4)throw std::runtime_error("Invalid original chooser count");
  for(unsigned i=0;i<candidates.candidates.count;++i)candidates.candidates.ids[i]=before[0x431+i];
  MatchRuntimeState state{};state.session=target;state.mode=MatchTimingMode::provisional_demo;state.demo_rng=1;state.target_chooser=MatchTargetChooser{before[0x625]};
  MatchContent content{};content.data=ShotContent{};MatchInput input{MatchInputKind::tick,1};input.pressed=static_cast<std::uint8_t>(std::stoul(argv[6]));
  auto result=step_match(state,input,content);const std::string boundary(argv[7]);
  if(result.unsupported)throw std::runtime_error("Runtime multi-recipient chooser unsupported");
  if(boundary=="poll"){
   if(!result.state.target_chooser || result.state.target_chooser->index!=after[0x625] || result.state.session.ordinary_pass_candidates->candidates.ids[after[0x625]]!=after[0x5fc])
    throw std::runtime_error("Runtime chooser cursor differs from original");
  }else if(boundary=="cancel"){
   if(result.state.target_chooser || result.state.session.phase!=FieldSessionPhase::ordinary_human_pass_target ||
      result.state.session.ordinary_pass_candidates->candidates.count!=after[0x430] || result.state.session.ordinary_pass_candidates->candidates.allied_count!=after[0x625])
    throw std::runtime_error("Runtime chooser cancel differs from original");
   for(unsigned i=0;i<after[0x430];++i)if(result.state.session.ordinary_pass_candidates->candidates.ids[i]!=after[0x431+i])throw std::runtime_error("Runtime rescan differs from original");
  }else if(boundary=="confirm"){
   if(result.state.target_chooser || result.state.session.phase!=FieldSessionPhase::ordinary_human_pass_confirmed ||
      result.state.session.ordinary_pass_recipient!=after[0x5fc] || *result.state.session.human_pass_cursor!=after[0x638])
    throw std::runtime_error("Runtime chooser recipient differs from original");
  }else throw std::invalid_argument("Unknown original chooser boundary");
  auto normalized=result.state.session;normalized.phase=FieldSessionPhase::attack_choice_pending;normalized.human_pass_cursor.reset();normalized.human_pass_steps=0;
  normalized.ordinary_pass_candidates.reset();normalized.ordinary_pass_recipient.reset();
  normalized.current.field.random_first=expected.current.field.random_first;normalized.current.field.random_second=expected.current.field.random_second;
  std::ostringstream actual(std::ios::binary),wanted(std::ios::binary);write_phase_session(actual,normalized);write_phase_session(wanted,expected);
  if(actual.str()!=wanted.str())throw std::runtime_error("Runtime chooser represented state differs from original");
  return 0;
 }
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
  if(result.unsupported || result.state.pass_stage!=MatchPassStage::flight_ready || result.state.flight_defenders!=selected)
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
  if(recipient==255)source.human_pass_cursor=original[0x638];else {source.ordinary_pass_candidates->candidates.count=1;source.ordinary_pass_candidates->candidates.allied_count=1;source.ordinary_pass_candidates->candidates.ids[0]=recipient;}
  source.ordinary_pass_recipient=recipient;
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

struct DeferredDefendedTest { captain::MatchRuntimeState state;captain::MatchContent content;std::uint64_t seq;captain::MatchContentIdentity identity; };
static std::unique_ptr<DeferredDefendedTest> deferred_defended_test;
static std::unique_ptr<DeferredDefendedTest> deferred_human_defended_test;
static std::unique_ptr<DeferredDefendedTest> deferred_free_pass_test;
static std::unique_ptr<DeferredDefendedTest> deferred_human_shot_test;
static std::unique_ptr<DeferredDefendedTest> deferred_retained_control_test;
static std::unique_ptr<DeferredDefendedTest> deferred_pass_prepared_test;
static std::unique_ptr<DeferredDefendedTest> deferred_unopposed_ai_pass_test;
static void test_opponent_shot_gate(const captain::MatchRuntimeState& base,const captain::MatchContent& original,const captain::MatchContentIdentity& identity) {
 using namespace captain;auto state=std::make_unique<MatchRuntimeState>(base);auto content=std::make_unique<MatchContent>(original);auto result=std::make_unique<MatchStepResult>();
 state->session.phase=FieldSessionPhase::ai_cell_decision;state->session.current.field.selected_defenders=0;state->session.scheduling->selected_ids.fill(0);state->session.recovery->ball_variant=0;
 state->session.current.field.roster[state->session.current.field.ball_owner].stamina=100;state->defense_menu.reset();state->ball_fraction=MatchBallFraction{5,9};state->retained_defense_action=6;
 content->data->attack.skill_selectors[0][0]=1;content->data->attack.moves[0].push_back({2,0,20,0});content->data->attack.strength_curve.fill(20);
 auto& ai=*content->data->ai_field_choice;ai.rows[0].fill(0x11);ai.no_variant_zero_shot_options.reset();
 const auto saved=[&](const MatchRuntimeState& s){std::ostringstream out(std::ios::binary);write_match_save(out,s,identity);return out.str();};
 const auto before=saved(*state);*result=step_match(*state,client_input(*state,0,0),*content);assert(result->unsupported==UnsupportedCapability::opponent_action && saved(result->state)==before);
 ai.no_variant_zero_shot_options=std::array<std::uint8_t,118>{};*result=step_match(*state,client_input(*state,0,0),*content);assert(result->unsupported==UnsupportedCapability::opponent_action && saved(result->state)==before);
 (*ai.no_variant_zero_shot_options)[23]=1;*result=step_match(*state,client_input(*state,0,0),*content);
 assert(!result->unsupported && result->state.defense_menu && result->state.defense_menu->stage==MatchDefenseStage::ai_chosen && result->state.session.action->action==0 && result->state.retained_defense_action==6);
 const auto choice=saved(result->state);std::istringstream in(choice,std::ios::binary);auto restored=std::make_unique<MatchRuntimeState>(load_match_save(in,identity));
 *result=step_match(*restored,client_input(*restored,0,0),*content);assert(!result->unsupported && result->state.session.phase==FieldSessionPhase::attack_prepared && !result->state.defense_menu);
}
static void test_unopposed_ai_pass(const captain::MatchRuntimeState& base,const captain::MatchContent& original,const captain::MatchContentIdentity& identity) {
 using namespace captain;
 auto state=std::make_unique<MatchRuntimeState>(base);auto content=std::make_unique<MatchContent>(original);auto result=std::make_unique<MatchStepResult>();
 state->session.phase=FieldSessionPhase::field;state->session.current.field.selected_defenders=0;state->session.scheduling->selected_ids.fill(0);
 state->session.kickoff_formation=KickoffFormationContext{{124,124}};state->session.recovery->ball_variant=0;
 state->defense_menu->defender_count=0;state->defense_menu->poll=DefenseMenuPollState{};state->ball_fraction=MatchBallFraction{5,9};state->retained_defense_action=3;
 const auto bytes=[&](const MatchRuntimeState& s){std::ostringstream out(std::ios::binary);write_match_save(out,s,identity);return out.str();};
 const auto reject_save=[&](const MatchRuntimeState& s){bool rejected=false;try{bytes(s);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);};
 const auto checkpoint=[&](){const auto saved=bytes(*state);std::istringstream in(saved,std::ios::binary);*state=load_match_save(in,identity);};
 const auto atomic=[&](const MatchContent& c,UnsupportedCapability why){const auto before=bytes(*state);*result=step_match(*state,client_input(*state,0,0),c);assert(result->unsupported==why && bytes(result->state)==before);};
 const auto next=[&](){*result=step_match(*state,client_input(*state,0,0),*content);assert(!result->unsupported);*state=result->state;checkpoint();};
 checkpoint();auto invalid=std::make_unique<MatchRuntimeState>(*state);invalid->session.action->action=2;reject_save(*invalid);
 *invalid=*state;invalid->defense_menu->stage=MatchDefenseStage::polling;reject_save(*invalid);
 *invalid=*state;invalid->session.current.field.selected_defenders=1;reject_save(*invalid);
 auto missing=std::make_unique<MatchContent>(*content);missing->data->attack.moves[1].clear();atomic(*missing,UnsupportedCapability::content);
 next();assert(state->defense_menu->stage==MatchDefenseStage::attack_prepared);next();assert(state->defense_menu->stage==MatchDefenseStage::clock_pending);
 content->data->area_prefix_elapsed.reset();const auto elapsed=state->session.current.field.elapsed;const auto remaining=state->session.current.clock.remaining;
 next();assert(state->defense_menu->stage==MatchDefenseStage::dispatch_pending && state->session.current.field.elapsed==elapsed && state->session.current.clock.remaining==remaining);
 *missing=*content;missing->data->movement.reset();atomic(*missing,UnsupportedCapability::content);
 next();assert(state->defense_menu->stage==MatchDefenseStage::ai_pass_handoff && state->session.control->last_carrier_cell==128);
 *missing=*content;missing->pass_distance.reset();atomic(*missing,UnsupportedCapability::content);
 next();assert(state->defense_menu->stage==MatchDefenseStage::ai_pass_target_ready);
 auto unknown=std::make_unique<MatchRuntimeState>(*state);unknown->ball_fraction.reset();const auto before=bytes(*unknown);
 *result=step_match(*unknown,client_input(*unknown,0,0),*content);assert(result->unsupported==UnsupportedCapability::ball_fraction && bytes(result->state)==before);
}
static void test_pass_field_handoff(const captain::MatchRuntimeState& input,const captain::MatchContent& original) {
 using namespace captain;
 auto state=std::make_unique<MatchRuntimeState>(input);auto content=std::make_unique<MatchContent>(original);
 state->session.current.field.elapsed=1;state->session.current.clock.remaining=200;state->ball_fraction=MatchBallFraction{5,9};
 content->data->movement->geometry.direction_thresholds.fill(65535);
 for(unsigned id=1;id<=21;++id)if(id!=11 && id!=state->session.current.field.ball_owner){auto& player=state->session.current.field.motion[field_motion_index(static_cast<std::uint8_t>(id))];player.x=100u<<8;player.y=100u<<8;state->session.current.field.roster[id].type=1;}
 auto missing=std::make_unique<MatchContent>(*content);missing->data->movement.reset();
 auto result=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::presentation_complete,state->last_sequence+1},*missing));
 assert(result->unsupported==UnsupportedCapability::content && result->state.last_sequence==state->last_sequence && result->state.session.current.field.elapsed==1 && !result->state.session.recovery->ball_variant);
 auto period=std::make_unique<MatchRuntimeState>(*state);period->session.current.clock.remaining=0;period->session.current.clock.extra_time=128;
 *result=step_match(*period,{MatchInputKind::presentation_complete,period->last_sequence+1},*content);
 assert(result->unsupported==UnsupportedCapability::period_transition && result->state.last_sequence==period->last_sequence && result->state.session.current.field.elapsed==1);
 *result=step_match(*state,{MatchInputKind::presentation_complete,state->last_sequence+1},*content);
 if(result->unsupported)throw std::runtime_error(std::string("Synthetic pass handoff: ")+capability_name(*result->unsupported));
 assert(!result->unsupported && result->state.pass_stage==MatchPassStage::handoff && result->state.session.current.field.elapsed==0 && result->state.ball_fraction && result->state.ball_fraction->x==5 && result->state.ball_fraction->y==9);
}
static void test_control_motion_fraction(const captain::MatchRuntimeState& input,const captain::MatchContent& original) {
 using namespace captain;
 auto state=std::make_unique<MatchRuntimeState>(input);auto content=std::make_unique<MatchContent>(original);
 state->session.control->decision_timer=0;state->ball_fraction=MatchBallFraction{5,9};
 content->data->movement->profiles.speeds.resize(1);content->data->control_movement->carrier_speeds[0]=257;
 MatchInput move{MatchInputKind::tick,state->last_sequence+1};move.held=1;
 auto result=std::make_unique<MatchStepResult>(step_match(*state,move,*content));
 assert(!result->unsupported && result->state.session.current.field.ball_x==state->session.current.field.ball_x+1 && result->state.ball_fraction && result->state.ball_fraction->x==5 && result->state.ball_fraction->y==9);
 state->ball_fraction.reset();*result=step_match(*state,move,*content);assert(!result->unsupported && !result->state.ball_fraction);
}
static void test_retained_action_control(const captain::MatchRuntimeState& base,const captain::MatchContent& original,const captain::MatchContentIdentity& identity) {
 using namespace captain;
 auto state=std::make_unique<MatchRuntimeState>(base);auto content=std::make_unique<MatchContent>(original);state->retained_defense_action=3;
 MatchConfig initial{};initial.initial=state->session;initial.mode=MatchTimingMode::provisional_demo;initial.seed=1;initial.retained_defense_action=3;
 auto created=std::make_unique<MatchStepResult>(create_match(initial,*content));assert(!created->unsupported && created->state.retained_defense_action==3);
 {
  auto interactive=std::make_unique<MatchRuntimeState>(created->state);auto step=std::make_unique<MatchStepResult>();
  content->data->action_menu->directions[0][0]=0;
  const auto advance=[&](std::uint8_t held,std::uint8_t pressed){*step=step_match(*interactive,client_input(*interactive,held,pressed),*content);assert(!step->unsupported);*interactive=step->state;};
  advance(64,64);assert(interactive->session.phase==FieldSessionPhase::action_menu_pending);
  advance(0,0);assert(interactive->session.phase==FieldSessionPhase::human_action_menu);
  advance(1,1);advance(128,128);assert(interactive->session.phase==FieldSessionPhase::attack_choice_pending && interactive->retained_defense_action==3);
  content->data->field_players.skills.resize(1);content->data->field_players.skills[0].fill(20);content->data->attack.skill_selectors[0][0]=1;content->data->attack.moves[0].push_back({2,0,20,0});content->data->attack.strength_curve.fill(20);
  advance(0,0);assert(interactive->session.phase==FieldSessionPhase::attack_prepared && interactive->retained_defense_action==3);
 }
 state->session.current.clock.remaining=200;content->data->world_step=WorldStepContent{};content->data->movement->profiles.speeds.resize(1);
 state->ball_fraction=MatchBallFraction{5,9};
 state->session.current.field.roster[0].type=1;state->session.current.field.roster[11].type=1;content->data->goalkeepers.skills.resize(1);
 for(unsigned id=1;id<=21;++id)if(id!=11 && id!=state->session.current.field.ball_owner){state->session.current.field.roster[id].type=1;auto& player=state->session.current.field.motion[field_motion_index(static_cast<std::uint8_t>(id))];player.x=(id<11 ? 52u : 200u)<<8;player.y=(id<11 ? 84u : 160u)<<8;}
 auto result=std::make_unique<MatchStepResult>();
 for(unsigned turn=0;turn<12 && state->session.phase==FieldSessionPhase::control_ready;++turn){
  *result=step_match(*state,{MatchInputKind::tick,state->last_sequence+1},*content);assert(!result->unsupported && result->state.retained_defense_action==3);*state=result->state;
 }
 assert(state->session.phase==FieldSessionPhase::control_clock_pending);
 std::ostringstream saved(std::ios::binary);write_match_save(saved,*state,identity);std::istringstream in(saved.str(),std::ios::binary);*state=load_match_save(in,identity);
 *result=step_match(*state,{MatchInputKind::tick,state->last_sequence+1},*content);assert(!result->unsupported && result->state.retained_defense_action==3 && result->state.session.phase==FieldSessionPhase::field_round_pending);
 *state=result->state;*result=step_match(*state,{MatchInputKind::tick,state->last_sequence+1},*content);assert(!result->unsupported);
 if(result->state.session.phase==FieldSessionPhase::control_ready)assert(result->state.retained_defense_action==3 && result->state.ball_fraction && result->state.ball_fraction->x==5 && result->state.ball_fraction->y==9);
 else assert(!result->state.retained_defense_action && !result->state.ball_fraction);
}
static void test_retained_action_invalidation(const captain::MatchRuntimeState& base,const captain::MatchContent& content,const captain::MatchContentIdentity& identity) {
 using namespace captain;
 auto state=std::make_unique<MatchRuntimeState>(base);state->retained_defense_action=3;
 state->session.current.field.ball_x=120;state->ball_fraction=MatchBallFraction{5,9};
 std::ostringstream saved(std::ios::binary);write_match_save(saved,*state,identity);std::istringstream in(saved.str(),std::ios::binary);*state=load_match_save(in,identity);
 assert(state->retained_defense_action && *state->retained_defense_action==3);
 MatchInput pass{MatchInputKind::action,state->last_sequence+1};pass.action=MatchAction::pass;
 auto result=std::make_unique<MatchStepResult>(step_match(*state,pass,content));
 assert(!result->unsupported && result->state.session.phase==FieldSessionPhase::ordinary_human_pass_target && result->state.retained_defense_action==3 && result->state.ball_fraction && result->state.ball_fraction->x==5 && result->state.ball_fraction->y==9);
 {std::ostringstream out(std::ios::binary);write_match_save(out,result->state,identity);std::istringstream input(out.str(),std::ios::binary);*state=load_match_save(input,identity);}
 MatchInput cancel{MatchInputKind::tick,state->last_sequence+1};cancel.pressed=64;
 *result=step_match(*state,cancel,content);assert(!result->unsupported && !result->state.retained_defense_action);
 MatchInput diagonal{MatchInputKind::tick,state->last_sequence+1};diagonal.held=3;diagonal.pressed=3;
 *result=step_match(*state,diagonal,content);assert(!result->unsupported && !result->state.retained_defense_action);
 auto blocked=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::tick,state->last_sequence},content));
 assert(blocked->unsupported==UnsupportedCapability::input_sequence && blocked->state.retained_defense_action==state->retained_defense_action);
}
static void test_goal_kickoff(const captain::MatchRuntimeState& base,const captain::MatchContent& original,const captain::MatchContentIdentity& identity) {
 using namespace captain;
 auto state=std::make_unique<MatchRuntimeState>(base);auto content=std::make_unique<MatchContent>(original);
 state->session.phase=FieldSessionPhase::goal_kickoff_pending;state->session.current.field.side=11;state->session.current.field.elapsed=0;
 state->session.action->action=0;
 state->session.current.field.selected_defenders=0;state->session.current.clock.ball_mode=0;state->session.score=MatchScoreContext{1,0,4};state->session.recovery->opposing_player=11;
 state->session.current.field.keepers[1]={};state->session.current.field.keepers[0].penalty=5;state->session.current.field.keepers[0].stun=3;
 state->session.current.field.motion[0].status=5;state->ball_fraction=MatchBallFraction{3,7};
 auto result=std::make_unique<MatchStepResult>(step_match(*state,{MatchInputKind::tick,state->last_sequence+1},*content));
 assert(result->unsupported==UnsupportedCapability::content && result->state.session.current.field.motion[0].status==5 && result->state.ball_fraction);
 auto kickoff=std::make_shared<RuntimeKickoffContent>();kickoff->formation.on_ball.fill(87);kickoff->formation.off_ball.fill(87);kickoff->receivers={{5,6,7,9,10}};content->data->kickoff=kickoff;
 {auto unknown=std::make_unique<MatchRuntimeState>(*state);unknown->ball_fraction.reset();auto unknown_result=std::make_unique<MatchStepResult>(step_match(*unknown,{MatchInputKind::tick,unknown->last_sequence+1},*content));assert(!unknown_result->unsupported && !unknown_result->state.ball_fraction);}
 *result=step_match(*state,{MatchInputKind::tick,state->last_sequence+1},*content);
 if(result->unsupported)throw std::runtime_error(std::string("Synthetic kickoff: ")+capability_name(*result->unsupported));
 assert(!result->unsupported && result->state.session.phase==FieldSessionPhase::kickoff_formation_placed && !result->state.session.current.field.motion[0].status && !result->state.session.current.field.keepers[0].penalty && result->state.ball_fraction && result->state.ball_fraction->x==3 && result->state.ball_fraction->y==7);
 *state=result->state;*result=step_match(*state,{MatchInputKind::tick,state->last_sequence+1},*content);assert(!result->unsupported && result->state.session.phase==FieldSessionPhase::kickoff_receiver_selected);
 std::ostringstream saved(std::ios::binary);write_match_save(saved,result->state,identity);std::istringstream in(saved.str(),std::ios::binary);*state=load_match_save(in,identity);
 *result=step_match(*state,{MatchInputKind::tick,state->last_sequence+1},*content);assert(!result->unsupported && result->state.session.phase==FieldSessionPhase::kickoff_ball_placed);
 *state=result->state;*result=step_match(*state,{MatchInputKind::tick,state->last_sequence+1},*content);assert(result->unsupported==UnsupportedCapability::presentation && result->state.last_sequence==state->last_sequence);
 *result=step_match(*state,{MatchInputKind::presentation_complete,state->last_sequence+1},*content);assert(!result->unsupported && result->state.session.phase==FieldSessionPhase::kickoff_control_pending);
 *state=result->state;*result=step_match(*state,{MatchInputKind::tick,state->last_sequence+1},*content);assert(!result->unsupported && result->state.session.phase==FieldSessionPhase::control_ready && !result->events.empty() && result->state.ball_fraction && result->state.ball_fraction->x==3 && result->state.ball_fraction->y==7);
}
static void test_human_shot(const captain::MatchRuntimeState& base,const captain::MatchContent& original,const captain::MatchContentIdentity& identity) {
 using namespace captain;
 auto content=std::make_unique<MatchContent>(original);auto state=std::make_unique<MatchRuntimeState>(base);state->session.action->action=0;state->session.current.clock.remaining=200;
 MatchConfig config{};config.initial=state->session;config.seed=1;config.mode=MatchTimingMode::provisional_demo;config.retained_defense_action=2;
 auto started=std::make_unique<MatchStepResult>(create_match(config,*content));assert(!started->unsupported && started->state.retained_defense_action==config.retained_defense_action);
 config.retained_defense_action=7;*started=create_match(config,*content);assert(started->unsupported==UnsupportedCapability::initialization);
 config.retained_defense_action=2;config.initial->phase=FieldSessionPhase::player_control;*started=create_match(config,*content);assert(started->unsupported==UnsupportedCapability::initialization);
 for(unsigned id=1;id<=21;++id)if(id!=11){state->session.current.field.roster[id].type=1;auto& player=state->session.current.field.motion[field_motion_index(static_cast<std::uint8_t>(id))];player.x=100u<<8;player.y=124u<<8;}
 content->data->attack.skill_selectors[0][0]=1;content->data->attack.moves[0].push_back({2,0,20,0});
 content->data->experience=ExperienceContent{};content->data->movement->profiles.speeds.resize(1);
 MatchInput shot{MatchInputKind::action,state->last_sequence+1};shot.action=MatchAction::shot;
 const auto owner=state->session.current.field.ball_owner;const auto stamina=state->session.current.field.roster[owner].stamina;
 state->session.current.field.roster[owner].stamina=0;auto result=std::make_unique<MatchStepResult>(step_match(*state,shot,*content));
 assert(result->unsupported==UnsupportedCapability::action && result->state.last_sequence==state->last_sequence && result->state.session.current.field.roster[owner].stamina==0);
 state->session.current.field.roster[owner].stamina=stamina;*result=step_match(*state,shot,*content);assert(!result->unsupported && result->state.session.phase==FieldSessionPhase::attack_prepared);
 *state=result->state;*result=step_match(*state,shot,*content);assert(result->unsupported==UnsupportedCapability::input_sequence && result->state.session.current.field.roster[owner].stamina==state->session.current.field.roster[owner].stamina);
 *result=step_match(*state,{MatchInputKind::tick,state->last_sequence+1},*content);assert(result->unsupported==UnsupportedCapability::presentation);
 std::ostringstream save(std::ios::binary);write_match_save(save,*state,identity);std::istringstream in(save.str(),std::ios::binary);*state=load_match_save(in,identity);
 *result=step_match(*state,{MatchInputKind::presentation_complete,state->last_sequence+1},*content);
 assert(!result->unsupported && result->state.session.phase==FieldSessionPhase::shot_dispatch_ready);
 *state=result->state;*result=step_match(*state,{MatchInputKind::tick,state->last_sequence+1},*content);
 assert(!result->unsupported && result->state.session.phase==FieldSessionPhase::selected_attack_presentation);
 *state=result->state;*result=step_match(*state,{MatchInputKind::presentation_complete,state->last_sequence+1},*content);
 assert(result->unsupported==UnsupportedCapability::defense_action_state && result->state.last_sequence==state->last_sequence);
 {std::ostringstream current(std::ios::binary);write_match_save(current,*state,identity);auto old=current.str();assert(old.back()==0);old.pop_back();
  auto old14=old;old14[8]=14;const auto length14=static_cast<std::uint32_t>(old14.size()-17);std::uint32_t crc14=0xffffffffu;
  for(std::size_t i=17;i<old14.size();++i){crc14^=static_cast<unsigned char>(old14[i]);for(unsigned j=0;j<8;++j)crc14=(crc14>>1)^(0xedb88320u&static_cast<std::uint32_t>(-static_cast<std::int32_t>(crc14&1)));}
  crc14=~crc14;for(unsigned i=0;i<4;++i){old14[9+i]=static_cast<char>(length14>>(8*i));old14[13+i]=static_cast<char>(crc14>>(8*i));}
  std::istringstream v14_input(old14,std::ios::binary);assert(!load_match_save(v14_input,identity).home_keeper_y);
  assert(old.back()==0);old.pop_back();old[8]=13;
  const auto length=static_cast<std::uint32_t>(old.size()-17);std::uint32_t crc=0xffffffffu;
  for(std::size_t i=17;i<old.size();++i){crc^=static_cast<unsigned char>(old[i]);for(unsigned j=0;j<8;++j)crc=(crc>>1)^(0xedb88320u&static_cast<std::uint32_t>(-static_cast<std::int32_t>(crc&1)));}
  crc=~crc;for(unsigned i=0;i<4;++i){old[9+i]=static_cast<char>(length>>(8*i));old[13+i]=static_cast<char>(crc>>(8*i));}
  std::istringstream old_input(old,std::ios::binary);auto old_state=load_match_save(old_input,identity);assert(!old_state.retained_defense_action);
 }
 state->retained_defense_action=2;content->data->contact=ContactPreludeContent{};
 std::ostringstream retained_save(std::ios::binary);write_match_save(retained_save,*state,identity);std::istringstream retained_in(retained_save.str(),std::ios::binary);*state=load_match_save(retained_in,identity);
 assert(state->retained_defense_action && *state->retained_defense_action==2);
 *result=step_match(*state,{MatchInputKind::presentation_complete,state->last_sequence+1},*content);
 assert(!result->unsupported && result->state.session.phase==FieldSessionPhase::shot_approach_ready && !result->state.retained_defense_action);
}
static void test_free_pass(const captain::MatchRuntimeState& base,const captain::MatchContent& original,std::uint64_t seq,const captain::MatchContentIdentity& identity) {
 using namespace captain;
 auto content=original;auto state=base;state.ball_fraction=MatchBallFraction{5,9};state.retained_defense_action=3;
 auto target=step_match(state,{MatchInputKind::tick,seq},content);
 assert(!target.unsupported && target.state.pass_target && target.state.pass_target->recipient==255 && target.state.retained_defense_action==3);
 std::ostringstream bytes(std::ios::binary);write_match_save(bytes,target.state,identity);std::istringstream saved(bytes.str(),std::ios::binary);state=load_match_save(saved,identity);
 auto next=step_match(state,{MatchInputKind::tick,seq+1},content);assert(!next.unsupported);state=next.state;
 for(std::uint64_t turn=seq+2;state.pass_stage!=MatchPassStage::flight_arrived && turn<seq+150;++turn){next=step_match(state,{MatchInputKind::tick,turn},content);assert(!next.unsupported);state=next.state;}
 assert(state.pass_stage==MatchPassStage::flight_arrived);
 {
  auto contested=std::make_unique<MatchRuntimeState>(state);contested->pass_stage=MatchPassStage::flight_cell;
  contested->pass_cell=field_cell_at(100,100);contested->session.current.field.random_first=1;contested->session.current.field.random_second=0;
  auto& defender=contested->session.current.field.motion[field_motion_index(12)];defender.x=100u<<8;defender.y=100u<<8;defender.status=0;
  auto contact=std::make_unique<MatchStepResult>(step_match(*contested,{MatchInputKind::tick,contested->last_sequence+1},content));
  assert(!contact->unsupported && !contact->state.flight_defenders.empty() && !contact->state.retained_defense_action);
 }
 auto missing=step_match(state,{MatchInputKind::presentation_complete,state.last_sequence+1},content);assert(missing.unsupported==UnsupportedCapability::content);
 auto race=std::make_shared<ClearanceRaceContent>();content.data->pass_race=race;
 content.data->movement->profiles.speeds.resize(1);for(unsigned id=1;id<=21;++id)if(id!=11)state.session.current.field.roster[id].type=1;
 for(unsigned id=1;id<=21;++id)if(id!=11 && id!=state.session.current.field.ball_owner){auto& p=state.session.current.field.motion[field_motion_index(static_cast<std::uint8_t>(id))];p.x=52u<<8;p.y=84u<<8;p.status=0;}
 auto timeout=step_match(state,{MatchInputKind::presentation_complete,state.last_sequence+1},content);
 assert(!timeout.unsupported && timeout.state.session.phase==FieldSessionPhase::player_control && !timeout.state.retained_defense_action);
 auto timeout_control=step_match(timeout.state,{MatchInputKind::tick,timeout.state.last_sequence+1},content);
 assert(!timeout_control.unsupported && timeout_control.state.session.phase==FieldSessionPhase::control_ready);
 auto long_race=state;long_race.pass_target->distance=5;timeout=step_match(long_race,{MatchInputKind::presentation_complete,long_race.last_sequence+1},content);
 assert(!timeout.unsupported && timeout.state.session.phase==FieldSessionPhase::player_control);
 long_race.pass_target->distance=6;auto unresolved=step_match(long_race,{MatchInputKind::presentation_complete,long_race.last_sequence+1},content);
 assert(unresolved.unsupported==UnsupportedCapability::action && unresolved.state.last_sequence==long_race.last_sequence);
 auto opposing=state;auto& away=opposing.session.current.field.motion[field_motion_index(12)];away.x=opposing.pass_trajectory->x;away.y=opposing.pass_trajectory->y;
 auto opposing_winner=step_match(opposing,{MatchInputKind::presentation_complete,opposing.last_sequence+1},content);
 assert(!opposing_winner.unsupported && opposing_winner.state.session.phase==FieldSessionPhase::player_control && opposing_winner.state.session.current.field.side==11 && opposing_winner.state.session.current.field.ball_owner==12 && !opposing_winner.state.retained_defense_action);
 auto opposing_control=step_match(opposing_winner.state,{MatchInputKind::tick,opposing_winner.state.last_sequence+1},content);
 assert(!opposing_control.unsupported && opposing_control.state.session.phase==FieldSessionPhase::control_ready);
 auto& winner=state.session.current.field.motion[field_motion_index(1)];winner.x=state.pass_trajectory->x;winner.y=state.pass_trajectory->y;winner.status=0;
 auto result=step_match(state,{MatchInputKind::presentation_complete,state.last_sequence+1},content);
 assert(!result.unsupported && result.state.session.phase==FieldSessionPhase::player_control && result.state.session.current.field.ball_owner==1 && result.state.retained_defense_action==3);
 auto control=step_match(result.state,{MatchInputKind::tick,result.state.last_sequence+1},content);
 assert(!control.unsupported && control.state.session.phase==FieldSessionPhase::control_ready && control.state.retained_defense_action==3);
}
static void test_human_defended_arrival(const captain::MatchRuntimeState& base,const captain::MatchContent& original,std::uint64_t seq,const captain::MatchContentIdentity& identity) {
 using namespace captain;
 auto content=original;auto arrival=base;arrival.flight_defenders={12};arrival.session.action->power=30;
 arrival.session.current.clock.remaining=200;
 for(unsigned id=1;id<=21;++id)if(id!=11)arrival.session.current.field.roster[id].type=1;
 arrival.session.current.field.motion[field_motion_index(12)].x=100u<<8;arrival.session.current.field.motion[field_motion_index(12)].y=124u<<8;
 arrival.session.current.field.random_first=1;arrival.session.current.field.random_second=0;
 content.data->pass_flight_comparison=PassFlightComparisonContent{1,255,0,{154,96,68,0}};
 content.data->defense.skill_selectors[2][0]=1;content.data->defense.moves[2].push_back({0,0,50,0});content.data->defense.strength_curve.fill(20);
 content.data->contact=ContactPreludeContent{};content.data->contact->field_elapsed[4]=2;content.data->contact->field_elapsed[5]=1;content.data->contact->defender_status[2]=6;
 content.data->experience=ExperienceContent{};
 content.data->experience->stage_bases[0]=64;content.data->experience->reward_factors[1]=64;
 auto unweakened=arrival;unweakened.session.action->power=255;
 auto missed=step_match(unweakened,{MatchInputKind::presentation_complete,seq},content);
 assert(!missed.unsupported && missed.state.session.experience->totals[0]==arrival.session.experience->totals[0]+16);
 auto survived=step_match(arrival,{MatchInputKind::presentation_complete,seq},content);
 assert(!survived.unsupported && survived.state.pass_stage==MatchPassStage::flight_arrived && survived.state.flight_defenders.empty() && survived.state.session.action->power==26);
 std::ostringstream save(std::ios::binary);write_match_save(save,survived.state,identity);std::istringstream in(save.str(),std::ios::binary);auto restored=load_match_save(in,identity);
 auto received=step_match(restored,{MatchInputKind::presentation_complete,seq+1},content);
 assert(!received.unsupported && received.state.session.current.field.ball_owner==6);
 auto intercepted=arrival;intercepted.session.action->power=1;auto steal=step_match(intercepted,{MatchInputKind::presentation_complete,seq},content);
 assert(!steal.unsupported && steal.state.session.phase==FieldSessionPhase::player_control && steal.state.session.current.field.ball_owner==12 && steal.state.session.current.field.side==11);
 auto deflected=arrival;deflected.session.action->power=23;auto recovery=step_match(deflected,{MatchInputKind::presentation_complete,seq},content);
 assert(!recovery.unsupported && recovery.state.session.phase==FieldSessionPhase::player_control && recovery.state.flight_defenders.empty());
 auto special=arrival;special.session.action->power=4;special.session.current.field.random_first=0;special.session.current.field.random_second=255;
 auto special_content=content;special_content.data->pass_flight_comparison->eligibility=128;
 auto special_steal=step_match(special,{MatchInputKind::presentation_complete,seq},special_content);
 assert(!special_steal.unsupported && special_steal.state.session.phase==FieldSessionPhase::player_control && special_steal.state.session.current.field.ball_owner==12);
 auto later=arrival;later.flight_defenders={12,12};later.session.action->power=34;
 auto first=step_match(later,{MatchInputKind::presentation_complete,seq},content);
 assert(!first.unsupported && first.state.processed_flight_defenders==std::vector<std::uint8_t>({12}));
 later=first.state;later.session.action->power=1;auto later_steal=step_match(later,{MatchInputKind::presentation_complete,seq+1},content);
 assert(!later_steal.unsupported && later_steal.state.session.current.field.ball_owner==12 && later_steal.state.session.current.field.selected_defenders==2);
 later.session.action->power=23;auto later_deflection=step_match(later,{MatchInputKind::presentation_complete,seq+1},content);
 assert(!later_deflection.unsupported && later_deflection.state.session.phase==FieldSessionPhase::player_control);
}
static void test_defended_arrival(const captain::MatchRuntimeState& base,const captain::MatchContent& original,std::uint64_t seq,const captain::MatchContentIdentity& identity) {
 using namespace captain;
 static MatchContent content;static MatchRuntimeState arrival;content=original;arrival=base;arrival.flight_defenders={1};arrival.session.action->power=30;
 arrival.session.current.field.random_first=1;arrival.session.current.field.random_second=0;
 content.data->pass_flight_comparison=PassFlightComparisonContent{1,255,0,{154,96,68,0}};
 content.data->defense.skill_selectors[2][0]=1;content.data->defense.moves[2].push_back({0,0,50,0});content.data->defense.strength_curve.fill(20);
 content.data->contact=ContactPreludeContent{};content.data->contact->field_elapsed[4]=2;content.data->contact->field_elapsed[5]=1;content.data->contact->defender_status[2]=6;
 content.data->experience=ExperienceContent{};
 const auto checkpoint=[&](const MatchRuntimeState& state){std::ostringstream bytes(std::ios::binary);write_match_save(bytes,state,identity);std::istringstream in(bytes.str(),std::ios::binary);return load_match_save(in,identity);};
 static MatchStepResult survived,first,second,blocked,received,control;
 survived=step_match(arrival,{MatchInputKind::presentation_complete,seq},content);
 assert(!survived.unsupported && survived.state.flight_defenders.empty() && survived.state.session.action->power==26);
 assert(survived.state.session.current.field.motion[field_motion_index(1)].status==6);
 assert(checkpoint(survived.state).session.action->power==26);
 static MatchRuntimeState repeated,saved,stopped;repeated=arrival;repeated.flight_defenders={1,1};repeated.session.action->power=34;
 stopped=arrival;stopped.session.action->power=255;blocked=step_match(stopped,{MatchInputKind::presentation_complete,seq},content);
 assert(!blocked.unsupported && blocked.state.flight_defenders.empty() && blocked.state.session.action->power==255 && blocked.state.session.current.field.motion[field_motion_index(1)].status==6);
 first=step_match(repeated,{MatchInputKind::presentation_complete,seq},content);
 assert(!first.unsupported && first.state.flight_defenders.size()==1 && first.state.session.current.field.elapsed==1);
 assert(first.state.processed_flight_defenders==std::vector<std::uint8_t>({1}));
 {auto invalid=first.state;invalid.processed_flight_defenders={12};bool rejected=false;
  try{std::ostringstream out(std::ios::binary);write_match_save(out,invalid,identity);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);}
 {auto invalid=first.state;invalid.processed_flight_defenders={1,1,1,1};bool rejected=false;
  try{std::ostringstream out(std::ios::binary);write_match_save(out,invalid,identity);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);}
 {std::ostringstream current(std::ios::binary);write_match_save(current,first.state,identity);auto old=current.str();
  assert(static_cast<unsigned char>(old.back())==0);old.pop_back(); // Version15 keeper position is unknown.
  assert(static_cast<unsigned char>(old.back())==0);old.pop_back(); // Version14 retained action is unknown.
  assert(static_cast<unsigned char>(old.back())==1);old.pop_back();assert(static_cast<unsigned char>(old.back())==1);old.pop_back();old[8]=12;
  const auto length=static_cast<std::uint32_t>(old.size()-17);std::uint32_t crc=0xffffffffu;
  for(std::size_t i=17;i<old.size();++i){crc^=static_cast<unsigned char>(old[i]);for(unsigned j=0;j<8;++j)crc=(crc>>1)^(0xedb88320u&static_cast<std::uint32_t>(-static_cast<std::int32_t>(crc&1)));}
  crc=~crc;for(unsigned i=0;i<4;++i){old[9+i]=static_cast<char>(length>>(8*i));old[13+i]=static_cast<char>(crc>>(8*i));}
  std::istringstream old_input(old,std::ios::binary);auto legacy=load_match_save(old_input,identity);assert(legacy.processed_flight_defenders.empty());
  auto uncertain=step_match(legacy,{MatchInputKind::presentation_complete,seq+1},content);assert(uncertain.unsupported==UnsupportedCapability::defender_contact && uncertain.state.last_sequence==legacy.last_sequence);}
 stopped=first.state;stopped.session.action->power=1;blocked=step_match(stopped,{MatchInputKind::presentation_complete,seq+1},content);
 assert(!blocked.unsupported && blocked.state.session.phase==FieldSessionPhase::player_control && blocked.state.session.current.field.ball_owner==1 && blocked.state.session.current.field.selected_defenders==2);
 assert(blocked.state.session.scheduling->selected_ids[0]==1 && blocked.state.session.scheduling->selected_ids[1]==1);
 saved=checkpoint(first.state);second=step_match(saved,{MatchInputKind::presentation_complete,seq+1},content);
 assert(!second.unsupported && second.state.flight_defenders.empty() && second.state.session.current.field.elapsed==0 && second.state.session.action->power==26);
 stopped=arrival;stopped.session.action->power=1;blocked=step_match(stopped,{MatchInputKind::presentation_complete,seq},content);
 assert(!blocked.unsupported && blocked.state.session.phase==FieldSessionPhase::player_control && blocked.state.session.current.field.side==0 && blocked.state.session.current.field.ball_owner==1 && blocked.state.flight_defenders.empty());
 assert(checkpoint(blocked.state).session.current.field.ball_owner==1);
 auto intercepted_control=step_match(blocked.state,{MatchInputKind::tick,seq+1},content);
 assert(!intercepted_control.unsupported && intercepted_control.state.session.phase==FieldSessionPhase::control_ready);
 stopped=arrival;stopped.session.action->power=23;blocked=step_match(stopped,{MatchInputKind::presentation_complete,seq},content);
 assert(!blocked.unsupported && blocked.state.session.phase==FieldSessionPhase::player_control && blocked.state.flight_defenders.empty());
 assert(checkpoint(blocked.state).session.current.field.ball_owner==blocked.state.session.current.field.ball_owner);
 stopped=arrival;stopped.session.action->power=23;stopped.session.current.field.random_first=0;stopped.session.current.field.random_second=3;
 stopped.session.current.field.motion[field_motion_index(1)].x=200u<<8;
 blocked=step_match(stopped,{MatchInputKind::presentation_complete,seq},content);
 assert(blocked.unsupported==UnsupportedCapability::defender_contact && blocked.state.last_sequence==stopped.last_sequence && blocked.state.flight_defenders==stopped.flight_defenders);
 received=step_match(survived.state,{MatchInputKind::presentation_complete,seq+1},content);
 assert(!received.unsupported && received.state.session.current.field.ball_owner==20);
 control=step_match(received.state,{MatchInputKind::tick,seq+2},content);
 assert(!control.unsupported && control.state.session.phase==FieldSessionPhase::control_ready);
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
 content.data->movement->profiles.speeds.resize(1);
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
 deferred_pass_prepared_test=std::make_unique<DeferredDefendedTest>();deferred_pass_prepared_test->state=prepared.state;deferred_pass_prepared_test->content=content;
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
 static MatchStepResult free_target;
 free_target=step_match(handoff.state,{MatchInputKind::tick,11},content);
 assert(!free_target.unsupported && free_target.state.pass_target && free_target.state.pass_target->recipient==255 && free_target.state.pass_target->destination==*handoff.state.session.human_pass_cursor);
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
 deferred_human_shot_test=std::make_unique<DeferredDefendedTest>();deferred_human_shot_test->state=confirmed.state;
 deferred_human_shot_test->content=content;deferred_human_shot_test->identity=identity;
 deferred_retained_control_test=std::make_unique<DeferredDefendedTest>();deferred_retained_control_test->state=c1.state;
 deferred_retained_control_test->content=content;deferred_retained_control_test->identity=identity;
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
  auto captured=step_match(contested,{MatchInputKind::tick,14},content);
  assert(!captured.unsupported && captured.state.pass_stage==MatchPassStage::flight_ready);
  assert(!captured.state.flight_defenders.empty() && captured.state.flight_defenders.front()==12);
  assert(verify_checkpoint(captured.state).flight_defenders==captured.state.flight_defenders);
  auto invalid_prefix=captured.state;invalid_prefix.flight_defenders={1};bool reject_prefix=false;
  try {std::ostringstream bad(std::ios::binary);write_match_save(bad,invalid_prefix,identity);}catch(const std::invalid_argument&){reject_prefix=true;}assert(reject_prefix);
  contested.flight_defenders={12,12,12,12};auto capped=step_match(contested,{MatchInputKind::tick,14},content);
  assert(!capped.unsupported && capped.state.flight_defenders==std::vector<std::uint8_t>({12,12,12,12,12}));
  auto defended=verify_checkpoint(captured.state);
  for(std::uint64_t turn=15;defended.pass_stage!=MatchPassStage::flight_arrived && turn<150;++turn){
   auto next=step_match(defended,{MatchInputKind::tick,turn},content);assert(!next.unsupported);defended=next.state;
  }
  assert(defended.pass_stage==MatchPassStage::flight_arrived);
  auto blocked=step_match(defended,{MatchInputKind::presentation_complete,defended.last_sequence+1},content);
  assert(blocked.unsupported==UnsupportedCapability::content && blocked.state.last_sequence==defended.last_sequence);
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
 deferred_free_pass_test=std::make_unique<DeferredDefendedTest>();deferred_free_pass_test->state=handoff.state;
 deferred_free_pass_test->content=content;deferred_free_pass_test->seq=11;deferred_free_pass_test->identity=identity;
 deferred_human_defended_test=std::make_unique<DeferredDefendedTest>();deferred_human_defended_test->state=flight_state;
 deferred_human_defended_test->content=content;deferred_human_defended_test->seq=seq;deferred_human_defended_test->identity=identity;
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
 assert(!refreshed.unsupported && refreshed.state.session.current.field.ball_x==124 && refreshed.state.ball_fraction && refreshed.state.ball_fraction->x==5 && refreshed.state.ball_fraction->y==9);
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
 auto multi_input=aimed.state;multi_input.session.ordinary_pass_candidates->scanned=true;
 multi_input.session.ordinary_pass_candidates->candidates.count=2;multi_input.session.ordinary_pass_candidates->candidates.allied_count=2;
 multi_input.session.ordinary_pass_candidates->candidates.ids={5,6,0,0};
 MatchInput open_multi{MatchInputKind::tick,8};open_multi.pressed=128;auto multi_open=step_match(multi_input,open_multi,content);
 assert(!multi_open.unsupported && multi_open.state.target_chooser && multi_open.state.target_chooser->index==0);
 auto restored_chooser=verify_checkpoint(multi_open.state);assert(restored_chooser.target_chooser->index==0);
 MatchInput down_multi{MatchInputKind::tick,9};down_multi.pressed=4;auto multi_down=step_match(multi_open.state,down_multi,content);
 assert(!multi_down.unsupported && multi_down.state.target_chooser->index==1);
 auto restored_down=step_match(restored_chooser,down_multi,content);
 assert(!restored_down.unsupported && restored_down.state.target_chooser->index==multi_down.state.target_chooser->index);
 MatchInput confirm_multi{MatchInputKind::tick,10};confirm_multi.pressed=128;auto multi_confirmed=step_match(multi_down.state,confirm_multi,content);
 assert(!multi_confirmed.unsupported && !multi_confirmed.state.target_chooser && multi_confirmed.state.session.ordinary_pass_recipient==6);
 assert(multi_confirmed.state.session.phase==FieldSessionPhase::ordinary_human_pass_confirmed);
 assert(verify_checkpoint(multi_confirmed.state).session.ordinary_pass_recipient==6);
 auto multi_prepared=step_match(multi_confirmed.state,{MatchInputKind::tick,11},content);
 assert(!multi_prepared.unsupported && multi_prepared.state.session.ordinary_pass_recipient==6);
 assert(multi_prepared.state.session.phase==FieldSessionPhase::ordinary_pass_prepared);
 auto forged_choice=multi_confirmed.state;forged_choice.session.ordinary_pass_recipient=4;bool reject_forged=false;
 try {std::ostringstream bad(std::ios::binary);write_match_save(bad,forged_choice,identity);}catch(const std::invalid_argument&){reject_forged=true;}assert(reject_forged);
 MatchInput cancel_multi{MatchInputKind::tick,9};cancel_multi.pressed=192;auto multi_cancel=step_match(multi_open.state,cancel_multi,content);
 assert(!multi_cancel.unsupported && !multi_cancel.state.target_chooser && multi_cancel.state.session.phase==FieldSessionPhase::ordinary_human_pass_target);
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
  assert(bytes.back()==0);bytes.pop_back(); // Version15 keeper position is unknown.
  if(bytes.back()!=0){assert(static_cast<unsigned char>(bytes.back())==6);bytes.pop_back();assert(bytes.back()==1);}
  bytes.pop_back(); // Version14 retained defensive action and presence flag.
  assert(bytes.back()==0);bytes.pop_back(); // Version13 processed flight defender count.
  assert(bytes.back()==0);bytes.pop_back(); // Version12 flight defender count.
  assert(bytes.back()==0);bytes.pop_back(); // Version11 target chooser flag.
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
 deferred_unopposed_ai_pass_test=std::make_unique<DeferredDefendedTest>();deferred_unopposed_ai_pass_test->state=ai_pass.state;
 deferred_unopposed_ai_pass_test->content=content;deferred_unopposed_ai_pass_test->identity=identity;
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
 auto save_v9=save_v9_source.str();assert(save_v9.back()==0);save_v9.pop_back();assert(save_v9.back()==0);save_v9.pop_back();assert(save_v9.back()==0);save_v9.pop_back();assert(save_v9.back()==0);save_v9.pop_back();assert(save_v9.back()==0);save_v9.pop_back();save_v9[8]=9;
 const auto v9_length=static_cast<std::uint32_t>(save_v9.size()-17);std::uint32_t v9_crc=0xffffffffu;
 for(std::size_t i=17;i<save_v9.size();++i){v9_crc^=static_cast<unsigned char>(save_v9[i]);for(unsigned j=0;j<8;++j)v9_crc=(v9_crc>>1)^(0xedb88320u&static_cast<std::uint32_t>(-static_cast<std::int32_t>(v9_crc&1)));}
 v9_crc=~v9_crc;for(unsigned i=0;i<4;++i){save_v9[9+i]=static_cast<char>(v9_length>>(8*i));save_v9[13+i]=static_cast<char>(v9_crc>>(8*i));}
 std::istringstream save_v9_stream(save_v9,std::ios::binary);
 assert(load_match_save(save_v9_stream,identity).defense_menu->stage==MatchDefenseStage::ai_flight_ready);
 auto ai_first_crossing=step_match(ai_flight.state,{MatchInputKind::tick,14},content);
 if(!ai_first_crossing.unsupported && ai_first_crossing.state.defense_menu->stage==MatchDefenseStage::ai_flight_cell){
  auto contested=ai_first_crossing.state;auto& defender=contested.session.current.field.motion[field_motion_index(1)];
  defender.x=contested.defense_menu->ai_trajectory->x;defender.y=contested.defense_menu->ai_trajectory->y;defender.status=0;
  contested.session.current.field.random_first=1;contested.session.current.field.random_second=0;
  auto collected=step_match(contested,{MatchInputKind::tick,15},content);
  assert(!collected.unsupported && !collected.state.flight_defenders.empty() && collected.state.flight_defenders.front()==1);
  auto accumulated=verify_checkpoint(collected.state);
  for(std::uint64_t turn=16;accumulated.defense_menu->stage!=MatchDefenseStage::ai_flight_arrived && turn<150;++turn){
   auto next=step_match(accumulated,{MatchInputKind::tick,turn},content);assert(!next.unsupported);accumulated=next.state;
  }
  assert(accumulated.defense_menu->stage==MatchDefenseStage::ai_flight_arrived);
  auto denied_flight=step_match(accumulated,{MatchInputKind::presentation_complete,accumulated.last_sequence+1},content);
  assert(denied_flight.unsupported==UnsupportedCapability::content && denied_flight.state.last_sequence==accumulated.last_sequence);
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
 deferred_defended_test=std::make_unique<DeferredDefendedTest>();deferred_defended_test->state=ai_flight_state;
 deferred_defended_test->content=content;deferred_defended_test->seq=ai_seq;deferred_defended_test->identity=identity;
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
 if(argc!=1){if(std::string(argv[1])=="--compare-human-keeper-menu")return human_keeper_menu_comparison(argc,argv);if(std::string(argv[1])=="--compare-opponent-shot-defender" || std::string(argv[1])=="--compare-opponent-shot-defender-terminal")return opponent_shot_defender_comparison(argc,argv);if(std::string(argv[1])=="--compare-field-defense-skip")return defense_skip_comparison(argc,argv);if(std::string(argv[1])=="--compare-unopposed-ai-shot")return unopposed_shot_comparison(argc,argv);if(std::string(argv[1]).rfind("--compare-human-shot",0)==0 || std::string(argv[1])=="--compare-goal-kickoff")return shot_comparisons(argc,argv);return comparisons(argc,argv);}
 const auto result=regular_tests();
 test_defended_arrival(deferred_defended_test->state,deferred_defended_test->content,deferred_defended_test->seq,deferred_defended_test->identity);
 test_human_defended_arrival(deferred_human_defended_test->state,deferred_human_defended_test->content,deferred_human_defended_test->seq,deferred_human_defended_test->identity);
 test_free_pass(deferred_free_pass_test->state,deferred_free_pass_test->content,deferred_free_pass_test->seq,deferred_free_pass_test->identity);
 test_human_shot(deferred_human_shot_test->state,deferred_human_shot_test->content,deferred_human_shot_test->identity);
 test_retained_action_invalidation(deferred_human_shot_test->state,deferred_human_shot_test->content,deferred_human_shot_test->identity);
 test_retained_action_control(deferred_retained_control_test->state,deferred_retained_control_test->content,deferred_retained_control_test->identity);
 test_control_motion_fraction(deferred_retained_control_test->state,deferred_retained_control_test->content);
 test_pass_field_handoff(deferred_pass_prepared_test->state,deferred_pass_prepared_test->content);
 test_goal_kickoff(deferred_human_shot_test->state,deferred_human_shot_test->content,deferred_human_shot_test->identity);
 test_unopposed_ai_pass(deferred_unopposed_ai_pass_test->state,deferred_unopposed_ai_pass_test->content,deferred_unopposed_ai_pass_test->identity);
 test_opponent_shot_gate(deferred_unopposed_ai_pass_test->state,deferred_unopposed_ai_pass_test->content,deferred_unopposed_ai_pass_test->identity);
 return result;
}

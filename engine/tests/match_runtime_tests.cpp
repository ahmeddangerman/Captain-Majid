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
int main(int argc,char** argv) {
 using namespace captain;
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
 assert(cancelled.unsupported==UnsupportedCapability::action && cancelled.state.last_sequence==7);
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
}

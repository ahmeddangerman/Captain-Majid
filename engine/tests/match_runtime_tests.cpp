#include "captain/match_runtime.hpp"
#include "captain/phase_session_io.hpp"
#include <cassert>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>
int main(int argc,char** argv) {
 using namespace captain;
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

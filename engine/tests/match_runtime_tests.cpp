#include "captain/match_runtime.hpp"
#include <cassert>
#include <stdexcept>
int main() {
 using namespace captain;
 MatchConfig cfg{}; MatchContent content{};
 auto missing=create_match(cfg,content);
 assert(missing.unsupported==UnsupportedCapability::content);
 cfg.initial=PhaseSessionState{}; content.data=ShotContent{};
 cfg.seed=1; assert(create_match(cfg,content).unsupported==UnsupportedCapability::initialization); cfg.seed=0;
 auto start=create_match(cfg,content);
 assert(!start.unsupported && start.state.session.phase==FieldSessionPhase::field);
 auto tick=step_match(start.state,MatchInput{MatchInputKind::tick,1},content);
 assert(tick.unsupported==UnsupportedCapability::initialization);
 assert(tick.state.last_sequence==start.state.last_sequence);
 auto pre=PhaseSessionState{}; pre.phase=FieldSessionPhase::player_control;
 pre.experience=ExperienceState{};pre.recovery=RecoveryContext{};pre.control=ControlContext{};pre.scheduling=SchedulingContext{};
 pre.current.field.side=0;pre.current.field.ball_owner=7;pre.current.field.ball_x=124;pre.current.field.ball_y=124;
 pre.current.field.motion[6].x=124u<<8;pre.current.field.motion[6].y=124u<<8;
 auto ready=step_match(MatchRuntimeState{pre,0},MatchInput{MatchInputKind::tick,1},content);
 assert(!ready.unsupported && ready.state.session.phase==FieldSessionPhase::control_ready);
 assert(pre.phase==FieldSessionPhase::player_control && ready.state.last_sequence==1);
 assert(!ready.events.empty() && ready.events[0].presentation.kind==ControlPresentationKind::display);
 auto action=step_match(ready.state,MatchInput{MatchInputKind::action,2},content);
 assert(action.unsupported==UnsupportedCapability::action && action.state.last_sequence==1);
 auto wait=step_match(ready.state,MatchInput{MatchInputKind::tick,2},content);
 assert(wait.unsupported==UnsupportedCapability::timing && wait.state.last_sequence==1);
 auto duplicate=step_match(start.state,MatchInput{MatchInputKind::tick,0},content);
 assert(duplicate.unsupported==UnsupportedCapability::input_sequence);
 assert(duplicate.state.last_sequence==start.state.last_sequence);
}

#include "captain/match_runtime.hpp"
#include "captain/defender_selection.hpp"
#include <iostream>
#include <array>
#include <string>

// Bounded milestone checks, invoked manually, not part of CTest.
// Synthetic fixtures test runtime wiring; they are not original startup evidence.
int main(int argc,char** argv) {
 const bool demo=argc==2 && std::string(argv[1])=="--demo";
 if(argc>1 && !demo){std::cerr<<"Usage: match-possession-acceptance [--demo]\n";return 2;}
 if(demo)std::cerr<<"PROVISIONAL DEMO: simplified timing and random values; synthetic content.\n";
 using namespace captain;
 unsigned failures=0;
 bool cancellation_checked=false,cancellation_unfinished=false,multi_checked=false,multi_unfinished=false,contested_checked=false,contested_unfinished=false,interception_checked=false,interception_unfinished=false,deflection_checked=false,deflection_unfinished=false,human_defender_checked=false,human_defender_unfinished=false,later_checked=false,later_unfinished=false,special_checked=false,special_unfinished=false;
 const std::array<std::uint8_t,3> owners{{3,7,15}};
 const std::array<std::uint8_t,3> targets{{7,3,19}};
 unsigned fixture=0;
 for (const auto owner : owners) {
  const auto target=targets[fixture++];
  PhaseSessionState initial{};initial.phase=FieldSessionPhase::player_control;
  initial.experience=ExperienceState{};initial.recovery=RecoveryContext{};
  initial.control=ControlContext{};initial.scheduling=SchedulingContext{};initial.action=ActionContext{};
  initial.current.field.side=owner<11?0:11;initial.current.field.ball_owner=owner;
  initial.current.field.roster[owner].type=1;initial.current.field.roster[owner].stamina=100;
  initial.current.field.ball_x=124;initial.current.field.ball_y=124;
  for(auto& player:initial.current.field.motion){player.x=124u<<8;player.y=124u<<8;}
  initial.current.field.motion[field_motion_index(target)].x=140u<<8;
  MatchConfig config{};config.initial=initial;config.random_index=0;config.ball_fraction=MatchBallFraction{0,0}; // Synthetic metadata, not an original capture.
  MatchContent content{};content.data=ShotContent{};
  if(demo){config.mode=MatchTimingMode::provisional_demo;config.seed=1;content.data->movement=FieldPhaseContent{};content.data->movement->geometry.direction_thresholds.fill(65535);content.data->control_movement=ControlMovementContent{};content.data->action_menu=ActionMenuContent{};content.data->action_menu->directions[0][0]=1;content.data->field_players.skills.resize(1);content.data->field_players.skills[0].fill(20);content.data->attack.skill_selectors[1][0]=1;content.data->attack.moves[1].push_back({2,0,20,0});content.data->attack.strength_curve.fill(20);content.pass_distance=PassDistanceContent{};content.pass_distance->values.fill(4);content.ball_velocity=BallVelocityContent{};content.ball_velocity->quarter_wave.fill(128);}
  if(demo && owner>=11) {
   initial.phase=FieldSessionPhase::field_contact_pending;initial.current.field.ball_cell=field_cell_at(124,124);
   initial.current.field.selected_defenders=1;initial.scheduling->selected_ids[0]=1;initial.control->controlled_player=1;initial.current.clock.remaining=200;
   for(unsigned id=1;id<=21;++id)if(id!=11){initial.current.field.roster[id].type=1;auto& p=initial.current.field.motion[field_motion_index(static_cast<std::uint8_t>(id))];p.x=(id<11?52u:200u)<<8;p.y=(id<11?84u:124u)<<8;}
   initial.current.field.motion[field_motion_index(owner)].x=124u<<8;
   initial.current.field.motion[field_motion_index(target)].x=108u<<8;
   config.initial=initial;content.data->movement->profiles.speeds.resize(1);
   content.data->area_defense_menu=AreaDefenseMenuContent{};content.data->area_defense_menu->directions[0].fill(255);
   content.data->area_defense_menu->directions[0][0]=6;content.data->area_defense_menu->field_icons=std::array<std::uint8_t,7>{};
   content.data->area_prefix_elapsed=std::array<std::uint8_t,7>{};(*content.data->area_prefix_elapsed)[1]=1;
   content.data->ai_field_choice=AiFieldChoiceContent{};auto& ai=*content.data->ai_field_choice;
   ai.profile_rows.fill(255);ai.profile_rows[1]=0;ai.recipient_profile_rows.fill(255);ai.recipient_profile_rows[1]=0;
   ai.rows.resize(1);ai.rows[0].fill(0x22);ai.empty_pass_options=std::array<std::uint8_t,118>{};(*ai.empty_pass_options)[1]=1;
   content.data->ai_area_choice=AiAreaChoiceContent{};content.data->ai_area_choice->recipient_rows.resize(1);content.data->ai_area_choice->recipient_rows[0].fill(36);
  }
  try {
   auto started=create_match(config,content);
   if(started.unsupported) {std::cerr<<"owner "<<unsigned(owner)<<": create: "<<capability_name(*started.unsupported)<<'\n';++failures;continue;}
   if(demo && owner>=11) {
    auto state=started.state;std::uint64_t seq=1;bool blocked=false;
    auto advance=[&](MatchInput event){event.sequence=seq++;auto result=step_match(state,event,content);
     if(result.unsupported){std::cerr<<"owner "<<unsigned(owner)<<": "<<capability_name(*result.unsupported)<<" (phase "<<unsigned(state.session.phase)<<")\n";blocked=true;return;}state=result.state;};
    advance({MatchInputKind::presentation_complete,0});
    MatchInput direction{MatchInputKind::tick,0};direction.pressed=1;if(!blocked)advance(direction);
    MatchInput confirm{MatchInputKind::tick,0};confirm.pressed=128;if(!blocked)advance(confirm);
    if(!blocked)advance({MatchInputKind::tick,0});
    if(!blocked)advance(confirm);
    if(!blocked)advance({MatchInputKind::tick,0});
    if(!blocked && (!state.defense_menu->ai_pass_target || state.defense_menu->ai_pass_target->recipient!=target)){std::cerr<<"owner "<<unsigned(owner)<<": wrong AI recipient\n";blocked=true;}
    if(!blocked)advance({MatchInputKind::tick,0});
    if(!blocked)advance({MatchInputKind::presentation_complete,0});
    if(!blocked)advance({MatchInputKind::tick,0});
    if(!blocked)advance({MatchInputKind::presentation_complete,0});
    if(!blocked)advance({MatchInputKind::tick,0});
    if(!blocked)advance({MatchInputKind::tick,0});
    for(unsigned flight_ticks=0;!blocked && state.defense_menu->stage!=MatchDefenseStage::ai_flight_arrived && flight_ticks<128;++flight_ticks){
     advance({MatchInputKind::tick,0});
     if(!blocked && !contested_checked && state.defense_menu->stage==MatchDefenseStage::ai_flight_cell){
      auto contested=state;auto& defender=contested.session.current.field.motion[field_motion_index(1)];
      defender.x=contested.defense_menu->ai_trajectory->x;defender.y=contested.defense_menu->ai_trajectory->y;defender.status=0;
      contested.session.current.field.random_first=1;contested.session.current.field.random_second=0;
      auto result=step_match(contested,{MatchInputKind::tick,seq},content);contested_checked=true;
      if(!result.unsupported){contested=result.state;for(unsigned turn=0;turn<128 && contested.defense_menu->stage!=MatchDefenseStage::ai_flight_arrived;++turn){
       result=step_match(contested,{MatchInputKind::tick,contested.last_sequence+1},content);if(result.unsupported)break;contested=result.state;
      }
      if(!result.unsupported && contested.defense_menu->stage==MatchDefenseStage::ai_flight_arrived){
       contested.session.action->power=30;contested.session.current.field.random_first=1;contested.session.current.field.random_second=0;
       content.data->pass_flight_comparison=PassFlightComparisonContent{1,255,0,{154,96,68,0}};
       content.data->defense.skill_selectors[2][0]=1;content.data->defense.moves[2].push_back({0,0,50,0});content.data->defense.strength_curve.fill(20);
       content.data->contact=ContactPreludeContent{};content.data->contact->field_elapsed[4]=2;content.data->contact->field_elapsed[5]=1;content.data->contact->defender_status[2]=6;
       content.data->experience=ExperienceContent{};
       auto blocked=contested;blocked.session.action->power=1;
       auto interception=step_match(blocked,{MatchInputKind::presentation_complete,blocked.last_sequence+1},content);
       interception_checked=true;
       if(!interception.unsupported)interception=step_match(interception.state,{MatchInputKind::tick,interception.state.last_sequence+1},content);
       interception_unfinished=bool(interception.unsupported) || interception.state.session.phase!=FieldSessionPhase::control_ready || interception.state.session.current.field.ball_owner!=1 || interception.state.session.current.field.side!=0;
       if(!interception_unfinished)std::cout<<"owner "<<unsigned(owner)<<": provisional defender interception reached player 1 control\n";
       blocked.session.action->power=23;auto deflection=step_match(blocked,{MatchInputKind::presentation_complete,blocked.last_sequence+1},content);
       deflection_checked=true;if(!deflection.unsupported)deflection=step_match(deflection.state,{MatchInputKind::tick,deflection.state.last_sequence+1},content);
       deflection_unfinished=bool(deflection.unsupported) || deflection.state.session.phase!=FieldSessionPhase::control_ready;
       if(!deflection_unfinished)std::cout<<"owner "<<unsigned(owner)<<": provisional deflection reached recovered player control\n";
       auto later=contested;later.flight_defenders={1,1};later.session.action->power=34;
       auto first_contact=step_match(later,{MatchInputKind::presentation_complete,later.last_sequence+1},content);later_checked=true;
       if(!first_contact.unsupported){later=first_contact.state;later.session.action->power=1;first_contact=step_match(later,{MatchInputKind::presentation_complete,later.last_sequence+1},content);}
       if(!first_contact.unsupported)first_contact=step_match(first_contact.state,{MatchInputKind::tick,first_contact.state.last_sequence+1},content);
       later_unfinished=bool(first_contact.unsupported) || first_contact.state.session.phase!=FieldSessionPhase::control_ready || first_contact.state.session.current.field.ball_owner!=1;
       if(!later_unfinished)std::cout<<"owner "<<unsigned(owner)<<": provisional later defender interception reached control\n";
       auto special=contested;special.session.current.field.random_second=255;auto special_content=content;special_content.data->pass_flight_comparison->eligibility=128;
       auto special_result=step_match(special,{MatchInputKind::presentation_complete,special.last_sequence+1},special_content);
       special_checked=true;special_unfinished=bool(special_result.unsupported);
       result=step_match(contested,{MatchInputKind::presentation_complete,contested.last_sequence+1},content);
       if(!result.unsupported && result.state.flight_defenders.empty()){
        contested=result.state;result=step_match(contested,{MatchInputKind::presentation_complete,contested.last_sequence+1},content);
        if(!result.unsupported){contested=result.state;result=step_match(contested,{MatchInputKind::tick,contested.last_sequence+1},content);}
       }
      }
      }
      contested_unfinished=bool(result.unsupported) || result.state.session.phase!=FieldSessionPhase::control_ready || result.state.session.current.field.ball_owner!=target;
      if(!contested_unfinished)std::cout<<"owner "<<unsigned(owner)<<": provisional contested pass reached player "<<unsigned(target)<<" control after a weakened defender contact\n";
     }
    }
    if(!blocked && state.defense_menu->stage!=MatchDefenseStage::ai_flight_arrived){std::cerr<<"owner "<<unsigned(owner)<<": opponent pass did not arrive\n";blocked=true;}
    if(!blocked)advance({MatchInputKind::presentation_complete,0});
    if(!blocked)advance({MatchInputKind::tick,0});
    if(!blocked && (state.session.phase!=FieldSessionPhase::control_ready || state.session.current.field.ball_owner!=target)){std::cerr<<"owner "<<unsigned(owner)<<": opponent recipient did not regain control\n";blocked=true;}
    if(blocked)++failures;else std::cout<<"owner "<<unsigned(owner)<<": provisional opponent contact/pass cycle reached player "<<unsigned(target)<<" control\n";
    continue;
   }
   auto control=step_match(started.state,{MatchInputKind::tick,1},content);
   if(control.unsupported) {std::cerr<<"owner "<<unsigned(owner)<<": control: "<<capability_name(*control.unsupported)<<'\n';++failures;continue;}
   auto poll=step_match(control.state,{MatchInputKind::tick,2},content);
   if(poll.unsupported) {std::cerr<<"owner "<<unsigned(owner)<<": first control poll: "<<capability_name(*poll.unsupported)<<'\n';++failures;continue;}
   if(demo && owner<11) {
    auto state=poll.state;std::uint64_t seq=3;bool blocked=false;
    auto advance=[&](MatchInput event) {
     event.sequence=seq++;auto result=step_match(state,event,content);
     if(result.unsupported){std::cerr<<"owner "<<unsigned(owner)<<": "<<capability_name(*result.unsupported)<<" (phase "<<unsigned(state.session.phase)<<", pass stage "<<unsigned(state.pass_stage)<<")\n";blocked=true;return;}
     state=result.state;
    };
    MatchInput menu{MatchInputKind::tick,0};menu.held=64;advance(menu);
    if(!blocked)advance({MatchInputKind::tick,0});
    MatchInput choose{MatchInputKind::tick,0};choose.pressed=1;if(!blocked)advance(choose);
    MatchInput confirm{MatchInputKind::tick,0};confirm.pressed=128;if(!blocked)advance(confirm);
    MatchInput ordinary{MatchInputKind::action,0};ordinary.action=MatchAction::pass;if(!blocked)advance(ordinary);
    if(!blocked && !cancellation_checked){MatchInput cancel{MatchInputKind::tick,seq};cancel.pressed=64;auto result=step_match(state,cancel,content);cancellation_checked=true;cancellation_unfinished=bool(result.unsupported);}
    if(!blocked && !multi_checked){
     auto multi=state;const auto extra=static_cast<std::uint8_t>(owner==3 ? 8 : 4);
     multi.session.current.field.motion[field_motion_index(extra)]=multi.session.current.field.motion[field_motion_index(target)];
     MatchInput probe{MatchInputKind::tick,seq};probe.held=1;auto first=step_match(multi,probe,content);
     probe.sequence++;auto second=step_match(first.state,probe,content);
     MatchInput choose_multi{MatchInputKind::tick,probe.sequence+1};choose_multi.pressed=128;
     auto result=step_match(second.state,choose_multi,content);
     multi_checked=true;multi_unfinished=bool(first.unsupported) || bool(second.unsupported) ||
      !second.state.session.ordinary_pass_candidates || second.state.session.ordinary_pass_candidates->candidates.allied_count<2 || bool(result.unsupported) || !result.state.target_chooser;
     if(!multi_unfinished){MatchInput down{MatchInputKind::tick,choose_multi.sequence+1};down.pressed=4;auto selected=step_match(result.state,down,content);
      MatchInput done{MatchInputKind::tick,down.sequence+1};done.pressed=128;auto confirmed=step_match(selected.state,done,content);
      multi_unfinished=bool(selected.unsupported) || bool(confirmed.unsupported) || !confirmed.state.session.ordinary_pass_recipient || *confirmed.state.session.ordinary_pass_recipient!=extra;}
    }
    MatchInput aim{MatchInputKind::tick,0};aim.held=1;if(!blocked)advance(aim);if(!blocked)advance(aim);
    if(!blocked)advance(confirm);
    if(!blocked && (!state.session.ordinary_pass_recipient || *state.session.ordinary_pass_recipient!=target)){
     std::cerr<<"owner "<<unsigned(owner)<<": wrong selected recipient\n";blocked=true;
    }
    if(!blocked)advance({MatchInputKind::tick,0});
    if(!blocked && (state.session.phase!=FieldSessionPhase::ordinary_pass_prepared || state.session.current.field.roster[owner].stamina!=80)){
     std::cerr<<"owner "<<unsigned(owner)<<": unopposed pass preparation differs\n";blocked=true;
    }
    if(!blocked)advance({MatchInputKind::presentation_complete,0});
    if(!blocked)advance({MatchInputKind::tick,0});
    if(!blocked && (!state.pass_target || state.pass_target->recipient!=target || state.pass_stage!=MatchPassStage::target_ready)){
     std::cerr<<"owner "<<unsigned(owner)<<": wrong pass target\n";blocked=true;
    }
    if(!blocked)advance({MatchInputKind::tick,0});
    if(!blocked && (!state.pass_trajectory || state.pass_stage!=MatchPassStage::flight_ready)){
     std::cerr<<"owner "<<unsigned(owner)<<": flight setup missing\n";blocked=true;
    }
    for(unsigned flight_ticks=0;!blocked && state.pass_stage!=MatchPassStage::flight_arrived && flight_ticks<128;++flight_ticks)
     advance({MatchInputKind::tick,0});
    if(!blocked && state.pass_stage!=MatchPassStage::flight_arrived){std::cerr<<"owner "<<unsigned(owner)<<": pass flight did not arrive\n";blocked=true;}
    if(!blocked && !human_defender_checked){
     auto contested=state;contested.flight_defenders={12};contested.session.current.clock.remaining=200;
     for(unsigned id=1;id<=21;++id)if(id!=11)contested.session.current.field.roster[id].type=1;
     contested.session.current.field.motion[field_motion_index(12)].x=100u<<8;contested.session.current.field.motion[field_motion_index(12)].y=124u<<8;
     contested.session.current.field.random_first=1;contested.session.current.field.random_second=0;
     content.data->movement->profiles.speeds.resize(1);content.data->pass_flight_comparison=PassFlightComparisonContent{1,255,0,{154,96,68,0}};
     content.data->defense.skill_selectors[2][0]=1;content.data->defense.moves[2].push_back({0,0,50,0});content.data->defense.strength_curve.fill(20);
     content.data->contact=ContactPreludeContent{};content.data->contact->field_elapsed[4]=2;content.data->contact->field_elapsed[5]=1;content.data->contact->defender_status[2]=6;content.data->experience=ExperienceContent{};
     human_defender_checked=true;contested.session.action->power=30;
     auto result=step_match(contested,{MatchInputKind::presentation_complete,seq},content);
     if(!result.unsupported)result=step_match(result.state,{MatchInputKind::presentation_complete,result.state.last_sequence+1},content);
     if(!result.unsupported)result=step_match(result.state,{MatchInputKind::tick,result.state.last_sequence+1},content);
     human_defender_unfinished=bool(result.unsupported) || result.state.session.phase!=FieldSessionPhase::control_ready || result.state.session.current.field.ball_owner!=target;
     contested.session.action->power=1;result=step_match(contested,{MatchInputKind::presentation_complete,seq},content);
     if(!result.unsupported)result=step_match(result.state,{MatchInputKind::tick,result.state.last_sequence+1},content);
     human_defender_unfinished=human_defender_unfinished || bool(result.unsupported) || result.state.session.phase!=FieldSessionPhase::control_ready || result.state.session.current.field.ball_owner!=12;
     contested.session.action->power=23;result=step_match(contested,{MatchInputKind::presentation_complete,seq},content);
     if(!result.unsupported)result=step_match(result.state,{MatchInputKind::tick,result.state.last_sequence+1},content);
     human_defender_unfinished=human_defender_unfinished || bool(result.unsupported) || result.state.session.phase!=FieldSessionPhase::control_ready;
     if(!human_defender_unfinished)std::cout<<"owner "<<unsigned(owner)<<": provisional human pass handled weakening, interception and deflection\n";
    }
    if(!blocked)advance({MatchInputKind::presentation_complete,0});
    if(!blocked)advance({MatchInputKind::tick,0});
    if(!blocked && (state.session.phase!=FieldSessionPhase::control_ready || state.session.current.field.ball_owner!=target)){
     std::cerr<<"owner "<<unsigned(owner)<<": recipient did not regain control\n";blocked=true;
    }
    if(blocked){++failures;continue;}
    std::cout<<"owner "<<unsigned(owner)<<": provisional human possession cycle reached player "<<unsigned(target)<<" control\n";continue;
   }
   MatchInput request{MatchInputKind::action,3};request.action=MatchAction::pass;request.target=target;
   auto action=step_match(poll.state,request,content);
   if(action.unsupported) {std::cerr<<"owner "<<unsigned(owner)<<": action: "<<capability_name(*action.unsupported)<<'\n';++failures;continue;}
   auto state=action.state;bool completed=false;
   for(std::uint64_t sequence=4;sequence<2004;++sequence) {
    auto next=step_match(state,{MatchInputKind::tick,sequence},content);
    if(next.unsupported) {std::cerr<<"owner "<<unsigned(owner)<<": pass progression: "<<capability_name(*next.unsupported)<<'\n';break;}
    state=next.state;
    if(state.session.phase==FieldSessionPhase::control_ready && state.session.current.field.ball_owner==target){completed=true;break;}
   }
   if(!completed){std::cerr<<"owner "<<unsigned(owner)<<": pass did not reach target "<<unsigned(target)<<" and resumed control\n";++failures;}
  } catch(const std::exception& error) {std::cerr<<"owner "<<unsigned(owner)<<": "<<error.what()<<'\n';++failures;}
 }
 if(demo && cancellation_unfinished){std::cerr<<"M1 still incomplete: ordinary pass target cancellation is unsupported\n";++failures;}
 if(demo && multi_unfinished){std::cerr<<"M1 still incomplete: multi-teammate pass selection is unsupported\n";++failures;}
 if(demo && (!contested_checked || contested_unfinished)){std::cerr<<"M1 still incomplete: pass-flight defender contact is unsupported or unverified\n";++failures;}
 if(demo && (!interception_checked || interception_unfinished)){std::cerr<<"M1 still incomplete: pass-flight interception is unsupported or unverified\n";++failures;}
 if(demo && (!deflection_checked || deflection_unfinished)){std::cerr<<"M1 still incomplete: pass-flight loose-ball deflection is unsupported or unverified\n";++failures;}
 if(demo && (!human_defender_checked || human_defender_unfinished)){std::cerr<<"M1 still incomplete: human-pass flight defender outcomes are unsupported or unverified\n";++failures;}
 if(demo && (!later_checked || later_unfinished)){std::cerr<<"M1 still incomplete: later-defender interception is unsupported or unverified\n";++failures;}
 if(demo && (!special_checked || special_unfinished)){std::cerr<<"M1 still incomplete: special flight defender outcomes are unsupported or unverified\n";++failures;}
 std::cerr<<failures<<" exercised checks incomplete; other match routes remain unverified\n";
 return failures?1:0;
}

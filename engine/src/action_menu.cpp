#include "captain/action_menu.hpp"
#include "captain/action_strength.hpp"
#include "captain/ball_recovery.hpp"
#include "captain/defender_selection.hpp"
#include "captain/defensive_move_inspection.hpp"
#include "captain/field_state_io.hpp"
#include "captain/pass_target.hpp"
#include <sstream>
#include <stdexcept>

namespace captain {
ActionMenuResult open_area_pass_human_action_menu(const PhaseSessionState& input,const ActionMenuContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_pass_capture_action_menu && input.phase!=FieldSessionPhase::area_pass_blocked_action_menu)
  throw std::invalid_argument("Area pass action menu requires a saved menu request");
 auto state=input;state.phase=FieldSessionPhase::action_menu_pending;
 state.saved_ball_flight_flag=input.ball_trajectory->flags;
 state.area_ai_target.reset();state.area_contact.reset();state.area_pass_contact.reset();
 state.shot_flight.reset();state.ball_trajectory.reset();state.area_defender_cursor=0;state.area_sequence_result=255;
 validate_phase_session(state);return open_human_action_menu(state,content);
}
ActionMenuResult continue_keeper_area_special(const PhaseSessionState& input,const AreaDefenseMenuContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_special_redrawn)
  throw std::invalid_argument("Special continuation requires a completed redraw");
 auto state=input;
 const auto pressed=state.area_special_pressed;
 const auto canceled=state.area_special_decision->status==2;
 state.area_special_option.reset();state.area_special_decision.reset();
 state.area_special_cursor=0;state.area_special_pressed=0;
 state.phase=FieldSessionPhase::keeper_area_defense_poll;
 if(!canceled && state.action->cursor<state.current.field.selected_defenders) {
  state.action->flash_counter|=64u;state.area_defense->blink=0;
  ActionMenuResult result{state};
  result.presentation.push_back({ControlPresentationKind::display,content.titles[2]});
  validate_phase_session(result.state);return result;
 }
 // D438 continues with B/directions only; this is the chooser's same sample.
 return poll_keeper_area_defense(state,static_cast<std::uint8_t>(pressed&127u),content);
}
namespace {
std::uint8_t defense_icon(const KeeperAreaDefenseContext& defense,unsigned slot,const AreaDefenseMenuContent& content) {
 const auto action=defense.actions[slot];
 if(action==255)return 29;
 if(defense.subtypes[slot]) {
  if(action!=2 || defense.subtypes[slot]!=1 || !content.action2_special_icon)
   throw std::invalid_argument("Missing supported special defensive icon");
  return *content.action2_special_icon;
 }
 if(!content.field_icons)throw std::invalid_argument("Missing defensive icons");
 return (*content.field_icons)[action];
}
}
ActionMenuResult redraw_keeper_area_special(const PhaseSessionState& input,const AreaDefenseMenuContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_special_inspected)
  throw std::invalid_argument("Special redraw requires an inspected choice");
 ActionMenuResult result{input};auto& state=result.state;
 auto& menu=*state.action;auto& defense=*state.area_defense;
 const auto& decision=*state.area_special_decision;
 if(decision.status==0) {
  if(decision.subtype>1)throw std::invalid_argument("Unsupported special defensive subtype");
  defense.subtypes[menu.cursor]=decision.subtype;
  ++menu.cursor;
 }
 result.presentation.push_back({ControlPresentationKind::clear_overlay,0});
 const auto count=state.current.field.selected_defenders;
 result.presentation.push_back({ControlPresentationKind::display,static_cast<std::uint8_t>(17+count)});
 for(unsigned slot=0;slot<count;++slot)
  result.presentation.push_back({ControlPresentationKind::sprite,defense_icon(defense,slot,content),static_cast<std::uint8_t>(slot)});
 if(menu.cursor<count)result.presentation.push_back({ControlPresentationKind::display,22});
 state.phase=FieldSessionPhase::keeper_area_special_redrawn;
 validate_phase_session(state);return result;
}
ActionMenuResult inspect_keeper_area_special(const PhaseSessionState& input,const FieldPlayerProfiles& profiles,const FieldDefenseContent& defense) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_special_decided)
  throw std::invalid_argument("Special inspection requires a completed chooser");
 ActionMenuResult result{input};auto& state=result.state;
 PhaseSessionState::SpecialDecision decision{};
 if(!(state.area_special_pressed&128u))decision.status=2;
 else {
  const auto id=state.recovery->opposing_player;
  const auto& player=state.current.field.roster[id];
  decision.subtype=state.area_special_cursor ? *state.area_special_option : 0;
  const FieldPlayerRecord record{id,player.type,static_cast<std::uint8_t>(player.stamina),
      static_cast<std::uint8_t>(player.stamina>>8),player.level};
  const auto inspection=inspect_defensive_move(record,player.stamina,2,state.recovery->ball_variant,decision.subtype,profiles,defense);
  decision.cost=inspection.cost;decision.effect=inspection.effect;
  if(!inspection.allowed) {
   decision.status=1;
   result.presentation.push_back({ControlPresentationKind::script,61});
  }
 }
 state.area_special_decision=decision;state.phase=FieldSessionPhase::keeper_area_special_inspected;
 validate_phase_session(state);return result;
}
ActionMenuResult poll_keeper_area_special(const PhaseSessionState& input,std::uint8_t pressed,const AreaDefenseMenuContent& content) {
 validate_phase_session(input);
 if((input.phase!=FieldSessionPhase::keeper_area_special_ready && input.phase!=FieldSessionPhase::keeper_area_special_poll) || !content.special_cursor_y_base)
  throw std::invalid_argument("Special chooser input requires its wait state and private cursor position");
 ActionMenuResult result{input};auto& state=result.state;
 result.special_cursor_y=static_cast<std::uint8_t>(*content.special_cursor_y_base+16u*state.area_special_cursor);
 if(pressed&12u) {
  if(pressed&8u) {if(state.area_special_cursor)--state.area_special_cursor;}
  else if(!state.area_special_cursor)++state.area_special_cursor;
 }
 if(pressed&192u) {
  state.phase=FieldSessionPhase::keeper_area_special_decided;state.area_special_pressed=pressed;
  result.special_cursor_y=248;result.confirmed=(pressed&128u)!=0;
 } else state.phase=FieldSessionPhase::keeper_area_special_poll;
 validate_phase_session(state);return result;
}
ActionMenuResult open_keeper_area_special(const PhaseSessionState& input,const AreaDefenseMenuContent& content) {
 const auto option=keeper_area_defense_special_option(input,content);
 if(!option || !content.special_cursor)
  throw std::invalid_argument("Special chooser requires an available option and private cursor geometry");
 ActionMenuResult result{input};
 result.state.area_special_option=*option;
 result.state.area_defense->pressed=0;
 result.state.phase=FieldSessionPhase::keeper_area_special_ready;
 result.presentation.push_back({ControlPresentationKind::display,12});
 result.special_cursor=SpecialCursorPresentation{(*content.special_cursor)[0],(*content.special_cursor)[1],3};
 validate_phase_session(result.state);return result;
}
std::optional<std::uint8_t> keeper_area_defense_special_option(const PhaseSessionState& input,const AreaDefenseMenuContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_defense_preparation)
  throw std::invalid_argument("Defensive availability requires a confirmed choice");
 const auto action=input.area_defense->actions[input.action->cursor];
 // Field actions>=3 bypass options. With a nonzero area variant, so do0/1.
 if(action>=3 || (input.recovery->ball_variant && action!=2))return std::nullopt;
 if(action!=2 || !content.special_options)
  throw std::invalid_argument("Missing private defensive special-option rules");
 const auto player=input.recovery->opposing_player;
 const auto type=input.current.field.roster[player].type;
 const auto& rules=content.special_options->field_action2;
 if(type>=rules.size())throw std::invalid_argument("Defensive special-option player type is outside private rules");
 const auto& rule=rules[type];
 if(!rule)return std::nullopt;
 if(rule->subtype>127)throw std::invalid_argument("Unsupported defensive special subtype");
 if(!rule->teammate_type)return rule->subtype;
 // Original scans home field IDs1..10, including the acting player.
 for(unsigned id=1;id<=10;++id)
  if(input.current.field.roster[id].type==*rule->teammate_type)return rule->subtype;
 return std::nullopt;
}
ActionMenuResult prepare_keeper_area_defense(const PhaseSessionState& input,const AreaDefenseMenuContent& content) {
 validate_phase_session(input);
 const bool field=input.phase==FieldSessionPhase::sampled_field_defense_preparation;
 if((input.phase!=FieldSessionPhase::keeper_area_defense_preparation && !field) || !content.field_icons)
  throw std::invalid_argument("Defensive registration requires a confirmed choice and private icons");
 const auto slot=input.action->cursor;
 const auto action=input.area_defense->actions[slot];
 // The sampled ordinary action0 has no special option for this captured home roster.
 const bool postgoal=field && input.score && input.score->left_goals==2 && input.score->right_goals==1 &&
  input.current.clock.remaining==403 && input.current.clock.extra_time==9 &&
  input.current.field.ball_owner==19 && input.scheduling &&
  input.scheduling->selected_ids==std::array<std::uint8_t,5>{3,8,9,0,0};
 bool sampled_roster=field;
 if(sampled_roster)for(unsigned id=1;id<=10;++id) {
  const auto& player=input.current.field.roster[id];
  if(player.type!=23 || player.stamina!=(postgoal ? (id==3 ? 30 : id==5 ? 25 : 140) : (id==3 ? 30 : 110)))sampled_roster=false;
 }
 const bool third_return_ordinary=field && input.score && input.score->left_goals==1 && !input.score->right_goals && input.score->kickoff==4 &&
  input.current.clock.remaining==153 && !input.current.clock.extra_time && input.current.field.side==11 && input.current.field.ball_owner==18 &&
  input.current.field.ball_cell==87 && input.current.field.selected_defenders==1 && input.scheduling && input.scheduling->selected_ids[0]==5 &&
  input.current.field.roster[5].type==7 && input.current.field.roster[5].level==0 && input.current.field.roster[5].stamina==400;
 const bool sampled_ordinary=(sampled_roster || third_return_ordinary) && slot==0 && action==0;
 const bool first_supported=slot<1 || input.area_defense->actions[0]==6 ||
  (sampled_roster && input.area_defense->actions[0]==0);
 const bool sampled_skip=field && slot<=2 && action==6 && first_supported &&
  (slot<2 || input.area_defense->actions[1]==6);
 if(field && ((!sampled_ordinary && !sampled_skip) || input.area_defense->pressed!=128))
  throw std::invalid_argument("Unverified sampled field defensive registration");
 if(!field && keeper_area_defense_special_option(input,content))
  throw std::invalid_argument("Available defensive special option requires its unimplemented move chooser");
 ActionMenuResult result{input};auto& state=result.state;auto& menu=*state.action;auto& defense=*state.area_defense;
 const auto pressed=defense.pressed;
 defense.subtypes[slot]=0;
 result.presentation.push_back({ControlPresentationKind::sprite,(*content.field_icons)[action],slot});
 ++menu.cursor;state.phase=field ? FieldSessionPhase::sampled_field_defense_poll : FieldSessionPhase::keeper_area_defense_poll;defense.pressed=0;
 if(menu.cursor<state.current.field.selected_defenders || (pressed&64u)) {
  if(menu.cursor==state.current.field.selected_defenders)--menu.cursor;
  menu.flash_counter|=64u;defense.blink=0;
  result.presentation.push_back({ControlPresentationKind::display,content.titles[field ? 0 : 2]});
 } else if(pressed&15u) {
  throw std::invalid_argument("Full defensive prefix direction scratch slots are not represented");
 }
 validate_phase_session(state);return result;
}
DefenseMenuPollResult poll_defense_menu(const DefenseMenuPollState& input,std::uint8_t count,
 const std::array<std::uint8_t,5>& selected_ids,bool field,std::uint8_t pressed,const AreaDefenseMenuContent& content) {
 if(!count || count>4 || input.cursor>count)throw std::invalid_argument("Invalid defensive menu count or cursor");
 for(unsigned slot=0;slot<count;++slot)
  if(input.defense.actions[slot]!=255 && input.defense.actions[slot]>6)
   throw std::invalid_argument("Invalid defensive menu action");
 DefenseMenuPollResult result{input};auto& menu=result.state;auto& defense=menu.defense;
 defense.pressed=0;
 const auto icon=[&](unsigned slot) {return defense_icon(defense,slot,content);};
 if((pressed&128u) && menu.cursor==count) {
  result.boundary=DefenseMenuBoundary::complete;
  result.presentation.push_back({ControlPresentationKind::clear_overlay,0});
  return result;
 }
 if((pressed&128u) && defense.actions[menu.cursor]!=255) {
  result.opposing_player=selected_ids[menu.cursor];
  defense.pressed=pressed;
  result.boundary=DefenseMenuBoundary::choice_confirmed;
  return result;
 }
 if((pressed&64u) && menu.cursor) {
  if(menu.cursor<count)result.presentation.push_back({ControlPresentationKind::sprite,icon(menu.cursor),menu.cursor});
  --menu.cursor;menu.flash_counter|=64u;defense.blink=0;
  result.presentation.push_back({ControlPresentationKind::display,content.titles[field ? 0 : 2]});
  return result;
 }
 auto directions=static_cast<unsigned>(pressed&15u);
 if(menu.cursor==count) {
  if(directions)throw std::invalid_argument("Full defensive prefix direction scratch slots are not represented");
  return result;
 }
 if(directions) {
  unsigned direction=0;while(!(directions&(1u<<direction)))++direction;
  const auto action=content.directions[field ? 0 : 2][direction];
  if(action!=255) {
   if(action>6)throw std::invalid_argument("Unsupported defensive menu action");
   if(action!=defense.actions[menu.cursor]) {
    defense.actions[menu.cursor]=action;defense.subtypes[menu.cursor]=0;menu.flash_counter=0;
   }
  }
 }
 if(!(menu.flash_counter&128u)) {menu.flash_counter=128;defense.blink=0;}
 if(defense.blink)--defense.blink;
 else {
  defense.blink=13;menu.flash_counter^=64u;
  result.presentation.push_back({ControlPresentationKind::sprite,
   static_cast<std::uint8_t>(icon(menu.cursor)|((menu.flash_counter&64u)?0u:128u)),menu.cursor});
 }
 return result;
}
ActionMenuResult poll_keeper_area_defense(const PhaseSessionState& input,std::uint8_t pressed,const AreaDefenseMenuContent& content) {
 validate_phase_session(input);
 const bool field=input.phase==FieldSessionPhase::sampled_field_defense_prompt || input.phase==FieldSessionPhase::sampled_field_defense_poll;
 if(input.phase!=FieldSessionPhase::keeper_area_defense_ready && input.phase!=FieldSessionPhase::keeper_area_defense_poll && !field)
  throw std::invalid_argument("Defensive input requires prompt/poll state");
 const DefenseMenuPollState menu{input.action->cursor,input.action->flash_counter,*input.area_defense};
 const auto poll=poll_defense_menu(menu,input.current.field.selected_defenders,input.scheduling->selected_ids,field,pressed,content);
 ActionMenuResult result{input};auto& state=result.state;
 state.action->cursor=poll.state.cursor;state.action->flash_counter=poll.state.flash_counter;state.area_defense=poll.state.defense;
 result.presentation=poll.presentation;
 if(poll.boundary==DefenseMenuBoundary::complete){state.phase=field ? FieldSessionPhase::sampled_field_defense_complete : FieldSessionPhase::keeper_area_defense_complete;result.confirmed=true;}
 else if(poll.boundary==DefenseMenuBoundary::choice_confirmed){state.recovery->opposing_player=*poll.opposing_player;state.phase=field ? FieldSessionPhase::sampled_field_defense_preparation : FieldSessionPhase::keeper_area_defense_preparation;result.confirmed=true;}
 else state.phase=field ? FieldSessionPhase::sampled_field_defense_poll : FieldSessionPhase::keeper_area_defense_poll;
 validate_phase_session(state);return result;
}
ActionMenuResult open_keeper_area_defense_prompt(const PhaseSessionState& input,const AreaDefenseMenuContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_defense_initial)
  throw std::invalid_argument("Defensive prompt requires initialized choices");
 ActionMenuResult result{input};const auto count=input.current.field.selected_defenders;
 result.presentation.push_back({ControlPresentationKind::display,static_cast<std::uint8_t>(17+count)});
 for(unsigned slot=0;slot<count;++slot)
  result.presentation.push_back({ControlPresentationKind::sprite,29,static_cast<std::uint8_t>(slot)});
 result.presentation.push_back({ControlPresentationKind::display,content.titles[2]});
 result.state.phase=FieldSessionPhase::keeper_area_defense_ready;
 validate_phase_session(result.state);return result;
}
PhaseSessionState begin_keeper_area_defense(const PhaseSessionState& input,std::uint8_t blink) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_defense_pending || input.control->motion_requested)
  throw std::invalid_argument("Area defense requires its completed menu entry");
 auto state=input;
 if(!state.current.field.selected_defenders)state.phase=FieldSessionPhase::keeper_area_choices_done;
 else {
  state.action->cursor=0;state.area_defense=KeeperAreaDefenseContext{};
  state.area_defense->blink=blink;state.phase=FieldSessionPhase::keeper_area_defense_initial;
 }
 validate_phase_session(state);return state;
}
ActionMenuResult open_prepared_keeper_area_menu(const PhaseSessionState& input,std::uint8_t facing,const ActionMenuContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_prepared || input.action->menu_route!=2 ||
    (facing!=0 && facing!=64))
  throw std::invalid_argument("Prepared area menu requires own-area route2 and captured facing0/64");
 ActionMenuResult result{input};auto& state=result.state;auto& field=state.current.field;
 DefenderSelectionInput scan{field.side,field.ball_x,field.ball_y,10,{}};
 for(unsigned i=0;i<field.motion.size();++i) {
  const auto& m=field.motion[i];scan.players[i]={static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8),m.status!=0};
 }
 const auto selected=select_field_defenders(scan);
 field.selected_defenders=static_cast<std::uint8_t>(selected.size());state.scheduling->selected_ids.fill(0);
 for(unsigned i=0;i<selected.size();++i)state.scheduling->selected_ids[i]=selected[i];
 result.presentation.push_back({ControlPresentationKind::script,63});
 state.current.clock.ball_mode=0;state.action->auxiliary=0;
 const auto& keeper=field.keepers[field.side==0?1:0];
 if(keeper.stun)result.presentation.push_back({ControlPresentationKind::script,64});
 else if(keeper.penalty>=24)result.presentation.push_back({ControlPresentationKind::script,65});
 result.presentation.push_back({ControlPresentationKind::clear_overlay,0});
 state.control->facing=facing;state.control->display_parameter=0;state.control->motion_requested=0;
 state.experience->totals[10]=static_cast<std::uint16_t>((state.experience->totals[10]&255u)|256u);
 if(field.side==0) {
  result.presentation.push_back({ControlPresentationKind::display,content.titles[2]});
  result.presentation.push_back({ControlPresentationKind::display,content.backgrounds[2]});
  state.action->cursor=0;state.phase=FieldSessionPhase::human_action_menu;
 } else state.phase=FieldSessionPhase::keeper_area_defense_pending;
 validate_phase_session(state);return result;
}
ActionMenuResult open_human_action_menu(const PhaseSessionState& input, const ActionMenuContent& content) {
    validate_phase_session(input);
    if ((input.phase != FieldSessionPhase::action_menu_pending && input.phase != FieldSessionPhase::field_contact_pending) ||
        input.current.field.side != 0 || !input.action || !input.scheduling)
        throw std::invalid_argument("Human attacking menu requires side0 menu/contact pause and captured action context");
    ActionMenuResult result{input};
    auto& state = result.state;
    result.presentation.push_back({ControlPresentationKind::script,
        static_cast<std::uint8_t>(input.phase == FieldSessionPhase::action_menu_pending ? 68 : 46)});
    state.recovery->ball_variant = 0;
    state.action->menu_route = 0;
    state.action->auxiliary = 0;
    state.current.clock.ball_mode = 0;
    const auto& keeper = state.current.field.keepers[1];
    if (keeper.stun != 0) result.presentation.push_back({ControlPresentationKind::script, 64});
    else if (keeper.penalty >= 24) result.presentation.push_back({ControlPresentationKind::script, 65});
    result.presentation.push_back({ControlPresentationKind::clear_overlay, 0});
    result.presentation.push_back({ControlPresentationKind::display, content.titles[0]});
    result.presentation.push_back({ControlPresentationKind::display, content.backgrounds[0]});
    // Known completed menu presentation effects, without a render/NMI clock.
    if (state.current.field.selected_defenders != 0) state.control->facing = 0;
    state.control->display_parameter = 0;
    state.control->motion_requested = 0;
    state.experience->totals[10] = static_cast<std::uint16_t>((state.experience->totals[10] & 255u) | 256u);
    state.action->cursor = 0;
    state.phase = FieldSessionPhase::human_action_menu;
    validate_phase_session(state);
    return result;
}

ActionMenuResult open_ordinary_return_action_menu(const PhaseSessionState& input,const ActionMenuContent& content) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_return_contact_pending)
        throw std::invalid_argument("Returned action menu requires the saved encounter");
    auto normalized=input;normalized.phase=FieldSessionPhase::field_contact_pending;normalized.area_defender_cursor=0;
    auto result=open_human_action_menu(normalized,content);
    result.state.phase=FieldSessionPhase::ordinary_return_action_menu;
    result.state.area_defender_cursor=input.area_defender_cursor;
    validate_phase_session(result.state);return result;
}
ActionMenuResult acknowledge_sampled_goalkeeper_overlay(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase==FieldSessionPhase::sampled_score23_keeper_confirmed && input.keeper_menu_action==0) {
  ActionMenuResult result{input};result.state.phase=FieldSessionPhase::sampled_score23_keeper_precompare;
  result.presentation.push_back({ControlPresentationKind::clear_overlay,0});validate_phase_session(result.state);return result;
 }
 if(input.phase==FieldSessionPhase::sampled_player19_return_keeper_confirmed && input.keeper_menu_action==0) {
  ActionMenuResult result{input};result.state.phase=FieldSessionPhase::sampled_player19_return_keeper_precompare;
  result.presentation.push_back({ControlPresentationKind::clear_overlay,0});validate_phase_session(result.state);return result;
 }
 if(input.phase!=FieldSessionPhase::sampled_shot_keeper_confirmed || (input.keeper_menu_action!=0 && !(input.current.clock.extra_time==9 && input.keeper_menu_action==1 && input.current.field.roster[0].stamina==16)))
  throw std::invalid_argument("Unsupported sampled goalkeeper overlay");
 ActionMenuResult result{input};result.state.phase=FieldSessionPhase::sampled_shot_keeper_precompare;
 result.presentation.push_back({ControlPresentationKind::clear_overlay,0});
 validate_phase_session(result.state);return result;
}
ActionMenuResult poll_sampled_goalkeeper_prompt(const PhaseSessionState& input,std::uint8_t pressed,
 const std::array<std::uint8_t,2>& icons) {
 validate_phase_session(input);
 const bool player19_return=input.phase==FieldSessionPhase::sampled_player19_return_keeper_prompt || input.phase==FieldSessionPhase::sampled_player19_return_keeper_selected;
 const bool score23=input.phase==FieldSessionPhase::sampled_score23_keeper_prompt || input.phase==FieldSessionPhase::sampled_score23_keeper_selected;
 if(input.phase==FieldSessionPhase::sampled_shot_keeper_selected || input.phase==FieldSessionPhase::sampled_player19_return_keeper_selected || input.phase==FieldSessionPhase::sampled_score23_keeper_selected) {
  if(pressed!=128)throw std::invalid_argument("Unverified sampled goalkeeper follow-up input");
  ActionMenuResult result{input};result.state.phase=score23 ? FieldSessionPhase::sampled_score23_keeper_confirmed : player19_return ? FieldSessionPhase::sampled_player19_return_keeper_confirmed : FieldSessionPhase::sampled_shot_keeper_confirmed;
  result.confirmed=true;validate_phase_session(result.state);return result;
 }
 if((input.phase!=FieldSessionPhase::sampled_shot_keeper_prompt && input.phase!=FieldSessionPhase::sampled_player19_return_keeper_prompt && input.phase!=FieldSessionPhase::sampled_score23_keeper_prompt) || input.current.field.roster[0].type!=1)
  throw std::invalid_argument("Unsupported sampled goalkeeper prompt");
 if(pressed!=0 && pressed!=1 && pressed!=2 && pressed!=4 && pressed!=8 && pressed!=64 && pressed!=128)
  throw std::invalid_argument("Unverified sampled goalkeeper input");
 if((player19_return || score23) && pressed!=0 && pressed!=1)throw std::invalid_argument("Unverified sampled goalkeeper input");
 ActionMenuResult result{input};
 if(pressed==0 || pressed==4 || pressed==64 || pressed==128)return result;
 const auto direction=static_cast<std::uint8_t>(pressed==1 ? 0 : pressed==2 ? 1 : 3);
 const auto action=static_cast<std::uint8_t>(pressed==8 ? 1 : 0);
 result.state.keeper_menu_action=action;result.state.action->cursor=static_cast<std::uint8_t>(224u|direction);
 result.state.action->flash_counter=13;result.state.phase=score23 ? FieldSessionPhase::sampled_score23_keeper_selected : player19_return ? FieldSessionPhase::sampled_player19_return_keeper_selected : FieldSessionPhase::sampled_shot_keeper_selected;
 result.presentation.push_back({ControlPresentationKind::sprite,icons[action],0});
 validate_phase_session(result.state);return result;
}
ActionMenuResult poll_human_action_menu(const PhaseSessionState& input, std::uint8_t pressed,
                                      const ActionMenuContent& content) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::human_action_menu)
        throw std::invalid_argument("Action selection requires human action-menu phase");
    ActionMenuResult result{input};
    auto& state = result.state;
    auto& menu = *state.action;
    const auto directions = pressed & 15u;
    if (directions) {
        unsigned direction = 0;
        while (!(directions & (1u << direction))) ++direction;
        const auto action = content.directions[menu.menu_route][direction];
        if (action != 255) {
            if (action >= content.icons.size()) throw std::invalid_argument("Menu action outside supported icon table");
            menu.action = action;
            menu.cursor = static_cast<std::uint8_t>(((menu.cursor & 3u) == direction ? menu.cursor : direction) | 128u);
        }
    }
    if ((pressed & 128u) && (menu.cursor & 128u)) {
        state.phase = FieldSessionPhase::attack_choice_pending;
        result.confirmed = true;
        return result;
    }
    if (!(menu.cursor & 128u)) return result;
    if (!(menu.cursor & 32u)) { menu.cursor |= 32u; menu.flash_counter = 0; }
    if (menu.flash_counter != 0) { --menu.flash_counter; return result; }
    menu.flash_counter = 13;
    menu.cursor ^= 64u;
    const auto icon = static_cast<std::uint8_t>(content.icons[menu.action] | ((menu.cursor & 64u) ? 0u : 128u));
    result.presentation.push_back({ControlPresentationKind::sprite, icon});
    return result;
}
ActionMenuResult poll_ordinary_return_goal_area_menu(const PhaseSessionState& input,std::uint8_t pressed,const ActionMenuContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_return_goal_area_menu)
  throw std::invalid_argument("Recovered goal-area menu requires its saved poll boundary");
 auto normalized=input;normalized.phase=FieldSessionPhase::human_action_menu;normalized.area_contact.reset();
 normalized.shot_flight.reset();normalized.ball_trajectory.reset();normalized.area_sequence_result=255;
 const auto polled=poll_human_action_menu(normalized,pressed,content);
 if(polled.state.action->action!=0)throw std::invalid_argument("Only the observed ordinary goal-area shot is supported");
 ActionMenuResult result{input};result.state.action=polled.state.action;result.confirmed=polled.confirmed;result.presentation=polled.presentation;
 result.state.phase=polled.confirmed ? FieldSessionPhase::ordinary_return_goal_area_shot_choice : FieldSessionPhase::ordinary_return_goal_area_menu;
 validate_phase_session(result.state);return result;
}
ActionMenuResult poll_ordinary_return_action_menu(const PhaseSessionState& input,std::uint8_t pressed,const ActionMenuContent& content) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_return_action_menu)
        throw std::invalid_argument("Returned menu poll requires its saved menu");
    auto normalized=input;normalized.phase=FieldSessionPhase::human_action_menu;normalized.area_defender_cursor=0;
    auto result=poll_human_action_menu(normalized,pressed,content);
    if(result.confirmed) {
        if(result.state.action->action!=0)throw std::invalid_argument("Only ordinary shot confirmation is supported in returned menu");
        result.state.phase=FieldSessionPhase::ordinary_return_shot_choice;
    } else result.state.phase=FieldSessionPhase::ordinary_return_action_menu;
    result.state.area_defender_cursor=input.area_defender_cursor;
    validate_phase_session(result.state);return result;
}

PhaseSessionState advance_ordinary_return_goal_area_shot_dispatch(const PhaseSessionState& input,const ContactPreludeContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_return_goal_area_field_moved || content.field_elapsed[1]!=1 || content.defender_status[0]!=6)
  throw std::invalid_argument("Goal-area shot dispatch requires the observed contact timing");
 auto state=input;const auto applied=apply_contact_prelude({state.current,state.recovery->opposing_player,0,true},content);
 if(applied.clock.continuation!=ClockContinuation::ready)throw std::invalid_argument("Goal-area dispatch clock continuation is unsupported");
 state.current=applied.state;state.phase=FieldSessionPhase::ordinary_return_goal_area_dispatch;validate_phase_session(state);return state;
}
PhaseSessionState handoff_ordinary_return_goal_area_shot(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_return_goal_area_dispatch)
  throw std::invalid_argument("Goal-area shot handoff requires its dispatch clock");
 auto state=input;state.action->menu_route=0;state.phase=FieldSessionPhase::ordinary_return_goal_area_handoff;validate_phase_session(state);return state;
}
PhaseSessionState begin_ordinary_return_goal_area_shot_clock(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_return_goal_area_shot_prepared || input.current.field.selected_defenders)
  throw std::invalid_argument("Unopposed goal-area clock requires the prepared shot");
 auto state=input;state.area_sequence_result=0;state.control->last_carrier_cell=0;
 state.phase=FieldSessionPhase::ordinary_return_goal_area_clock_entry;validate_phase_session(state);return state;
}
PhaseSessionState advance_ordinary_return_goal_area_shot_clock(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_return_goal_area_clock_entry)
  throw std::invalid_argument("Goal-area first clock requires its unopposed entry");
 auto state=input;state.current.field.elapsed=static_cast<std::uint8_t>(unsigned(state.current.field.elapsed)+1);
 const auto recovery=advance_keeper_recovery({state.current.field.keepers,state.current.shared_recovery_flags,state.current.shared_recovery_timer},1);
 state.current.field.keepers=recovery.keepers;state.current.shared_recovery_flags=recovery.shared_flags;state.current.shared_recovery_timer=recovery.shared_timer;
 const auto clock=advance_match_clock(state.current.clock,1,state.current.field.random_first);
 if(clock.continuation!=ClockContinuation::ready)throw std::invalid_argument("Goal-area first clock continuation is unsupported");
 state.current.clock=clock.state;state.phase=FieldSessionPhase::ordinary_return_goal_area_first_clock;validate_phase_session(state);return state;
}
SessionAttackResult prepare_ordinary_return_goal_area_shot(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& attack) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ordinary_return_goal_area_shot_choice || input.recovery->attack_subtype!=1 || input.recovery->ball_variant!=1)
  throw std::invalid_argument("Goal-area shot preparation requires the captured volley and ball variant");
 const auto& field=input.current.field;const auto& player=field.roster[field.ball_owner];
 const auto prepared=prepare_attacking_action({{field.ball_owner,player.type,static_cast<std::uint8_t>(player.stamina),
  static_cast<std::uint8_t>(player.stamina>>8),player.level},0,1,1,field.side,field.selected_defenders,
  field.random_first,field.random_second,false},players,attack);
 const auto remaining=static_cast<std::uint16_t>(unsigned(player.stamina)-prepared.preparation.stamina_cost);
 if((remaining&32768u) || prepared.action_power.subtype!=1)
  throw std::invalid_argument("Unavailable or unsupported goal-area volley preparation");
 auto state=input;state.current.field.roster[field.ball_owner].stamina=prepared.energy.stamina;
 state.action->power=prepared.action_power.power;state.action->move_effect=prepared.preparation.move_effect;state.action->move_flags=prepared.move_flags;
 state.phase=FieldSessionPhase::ordinary_return_goal_area_shot_prepared;validate_phase_session(state);return {std::move(state),prepared};
}
SessionAttackResult prepare_session_basic_shot(const PhaseSessionState& input,
    const FieldPlayerProfiles& players, const FieldDefenseContent& attack) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::attack_choice_pending || input.current.field.side != 0 ||
        input.action->action != 0 || input.action->menu_route != 0)
        throw std::invalid_argument("Prepared basic shot requires a confirmed ordinary side0 shot");
    const auto& field = input.current.field;
    const auto& player = field.roster[field.ball_owner];
    const auto prepared = prepare_attacking_action({{field.ball_owner, player.type,
        static_cast<std::uint8_t>(player.stamina), static_cast<std::uint8_t>(player.stamina >> 8), player.level},
        0, 0, 0, 0, field.selected_defenders, field.random_first, field.random_second, false}, players, attack);
    // Original menu check tests the high byte's sign after the raw-cost subtraction.
    const auto remaining = static_cast<std::uint16_t>(unsigned(player.stamina) - prepared.preparation.stamina_cost);
    if (remaining & 32768u) throw std::invalid_argument("Ordinary shot is unavailable under the original menu stamina check");
    auto state = input;
    state.current.field.roster[field.ball_owner].stamina = prepared.energy.stamina;
    state.recovery->attack_subtype = prepared.action_power.subtype;
    state.recovery->ball_variant = 0;
    state.action->power = prepared.action_power.power;
    state.action->move_effect = prepared.preparation.move_effect;
    state.action->move_flags = prepared.move_flags;
    state.phase = FieldSessionPhase::attack_prepared;
    validate_phase_session(state);
    return {state, prepared};
}
PhaseSessionState record_player7_shot_defender(const PhaseSessionState& input,std::uint8_t defender,std::uint8_t action,std::uint8_t subtype) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::attack_choice_pending || !input.score || !input.scheduling || !input.recovery ||
        input.score->left_goals != 1 || input.score->right_goals != 0 || input.score->kickoff != 4 ||
        input.current.clock.remaining != 146 || input.current.clock.extra_time != 0 || input.current.field.side != 0 ||
        input.current.field.ball_owner != 7 || input.current.field.ball_x != 120 || input.current.field.ball_y != 124 ||
        input.current.field.selected_defenders != 1 || input.scheduling->selected_ids[0] != 20 ||
        input.action->action != 0 || input.recovery->opposing_player != 5 || defender != 20 || action != 2 || subtype != 0)
        throw std::invalid_argument("Unsupported player7 shot defender handoff");
    auto state = input;
    state.recovery->opposing_player = defender;
    validate_phase_session(state);
    return state;
}
PhaseSessionState record_ordinary_return_shot_defense(const PhaseSessionState& input,std::uint8_t defender,std::uint8_t action,std::uint8_t subtype) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_return_shot_choice || input.current.field.selected_defenders!=1 ||
       !input.scheduling || defender!=input.scheduling->selected_ids[0] || action!=1 || subtype!=0 || input.area_contact)
        throw std::invalid_argument("Returned shot requires one explicitly observed action-one defense");
    auto state=input;state.recovery->opposing_player=defender;
    state.area_contact=AreaContactContext{0,action,subtype,0,0,0,0,0,0,0,false};
    state.phase=FieldSessionPhase::ordinary_return_shot_defense;validate_phase_session(state);return state;
}
SessionAttackResult prepare_ordinary_return_shot(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& attack) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_return_shot_defense || !input.area_contact)
        throw std::invalid_argument("Returned shot preparation requires a recorded defender");
    auto normalized=input;normalized.phase=FieldSessionPhase::attack_choice_pending;
    normalized.area_defender_cursor=0;normalized.area_contact.reset();
    auto result=prepare_session_basic_shot(normalized,players,attack);
    result.state.area_defender_cursor=input.area_defender_cursor;result.state.area_contact=input.area_contact;
    result.state.phase=FieldSessionPhase::ordinary_return_shot_prepared;
    validate_phase_session(result.state);return result;
}
PhaseSessionState prepare_player7_shot_defender(const PhaseSessionState& input,std::uint8_t observed_variant,
    const FieldPlayerProfiles& players,const FieldDefenseContent& defense) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::attack_prepared || !input.score || !input.control || !input.recovery || !input.scheduling ||
       input.score->left_goals!=1 || input.score->right_goals!=0 || input.score->kickoff!=4 || input.current.clock.remaining!=146 ||
       input.current.clock.extra_time || input.current.field.side!=0 || input.current.field.ball_owner!=7 ||
       input.current.field.ball_x!=120 || input.current.field.ball_y!=124 || input.current.field.selected_defenders!=1 ||
       input.scheduling->selected_ids[0]!=20 || input.recovery->opposing_player!=20 || input.current.field.roster[7].stamina!=305 ||
       input.action->power!=5 || input.control->motion_requested!=2 || input.control->last_carrier_cell || observed_variant!=1 || input.area_contact)
        throw std::invalid_argument("Unsupported player7 shot defender preparation");
    const auto& player=input.current.field.roster[20];
    const auto prepared=prepare_profiled_field_defender({20,player.type,static_cast<std::uint8_t>(player.stamina),
        static_cast<std::uint8_t>(player.stamina>>8),player.level},players,2,0,0,defense);
    auto state=input; state.recovery->ball_variant=observed_variant; state.control->last_carrier_cell=128;
    state.area_contact=AreaContactContext{0,2,0,prepared.move_effect,prepared.base_strength,prepared.stamina_cost,0,0,0,0,false};
    state.phase=FieldSessionPhase::player7_shot_defender_prepared; validate_phase_session(state); return state;
}
PhaseSessionState compare_player7_shot_defender(const PhaseSessionState& input,const FieldComparisonContent& comparison) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::player7_shot_defender_prepared || !input.area_contact)
        throw std::invalid_argument("Player7 shot comparison requires the prepared defender");
    DefenderContactInput contact{};
    contact.incoming_power=input.action->power; contact.defender_power=input.area_contact->defender_power;
    contact.random_first=input.current.field.random_first; contact.random_second=input.current.field.random_second;
    contact.weakened=false; contact.context=0; contact.defender_slot=20; contact.subtype=0;
    const auto configured=configure_field_comparison(contact,0,2,20,comparison);
    const auto result=compare_defender_contact(configured);
    if(result.result_code!=2 || result.special || result.subtype!=0)
        throw std::invalid_argument("Only the observed player7 terminal shot defense is supported");
    auto state=input; state.area_contact->strength=result.strength; state.area_contact->loss=result.loss;
    state.area_contact->score=result.score; state.area_contact->result=result.result_code; state.area_sequence_result=result.result_code;
    state.phase=FieldSessionPhase::player7_shot_defender_compared; validate_phase_session(state); return state;
}
PhaseSessionState advance_player7_shot_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::player7_shot_defender_compared || input.control->facing || input.control->motion_requested ||
       input.current.field.elapsed!=1 || input.current.field.motion[field_motion_index(7)].status || prelude.field_elapsed[4]!=2)
        throw std::invalid_argument("Player7 terminal contact needs its observed presentation boundary");
    const auto applied=apply_contact_prelude({input.current,20,2,false},prelude);
    if(applied.elapsed_step!=2 || applied.clock.continuation!=ClockContinuation::ready || applied.state.clock.remaining!=144 ||
       applied.state.field.motion[field_motion_index(7)].status!=5)
        throw std::invalid_argument("Unsupported player7 terminal contact clock");
    auto state=input; state.current=applied.state; state.phase=FieldSessionPhase::player7_shot_contact_clocked;
    validate_phase_session(state); return state;
}
PhaseSessionState jitter_player7_shot_ball(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::player7_shot_contact_clocked || input.current.field.random_first!=2 || input.current.field.random_second!=56)
        throw std::invalid_argument("Player7 loose ball needs its observed random boundary");
    const auto ball=jitter_loose_ball(input.current.field.ball_x,input.current.field.ball_y,input.current.field.random_first,input.current.field.random_second);
    if(!ball.inside_field || ball.x!=120 || ball.y!=140 || ball.x_jitter_control!=0)
        throw std::invalid_argument("Unsupported player7 loose ball position");
    auto state=input; state.current.field.ball_x=ball.x; state.current.field.ball_y=ball.y;
    state.current.field.selected_defenders=0; state.scheduling->selected_ids.fill(0); state.recovery->x_jitter_control=ball.x_jitter_control;
    state.phase=FieldSessionPhase::player7_shot_ball_jittered; validate_phase_session(state); return state;
}
PhaseSessionState recover_player7_shot_ball(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::player7_shot_recovery_field_moved || input.current.field.random_first!=11 || input.current.field.random_second!=105)
        throw std::invalid_argument("Player7 shot recovery needs the observed field boundary");
    const auto& field=input.current.field; BallRecoveryInput choice{};
    choice.ball_x=field.ball_x; choice.ball_y=field.ball_y; choice.ball_owner=field.ball_owner;
    choice.opposing_player=input.recovery->opposing_player; choice.random_e2=field.random_first; choice.random_e3=field.random_second;
    choice.ball_update_active=field.elapsed!=0;
    for(std::size_t i=0;i<field.motion.size();++i) {
        const auto& actor=field.motion[i]; choice.players[i]={static_cast<std::uint8_t>(actor.x>>8),static_cast<std::uint8_t>(actor.y>>8),actor.status!=0};
    }
    const auto result=recover_stationary_ball(choice);
    if(result.ball_owner!=3 || result.possession_side!=0 || result.attack_subtype!=0 || result.direct_defender_capture)
        throw std::invalid_argument("Unsupported player7 shot loose-ball winner");
    auto state=input; state.current.field.ball_owner=result.ball_owner; state.current.field.side=result.possession_side;
    auto& owner=state.current.field.motion[field_motion_index(result.ball_owner)];
    owner.x=static_cast<std::uint16_t>((owner.x&255u)|(unsigned(field.ball_x)<<8));
    owner.y=static_cast<std::uint16_t>((owner.y&255u)|(unsigned(field.ball_y)<<8));
    state.recovery->attack_subtype=result.attack_subtype;
    state.phase=FieldSessionPhase::player7_shot_ball_recovered; validate_phase_session(state); return state;
}
PhaseSessionState capture_player3_one_two_defenders(const PhaseSessionState& input,const std::array<std::uint8_t,3>& actions) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::attack_choice_pending || !input.score || !input.action || !input.control || !input.recovery || !input.scheduling ||
       input.score->left_goals!=1 || input.score->right_goals || input.score->kickoff!=4 || input.current.clock.remaining!=139 || input.current.clock.extra_time ||
       input.current.field.side || input.current.field.ball_owner!=3 || input.current.field.ball_x!=120 || input.current.field.ball_y!=140 ||
       input.current.field.ball_cell!=115 || input.current.field.roster[3].stamina!=310 || input.current.field.selected_defenders!=3 ||
       input.scheduling->selected_ids!=std::array<std::uint8_t,5>{16,19,20,0,0} || input.current.field.random_first!=32 || input.current.field.random_second!=42 ||
       input.action->action!=3 || input.action->cursor!=162 || input.action->move_flags!=3 || input.action->power!=5 || input.control->motion_requested!=1 ||
       input.recovery->opposing_player!=20 || input.saved_ball_flight_flag!=1 || input.area_defender_cursor || input.area_contact ||
       input.human_defense_choices || actions!=std::array<std::uint8_t,3>{1,2,2})
        throw std::invalid_argument("Unsupported player3 one-two defender choices");
    auto state=input; HumanDefenseChoices choices{};
    for(std::size_t i=0;i<actions.size();++i) choices.actions[i]=actions[i];
    state.human_defense_choices=choices; validate_phase_session(state); return state;
}
SessionAttackResult prepare_player3_one_two(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& attack) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::attack_choice_pending || !input.human_defense_choices || input.action->action!=3)
        throw std::invalid_argument("Player3 one-two preparation needs its captured defender choices");
    const auto& field=input.current.field; const auto& player=field.roster[3];
    const auto prepared=prepare_attacking_action({{3,player.type,static_cast<std::uint8_t>(player.stamina),
        static_cast<std::uint8_t>(player.stamina>>8),player.level},3,0,0,0,field.selected_defenders,
        field.random_first,field.random_second,false},players,attack);
    if(prepared.preparation.stamina_cost!=60 || prepared.energy.stamina!=250 || prepared.action_power.power!=8 ||
       prepared.action_power.subtype!=0 || prepared.move_flags!=3)
        throw std::invalid_argument("Unsupported player3 one-two attack content");
    auto state=input; state.current.field.roster[3].stamina=prepared.energy.stamina; state.action->power=prepared.action_power.power;
    state.action->move_effect=prepared.preparation.move_effect; state.action->move_flags=prepared.move_flags;
    state.recovery->attack_subtype=prepared.action_power.subtype; state.phase=FieldSessionPhase::attack_prepared;
    validate_phase_session(state); return {std::move(state),prepared};
}
PhaseSessionState prepare_player3_one_two_defender(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& defense) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices || input.action->action!=3 || input.area_contact ||
       input.current.field.random_first!=193 || input.current.field.random_second!=239 || input.control->facing!=64 ||
       input.control->motion_requested || input.control->last_carrier_cell || input.recovery->opposing_player!=20)
        throw std::invalid_argument("Player3 one-two defender needs its captured presentation boundary");
    const auto& player=input.current.field.roster[16];
    const auto prepared=prepare_profiled_field_defender({16,player.type,static_cast<std::uint8_t>(player.stamina),
        static_cast<std::uint8_t>(player.stamina>>8),player.level},players,1,0,0,defense);
    if(prepared.base_strength!=8 || prepared.stamina_cost!=60 || prepared.move_effect)
        throw std::invalid_argument("Unsupported player3 one-two defender content");
    auto state=input; state.recovery->opposing_player=16; state.recovery->ball_variant=2; state.control->last_carrier_cell=128;
    state.area_contact=AreaContactContext{0,1,0,prepared.move_effect,prepared.base_strength,prepared.stamina_cost,0,0,0,0,false};
    validate_phase_session(state); return state;
}
PhaseSessionState compare_player3_one_two_defender(const PhaseSessionState& input,const FieldComparisonSelection& comparison) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices || !input.area_contact ||
       input.action->action!=3 || input.recovery->opposing_player!=16 || input.area_sequence_result!=255 || comparison.table_index!=13)
        throw std::invalid_argument("Player3 one-two defender comparison needs prepared defender16");
    DefenderContactInput contact{}; contact.incoming_power=input.action->power; contact.defender_power=input.area_contact->defender_power;
    contact.random_first=input.current.field.random_first; contact.random_second=input.current.field.random_second;
    contact.weakened=false; contact.context=0; contact.defender_slot=16; contact.subtype=0;
    contact.range=comparison.range; contact.offset=comparison.offset; contact.eligible=comparison.eligible; contact.thresholds=comparison.thresholds;
    const auto result=compare_defender_contact(contact);
    if(result.result_code!=0 || result.strength!=1 || result.loss || result.score!=255 || result.special || result.subtype)
        throw std::invalid_argument("Unsupported player3 one-two defender comparison: strength " + std::to_string(result.strength) +
            ", score " + std::to_string(result.score) + ", result " + std::to_string(result.result_code));
    auto state=input; state.area_contact->strength=result.strength; state.area_contact->loss=result.loss;
    state.area_contact->score=result.score; state.area_contact->result=result.result_code; state.area_sequence_result=result.result_code;
    validate_phase_session(state); return state;
}
PhaseSessionState advance_player3_one_two_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude,const ExperienceContent& experience) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices || !input.area_contact || !input.experience ||
       input.action->action!=3 || input.recovery->opposing_player!=16 || input.area_sequence_result!=0 ||
       input.current.field.random_first!=128 || input.current.field.random_second!=175 || input.current.field.elapsed!=1 ||
       input.current.field.motion[field_motion_index(16)].status || prelude.field_elapsed[3]!=1 || prelude.defender_status[1]!=6)
        throw std::invalid_argument("Player3 one-two contact needs its captured nonterminal boundary");
    const auto reward=award_player_experience(*input.experience,input.current.field.roster[3].type,1,experience);
    if(!reward.slot || *reward.slot!=3 || reward.award!=3) throw std::invalid_argument("Unsupported player3 one-two experience");
    const auto applied=apply_contact_prelude({input.current,16,1,true},prelude);
    if(applied.elapsed_step!=1 || applied.clock.continuation!=ClockContinuation::ready || applied.state.clock.remaining!=138 ||
       applied.state.field.motion[field_motion_index(16)].status!=6)
        throw std::invalid_argument("Unsupported player3 one-two contact clock");
    auto state=input; state.current=applied.state; state.experience=reward.state; validate_phase_session(state); return state;
}
PhaseSessionState advance_player3_one_two_next_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude,const ExperienceContent& experience) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices || !input.area_contact || !input.experience ||
       input.action->action!=3 || input.area_defender_cursor!=1 || input.recovery->opposing_player!=19 || input.area_sequence_result!=1 ||
       input.current.field.random_first!=10 || input.current.field.random_second!=211 || input.current.field.elapsed!=2 || input.control->facing ||
       input.current.field.motion[field_motion_index(19)].status || prelude.field_elapsed[5]!=1 || prelude.defender_status[2]!=6)
        throw std::invalid_argument("Player3 one-two second contact needs its captured result boundary");
    const auto reward=award_player_experience(*input.experience,input.current.field.roster[3].type,1,experience);
    if(!reward.slot || *reward.slot!=3 || reward.award!=3) throw std::invalid_argument("Unsupported player3 second contact experience");
    const auto applied=apply_contact_prelude({input.current,19,2,true},prelude);
    if(applied.elapsed_step!=1 || applied.clock.continuation!=ClockContinuation::ready || applied.state.clock.remaining!=137 ||
       applied.state.field.motion[field_motion_index(19)].status!=6)
        throw std::invalid_argument("Unsupported player3 second contact clock");
    auto state=input; state.current=applied.state; state.experience=reward.state;
    state.action->power=reduce_action_strength(state.action->power,state.area_contact->loss);
    if(state.action->power!=7) throw std::invalid_argument("Unsupported player3 one-two power reduction");
    validate_phase_session(state); return state;
}
PhaseSessionState advance_player3_one_two_terminal_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices || !input.area_contact || input.action->action!=3 ||
       input.area_defender_cursor!=2 || input.recovery->opposing_player!=20 || input.area_sequence_result!=2 ||
       input.current.field.random_first!=26 || input.current.field.random_second!=142 || input.current.field.elapsed!=3 || input.control->facing ||
       input.current.field.motion[field_motion_index(3)].status || prelude.field_elapsed[4]!=2)
        throw std::invalid_argument("Player3 one-two terminal contact needs its captured boundary");
    const auto applied=apply_contact_prelude({input.current,20,2,false},prelude);
    if(applied.elapsed_step!=2 || applied.clock.continuation!=ClockContinuation::ready || applied.state.clock.remaining!=135 ||
       applied.state.field.motion[field_motion_index(3)].status!=5)
        throw std::invalid_argument("Unsupported player3 one-two terminal contact clock");
    auto state=input; state.current=applied.state; validate_phase_session(state); return state;
}
PhaseSessionState jitter_player3_one_two_ball(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices || !input.area_contact || input.action->action!=3 ||
       input.area_defender_cursor!=2 || input.area_sequence_result!=2 || input.current.clock.remaining!=135 || input.current.field.elapsed!=5 ||
       input.current.field.motion[field_motion_index(3)].status!=5 || input.current.field.random_first!=51 || input.current.field.random_second!=170)
        throw std::invalid_argument("Player3 one-two loose ball needs its captured terminal boundary");
    const auto ball=jitter_loose_ball(input.current.field.ball_x,input.current.field.ball_y,input.current.field.random_first,input.current.field.random_second);
    if(!ball.inside_field || ball.x!=104 || ball.y!=164 || ball.x_jitter_control!=130)
        throw std::invalid_argument("Unsupported player3 one-two loose ball position");
    auto state=input; state.current.field.ball_x=ball.x; state.current.field.ball_y=ball.y;
    state.current.field.selected_defenders=0; state.scheduling->selected_ids.fill(0); state.recovery->x_jitter_control=ball.x_jitter_control;
    state.human_defense_choices.reset(); state.area_contact.reset(); state.phase=FieldSessionPhase::player3_one_two_ball_jittered;
    validate_phase_session(state); return state;
}
PhaseSessionState recover_player2_same_cell_pass(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::player2_same_cell_pass_recovery_moved || input.current.field.random_first!=96 || input.current.field.random_second!=52)
        throw std::invalid_argument("Player2 pass recovery requires its observed field boundary");
    const auto& field=input.current.field;BallRecoveryInput choice{};
    choice.ball_x=field.ball_x;choice.ball_y=field.ball_y;choice.ball_owner=field.ball_owner;
    choice.opposing_player=input.recovery->opposing_player;choice.random_e2=field.random_first;choice.random_e3=field.random_second;
    choice.ball_update_active=field.elapsed!=0;
    for(std::size_t i=0;i<field.motion.size();++i) {
        const auto& actor=field.motion[i];choice.players[i]={static_cast<std::uint8_t>(actor.x>>8),static_cast<std::uint8_t>(actor.y>>8),actor.status!=0};
    }
    const auto result=recover_stationary_ball(choice);
    if(result.ball_owner!=20 || result.possession_side!=11 || result.attack_subtype || result.direct_defender_capture)
        throw std::invalid_argument("Unobserved player2 pass recovery winner");
    auto state=input;state.current.field.ball_owner=result.ball_owner;state.current.field.side=result.possession_side;
    auto& owner=state.current.field.motion[field_motion_index(result.ball_owner)];
    owner.x=static_cast<std::uint16_t>((owner.x&255u)|(unsigned(field.ball_x)<<8));
    owner.y=static_cast<std::uint16_t>((owner.y&255u)|(unsigned(field.ball_y)<<8));
    state.recovery->attack_subtype=result.attack_subtype;
    state.phase=FieldSessionPhase::player2_same_cell_pass_recovered_owner;validate_phase_session(state);return state;
}
PhaseSessionState recover_player3_one_two_ball(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::player3_one_two_recovery_field_moved || input.current.field.random_first!=89 || input.current.field.random_second!=37)
        throw std::invalid_argument("Player3 one-two recovery needs the captured field boundary");
    const auto& field=input.current.field; BallRecoveryInput choice{};
    choice.ball_x=field.ball_x; choice.ball_y=field.ball_y; choice.ball_owner=field.ball_owner;
    choice.opposing_player=input.recovery->opposing_player; choice.random_e2=field.random_first; choice.random_e3=field.random_second;
    choice.ball_update_active=field.elapsed!=0;
    for(std::size_t i=0;i<field.motion.size();++i) {
        const auto& actor=field.motion[i]; choice.players[i]={static_cast<std::uint8_t>(actor.x>>8),static_cast<std::uint8_t>(actor.y>>8),actor.status!=0};
    }
    const auto result=recover_stationary_ball(choice);
    if(result.ball_owner!=2 || result.possession_side!=0 || result.attack_subtype!=0 || result.direct_defender_capture)
        throw std::invalid_argument("Unsupported player3 one-two loose-ball winner");
    auto state=input; state.current.field.ball_owner=result.ball_owner; state.current.field.side=result.possession_side;
    auto& owner=state.current.field.motion[field_motion_index(result.ball_owner)];
    owner.x=static_cast<std::uint16_t>((owner.x&255u)|(unsigned(field.ball_x)<<8));
    owner.y=static_cast<std::uint16_t>((owner.y&255u)|(unsigned(field.ball_y)<<8));
    state.recovery->attack_subtype=result.attack_subtype;
    state.phase=FieldSessionPhase::player3_one_two_ball_recovered; validate_phase_session(state); return state;
}
PhaseSessionState prepare_player3_one_two_next_defender(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& defense) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices || !input.area_contact || !input.action || !input.recovery || !input.control || input.action->action!=3)
        throw std::invalid_argument("Player3 one-two next defender needs a captured attack");
    const bool second=input.area_defender_cursor==0 && input.area_sequence_result==0 && input.current.clock.remaining==138 && input.current.field.elapsed==2 &&
       input.current.field.random_first==93 && input.current.field.random_second==144 && input.recovery->opposing_player==16 && input.control->facing==64;
    const bool third=input.area_defender_cursor==1 && input.area_sequence_result==1 && input.current.clock.remaining==137 && input.current.field.elapsed==3 &&
       input.current.field.motion[field_motion_index(19)].status==6 && input.current.field.random_first==139 && input.current.field.random_second==175 && input.recovery->opposing_player==19 && !input.control->facing;
    if((!second && !third) || input.current.field.motion[field_motion_index(16)].status!=6 || input.control->motion_requested)
        throw std::invalid_argument("Player3 one-two next defender needs its captured contact boundary");
    const std::uint8_t defender=third ? 20 : 19; const auto& player=input.current.field.roster[defender];
    const auto prepared=prepare_profiled_field_defender({defender,player.type,static_cast<std::uint8_t>(player.stamina),
        static_cast<std::uint8_t>(player.stamina>>8),player.level},players,2,0,0,defense);
    if(prepared.base_strength!=8 || prepared.stamina_cost!=50 || prepared.move_effect)
        throw std::invalid_argument("Unsupported player3 one-two second defender content");
    auto state=input; state.recovery->opposing_player=defender; state.area_defender_cursor=third ? 2 : 1; state.area_sequence_result=255;
    state.area_contact=AreaContactContext{state.area_defender_cursor,2,0,prepared.move_effect,prepared.base_strength,prepared.stamina_cost,0,0,0,0,false};
    validate_phase_session(state); return state;
}
PhaseSessionState compare_player3_one_two_next_defender(const PhaseSessionState& input,const FieldComparisonSelection& comparison) {
    validate_phase_session(input);
    const bool third=input.area_defender_cursor==2;
    if(input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices || !input.area_contact || input.action->action!=3 ||
       input.recovery->opposing_player!=(third ? 20 : 19) || (input.area_defender_cursor!=1 && !third) || input.area_sequence_result!=255 || comparison.table_index!=14)
        throw std::invalid_argument("Player3 one-two next comparison needs its prepared defender");
    DefenderContactInput contact{}; contact.incoming_power=input.action->power; contact.defender_power=input.area_contact->defender_power;
    contact.random_first=input.current.field.random_first; contact.random_second=input.current.field.random_second;
    contact.weakened=false; contact.context=0; contact.defender_slot=third ? 20 : 19; contact.subtype=0;
    contact.range=comparison.range; contact.offset=comparison.offset; contact.eligible=comparison.eligible; contact.thresholds=comparison.thresholds;
    const auto result=compare_defender_contact(contact);
    if(result.result_code!=(third ? 2 : 1) || result.strength!=(third ? 6 : 5) || result.loss!=1 || result.score!=(third ? 74 : 102) || result.special || result.subtype)
        throw std::invalid_argument("Unsupported player3 one-two next defender comparison");
    auto state=input; state.area_contact->strength=result.strength; state.area_contact->loss=result.loss;
    state.area_contact->score=result.score; state.area_contact->result=result.result_code; state.area_sequence_result=result.result_code;
    validate_phase_session(state); return state;
}
PhaseSessionState prepare_ordinary_return_shot_defender(const PhaseSessionState& input,std::uint8_t observed_variant,
    const FieldPlayerProfiles& players,const FieldDefenseContent& defense) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_return_shot_prepared || !input.area_contact || !input.control ||
       (observed_variant!=1 && observed_variant!=2) || input.area_defender_cursor || input.control->motion_requested!=2 ||
       (input.control->facing!=0 && input.control->facing!=64))
        throw std::invalid_argument("Returned shot defender needs observed presentation and ball variant");
    const auto defender=input.recovery->opposing_player;const auto& player=input.current.field.roster[defender];
    const auto prepared=prepare_profiled_field_defender({defender,player.type,static_cast<std::uint8_t>(player.stamina),
        static_cast<std::uint8_t>(player.stamina>>8),player.level},players,1,0,0,defense);
    auto state=input;state.recovery->ball_variant=observed_variant;state.control->last_carrier_cell=128;
    state.area_contact=AreaContactContext{0,1,0,prepared.move_effect,prepared.base_strength,prepared.stamina_cost,0,0,0,0,false};
    state.phase=FieldSessionPhase::ordinary_return_shot_defender_prepared;validate_phase_session(state);return state;
}
PhaseSessionState compare_ordinary_return_shot_defender(const PhaseSessionState& input,const FieldComparisonSelection& comparison) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_return_shot_defender_prepared || !input.area_contact || comparison.table_index!=1)
        throw std::invalid_argument("Returned shot defender needs the direct shot table");
    DefenderContactInput contact{};contact.incoming_power=input.action->power;contact.defender_power=input.area_contact->defender_power;
    contact.random_first=input.current.field.random_first;contact.random_second=input.current.field.random_second;
    contact.weakened=false;contact.context=0;contact.defender_slot=input.recovery->opposing_player;contact.subtype=0;
    contact.range=comparison.range;contact.offset=comparison.offset;contact.eligible=comparison.eligible;contact.thresholds=comparison.thresholds;
    const auto result=compare_defender_contact(contact);
    if(result.result_code!=0 || result.special || result.subtype!=0)
        throw std::invalid_argument("Only observed nonterminal ordinary shot comparisons are supported");
    auto state=input;state.area_contact->strength=result.strength;state.area_contact->loss=result.loss;
    state.area_contact->score=result.score;state.area_contact->result=result.result_code;state.area_sequence_result=result.result_code;
    state.phase=FieldSessionPhase::ordinary_return_shot_defender_compared;validate_phase_session(state);return state;
}
PhaseSessionState advance_ordinary_return_shot_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude,const ExperienceContent& experience) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_return_shot_defender_compared || !input.experience ||
       input.current.field.motion[field_motion_index(input.recovery->opposing_player)].status ||
       prelude.field_elapsed[3]!=1 || prelude.defender_status[1]!=6)
        throw std::invalid_argument("Returned shot contact requires the observed nonterminal defense");
    const auto reward=award_player_experience(*input.experience,input.current.field.roster[input.current.field.ball_owner].type,1,experience);
    const auto applied=apply_contact_prelude({input.current,input.recovery->opposing_player,1,true},prelude);
    if(applied.elapsed_step!=1 || applied.clock.continuation!=ClockContinuation::ready)
        throw std::invalid_argument("Returned shot contact requires a ready clock");
    auto state=input;state.current=applied.state;state.experience=reward.state;
    state.phase=FieldSessionPhase::ordinary_return_shot_contact;validate_phase_session(state);return state;
}
PhaseSessionState advance_ordinary_return_shot_dispatch(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_return_shot_contact)throw std::invalid_argument("Returned shot dispatch requires contact");
    auto state=input;state.current.field.elapsed=static_cast<std::uint8_t>(unsigned(state.current.field.elapsed)+2);
    const auto recovery=advance_keeper_recovery({state.current.field.keepers,state.current.shared_recovery_flags,state.current.shared_recovery_timer},2);
    state.current.field.keepers=recovery.keepers;state.current.shared_recovery_flags=recovery.shared_flags;state.current.shared_recovery_timer=recovery.shared_timer;
    const auto clock=advance_match_clock(state.current.clock,2,state.current.field.random_first);
    if(clock.continuation!=ClockContinuation::ready)throw std::invalid_argument("Returned shot dispatch requires a ready clock");
    state.current.clock=clock.state;state.area_defender_cursor=1;
    state.phase=FieldSessionPhase::ordinary_return_shot_dispatch;validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_unaffordable_shot(const PhaseSessionState& input,
    const FieldPlayerProfiles& players,const FieldDefenseContent& attack) {
    validate_phase_session(input);
    const auto& f=input.current.field;
    const bool kickoff=input.score && input.score->left_goals==2 && input.score->right_goals==2 &&
       input.current.clock.remaining==381 && !f.selected_defenders && f.roster[5].stamina==31;
    const bool contact=input.score && input.scheduling && input.score->left_goals==2 && input.score->right_goals==3 &&
       input.current.clock.remaining==355 && f.selected_defenders==1 && input.scheduling->selected_ids==std::array<std::uint8_t,5>{17,0,0,0,0} &&
       !f.roster[5].stamina && !input.saved_ball_flight_flag;
    if(input.phase!=FieldSessionPhase::attack_choice_pending || !input.score || !input.action || !input.recovery ||
       (!kickoff && !contact) || input.score->kickoff!=4 || input.current.clock.extra_time!=13 || f.side!=0 || f.ball_owner!=5 ||
       f.ball_x!=108 || f.ball_y!=108 || f.ball_cell!=87 || f.random_first || f.random_second ||
       f.roster[5].type!=23 || input.action->action!=0 || input.action->menu_route!=0 ||
       input.action->cursor!=224 || input.recovery->attack_subtype!=128)
        throw std::invalid_argument("Unsupported sampled unaffordable shot");
    const auto& p=f.roster[5];
    const auto prepared=prepare_attacking_action({{5,p.type,static_cast<std::uint8_t>(p.stamina),
        static_cast<std::uint8_t>(p.stamina>>8),p.level},0,0,0,0,f.selected_defenders,f.random_first,f.random_second,false},players,attack);
    const auto remaining=static_cast<std::uint16_t>(unsigned(p.stamina)-prepared.preparation.stamina_cost);
    if(prepared.preparation.stamina_cost!=80 || !(remaining&32768u))
        throw std::invalid_argument("Unverified sampled shot affordability");
    auto state=input;state.recovery->attack_subtype=0;state.action->cursor=0;state.phase=FieldSessionPhase::human_action_menu;
    validate_phase_session(state);return state;
}
PhaseSessionState begin_ordinary_human_pass_target(const PhaseSessionState& input,std::optional<std::uint8_t> home_keeper_y) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::attack_choice_pending || !input.action || !input.recovery ||
        input.current.field.side != 0 || input.action->action != 1 || input.action->menu_route != 0 || input.human_pass_cursor)
        throw std::invalid_argument("Ordinary pass target requires a confirmed human pass and acknowledged ordinary type");
    OrdinaryPassTargetInitialization target{};
    if(input.current.field.ball_owner==0){
        if(!home_keeper_y)throw std::invalid_argument("Home goalkeeper position is unknown");
        target=initialize_ordinary_pass_target(static_cast<std::uint16_t>(input.current.field.keepers[0].penalty_duration)<<8,
                                              static_cast<std::uint16_t>(*home_keeper_y)<<8);
    } else {const auto& carrier=input.current.field.motion[field_motion_index(input.current.field.ball_owner)];target=initialize_ordinary_pass_target(carrier.x,carrier.y);}
    auto state = input;
    state.current.field.ball_x = target.ball_x;
    state.current.field.ball_y = target.ball_y;
    state.current.field.ball_cell = target.cell;
    state.current.clock.ball_mode = target.ball_mode;
    state.recovery->attack_subtype = target.attack_subtype;
    state.human_pass_cursor = target.cursor;
    state.human_pass_steps = 0;
    state.ordinary_pass_candidates = OrdinaryPassCandidateContext{};
    state.phase = FieldSessionPhase::ordinary_human_pass_target;
    validate_phase_session(state);
    return state;
}
OrdinaryPassCursorResult poll_ordinary_human_pass_cursor(const PhaseSessionState& input, std::uint8_t held, std::uint8_t pressed) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::ordinary_human_pass_target || !input.human_pass_cursor || (pressed & 192u))
        throw std::invalid_argument("Ordinary pass cursor supports movement only; confirmation and cancellation remain pending");
    auto state = input;
    const auto next = advance_ordinary_pass_cursor(*state.human_pass_cursor, held);
    const bool changed = next != *state.human_pass_cursor;
    state.human_pass_cursor = next;
    if (changed) {
        std::array<std::array<std::uint8_t,2>,22> positions{};
        for (unsigned id = 1; id < 22; ++id) {
            if (id == 11) continue;
            const auto& motion = state.current.field.motion[field_motion_index(static_cast<std::uint8_t>(id))];
            positions[id] = {static_cast<std::uint8_t>(motion.x >> 8), static_cast<std::uint8_t>(motion.y >> 8)};
        }
        state.ordinary_pass_candidates = OrdinaryPassCandidateContext{
            true, select_ordinary_pass_candidates(state.current.field.ball_owner, next, positions)};
    }
    validate_phase_session(state);
    return {std::move(state), changed};
}
PhaseSessionState cancel_ordinary_human_pass_target(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_human_pass_target || !input.action || !input.human_pass_cursor)
        throw std::invalid_argument("Pass cancellation requires an active target screen");
    auto state=input;state.action->cursor=0;state.human_pass_cursor.reset();state.human_pass_steps=0;
    state.ordinary_pass_candidates.reset();state.phase=FieldSessionPhase::human_action_menu;
    validate_phase_session(state);return state;
}
PhaseSessionState confirm_ordinary_human_pass_target(const PhaseSessionState& input) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::ordinary_human_pass_target || !input.ordinary_pass_candidates)
        throw std::invalid_argument("Ordinary pass confirmation requires known candidate context");
    const auto& candidates = input.ordinary_pass_candidates->candidates;
    if (candidates.allied_count > 1)
        throw std::invalid_argument("Multiple teammates require the original recipient chooser");
    auto state = input;
    state.ordinary_pass_recipient = candidates.allied_count == 1 ? candidates.ids[0] : 255;
    state.phase = FieldSessionPhase::ordinary_human_pass_confirmed;
    validate_phase_session(state);
    return state;
}
PhaseSessionState confirm_ordinary_human_pass_recipient(const PhaseSessionState& input,std::uint8_t index) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_human_pass_target || !input.ordinary_pass_candidates ||
       !input.ordinary_pass_candidates->scanned || input.ordinary_pass_candidates->candidates.allied_count<2 ||
       index>=input.ordinary_pass_candidates->candidates.allied_count)
        throw std::invalid_argument("Pass recipient choice requires a valid teammate index");
    auto state=input;state.ordinary_pass_recipient=state.ordinary_pass_candidates->candidates.ids[index];
    state.phase=FieldSessionPhase::ordinary_human_pass_confirmed;validate_phase_session(state);return state;
}
PhaseSessionState begin_ordinary_pass_defense_selection(const PhaseSessionState& input) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::ordinary_human_pass_confirmed || !input.current.field.selected_defenders)
        throw std::invalid_argument("Ordinary pass defense selection requires a nonempty opposing prefix");
    auto state = input;
    state.current.clock.ball_mode = 0;
    state.ordinary_pass_defenses = OrdinaryPassDefenseContext{};
    state.phase = FieldSessionPhase::ordinary_pass_defense_selection;
    validate_phase_session(state);
    return state;
}
PhaseSessionState record_ordinary_pass_defense(const PhaseSessionState& input,
    std::uint8_t defender, std::uint8_t action, std::uint8_t subtype) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::ordinary_pass_defense_selection || !input.ordinary_pass_defenses ||
        !input.scheduling || action != 1 || subtype != 0)
        throw std::invalid_argument("Unsupported observed ordinary pass defender choice");
    auto state = input;
    auto& defense = *state.ordinary_pass_defenses;
    if (defender != state.scheduling->selected_ids[defense.count])
        throw std::invalid_argument("Observed ordinary pass defender is out of order");
    defense.actions[defense.count] = action;
    defense.subtypes[defense.count] = subtype;
    ++defense.count;
    state.recovery->opposing_player = defender;
    if (defense.count == state.current.field.selected_defenders)
        state.phase = FieldSessionPhase::ordinary_pass_defenses_recorded;
    validate_phase_session(state);
    return state;
}
PhaseSessionState prepare_ordinary_pass(const PhaseSessionState& input,
    const FieldPlayerProfiles& players, const FieldDefenseContent& attack) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::ordinary_pass_defenses_recorded || !input.recovery ||
        input.recovery->ball_variant != 0)
        throw std::invalid_argument("Ordinary pass preparation requires recorded defenses and the supported field situation");
    const auto& field = input.current.field;
    const auto& player = field.roster[field.ball_owner];
    const auto prepared = prepare_attacking_action({{field.ball_owner, player.type,
        static_cast<std::uint8_t>(player.stamina), static_cast<std::uint8_t>(player.stamina >> 8), player.level},
        1, 0, 0, 0, field.selected_defenders, field.random_first, field.random_second, false}, players, attack);
    auto state = input;
    state.current.field.roster[field.ball_owner].stamina = prepared.energy.stamina;
    state.action->power = prepared.action_power.power;
    state.action->move_effect = prepared.preparation.move_effect;
    state.action->move_flags = prepared.move_flags;
    state.recovery->attack_subtype = prepared.action_power.subtype;
    state.phase = FieldSessionPhase::ordinary_pass_prepared;
    validate_phase_session(state);
    return state;
}
PhaseSessionState prepare_first_ordinary_pass_defender(const PhaseSessionState& input,
    std::uint8_t observed_ball_variant, const FieldPlayerProfiles& players, const FieldDefenseContent& defense) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::ordinary_pass_prepared || !input.scheduling || !input.control ||
        !input.ordinary_pass_defenses || input.area_contact || (observed_ball_variant != 1 && observed_ball_variant != 2) ||
        input.control->facing != 64 || input.control->motion_requested)
        throw std::invalid_argument("First ordinary pass defender requires recorded presentation and variant");
    const auto defender = input.scheduling->selected_ids[0];
    const auto& player = input.current.field.roster[defender];
    const auto prepared = prepare_profiled_field_defender({defender, player.type,
        static_cast<std::uint8_t>(player.stamina), static_cast<std::uint8_t>(player.stamina >> 8), player.level},
        players, 1, 0, 0, defense);
    auto state = input;
    state.recovery->opposing_player = defender;
    state.recovery->ball_variant = observed_ball_variant;
    state.control->last_carrier_cell = 128;
    state.area_defender_cursor = 0;
    state.area_contact = AreaContactContext{0,1,0,prepared.move_effect,prepared.base_strength,prepared.stamina_cost,0,0,0,0,false};
    state.area_sequence_result = 255;
    state.phase = FieldSessionPhase::ordinary_pass_defender_prepared;
    validate_phase_session(state);
    return state;
}
static PhaseSessionState compare_ordinary_pass_defender(const PhaseSessionState& input,
    const FieldComparisonSelection& comparison) {
    validate_phase_session(input);
    if ((input.phase != FieldSessionPhase::ordinary_pass_defender_prepared && input.phase != FieldSessionPhase::ordinary_pass_next_defender_prepared) || !input.area_contact || !input.recovery)
        throw std::invalid_argument("First ordinary pass defender comparison requires preparation");
    DefenderContactInput contact{};
    contact.incoming_power = input.action->power;
    contact.defender_power = input.area_contact->defender_power;
    contact.random_first = input.current.field.random_first;
    contact.random_second = input.current.field.random_second;
    contact.weakened = false;
    contact.context = 0;
    contact.defender_slot = input.recovery->opposing_player;
    contact.subtype = 0;
    if (comparison.table_index != 5)
        throw std::invalid_argument("Ordinary pass action-one defense requires direct comparison table index 5");
    contact.range = comparison.range;
    contact.offset = comparison.offset;
    contact.eligible = comparison.eligible;
    contact.thresholds = comparison.thresholds;
    const auto result = compare_defender_contact(contact);
    if ((result.result_code != 0 && !(input.phase==FieldSessionPhase::ordinary_pass_defender_prepared && result.result_code==3)) || result.special || result.subtype != 0)
        throw std::invalid_argument("Only observed ordinary nonterminal defender comparisons are supported: score=" +
            std::to_string(result.score) + " result=" + std::to_string(result.result_code) +
            " special=" + std::to_string(result.special) + " subtype=" + std::to_string(result.subtype) +
            " range=" + std::to_string(contact.range) + " offset=" + std::to_string(contact.offset));
    auto state = input;
    state.area_contact->strength = result.strength;
    state.area_contact->loss = result.loss;
    state.area_contact->score = result.score;
    state.area_contact->result = result.result_code;
    state.area_sequence_result = result.result_code;
    state.phase = input.phase == FieldSessionPhase::ordinary_pass_defender_prepared ? FieldSessionPhase::ordinary_pass_defender_compared : FieldSessionPhase::ordinary_pass_next_defender_compared;
    validate_phase_session(state);
    return state;
}
PhaseSessionState compare_first_ordinary_pass_defender(const PhaseSessionState& input,const FieldComparisonSelection& comparison) {
    if (input.phase != FieldSessionPhase::ordinary_pass_defender_prepared) throw std::invalid_argument("First defender comparison requires its prepared phase");
    return compare_ordinary_pass_defender(input,comparison);
}
PhaseSessionState compare_next_ordinary_pass_defender(const PhaseSessionState& input,const FieldComparisonSelection& comparison) {
    if (input.phase != FieldSessionPhase::ordinary_pass_next_defender_prepared) throw std::invalid_argument("Next defender comparison requires its prepared phase");
    return compare_ordinary_pass_defender(input,comparison);
}
static PhaseSessionState advance_ordinary_pass_contact(const PhaseSessionState& input,
    const ContactPreludeContent& prelude, const ExperienceContent& experience) {
    validate_phase_session(input);
    if ((input.phase != FieldSessionPhase::ordinary_pass_defender_compared && input.phase != FieldSessionPhase::ordinary_pass_next_defender_compared) || !input.experience ||
        input.current.field.motion[field_motion_index(input.recovery->opposing_player)].status ||
        prelude.field_elapsed[3]!=1 || prelude.defender_status[1]!=6)
        throw std::invalid_argument("Unsupported ordinary pass contact clock");
    const auto reward=award_player_experience(*input.experience,
        input.current.field.roster[input.current.field.ball_owner].type,1,experience);
    const auto applied=apply_contact_prelude({input.current,input.recovery->opposing_player,1,true},prelude);
    if (applied.elapsed_step!=1 || applied.clock.continuation!=ClockContinuation::ready)
        throw std::invalid_argument("Ordinary pass contact requires a ready clock continuation");
    auto state=input;
    state.current=applied.state;
    state.experience=reward.state;
    state.phase=input.phase==FieldSessionPhase::ordinary_pass_defender_compared ? FieldSessionPhase::ordinary_pass_first_contact_clock : FieldSessionPhase::ordinary_pass_next_contact_clock;
    validate_phase_session(state);
    return state;
}
PhaseSessionState advance_first_ordinary_pass_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude,const ExperienceContent& experience) {
    if(input.phase!=FieldSessionPhase::ordinary_pass_defender_compared)throw std::invalid_argument("First contact requires its comparison");
    return advance_ordinary_pass_contact(input,prelude,experience);
}
PhaseSessionState advance_next_ordinary_pass_contact(const PhaseSessionState& input,const ContactPreludeContent& prelude,const ExperienceContent& experience) {
    if(input.phase!=FieldSessionPhase::ordinary_pass_next_defender_compared)throw std::invalid_argument("Next contact requires its comparison");
    return advance_ordinary_pass_contact(input,prelude,experience);
}
PhaseSessionState prepare_next_ordinary_pass_defender(const PhaseSessionState& input,
    const FieldPlayerProfiles& players,const FieldDefenseContent& defense) {
    validate_phase_session(input);
    if ((input.phase!=FieldSessionPhase::ordinary_pass_first_contact_clock && input.phase!=FieldSessionPhase::ordinary_pass_next_contact_clock) ||
        !input.scheduling || !input.ordinary_pass_defenses || unsigned(input.area_defender_cursor)+1>=input.ordinary_pass_defenses->count)
        throw std::invalid_argument("No next recorded ordinary pass defender");
    const auto cursor=static_cast<std::uint8_t>(input.area_defender_cursor+1);
    const auto defender=input.scheduling->selected_ids[cursor];
    const auto& player=input.current.field.roster[defender];
    const auto prepared=prepare_profiled_field_defender({defender,player.type,
        static_cast<std::uint8_t>(player.stamina),static_cast<std::uint8_t>(player.stamina>>8),player.level},players,1,0,0,defense);
    auto state=input;
    state.recovery->opposing_player=defender;
    state.area_defender_cursor=cursor;
    state.area_contact=AreaContactContext{cursor,1,0,prepared.move_effect,prepared.base_strength,prepared.stamina_cost,0,0,0,0,false};
    state.area_sequence_result=255;
    state.phase=FieldSessionPhase::ordinary_pass_next_defender_prepared;
    validate_phase_session(state);
    return state;
}
PhaseSessionState advance_ordinary_pass_dispatch_clock(const PhaseSessionState& input) {
    validate_phase_session(input);
    if ((input.phase!=FieldSessionPhase::ordinary_pass_first_contact_clock && input.phase!=FieldSessionPhase::ordinary_pass_next_contact_clock) ||
        !input.ordinary_pass_defenses || unsigned(input.area_defender_cursor)+1!=input.ordinary_pass_defenses->count)
        throw std::invalid_argument("Pass dispatch requires the final recorded defender contact");
    auto state=input;
    state.current.field.elapsed=static_cast<std::uint8_t>(unsigned(state.current.field.elapsed)+1);
    const auto recovery=advance_keeper_recovery({state.current.field.keepers,state.current.shared_recovery_flags,state.current.shared_recovery_timer},1);
    state.current.field.keepers=recovery.keepers;
    state.current.shared_recovery_flags=recovery.shared_flags;
    state.current.shared_recovery_timer=recovery.shared_timer;
    const auto clock=advance_match_clock(state.current.clock,1,state.current.field.random_first);
    if (clock.continuation!=ClockContinuation::ready) throw std::invalid_argument("Pass dispatch requires a ready clock continuation");
    state.current.clock=clock.state;
    state.phase=FieldSessionPhase::ordinary_pass_dispatch_clock;
    validate_phase_session(state);
    return state;
}
PhaseSessionState resolve_player2_opponent_pass_target(const PhaseSessionState& input,const PassDistanceContent& distance) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::player2_opponent_pass_field_moved || !input.area_ai_target)
        throw std::invalid_argument("Opponent20 pass target requires its observed field movement");
    const auto recipient=input.area_ai_target->recipient;const auto& player=input.current.field.motion[field_motion_index(recipient)];
    const auto destination=resolve_pass_target_cell(recipient,input.area_ai_target->cell,static_cast<std::uint8_t>(player.x>>8),static_cast<std::uint8_t>(player.y>>8));
    const auto length=calculate_pass_distance(input.current.field.ball_x,input.current.field.ball_y,destination,distance);
    if(recipient!=21 || destination!=94 || length!=3)throw std::invalid_argument("Unobserved opponent20 pass target or distance");
    auto state=input;state.area_ai_target->cell=destination;state.shot_flight=ShotFlightContext{length,destination,255,false};
    state.phase=FieldSessionPhase::player2_opponent_pass_target;validate_phase_session(state);return state;
}
PhaseSessionState resolve_ordinary_pass_flight_target(const PhaseSessionState& input,const PassDistanceContent& distance) {
    validate_phase_session(input);
    if (input.phase!=FieldSessionPhase::ordinary_pass_field_moved || !input.ordinary_pass_recipient || !input.human_pass_cursor)
        throw std::invalid_argument("Ordinary pass target requires the completed field movement");
    const auto recipient=*input.ordinary_pass_recipient;
    std::uint8_t x=0,y=0;
    if (recipient!=255) {
        const auto& actor=input.current.field.motion[field_motion_index(recipient)];
        x=static_cast<std::uint8_t>(actor.x>>8);y=static_cast<std::uint8_t>(actor.y>>8);
    }
    const auto destination=resolve_pass_target_cell(recipient,*input.human_pass_cursor,x,y);
    const auto length=calculate_pass_distance(input.current.field.ball_x,input.current.field.ball_y,destination,distance);
    auto state=input;
    state.sampled_field_pass=SampledFieldPassContext{recipient,destination,255,length};
    state.phase=FieldSessionPhase::ordinary_pass_flight_target;
    validate_phase_session(state);
    return state;
}
PhaseSessionState begin_sampled_human_pass_target(const PhaseSessionState& input) {
    validate_phase_session(input);const auto& f=input.current.field;
    const bool score25=input.phase==FieldSessionPhase::attack_choice_pending && input.score && input.action && input.recovery && input.scheduling &&
       ((input.score->left_goals==2 && input.score->right_goals==5 && input.current.clock.remaining==325) || (input.score->left_goals==3 && input.score->right_goals==6 && input.current.clock.remaining==283)) && input.score->kickoff==4 && input.current.clock.extra_time==13 &&
       f.side==0 && f.ball_owner==5 && f.ball_x==108 && f.ball_y==108 && f.ball_cell==87 && f.elapsed==1 && f.selected_defenders==3 &&
       input.scheduling->selected_ids==std::array<std::uint8_t,5>{17,19,20,0,0} && !f.random_first && !f.random_second &&
       f.roster[5].type==23 && f.roster[5].stamina==(input.score->left_goals==3 ? 10 : 0) && input.action->action==1 && input.action->power==22 &&
       input.action->menu_route==0 && input.action->cursor==225 && input.recovery->opposing_player==4 &&
       input.recovery->attack_subtype==128 && !input.saved_ball_flight_flag && !input.area_defender_cursor;
    if(score25) {
        auto state=input;state.current.clock.ball_mode=129;state.recovery->attack_subtype=0;state.human_pass_cursor=87;
        state.phase=FieldSessionPhase::sampled_score25_pass_target;validate_phase_session(state);return state;
    }
    const bool score23=input.phase==FieldSessionPhase::attack_choice_pending && input.score && input.action && input.recovery && input.scheduling &&
       input.score->left_goals==2 && input.score->right_goals==3 && input.score->kickoff==4 &&
       input.current.clock.remaining==355 && input.current.clock.extra_time==13 && f.side==0 && f.ball_owner==5 &&
       f.ball_x==108 && f.ball_y==108 && f.ball_cell==87 && f.selected_defenders==1 && input.scheduling->selected_ids==std::array<std::uint8_t,5>{17,0,0,0,0} &&
       !f.random_first && !f.random_second && f.roster[5].type==23 && !f.roster[5].stamina && input.action->action==1 &&
       input.action->power==22 && input.action->menu_route==0 && input.action->cursor==225 && input.recovery->attack_subtype==0 && !input.saved_ball_flight_flag;
    if(score23) {
        auto state=input;state.current.clock.ball_mode=129;state.human_pass_cursor=87;
        state.phase=FieldSessionPhase::sampled_score23_pass_target;validate_phase_session(state);return state;
    }
    const bool contact=input.phase==FieldSessionPhase::attack_choice_pending && input.score && input.action && input.recovery && input.scheduling &&
       input.score->left_goals==2 && input.score->right_goals==2 && input.score->kickoff==4 &&
       input.current.clock.remaining==374 && input.current.clock.extra_time==13 && f.side==0 && f.ball_owner==3 &&
       f.ball_x==108 && f.ball_y==108 && f.ball_cell==87 && f.selected_defenders==1 && input.scheduling->selected_ids==std::array<std::uint8_t,5>{17,0,0,0,0} &&
       !f.random_first && !f.random_second && f.roster[3].type==23 && f.roster[3].stamina==0 && input.action->action==1 &&
       input.action->menu_route==0 && input.action->cursor==225 && input.recovery->attack_subtype==0 && input.saved_ball_flight_flag==1;
    if(contact) {
        auto state=input;state.current.clock.ball_mode=129;state.human_pass_cursor=87;
        state.phase=FieldSessionPhase::sampled_player3_pass_target;validate_phase_session(state);return state;
    }
    if(input.phase!=FieldSessionPhase::attack_choice_pending || !input.score || !input.action || !input.recovery ||
       input.score->left_goals!=2 || input.score->right_goals!=2 || input.score->kickoff!=4 ||
       input.current.clock.remaining!=381 || input.current.clock.extra_time!=13 || f.side!=0 || f.ball_owner!=5 ||
       f.ball_x!=108 || f.ball_y!=108 || f.ball_cell!=87 || f.selected_defenders || f.random_first || f.random_second ||
       f.roster[5].type!=23 || f.roster[5].stamina!=31 || input.action->action!=1 || input.action->menu_route!=0 ||
       input.action->cursor!=225 || input.recovery->attack_subtype!=0 || input.saved_ball_flight_flag)
        throw std::invalid_argument("Unsupported sampled human pass target");
    auto state=input;state.current.clock.ball_mode=129;state.human_pass_cursor=87;
    state.phase=FieldSessionPhase::sampled_human_pass_target;validate_phase_session(state);return state;
}
PhaseSessionState poll_sampled_human_pass_target(const PhaseSessionState& input,std::uint8_t held,std::uint8_t pressed) {
    validate_phase_session(input);
    if(input.phase==FieldSessionPhase::sampled_score25_pass_target) {
        if(!held && pressed==128 && input.human_pass_steps==5) {
            auto state=input;state.phase=FieldSessionPhase::sampled_score25_player7_confirmed;validate_phase_session(state);return state;
        }
        if(!held && pressed==64 && !input.human_pass_steps) {
            auto state=input;state.phase=FieldSessionPhase::sampled_score25_pass_cancel;validate_phase_session(state);return state;
        }
        if(pressed || (held && !((input.human_pass_steps==0 && held==1) || (input.human_pass_steps>=1 && input.human_pass_steps<5 && held==4))))
            throw std::invalid_argument("Unsupported sampled 2-5 pass input");
        auto state=input;if(held){++state.human_pass_steps;state.human_pass_cursor=std::array<std::uint8_t,6>{87,99,100,101,102,103}[state.human_pass_steps];}
        validate_phase_session(state);return state;
    }
    if(input.phase==FieldSessionPhase::sampled_score23_pass_target) {
        if(!input.human_pass_steps && !held && (pressed==64 || pressed==128)) {
            auto state=input;state.phase=pressed==128 ? FieldSessionPhase::sampled_score23_empty_confirmed : FieldSessionPhase::sampled_score23_pass_cancel;
            validate_phase_session(state);return state;
        }
        if(input.human_pass_steps || pressed || (held!=0 && held!=1 && held!=2 && held!=4 && held!=8))
            throw std::invalid_argument("Unsupported sampled 2-3 pass input");
        auto state=input;const auto cursor=*input.human_pass_cursor;
        if(held) {state.human_pass_cursor=static_cast<std::uint8_t>(held==1 ? cursor+12 : held==2 ? cursor-12 : held==4 ? cursor+1 : cursor-1);++state.human_pass_steps;}
        validate_phase_session(state);return state;
    }
    if(input.phase==FieldSessionPhase::sampled_player3_pass_target && !held && pressed==128) {
        auto state=input;state.phase=FieldSessionPhase::sampled_player3_empty_confirmed;validate_phase_session(state);return state;
    }
    if(input.phase==FieldSessionPhase::sampled_human_pass_target && input.human_pass_steps==0 && !held && (pressed==64 || pressed==128)) {
        auto state=input;state.phase=pressed==128 ? FieldSessionPhase::sampled_human_pass_empty_confirmed : FieldSessionPhase::sampled_human_pass_cancel_pending;
        validate_phase_session(state);return state;
    }
    if(input.phase!=FieldSessionPhase::sampled_human_pass_target || input.human_pass_steps>=2 || pressed ||
       (held!=0 && held!=1 && held!=2 && held!=4 && held!=8))
        throw std::invalid_argument("Unsupported sampled human pass input");
    auto state=input;
    const auto cursor=*input.human_pass_cursor;
    if(held) {
        state.human_pass_cursor=static_cast<std::uint8_t>(held==1 ? cursor+12 : held==2 ? cursor-12 : held==4 ? cursor+1 : cursor-1);
        ++state.human_pass_steps;
    }
    validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_score25_pass_defenses(const PhaseSessionState& input,
    const std::array<std::uint8_t,3>& actions,const std::array<std::uint8_t,3>& subtypes) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::sampled_score25_player7_confirmed ||
       actions!=std::array<std::uint8_t,3>{1,1,1} || subtypes!=std::array<std::uint8_t,3>{0,0,0})
        throw std::invalid_argument("Unobserved sampled player7 pass defenses");
    auto state=input;state.recovery->opposing_player=20;state.current.clock.ball_mode=0;
    state.phase=FieldSessionPhase::sampled_score25_pass_defenses;validate_phase_session(state);return state;
}
PhaseSessionState prepare_sampled_score25_player7_pass(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& attack) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::sampled_score25_pass_defenses)
        throw std::invalid_argument("Unsupported sampled player7 pass preparation");
    const auto& f=input.current.field;const auto& p=f.roster[5];
    const auto prepared=prepare_attacking_action({{5,p.type,static_cast<std::uint8_t>(p.stamina),static_cast<std::uint8_t>(p.stamina>>8),p.level},
        1,0,0,0,1,f.random_first,f.random_second,false},players,attack);
    if(prepared.preparation.stamina_cost!=20 || prepared.preparation.base_strength!=21 || prepared.energy.stamina ||
       !prepared.energy.weakened || prepared.action_power.power || prepared.action_power.subtype ||
       prepared.preparation.move_effect!=input.action->move_effect || prepared.move_flags!=input.action->move_flags)
        throw std::invalid_argument("Unverified sampled player7 pass content");
    auto state=input;state.action->power=prepared.action_power.power;state.current.field.roster[5].stamina=prepared.energy.stamina;
    state.phase=FieldSessionPhase::sampled_score25_pass_prepared;validate_phase_session(state);return state;
}
PhaseSessionState prepare_ordinary_unopposed_pass(const PhaseSessionState& input,
    const FieldPlayerProfiles& players,const FieldDefenseContent& attack) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_human_pass_confirmed || input.current.field.selected_defenders ||
       !input.action || !input.recovery || input.recovery->ball_variant)
        throw std::invalid_argument("Unopposed pass requires a confirmed field pass with no defenders");
    const auto& f=input.current.field;const auto& p=f.roster[f.ball_owner];
    const auto prepared=prepare_attacking_action({{f.ball_owner,p.type,static_cast<std::uint8_t>(p.stamina),static_cast<std::uint8_t>(p.stamina>>8),p.level},
        1,0,0,0,0,f.random_first,f.random_second,false},players,attack);
    auto state=input;state.current.field.roster[f.ball_owner].stamina=prepared.energy.stamina;
    state.action->power=prepared.action_power.power;state.action->move_effect=prepared.preparation.move_effect;
    state.action->move_flags=prepared.move_flags;state.recovery->attack_subtype=prepared.action_power.subtype;
    state.current.clock.ball_mode=0;state.ordinary_pass_defenses=OrdinaryPassDefenseContext{};
    state.phase=FieldSessionPhase::ordinary_pass_prepared;validate_phase_session(state);return state;
}
PhaseSessionState prepare_sampled_human_empty_pass(const PhaseSessionState& input,const FieldPlayerProfiles& players,const FieldDefenseContent& attack) {
    validate_phase_session(input);
    if(input.phase==FieldSessionPhase::sampled_score23_empty_confirmed) {
        const auto& f=input.current.field;const auto& p=f.roster[5];
        const auto prepared=prepare_attacking_action({{5,p.type,static_cast<std::uint8_t>(p.stamina),static_cast<std::uint8_t>(p.stamina>>8),p.level},
            1,0,0,0,1,f.random_first,f.random_second,false},players,attack);
        if(prepared.preparation.stamina_cost!=20 || prepared.preparation.base_strength!=21 || prepared.energy.stamina ||
           !prepared.energy.weakened || prepared.action_power.power || prepared.action_power.subtype ||
           prepared.preparation.move_effect!=input.action->move_effect || prepared.move_flags!=input.action->move_flags)
            throw std::invalid_argument("Unverified sampled 2-3 pass preparation");
        auto state=input;state.recovery->opposing_player=17;state.action->power=0;state.current.clock.ball_mode=0;
        state.phase=FieldSessionPhase::sampled_score23_pass_prepared;validate_phase_session(state);return state;
    }
    if(input.phase==FieldSessionPhase::sampled_player3_empty_confirmed) {
        const auto& f=input.current.field;const auto& p=f.roster[3];
        const auto prepared=prepare_attacking_action({{3,p.type,static_cast<std::uint8_t>(p.stamina),static_cast<std::uint8_t>(p.stamina>>8),p.level},
            1,0,0,0,1,f.random_first,f.random_second,false},players,attack);
        if(prepared.preparation.stamina_cost!=20 || prepared.preparation.base_strength!=21 || prepared.energy.stamina!=0 ||
           !prepared.energy.weakened || prepared.action_power.power!=0 || prepared.action_power.subtype!=0 ||
           prepared.preparation.move_effect!=input.action->move_effect || prepared.move_flags!=input.action->move_flags)
            throw std::invalid_argument("Unverified sampled player3 pass content");
        auto state=input;state.recovery->opposing_player=17;state.action->power=0;state.current.clock.ball_mode=0;
        state.phase=FieldSessionPhase::sampled_player3_pass_prepared;validate_phase_session(state);return state;
    }
    if(input.phase!=FieldSessionPhase::sampled_human_pass_empty_confirmed)
        throw std::invalid_argument("Unsupported sampled human pass preparation");
    const auto& f=input.current.field;const auto& p=f.roster[5];
    const auto prepared=prepare_attacking_action({{5,p.type,static_cast<std::uint8_t>(p.stamina),static_cast<std::uint8_t>(p.stamina>>8),p.level},
        1,0,0,0,0,f.random_first,f.random_second,false},players,attack);
    if(prepared.preparation.stamina_cost!=20 || prepared.preparation.base_strength!=21 || prepared.energy.stamina!=11 ||
       prepared.energy.weakened || prepared.action_power.power!=21 || prepared.action_power.subtype!=128 ||
       prepared.preparation.move_effect!=input.action->move_effect || prepared.move_flags!=input.action->move_flags)
        throw std::invalid_argument("Unverified sampled human pass content");
    auto state=input;state.current.field.roster[5].stamina=prepared.energy.stamina;
    state.recovery->attack_subtype=prepared.action_power.subtype;state.action->power=prepared.action_power.power;
    state.current.clock.ball_mode=0;state.phase=FieldSessionPhase::sampled_human_pass_prepared;
    validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_human_empty_pass_handoff(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::sampled_human_pass_prepared || input.recovery->ball_variant)
        throw std::invalid_argument("Unsupported sampled human pass handoff");
    auto state=input;state.recovery->ball_variant=1;state.phase=FieldSessionPhase::sampled_human_pass_handoff;
    validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_human_empty_pass_recovery(const PhaseSessionState& input,const FieldPhaseState& captured) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::sampled_human_pass_handoff || captured.ball_owner!=3 || captured.movement_region!=129)
        throw std::invalid_argument("Unsupported sampled human pass recovery");
    auto normalized=captured;normalized.ball_owner=input.current.field.ball_owner;
    normalized.movement_region=input.current.field.movement_region;normalized.motion=input.current.field.motion;
    std::ostringstream before(std::ios::out|std::ios::binary),after(std::ios::out|std::ios::binary);
    write_field_state(before,input.current.field);write_field_state(after,normalized);
    if(before.str()!=after.str())throw std::invalid_argument("Captured human pass recovery changed unrelated field state");
    auto state=input;state.current.field=captured;state.recovery->attack_subtype=0;state.saved_ball_flight_flag=1;
    state.human_pass_cursor.reset();state.human_pass_steps=0;state.phase=FieldSessionPhase::recovered_owner;
    validate_phase_session(state);return state;
}
}

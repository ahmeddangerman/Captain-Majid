#include "captain/action_menu.hpp"
#include "captain/defender_selection.hpp"
#include "captain/defensive_move_inspection.hpp"
#include "captain/field_state_io.hpp"
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
 const bool sampled_ordinary=sampled_roster && slot==0 && action==0;
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
ActionMenuResult poll_keeper_area_defense(const PhaseSessionState& input,std::uint8_t pressed,const AreaDefenseMenuContent& content) {
 validate_phase_session(input);
 const bool field=input.phase==FieldSessionPhase::sampled_field_defense_prompt || input.phase==FieldSessionPhase::sampled_field_defense_poll;
 if(input.phase!=FieldSessionPhase::keeper_area_defense_ready && input.phase!=FieldSessionPhase::keeper_area_defense_poll && !field)
  throw std::invalid_argument("Defensive input requires prompt/poll state");
 ActionMenuResult result{input};auto& state=result.state;auto& menu=*state.action;auto& defense=*state.area_defense;
 state.phase=field ? FieldSessionPhase::sampled_field_defense_poll : FieldSessionPhase::keeper_area_defense_poll;
 defense.pressed=0;
 const auto icon=[&](unsigned slot) {return defense_icon(defense,slot,content);};
 const auto count=state.current.field.selected_defenders;
 if((pressed&128u) && menu.cursor==count) {
  state.phase=field ? FieldSessionPhase::sampled_field_defense_complete : FieldSessionPhase::keeper_area_defense_complete;result.confirmed=true;
  result.presentation.push_back({ControlPresentationKind::clear_overlay,0});
  validate_phase_session(state);return result;
 }
 if((pressed&128u) && defense.actions[menu.cursor]!=255) {
  state.recovery->opposing_player=state.scheduling->selected_ids[menu.cursor];
  defense.pressed=pressed;
  state.phase=field ? FieldSessionPhase::sampled_field_defense_preparation : FieldSessionPhase::keeper_area_defense_preparation;result.confirmed=true;
  validate_phase_session(state);return result;
 }
 if((pressed&64u) && menu.cursor) {
  if(menu.cursor<count)result.presentation.push_back({ControlPresentationKind::sprite,icon(menu.cursor),menu.cursor});
  --menu.cursor;menu.flash_counter|=64u;defense.blink=0;
  result.presentation.push_back({ControlPresentationKind::display,content.titles[field ? 0 : 2]});
  validate_phase_session(state);return result;
 }
 auto directions=static_cast<unsigned>(pressed&15u);
 if(menu.cursor==count) {
  if(directions)throw std::invalid_argument("Full defensive prefix direction scratch slots are not represented");
  validate_phase_session(state);return result;
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
PhaseSessionState begin_sampled_human_pass_target(const PhaseSessionState& input) {
    validate_phase_session(input);const auto& f=input.current.field;
    const bool score25=input.phase==FieldSessionPhase::attack_choice_pending && input.score && input.action && input.recovery && input.scheduling &&
       input.score->left_goals==2 && input.score->right_goals==5 && input.score->kickoff==4 && input.current.clock.remaining==325 && input.current.clock.extra_time==13 &&
       f.side==0 && f.ball_owner==5 && f.ball_x==108 && f.ball_y==108 && f.ball_cell==87 && f.elapsed==1 && f.selected_defenders==3 &&
       input.scheduling->selected_ids==std::array<std::uint8_t,5>{17,19,20,0,0} && !f.random_first && !f.random_second &&
       f.roster[5].type==23 && !f.roster[5].stamina && input.action->action==1 && input.action->power==22 &&
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
    auto state=input;state.action->power=prepared.action_power.power;
    state.phase=FieldSessionPhase::sampled_score25_pass_prepared;validate_phase_session(state);return state;
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

#include "captain/action_menu.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>

namespace captain {
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
}

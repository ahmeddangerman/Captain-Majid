#include "captain/phase_session.hpp"
#include "captain/defender_selection.hpp"
#include "captain/defense_affordability.hpp"
#include <stdexcept>
#include <string>
#include <utility>

namespace captain {
namespace {
void validate_contact(const FieldPhaseState& field, const PendingTerminalContact& contact) {
    if ((contact.result_code != 2 && contact.result_code != 3) ||
        (contact.defense_action != 0 && contact.defense_action != 2) ||
        (contact.defense_subtype != 0 && contact.defense_subtype != 128))
        throw std::invalid_argument("Unsupported terminal contact");
    field_motion_index(contact.defender_id);
    if ((contact.defender_id < 11 ? 0 : 11) == field.side)
        throw std::invalid_argument("Terminal contact requires an opposing field player");
}
PhaseSessionResult finish(PhaseSessionState state, PendingTerminalContact contact,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement) {
    state.pending.reset();
    TerminalFlightPhaseResult terminal;
    if (contact.result_code == 2) {
        // Placement happened before the clock. Do not re-read the defender's
        // position after a deferred presentation; preserve that earlier write.
        state.current.field.selected_defenders = 0;
        if (state.scheduling) state.scheduling->selected_ids.fill(0);
        terminal = {state.current.field, TerminalFlightNextPhase::ball_recovery, std::nullopt};
        state.phase = FieldSessionPhase::ball_recovery;
    } else {
        terminal = resolve_prepared_terminal_flight({state.current.field, contact.result_code,
            contact.defender_id, contact.defense_action, contact.defense_subtype}, primary, movement);
        state.current.field = terminal.state;
        state.phase = FieldSessionPhase::player_control;
    }
    return {std::move(state), std::nullopt, std::move(terminal)};
}
}

void validate_phase_session(const PhaseSessionState& state) {
    if(state.saved_ball_flight_flag>1)throw std::invalid_argument("Unsupported saved ball flight flag");
    if ((state.current.field.side != 0 && state.current.field.side != 11) ||
        state.current.field.ball_owner > 21 || static_cast<unsigned>(state.phase) > 201 ||
        static_cast<unsigned>(state.phase)==111 || static_cast<unsigned>(state.phase)==112)
        throw std::invalid_argument("Unsupported phase session state");
    const auto phase = static_cast<unsigned>(state.phase);
    const bool shot_pass_context=(phase>=152 && phase<=156) || phase==158 || phase==159;
    const bool shot_pass_compared=(phase>=153 && phase<=156) || phase==158 || phase==159;
    const bool area_flight=(phase>=96 && phase<=101) || phase==105 || (phase>=114 && phase<=149);
    const bool pass_capture_tick=phase>=142 && phase<=145;
    const bool pass_blocked_tick=phase>=146 && phase<=149;
    const bool area_action4=phase==102 || phase==103 || (phase>=160 && phase<=187) || phase==190 || phase==191 || phase==192 || phase==193;
    if(bool(state.area_action4_restart)!=((phase>=179 && phase<=186) || phase==192 || phase==193) ||
       (state.area_action4_restart && ((phase==192 || phase==193) ? (state.area_action4_restart->action!=9 || state.area_action4_restart->subtype!=128 || state.area_action4_restart->choice!=5 || state.current.field.ball_owner!=20) :
       (state.area_action4_restart->action!=1 || state.area_action4_restart->subtype || state.area_action4_restart->choice!=6 || state.current.field.ball_owner!=14))))
        throw std::invalid_argument("Invalid captured action4 restart choice");
    if(bool(state.area_action4_restart_power)!=((phase>=181 && phase<=186) || phase==192 || phase==193) ||
       (state.area_action4_restart_power && (state.area_action4_restart_power->cost!=((phase==192 || phase==193) ? 40 : 20) || state.area_action4_restart_power->power!=21)))
        throw std::invalid_argument("Invalid action4 restart action power");
    if(bool(state.area_action4_restart_pass_counter)!=(phase==186) ||
       (state.area_action4_restart_pass_counter && *state.area_action4_restart_pass_counter!=77))
        throw std::invalid_argument("Invalid action4 restart pass counter");
    const bool area_action6=phase==104 || (phase>=106 && phase<=110) || phase==113;
    const bool area_action6_race=(phase>=106 && phase<=110) || phase==113;
    const bool area_setup=phase==65 || phase==105;
    const bool area_menu=(phase>=66 && phase<=94) || ((phase==18 || phase==19) && state.keeper_area);
    const bool area_state=area_setup || area_menu;
    if((((phase>=65 && phase<=94) || phase==105) && !state.keeper_area) || (state.keeper_area && !area_state))
        throw std::invalid_argument("Keeper-area setup needs exactly one captured selection counter");
    if(state.keeper_area && (!state.action || !state.recovery || !state.scheduling ||
       state.keeper_area->selection_counter!=255 || !state.recovery->ball_variant ||
       state.current.field.ball_y<96 || state.current.field.ball_y>=160 ||
       (state.current.field.ball_x>=80 && state.current.field.ball_x<176) ||
       state.action->menu_route!=(((state.current.field.ball_x<80 ? 0 : 11)==state.current.field.side)?2:1)))
        throw std::invalid_argument("Keeper-area setup requires its original area and route (phase " + std::to_string(phase) + ", ball " + std::to_string(state.current.field.ball_x) + "," + std::to_string(state.current.field.ball_y) + ", side " + std::to_string(state.current.field.side) + ", route " + std::to_string(state.action ? state.action->menu_route : 255) + ")");
    if(area_menu && (state.action->menu_route!=2 || (phase>=66 ? state.current.field.side!=11 : state.current.field.side!=0)))
        throw std::invalid_argument("Prepared own-area menu requires its matching attack or defense side");
    if(bool(state.area_defense)!=(phase==67 || (phase>=69 && phase<=92) || phase==94 || area_action4 || ((phase==93 || area_action6) && state.action && (state.action->action==4 || state.action->action==6) && state.area_defense)))
        throw std::invalid_argument("Initial area defense requires its captured choice arrays");
    if(phase==93 && state.area_defense)for(unsigned i=0;i<state.current.field.selected_defenders;++i)
        if(state.area_defense->actions[i]>6)throw std::invalid_argument("Area continuation needs all captured defender choices");
    if(area_action4 && (!state.action || state.action->action!=4 || !state.recovery || state.recovery->ball_variant || state.current.field.side!=11 || ((phase>=162 && phase<=168) ? state.current.field.selected_defenders!=0 : !state.current.field.selected_defenders) || state.current.field.selected_defenders>4))
        throw std::invalid_argument("Area action4 boundary needs its retained defender choices");
    if(phase==160 && state.action->menu_route!=0)
        throw std::invalid_argument("Area action4 field entry needs its reset comparison route");
    if(phase==165 && state.current.field.elapsed!=1)
        throw std::invalid_argument("Area action4 field round requires elapsed1");
    if(phase==166 && (state.current.field.elapsed!=1 || state.current.clock.ball_mode || state.current.clock.script_flags))
        throw std::invalid_argument("Area action4 goal-area boundary requires its cleared control flags");
    if(phase==167 && (state.current.clock.remaining || !state.current.clock.extra_time || state.current.clock.ball_mode || (state.current.clock.script_flags&64u)))
        throw std::invalid_argument("Invalid area action4 added-time boundary");
    if(phase==168 && state.current.clock.remaining)throw std::invalid_argument("Invalid area action4 period-end boundary");
    if(((phase>=169 && phase<=187) || phase==190 || phase==191 || phase==192 || phase==193) && (!state.scheduling || state.current.field.selected_defenders!=1 || state.scheduling->selected_ids[0]!=0 ||
       state.recovery->opposing_player!=0 || state.action->menu_route!=3 || state.current.field.elapsed!=((phase>=182 && phase<=186)?19:1)))
        throw std::invalid_argument("Area action4 keeper entry requires the selected side0 goalkeeper");
    if(phase==170 && (!state.control || (state.control->facing!=0 && state.control->facing!=64)))
        throw std::invalid_argument("Area action4 keeper presentation requires captured facing");
    if(phase==67 || phase==69) {
        if(!state.current.field.selected_defenders || state.current.field.selected_defenders>4 || state.action->cursor || state.area_defense->pressed)
            throw std::invalid_argument("Area defense initialization requires a nonempty prefix and cursor0");
        for(unsigned i=0;i<4;++i)if(state.area_defense->actions[i]!=255 || state.area_defense->subtypes[i])
            throw std::invalid_argument("Initial defense actions must be unselected with unused subtypes zero");
    }
    if((phase>=70 && phase<=92) || phase==94) {
        const auto count=state.current.field.selected_defenders;
        if(!count || count>4 || state.action->cursor>count || ((phase==71 || (phase>=73 && phase<=76)) && state.action->cursor==count))
            throw std::invalid_argument("Defensive poll requires a valid selected cursor");
        for(unsigned i=0;i<4;++i) {
            const auto action=state.area_defense->actions[i];
            if((action!=255 && action>6) || (state.area_defense->subtypes[i] && (state.area_defense->subtypes[i]!=1 || action!=2)) || (i>=count && action!=255))
                throw std::invalid_argument("Defensive poll supports ordinary field actions in the active prefix");
        }
        if(((phase==70 || phase==72 || (phase>=73 && phase<=77)) && state.area_defense->pressed) || (phase==71 && !(state.area_defense->pressed&128u)))
            throw std::invalid_argument("Pending defensive preparation requires the captured confirm input");
        if(phase==72 && state.action->cursor!=count)
            throw std::invalid_argument("Completed defense requires the full selected prefix");
        if(state.action->cursor==count)for(unsigned i=0;i<count;++i)
            if(state.area_defense->actions[i]==255)
                throw std::invalid_argument("Full defensive prefix requires selected actions");
        const bool advanced=phase==77 && state.area_special_decision && state.area_special_decision->status==0;
        if(advanced && !state.action->cursor)throw std::invalid_argument("Accepted special redraw requires advanced cursor");
        const auto chosen=state.action->cursor-(advanced?1u:0u);
        if((phase==71 || (phase>=73 && phase<=77)) && (chosen>=count || state.area_defense->actions[chosen]==255 ||
           state.recovery->opposing_player!=state.scheduling->selected_ids[chosen]))
            throw std::invalid_argument("Pending defensive preparation requires a confirmed selected field player");
    }
    if(bool(state.area_special_option)!=(phase>=73 && phase<=77) ||
       (state.area_special_option && (*state.area_special_option>127 ||
        state.area_defense->actions[state.action->cursor-((phase==77 && state.area_special_decision && state.area_special_decision->status==0)?1u:0u)]!=2)))
        throw std::invalid_argument("Special chooser requires its captured option and action2");
    if((phase<73 && (state.area_special_cursor || state.area_special_pressed)) ||
       (phase>=73 && (state.area_special_cursor>1 || (phase==73 && state.area_special_cursor) ||
        (phase!=75 && phase!=76 && phase!=77 && state.area_special_pressed) || ((phase==75 || phase==76 || phase==77) && !(state.area_special_pressed&192u)))))
        throw std::invalid_argument("Invalid special chooser cursor or sampled input");
    if(bool(state.area_special_decision)!=(phase==76 || phase==77))
        throw std::invalid_argument("Special inspection needs its decision context");
    if(state.area_special_decision) {
        const auto& d=*state.area_special_decision;
        const bool cancel=!(state.area_special_pressed&128u);
        const auto subtype=state.area_special_cursor ? *state.area_special_option : 0;
        if(d.status>2 || (cancel ? (d.status!=2 || d.subtype || d.cost || d.effect) :
           (d.status==2 || d.subtype!=subtype || (d.subtype==0 && d.status!=0))))
            throw std::invalid_argument("Invalid special inspection decision");
        if(phase==77 && d.status==0 && state.area_defense->subtypes[state.action->cursor-1]!=d.subtype)
            throw std::invalid_argument("Accepted special redraw must retain chosen subtype");
        if(!cancel && (state.recovery->opposing_player>21 || d.cost>1023 ||
           (d.status==0)!=can_select_defense_move(state.current.field.roster[state.recovery->opposing_player].stamina,d.cost,d.subtype)))
            throw std::invalid_argument("Special inspection decision disagrees with stamina");
    }
    if(bool(state.area_ai_target)!=((phase>=78 && phase<=94) || area_flight || area_action4 || area_action6))throw std::invalid_argument("Area AI choice needs saved target context");
    if(area_action6 && (!state.action || state.action->action!=6 || !state.area_ai_target || state.area_ai_target->recipient || state.area_ai_target->cell!=state.current.field.ball_cell || ((phase==104 || phase==107) && state.current.field.side!=11) || state.current.field.selected_defenders>4))
        throw std::invalid_argument("Area action6 destination needs its captured target cell");
    if(area_flight && (!state.action || state.action->action!=1 || !state.area_ai_target || state.area_ai_target->recipient<12 || state.area_ai_target->recipient>21 || state.area_ai_target->cell>=240 || state.area_ai_target->cost>1023 || (phase<135 && state.current.field.side!=((phase>=128 && phase<=133) ? 0 : 11)) || ((pass_capture_tick || pass_blocked_tick) && state.current.field.side!=0) || state.current.field.selected_defenders>4 || (phase<114 && state.area_defender_cursor) || (((phase>=117 && phase<=122) || (phase>=124 && phase<=149)) ? state.area_sequence_result>((phase>=120 && phase<=122) ? 1 : 3) : state.area_sequence_result!=255)))
        throw std::invalid_argument("Area flight requires its captured recipient and cleared sequence");
    if(((phase>=114 && phase<=122) || (phase>=124 && phase<=129)) && !state.current.field.selected_defenders)
        throw std::invalid_argument("Defended area arrival needs a selected defender");
    if(((phase>=114 && phase<=122) || (phase>=124 && phase<=129)) && state.area_defender_cursor>=state.current.field.selected_defenders)
        throw std::invalid_argument("Invalid queued pass defender cursor");
    if(phase==123 && (state.current.field.selected_defenders || !state.area_defender_cursor || state.area_defender_cursor>4 || state.current.field.elapsed))
        throw std::invalid_argument("Completed pass defenders need cleared selection and time");
    if(bool(state.area_pass_contact)!=((phase>=116 && phase<=122) || (phase>=124 && phase<=149) || shot_pass_context))throw std::invalid_argument("Prepared pass defender needs saved context");
    if(shot_pass_context && (!state.recovery || !state.scheduling || state.area_pass_contact->saved_ball_variant!=0 ||
       state.recovery->ball_variant!=0 || state.area_defender_cursor>=state.current.field.selected_defenders ||
       state.recovery->opposing_player!=state.scheduling->selected_ids[state.area_defender_cursor]))
        throw std::invalid_argument("Shot pass defender preparation needs its zero stacked variant and selected defender");
    if(((phase>=116 && phase<=122) || (phase>=124 && phase<=149)) && (!state.recovery || !state.scheduling || state.area_pass_contact->saved_ball_variant<1 || state.area_pass_contact->saved_ball_variant>2 ||
       (phase>=118 ? (((phase>=130 && phase<=133) || phase>=138) ? state.recovery->ball_variant!=0 : state.recovery->ball_variant!=state.area_pass_contact->saved_ball_variant) : state.recovery->ball_variant!=0) || (((phase>=130 && phase<=133) || pass_capture_tick) ? state.recovery->opposing_player!=state.current.field.ball_owner : phase>=138 ? state.recovery->opposing_player>10 : phase>=134 ? (state.recovery->opposing_player<1 || state.recovery->opposing_player>10) : state.recovery->opposing_player!=state.scheduling->selected_ids[state.area_defender_cursor])))
        throw std::invalid_argument("Invalid prepared pass defender context");
    if(phase>=78 && phase<=94) {
        const auto& target=*state.area_ai_target;
        const auto subtype=phase>=79 ? state.recovery->attack_subtype&127u : state.recovery->attack_subtype;
        const bool terminal_presentation=(phase==86 || phase==89 || phase==94) && state.area_contact && state.area_contact->result>=2;
        if((phase>=81 && phase<=94) && (state.control->motion_requested!=(terminal_presentation?0:16) ||
           (terminal_presentation ? state.control->facing!=0 : (state.control->facing!=0 && state.control->facing!=64))))throw std::invalid_argument("Invalid announced area presentation state");
        if(phase==80 && state.control->motion_requested!=(state.recovery->ball_variant==1 ? 2 : 0))throw std::invalid_argument("Area defense entry needs acknowledged motion request");
        if(target.cost>1023 || (phase==78 && target.cost))throw std::invalid_argument("Invalid prepared area cost");
        if(state.action->cursor!=state.current.field.selected_defenders ||
           (state.action->action!=1 && state.action->action!=4 && state.action->action!=6) ||
           (state.recovery->ball_variant!=1 && state.recovery->ball_variant!=2) ||
           (state.action->action==1 ? (target.recipient<12 || target.recipient>21 || subtype) :
            (target.recipient || target.cell || (state.action->action==4 ? subtype!=0 : subtype>1))))
            throw std::invalid_argument("Unsupported saved area AI choice");
        if(state.action->action==1) {
            const auto& m=state.current.field.motion[field_motion_index(target.recipient)];
            if(target.cell!=field_cell_at(static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8)))
                throw std::invalid_argument("Area AI target cell disagrees with saved player position");
        }
    }
    if(bool(state.area_contact)!=((phase>=82 && phase<=86) || phase==89 || phase==94 || (phase>=117 && phase<=122) || (phase>=124 && phase<=149) || shot_pass_compared || phase==171 || phase==172 || phase==173 || phase==174 || phase==175 || phase==176 || phase==177 || phase==178 || phase==179 || phase==180 || phase==181 || phase==182 || phase==183 || phase==184 || phase==185 || phase==186 || phase==187 || phase==189 || phase==190 || phase==191 || phase==192 || phase==193))throw std::invalid_argument("Area comparison requires saved contact context");
    if((phase==171 || phase==172) && (state.area_contact->cursor || state.area_contact->defense_action!=6 || state.area_contact->defense_subtype ||
       state.area_contact->effect || state.area_contact->strength || state.area_contact->loss || state.area_contact->score ||
       state.area_contact->result || (phase==171 && state.area_contact->weakened)))
        throw std::invalid_argument("Invalid captured action4 goalkeeper preparation");
    if(((phase>=173 && phase<=187) || phase==190 || phase==191 || phase==192 || phase==193) && (state.area_contact->cursor || state.area_contact->defense_action!=6 ||
       (state.area_contact->defense_subtype!=0 && state.area_contact->defense_subtype!=128) ||
       state.area_contact->effect || state.area_contact->loss!=state.area_contact->strength/4 ||
       (state.area_contact->result>1 && state.area_contact->result!=4) || state.area_sequence_result!=state.area_contact->result))
        throw std::invalid_argument("Invalid action4 goalkeeper comparison context");
    if(((phase==175 || phase==187) && state.area_contact->result!=0) || ((phase>=176 && phase<=186) && state.area_contact->result!=1))
        throw std::invalid_argument("Action4 goalkeeper handler requires its corresponding result");
    if((phase==190 || phase==191 || phase==192 || phase==193) && state.area_contact->result!=4)throw std::invalid_argument("Sampled keeper restart requires result4");
    if((phase==191 || phase==192 || phase==193) && (state.current.field.ball_owner!=20 || state.current.field.ball_x!=68 || state.current.field.ball_y!=132 || state.current.field.ball_cell!=30))throw std::invalid_argument("Invalid captured result4 restart formation");
    if(phase==187 && (!state.score || state.score->left_goals!=0 || state.score->right_goals!=1 || state.score->kickoff!=0))throw std::invalid_argument("Invalid sampled result0 goal score");
    if(phase==183 && (state.current.clock.remaining || !state.current.clock.extra_time || state.current.clock.ball_mode || (state.current.clock.script_flags&64u)))
        throw std::invalid_argument("Invalid action4 restart added-time boundary");
    if(phase==184 && state.current.clock.remaining)throw std::invalid_argument("Invalid action4 restart period-end boundary");
    if(shot_pass_compared && (!state.action || !state.area_pass_contact || state.area_contact->cursor!=state.area_defender_cursor || state.area_contact->defense_action!=2 ||
       state.area_contact->defense_subtype || state.area_contact->effect || state.area_contact->cost!=state.area_pass_contact->captured_cost ||
       state.area_contact->strength || state.area_contact->loss || state.area_contact->score!=255 || state.area_contact->result ||
       state.area_contact->weakened || state.area_sequence_result || state.action->menu_route))
        throw std::invalid_argument("Invalid shot pass zero-range comparison");
    if(phase==84 && (state.current.clock.remaining || !state.current.clock.extra_time ||
       state.current.clock.ball_mode || (state.current.clock.script_flags&64u)))
        throw std::invalid_argument("Invalid area contact added-time boundary");
    if(phase==91 && (state.current.clock.remaining || !state.current.clock.extra_time || state.current.clock.ball_mode || (state.current.clock.script_flags&64u)))throw std::invalid_argument("Invalid area prefix added-time boundary");
    if(phase==92 && state.current.clock.remaining)throw std::invalid_argument("Area prefix period end requires exhausted match clock");
    if(phase==85 && state.current.clock.remaining)
        throw std::invalid_argument("Area period end requires exhausted match clock");
    if((phase<82 && state.area_defender_cursor) ||
       (phase>=82 && phase<=94 && (state.area_defender_cursor>state.current.field.selected_defenders ||
        ((phase==88 || (phase>=90 && phase<=93)) ? state.area_defender_cursor!=state.current.field.selected_defenders :
         state.area_defender_cursor>=state.current.field.selected_defenders))) ||
       (phase==87 && !state.area_defender_cursor) ||
       (phase==89 && state.area_contact->result<2))
        throw std::invalid_argument("Invalid saved area defender cursor or exit");
    if(state.area_contact && !((phase>=117 && phase<=122) || (phase>=124 && phase<=149) || shot_pass_compared || phase==171 || phase==172 || phase==173 || phase==174 || phase==175 || phase==176 || phase==177 || phase==178 || phase==179 || phase==180 || phase==181 || phase==182 || phase==183 || phase==184 || phase==185 || phase==186 || phase==187 || phase==189 || phase==190 || phase==191 || phase==192 || phase==193)) {
        const auto& c=*state.area_contact;
        if(c.cursor!=state.area_defender_cursor || c.cursor>=state.current.field.selected_defenders ||
           state.recovery->opposing_player!=state.scheduling->selected_ids[c.cursor] ||
           c.defense_action!=state.area_defense->actions[c.cursor] ||
           (c.defense_action!=2 && c.defense_action!=4) ||
           (c.defense_subtype&127u)!=state.area_defense->subtypes[c.cursor] ||
           c.result>3 || c.cost>1534 || c.loss!=c.strength/4)
            throw std::invalid_argument("Invalid saved area comparison");
    }
    if(((phase>=117 && phase<=122) || (phase>=124 && phase<=149)) && (state.area_contact->cursor!=state.area_defender_cursor || state.area_contact->defense_action!=2 ||
       (state.area_contact->defense_subtype!=0 && state.area_contact->defense_subtype!=128) ||
       state.area_contact->result!=state.area_sequence_result || state.area_contact->weakened ||
       state.area_contact->loss!=state.area_contact->strength/4))
        throw std::invalid_argument("Invalid pass defender comparison context");
    if(phase==124 && state.area_sequence_result!=2)throw std::invalid_argument("Positioned blocked pass requires result2");
    if(phase>=125 && phase<=127 && state.area_sequence_result<2)throw std::invalid_argument("Pass terminal clock requires result2 or3");
    if((phase>=128 && phase<=133) && (state.area_sequence_result!=3 || state.current.field.ball_owner!=state.recovery->opposing_player || ((phase==129 || phase==130) && state.current.field.elapsed) || ((phase>=131 && phase<=133) && state.current.field.elapsed>1) || (phase>=130 && state.current.field.selected_defenders)))
        throw std::invalid_argument("Captured pass requires the defending player as owner");
    if((phase>=134 && phase<=141) && (state.area_sequence_result!=2 || state.current.field.selected_defenders || state.area_defender_cursor>3 || ((phase==135 || phase==137 || phase==138) && state.current.field.elapsed) || ((phase>=139 && phase<=141) && state.current.field.elapsed>1) || (phase>=139 && state.current.field.side!=0)))
        throw std::invalid_argument("Blocked pass recovery needs a cleared defender queue");
    if((pass_capture_tick || pass_blocked_tick) && (state.area_sequence_result!=(pass_capture_tick ? 3 : 2) || state.current.field.selected_defenders || state.current.field.elapsed!=1))
        throw std::invalid_argument("Area pass control tick requires its completed clock state");
    if((phase==81 && state.area_sequence_result!=0) ||
       (phase>=82 && phase<=86 && state.area_sequence_result!=state.area_contact->result) ||
       (phase==89 && state.area_sequence_result!=state.area_contact->result) ||
       ((phase==87 || phase==88 || (phase>=90 && phase<=92)) &&
        state.area_sequence_result!=255 && state.area_sequence_result>1) ||
       (phase==93 && state.area_sequence_result>1) ||
       (phase==94 && state.area_sequence_result!=state.area_contact->result) ||
       (phase==95 && state.area_sequence_result!=255))
        throw std::invalid_argument("Invalid saved final area sequence result");
    if(phase==68 && state.current.field.selected_defenders)
        throw std::invalid_argument("Empty area defense completion requires no selected players");
    if (bool(state.shot_flight) != ((phase >= 46 && phase <= 56) || phase==59 || (phase>=150 && phase<=159) || phase==188 || phase==189 || area_flight))
        throw std::invalid_argument("Flight setup requires exactly one shot context");
    if (state.shot_flight && (state.shot_flight->destination >= 240 || (state.shot_flight->counter != 255 && !(area_flight && (state.shot_flight->counter==38 || state.shot_flight->counter==77))) || (area_flight && (state.shot_flight->destination!=state.area_ai_target->cell || state.shot_flight->loose_ball))))
        throw std::invalid_argument("Unsupported initial shot flight context");
    const bool trajectory=(phase>=47 && phase<=56) || phase==59 || (phase>=150 && phase<=159) || phase==188 || phase==189 || area_flight;
    if(bool(state.ball_trajectory)!=trajectory)
        throw std::invalid_argument("Numeric flight requires exactly one trajectory context");
    if(state.ball_trajectory) {
        const auto& t=*state.ball_trajectory;const auto& f=state.current.field;
        if(!state.recovery || !state.scheduling || t.flags!=(area_flight?1:state.saved_ball_flight_flag) || t.direction!=state.recovery->x_jitter_control ||
            (t.x>>8)!=f.ball_x || (t.y>>8)!=f.ball_y ||
            (!(((phase>=124 && phase<=127) || (phase>=134 && phase<=137)) && state.area_sequence_result==2) && field_cell_at(f.ball_x,f.ball_y)!=f.ball_cell) || (f.elapsed && !((phase>=114 && phase<=122) || (phase>=124 && phase<=128) || (phase>=131 && phase<=133) || phase==134 || phase==136 || (phase>=139 && phase<=141) || (phase>=150 && phase<=156) || phase==158 || phase==159 || pass_capture_tick || pass_blocked_tick)) ||
            ((phase==50 || phase==51 || phase==52 || phase==54 || phase==55) && f.selected_defenders) ||
            (phase==47 && (!t.vx && !t.vy)) ||
            (phase==47 && f.ball_cell==state.shot_flight->destination) ||
            (phase==48 && f.ball_cell==state.shot_flight->destination) ||
            (((phase>=49 && phase<=56) || phase==59 || phase==97 || (phase>=150 && phase<=159) || phase==188 || phase==189) && f.ball_cell!=state.shot_flight->destination) ||
            (phase==96 && (f.ball_cell==state.shot_flight->destination || (!t.vx && !t.vy))) ||
            (phase==97 && f.selected_defenders) ||
            (phase==98 && f.ball_cell==state.shot_flight->destination) ||
            (phase==99 && f.ball_cell!=state.shot_flight->destination) ||
            (phase==100 && (f.ball_cell!=state.shot_flight->destination || f.selected_defenders || f.ball_owner!=state.area_ai_target->recipient)) ||
            (phase==101 && (f.selected_defenders || f.ball_owner!=state.area_ai_target->recipient)) ||
            (phase==50 && (t.vx || t.vy)))
            throw std::invalid_argument("Unsupported numeric flight boundary");
    }
    const bool sampled_kickoff_loose_defenders=(phase==150 || phase==151 || phase==188 || phase==189) && !state.saved_ball_flight_flag && state.shot_flight && state.shot_flight->loose_ball &&
       state.current.field.side==0 && state.current.field.ball_owner==7 && state.current.field.ball_x==188 && state.current.field.ball_y==124 &&
       state.current.field.ball_cell==209 && state.current.field.random_first==0 && state.current.field.random_second==128 &&
       state.current.field.selected_defenders==2 && state.scheduling && state.scheduling->selected_ids[0]==19 && state.scheduling->selected_ids[1]==14 &&
       state.score && state.score->left_goals==0 && state.score->right_goals==1 && state.score->kickoff==4 && state.kickoff_formation;
    if(((phase>=150 && phase<=156) || phase==158 || phase==159 || phase==188 || phase==189) && ((!state.saved_ball_flight_flag && !sampled_kickoff_loose_defenders) || state.current.field.side!=0 ||
       !state.current.field.selected_defenders || state.current.field.selected_defenders>5 || state.area_defender_cursor>=state.current.field.selected_defenders))
        throw std::invalid_argument("Shot pass defenders require a nonempty side0 prefix and valid cursor");
    if((phase==188 || phase==189) && (!sampled_kickoff_loose_defenders || state.area_defender_cursor || state.area_sequence_result!=(phase==188 ? 255 : 2) ||
       !state.recovery || state.recovery->opposing_player!=19 || state.recovery->ball_variant))
        throw std::invalid_argument("Prepared sampled loose-shot defender requires the first observed player");
    if(phase==189 && (!state.area_contact || state.area_contact->cursor || state.area_contact->defense_action || state.area_contact->defense_subtype ||
       state.area_contact->effect || state.area_contact->defender_power!=19 || state.area_contact->cost || state.area_contact->strength!=9 ||
       state.area_contact->loss!=2 || state.area_contact->score!=78 || state.area_contact->result!=2 || state.area_contact->weakened))
        throw std::invalid_argument("Unsupported sampled loose-shot comparison");
    if(phase>=150 && phase<=152 && state.area_sequence_result!=255)
        throw std::invalid_argument("Pending shot pass defender must not retain the prior result");
    if(phase==157 && (!state.saved_ball_flight_flag || state.current.field.side!=0 || state.current.field.selected_defenders ||
       state.current.field.elapsed || !state.area_defender_cursor || state.area_defender_cursor>5 || state.area_sequence_result!=255))
        throw std::invalid_argument("Completed shot pass defenders require a cleared prefix and elapsed time");
    if(phase==158 && (state.current.clock.remaining || !state.current.clock.extra_time || state.current.clock.ball_mode || (state.current.clock.script_flags&64u)))
        throw std::invalid_argument("Invalid shot pass added-time boundary");
    if(phase==159 && state.current.clock.remaining)throw std::invalid_argument("Invalid shot pass period-end boundary");
    if(phase==155 && (!state.control || (state.control->facing!=0 && state.control->facing!=64)))
        throw std::invalid_argument("Shot pass result needs captured facing");
    if((phase==51 && !state.shot_flight->loose_ball) || (((phase>=52 && phase<=56) || phase==59) && state.shot_flight->loose_ball))
        throw std::invalid_argument("Flight return phase does not match saved loose/keeper route");
    if(bool(state.keeper_decision)!=(phase==55 || phase==56 || phase==59) ||
       (state.keeper_decision && (state.keeper_decision->cursor ||
        state.keeper_decision->result_code>(phase==55?1:3) || state.keeper_decision->defense_action>3 ||
        (phase==55 && (state.keeper_decision->defense_action || state.keeper_decision->defense_subtype || state.keeper_decision->effect)))))
        throw std::invalid_argument("Stunned keeper decision needs one valid result/cursor context");
    if((phase>=194 && phase<=200) && (!state.action || !state.recovery || !state.control || !state.experience || !state.score ||
       !state.kickoff_formation || state.current.field.side!=11 || state.current.field.ball_owner!=16 ||
       state.current.field.ball_x!=141 || state.current.field.ball_y!=148 || state.current.field.ball_cell!=140 ||
       state.current.field.roster[16].type!=23 || state.current.field.selected_defenders ||
       state.current.field.random_first || state.current.field.random_second || state.experience->opponent_stage ||
       state.current.clock.remaining!=443 || state.current.clock.extra_time!=5 || state.control->last_carrier_cell!=(phase>=199 ? 128 : phase>=197 ? 0 : 140) ||
       state.score->left_goals!=1 || state.score->right_goals!=0 || state.score->kickoff!=4 ||
       state.current.clock.script_flags!=(phase>=196 ? 0 : 128) || state.current.clock.ball_mode!=(phase>=196 ? 0 : 128) ||
       (phase>=198 && (state.current.field.elapsed!=0 || state.current.field.movement_region!=140)) ||
       (phase>=199 && (state.recovery->special_owner_flags!=0 || state.recovery->ball_variant!=1 || state.control->facing!=64 || state.control->display_parameter!=0 || state.control->motion_requested!=0)) ||
       state.action->action!=1 || state.action->power!=(phase==194 ? 22 : 21) || state.recovery->attack_subtype!=(phase==194 ? 0 : 128)))
        throw std::invalid_argument("Invalid sampled opponent AI choice state");
    if(bool(state.sampled_field_pass)!=(phase==200) || (state.sampled_field_pass &&
       (state.sampled_field_pass->recipient!=19 || state.sampled_field_pass->destination!=103 ||
        state.sampled_field_pass->counter!=255 || state.sampled_field_pass->distance!=3)))
        throw std::invalid_argument("Invalid sampled opponent pass target context");
    if(phase==200) {
        const auto& recipient=state.current.field.motion[field_motion_index(state.sampled_field_pass->recipient)];
        if(field_cell_at(static_cast<std::uint8_t>(recipient.x>>8),static_cast<std::uint8_t>(recipient.y>>8))!=state.sampled_field_pass->destination)
            throw std::invalid_argument("Sampled opponent pass target disagrees with receiver position");
    }
    if(phase==201 && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling ||
       !state.kickoff_formation || state.saved_ball_flight_flag!=1 || state.current.field.side!=11 || state.current.field.ball_owner!=19 ||
       state.current.field.ball_x!=112 || state.current.field.ball_y!=140 || state.current.field.ball_cell!=103 || state.current.field.elapsed!=1 ||
       state.current.field.selected_defenders!=3 || state.scheduling->selected_ids!=std::array<std::uint8_t,5>{3,7,8,0,0} ||
       state.current.field.random_first || state.current.field.random_second || state.experience->opponent_stage ||
       state.current.clock.remaining!=440 || state.current.clock.extra_time!=5 || state.current.clock.script_flags || state.current.clock.ball_mode ||
       state.score->left_goals!=1 || state.score->right_goals!=0 || state.score->kickoff!=4 ||
       state.control->display_parameter || state.control->motion_requested || state.action->action!=1 || state.action->power!=21))
        throw std::invalid_argument("Invalid sampled opponent contact presentation");
    const bool keeper_selected=phase==53 || phase==56 || ((phase>=169 && phase<=187) || phase==190 || phase==191 || phase==192 || phase==193) || (phase==59 && !state.current.field.keepers[1].stun);
    const bool keeper_possession=phase==57;
    const bool goal_restart=phase==60;
    const bool kickoff_placed=phase>=61 && phase<=64;
    const bool kickoff_shot=(phase>=194 && phase<=201) || phase==15 || phase==1 || phase==2 || phase==6 || phase==7 || (phase==8 && state.current.field.selected_defenders) || phase==27 || phase==43 || (phase>=45 && phase<=53) || (phase>=56 && phase<=60) || phase==95 || (phase>=150 && phase<=159) || phase==188 || phase==189 || area_flight || area_action4 || area_action6 || area_state;
    if((kickoff_placed && !state.kickoff_formation) ||
       (!kickoff_placed && !(phase>=9 && phase<=13) && !(phase>=18 && phase<=20) && !kickoff_shot && state.kickoff_formation))
        throw std::invalid_argument("Kickoff formation phase needs captured keeper positions");
    if (bool(state.kickoff_receiver)!=(phase>=62 && phase<=64) ||
        (state.kickoff_receiver && (*state.kickoff_receiver<=state.current.field.side ||
         *state.kickoff_receiver>state.current.field.side+10)))
        throw std::invalid_argument("Kickoff receiver requires a field player on the selected side");
    if (phase==63) {
        const auto& field=state.current.field;
        const auto& owner=field.motion[field_motion_index(field.ball_owner)];
        if(field.ball_x!=(owner.x>>8) || field.ball_y!=(owner.y>>8) ||
           field.ball_cell!=field_cell_at(field.ball_x,field.ball_y))
            throw std::invalid_argument("Kickoff ball must be at the initial carrier");
    }
    const bool sampled_right_goal_formation=(phase>=61 && phase<=64) && state.score && state.score->left_goals==0 && state.score->right_goals==1 && state.score->kickoff==4 &&
       state.current.field.side==0 && state.current.field.ball_owner==(phase==64 ? 7 : 8) && (phase==61 || (state.kickoff_receiver && *state.kickoff_receiver==7)) && state.current.field.elapsed==1 && state.current.field.selected_defenders==1 &&
       state.scheduling && state.scheduling->selected_ids[0]==0 && state.recovery && state.recovery->opposing_player==0 &&
       state.current.field.random_first==127 && state.current.field.random_second==128 && !state.current.field.keepers[1].penalty;
    if(kickoff_placed && !sampled_right_goal_formation && (!state.score || state.score->kickoff!=4 || !state.scheduling ||
       (phase==64 ? state.current.field.ball_owner!=*state.kickoff_receiver :
                    state.current.field.ball_owner!=state.current.field.side+8) || state.current.field.elapsed ||
       state.current.field.selected_defenders>1 || (state.current.field.selected_defenders && state.scheduling->selected_ids[0]!=11)))
        throw std::invalid_argument("Prepared kickoff formation requires the selected side and initial carrier");
    if(phase==64 && (!state.control || state.control->controlled_player!=1))
        throw std::invalid_argument("Kickoff control handoff needs the initialized controlled player");
    if(phase>=53 && phase<=56) {
        const auto& f=state.current.field;
        const auto keeper=f.side^11u;
        if(!state.recovery || state.recovery->opposing_player!=keeper ||
           (keeper_selected && (f.selected_defenders!=1 || !state.scheduling ||
             state.scheduling->selected_ids[0]!=keeper || f.keepers[f.side==0?1:0].stun)) ||
           (!keeper_selected && (!f.keepers[f.side==0?1:0].stun || !state.action || state.action->action)))
            throw std::invalid_argument("Keeper phase does not match opposing keeper setup/status");
    }
    if(keeper_possession && (state.current.field.side!=11 || state.current.field.ball_owner!=11 ||
       state.current.field.selected_defenders!=1 || !state.scheduling || state.scheduling->selected_ids[0]!=11 ||
       !state.recovery || state.recovery->opposing_player!=11 || state.current.field.elapsed || state.current.field.keepers[1].stun))
        throw std::invalid_argument("Keeper possession requires its own captured keeper and selected prefix");
    if(phase==58 && (state.current.field.side!=0 || state.current.field.selected_defenders || state.current.field.elapsed!=1 ||
       !state.recovery || state.recovery->opposing_player!=11))
        throw std::invalid_argument("Punch recovery requires the prepared empty no-jitter handoff");
    if(phase==59 && (!state.score || !state.keeper_decision || state.keeper_decision->result_code>1 ||
       !state.recovery || state.recovery->opposing_player!=11 ||
       (state.keeper_decision->result_code==1 && (state.recovery->attack_subtype&63u)>=3) ||
       state.current.field.selected_defenders!=(state.current.field.keepers[1].stun?0:1) ||
       (state.current.field.selected_defenders && (!state.scheduling || state.scheduling->selected_ids[0]!=11)) ||
       (state.current.field.keepers[1].stun && (state.keeper_decision->defense_action || state.keeper_decision->defense_subtype || state.keeper_decision->effect))))
        throw std::invalid_argument("Goal celebration needs a scored keeper result and matching normal/stunned prefix");
    const bool sampled_right_goal_restart=goal_restart && state.score && state.score->left_goals==0 && state.score->right_goals==1 && state.score->kickoff==4 &&
       state.current.clock.ball_mode==0 && state.current.field.side==0 && state.current.field.ball_owner==12 && state.current.field.elapsed==1 &&
       state.current.field.ball_x==56 && state.current.field.ball_y==124 && state.current.field.ball_cell==17 &&
       state.current.field.random_first==127 && state.current.field.random_second==128 && state.current.field.selected_defenders==1 &&
       state.scheduling && state.scheduling->selected_ids[0]==0 && state.recovery && state.recovery->opposing_player==0 &&
       !state.current.field.keepers[0].timer && !state.current.field.keepers[0].penalty && !state.current.field.keepers[0].stun;
    if(goal_restart && !sampled_right_goal_restart && (!state.score || state.score->kickoff!=4 || state.current.clock.ball_mode!=0 ||
       state.current.field.side!=11 || state.current.field.ball_owner<1 || state.current.field.ball_owner>10 ||
       state.current.field.elapsed || state.current.field.selected_defenders>1 || !state.scheduling || !state.recovery ||
       state.recovery->opposing_player!=11 || state.current.field.keepers[1].timer || state.current.field.keepers[1].penalty ||
       state.current.field.keepers[1].stun || (state.current.field.selected_defenders && state.scheduling->selected_ids[0]!=11)))
        throw std::invalid_argument("Post-goal kickoff requires switched side, saved score and cleared keeper");
    const bool restart_options = phase >= 40 && phase <= 42;
    if (bool(state.restart_options) != restart_options)
        throw std::invalid_argument("Restart options require exactly one menu context");
    if (state.restart_options) {
        const auto cursor = state.restart_options->cursor;
        if ((state.phase == FieldSessionPhase::restart_options && cursor != 4 && cursor != 68) ||
            (state.phase == FieldSessionPhase::restart_tactics && cursor != 0) ||
            (state.phase == FieldSessionPhase::restart_options_done && cursor != 4) ||
            state.current.clock.ball_mode != 0)
            throw std::invalid_argument("Invalid restart menu boundary");
    }
    const bool race_pending = (phase>=34 && phase<=37) || area_action6_race;
    const bool clearance = state.phase == FieldSessionPhase::capture_clearance_flight || race_pending;
    const bool transitional_clearance = state.phase == FieldSessionPhase::capture_clearance_flight ||
        state.phase == FieldSessionPhase::clearance_race_timeout || state.phase == FieldSessionPhase::clearance_recovery_ready;
    const bool post_possession = area_flight || area_action4 || area_action6 || ((state.phase == FieldSessionPhase::keeper_interaction || state.phase == FieldSessionPhase::story_transition) && state.current.field.selected_defenders != 0) || state.phase == FieldSessionPhase::selected_recovered_owner ||
        state.phase == FieldSessionPhase::keeper_area_recovered_owner || clearance || keeper_possession || goal_restart || kickoff_placed;
    if (bool(state.clearance_flight) != clearance)
        throw std::invalid_argument("Clearance flight phase requires exactly one setup context");
    if (transitional_clearance && (state.current.field.side != 11 || state.current.field.ball_owner == 0 || state.current.field.ball_owner >= 11))
        throw std::invalid_argument("Clearance requires the original side0 carrier and active side11");
    if (bool(state.clearance_race) != race_pending)
        throw std::invalid_argument("Race handoff requires exactly one race context");
    if (state.clearance_race) {
        const auto& r = *state.clearance_race;
        if (r.active_candidate < 12 || r.active_candidate > 21 || r.opposing_candidate < 1 || r.opposing_candidate > 10 ||
            (phase==113 ? state.current.field.elapsed>1 : state.current.field.elapsed != ((state.phase == FieldSessionPhase::clearance_recovered_owner || phase==109 || phase==110) ? 0 : 1)) ||
            (((phase>=35 && phase<=37) || phase==107 || phase==109 || phase==110) && r.remaining != 0) ||
            (state.phase == FieldSessionPhase::clearance_race_winner &&
             state.current.field.ball_owner != r.active_candidate && state.current.field.ball_owner != r.opposing_candidate))
            throw std::invalid_argument("Unsupported clearance race handoff");
    }
    const bool staged = phase >= 23 && phase <= 46;
    if (staged != bool(state.shot_progress) && !(state.phase == FieldSessionPhase::period_end && state.shot_progress))
        throw std::invalid_argument("Staged shot phase requires exactly one progress context");
    if (state.shot_progress) {
        const auto& p = *state.shot_progress;
        const auto count = state.current.field.selected_defenders;
        if (!state.scheduling || !state.action || !state.recovery || !state.control || !state.experience ||
            (state.current.field.side != 0 && !post_possession) || state.action->action != 0 || count > 5 ||
            p.cursor > count || p.result_code > 4 ||
            (p.defense_action != 0 && p.defense_action != 2) ||
            (p.defense_subtype != 0 && p.defense_subtype != 128))
            throw std::invalid_argument("Unsupported shot progress context");
        if ((state.phase == FieldSessionPhase::selected_ball_recovery && p.result_code != 2) ||
            (state.phase == FieldSessionPhase::selected_defense_animation &&
             (p.result_code != 3 || p.defense_action != 0)) ||
            (state.phase == FieldSessionPhase::selected_restart_entry && p.result_code != 4) ||
            ((state.phase == FieldSessionPhase::selected_recovered_owner ||
              state.phase == FieldSessionPhase::selected_boundary_restart) && p.result_code != 2) ||
            (clearance && (p.result_code != 3 || p.defense_action != 0)) ||
            (phase>=38 && phase<=42 && p.result_code!=4) ||
            (phase >= 28 && phase<=42 && p.cursor == 0))
            throw std::invalid_argument("Selected handoff phase does not match its terminal result");
        if (phase >= 43 && phase <= 46 &&
            (p.result_code >= 2 || p.cursor != count))
            throw std::invalid_argument("Attacking presentation requires a completed nonterminal prefix");
        if (phase >= 44 && (state.control->last_carrier_cell ||
            state.action->menu_route || (p.cursor && state.recovery->opposing_player != p.defender_id)))
            throw std::invalid_argument("Attack clock state needs ordinary selected field context");
        if (p.cursor == 0) {
            if (p.result_code || p.defender_id || p.defense_action || p.defense_subtype)
                throw std::invalid_argument("Unused shot contact data must be zero");
        } else if (p.defender_id != state.scheduling->selected_ids[p.cursor - 1])
            throw std::invalid_argument("Shot cursor must retain the last selected defender");
        if (state.phase == FieldSessionPhase::shot_defender_next && (p.cursor >= count || p.result_code >= 2))
            throw std::invalid_argument("No next selected defender");
        if (state.phase == FieldSessionPhase::shot_contact_added_time && p.cursor == 0)
            throw std::invalid_argument("Contact clock pause requires a completed contact");
        if ((state.phase == FieldSessionPhase::shot_dispatch_clock ||
             state.phase == FieldSessionPhase::shot_dispatch_added_time ||
             state.phase == FieldSessionPhase::shot_dispatch_ready) &&
            p.result_code < 2 && p.cursor != count)
            throw std::invalid_argument("Nonterminal dispatch requires the completed prefix");
        if ((state.phase == FieldSessionPhase::shot_dispatch_clock ||
             state.phase == FieldSessionPhase::shot_dispatch_added_time) && p.result_code >= 2)
            throw std::invalid_argument("Terminal contact skips final dispatch clock");
    }
    if ((state.phase == FieldSessionPhase::added_time) != bool(state.pending))
        throw std::invalid_argument("Added-time session needs exactly one pending terminal contact");
    if (state.pending) validate_contact(state.current.field, *state.pending);
    if (state.experience) validate_experience_state(*state.experience);
    if (state.recovery && state.recovery->opposing_player > 21)
        throw std::invalid_argument("Unsupported recovery opposing player");
    if (static_cast<unsigned>(state.phase) >= 5 && !state.recovery)
        throw std::invalid_argument("Recovery continuation needs captured recovery context");
    const bool selected_phase = state.phase == FieldSessionPhase::field_contact_pending ||
        ((state.phase == FieldSessionPhase::keeper_interaction || state.phase == FieldSessionPhase::story_transition) && state.current.field.selected_defenders != 0) || static_cast<unsigned>(state.phase) >= 18;
    if ((state.phase == FieldSessionPhase::keeper_interaction || state.phase == FieldSessionPhase::story_transition) && state.current.field.selected_defenders && !state.scheduling)
        throw std::invalid_argument("Selected recovery continuation needs saved scheduling context");
    if (static_cast<unsigned>(state.phase) >= 5 && !selected_phase &&
        state.current.field.selected_defenders != 0)
        throw std::invalid_argument("Recovery continuation requires zero selected defenders");
    if (static_cast<unsigned>(state.phase) >= 6 && state.phase != FieldSessionPhase::selected_boundary_restart) {
        if(!keeper_possession) field_motion_index(state.current.field.ball_owner);
        if (!transitional_clearance && !goal_restart && (state.current.field.ball_owner < 11 ? 0 : 11) != state.current.field.side)
            throw std::invalid_argument("Recovered owner must belong to the possession side");
        const auto& field = state.current.field;
        if (phase!=177 && phase!=178 && phase!=179 && (field.ball_x < 48 || field.ball_x >= 208 || field.ball_y < 80 || field.ball_y >= 176))
            throw std::invalid_argument("Recovered-owner continuation requires an interior ball");
    }
    if (static_cast<unsigned>(state.phase) >= 9 && (!state.control || !state.experience))
        throw std::invalid_argument("Control-ready state needs captured control/experience context (phase " + std::to_string(phase) + ", control " + std::to_string(bool(state.control)) + ", experience " + std::to_string(bool(state.experience)) + ")");
    if (static_cast<unsigned>(state.phase) >= 9 && !post_possession && state.current.field.side == 11 &&
        (state.control->controlled_player < 1 || state.control->controlled_player > 10))
        throw std::invalid_argument("Opposing control-ready state needs a side0 field defender");
    if (static_cast<unsigned>(state.phase) >= 13 && !state.scheduling)
        throw std::invalid_argument("Control-cycle continuation needs captured scheduling context");
    if ((state.phase == FieldSessionPhase::field_round_pending || state.phase == FieldSessionPhase::control_added_time) &&
        state.current.field.elapsed != 1)
        throw std::invalid_argument("Control-cycle continuation requires elapsed1");
    if (state.scheduling) {
        const auto count = state.current.field.selected_defenders;
        if (count > 5) throw std::invalid_argument("Selected defender prefix exceeds five slots");
        for (unsigned i = 0; i < 5; ++i) {
            if (i < count && !keeper_selected && !keeper_possession && !goal_restart && !kickoff_placed) field_motion_index(state.scheduling->selected_ids[i]);
            else if (i < count) { /* Keeper prefix checked above; not a field-motion slot. */ }
            else if (state.scheduling->selected_ids[i] != 0)
                throw std::invalid_argument("Nonzero unused selected defender slot");
        }
    }
    if (selected_phase) {
        const auto& field = state.current.field;
        if ((state.phase == FieldSessionPhase::field_contact_pending && field.selected_defenders == 0) ||
            field.selected_defenders > (field.side == 0 || post_possession ? 5 : 4))
            throw std::invalid_argument("Field-contact continuation needs a selected opposing prefix");
        for (unsigned i = 0; i < field.selected_defenders; ++i)
            if (area_flight && (state.scheduling->selected_ids[i]<1 || state.scheduling->selected_ids[i]>10))
                throw std::invalid_argument("Area flight selected players must oppose possession");
            else if (!post_possession &&
                (state.scheduling->selected_ids[i] < 11 ? 0 : 11) == field.side)
                throw std::invalid_argument("Selected contact player must oppose possession");
    }
    if (state.action && state.action->action > 9 &&
        (static_cast<unsigned>(state.phase) >= 19 ||
         (state.phase == FieldSessionPhase::human_action_menu && (state.action->cursor & 128u))))
        throw std::invalid_argument("Unsupported captured action ID");
    if (static_cast<unsigned>(state.phase) >= 18 && (!state.action || (state.current.field.side != 0 && !post_possession && !area_state && !(phase>=194 && phase<=201)) || state.action->menu_route >= 6))
        throw std::invalid_argument("Human attacking menu requires captured action context and side0");
    if (static_cast<unsigned>(state.phase) >= 20 && !(phase>=194 && phase<=201) && !area_flight && !area_action4 && !area_action6 && phase!=78 && phase!=79 && phase!=80 && phase!=81 && phase!=82 && phase!=83 && phase!=84 && phase!=85 && phase!=86 && phase!=87 && phase!=88 && phase!=89 && phase!=90 && phase!=91 && phase!=92 && phase!=93 && phase!=94 && state.action->action != 0)
        throw std::invalid_argument("Only ordinary shots are supported at attack-prepared phase");
}

PhaseSessionResult resolve_session_terminal(const PhaseSessionState& input,
    PendingTerminalContact contact, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ContactPreludeContent& prelude) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::field && input.phase != FieldSessionPhase::player_control &&
        input.phase != FieldSessionPhase::ball_recovery && input.phase != FieldSessionPhase::attack_prepared)
        throw std::invalid_argument("Session is waiting at a continuation boundary");
    validate_contact(input.current.field, contact);
    auto state = input;
    if (state.recovery) state.recovery->opposing_player = contact.defender_id;
    if (contact.result_code == 2) {
        const auto& defender = state.current.field.motion[field_motion_index(contact.defender_id)];
        state.current.field.ball_x = static_cast<std::uint8_t>(defender.x >> 8);
        state.current.field.ball_y = static_cast<std::uint8_t>(defender.y >> 8);
    }
    auto applied = apply_contact_prelude({state.current, contact.defender_id, contact.defense_action, false}, prelude);
    state.current = applied.state;
    if (applied.clock.continuation == ClockContinuation::period_end) {
        state.phase = FieldSessionPhase::period_end;
        state.pending.reset();
        return {std::move(state), std::move(applied), std::nullopt};
    }
    if (applied.clock.continuation == ClockContinuation::added_time_presentation) {
        state.phase = FieldSessionPhase::added_time;
        state.pending = contact;
        return {std::move(state), std::move(applied), std::nullopt};
    }
    auto result = finish(std::move(state), contact, primary, movement);
    result.prelude = std::move(applied);
    return result;
}

PhaseSessionResult resume_session_terminal(const PhaseSessionState& post_presentation,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement) {
    validate_phase_session(post_presentation);
    if (post_presentation.phase != FieldSessionPhase::added_time || !post_presentation.pending)
        throw std::invalid_argument("No added-time terminal contact to resume");
    return finish(post_presentation, *post_presentation.pending, primary, movement);
}

PhaseSessionResult resolve_session_rewarded_terminal(const PhaseSessionState& input,
    PendingTerminalContact contact, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ContactPreludeContent& prelude,
    const ExperienceContent& experience) {
    validate_phase_session(input);
    validate_contact(input.current.field, contact);
    if (!input.experience) throw std::invalid_argument("Rewarded contact needs captured experience context");
    auto state = input;
    std::vector<ExperienceRewardResult> rewards;
    const auto count = contact.result_code == 2 ? 2u : 1u;
    for (unsigned i = 0; i < count; ++i) {
        auto reward = award_player_experience(*state.experience,
            state.current.field.roster[contact.defender_id].type, 2, experience);
        state.experience = reward.state;
        rewards.push_back(std::move(reward));
    }
    auto result = resolve_session_terminal(state, contact, primary, movement, prelude);
    result.rewards = std::move(rewards);
    return result;
}
}

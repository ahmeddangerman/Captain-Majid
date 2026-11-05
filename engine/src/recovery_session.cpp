#include "captain/recovery_session.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>
#include <utility>

namespace captain {
PhaseSessionState continue_session_area_pass_blocked_recovery(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_pass_blocked_recovered_owner || (facing!=0 && facing!=64))
  throw std::invalid_argument("Blocked pass continuation needs recovered owner and captured presentation");
 const auto& field=input.current.field;const auto& owner=field.motion[field_motion_index(field.ball_owner)];
 if((field.ball_y>=96 && field.ball_y<160 && (field.ball_x<80 || field.ball_x>=176)) ||
    (input.recovery->game_mode==4 && field.side==0 && field.roster[field.ball_owner].type!=1 && (owner.x>>8)>=128))
  throw std::invalid_argument("Blocked pass awaits keeper-area or story continuation");
 auto state=input;state.control->facing=facing;state.control->motion_requested=motion;
 state.phase=FieldSessionPhase::area_pass_blocked_control_entry;validate_phase_session(state);return state;
}
PhaseSessionState prepare_area_pass_keeper(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_pass_keeper_entry)
  throw std::invalid_argument("Area pass keeper setup needs its saved entry");
 const auto& f=input.current.field;
 if(f.ball_y<96 || f.ball_y>=160 || (f.ball_x>=80 && f.ball_x<176))
  throw std::invalid_argument("Area pass keeper setup requires an in-area recipient");
 auto state=input;auto& field=state.current.field;
 if(!state.recovery->ball_variant)state.recovery->ball_variant=(field.random_first&1u)+1;
 state.action->menu_route=(field.ball_x<80 ? 0 : 11)==field.side ? 2 : 1;
 field.selected_defenders=0;state.scheduling->selected_ids.fill(0);state.shot_flight->counter=255;
 FlightDefenderSelectionInput selection{};selection.possession_side=field.side;selection.ball_cell=field.ball_cell;
 selection.random_first=field.random_first;selection.random_second=field.random_second;selection.threshold=255;
 for(unsigned i=0;i<field.motion.size();++i){const auto& p=field.motion[i];selection.players[i]={static_cast<std::uint8_t>(p.x>>8),static_cast<std::uint8_t>(p.y>>8),p.status!=0};}
 const auto selected=select_flight_defenders(selection);field.selected_defenders=static_cast<std::uint8_t>(selected.size());
 for(unsigned i=0;i<selected.size();++i)state.scheduling->selected_ids[i]=selected[i];
 state.keeper_area=KeeperAreaContext{};state.phase=FieldSessionPhase::area_pass_keeper_prepared;
 validate_phase_session(state);return state;
}
PhaseSessionState prepare_session_keeper_area(const PhaseSessionState& input) {
 validate_phase_session(input);
 const auto& f=input.current.field;
 if(input.phase!=FieldSessionPhase::keeper_interaction || !input.action || !input.scheduling ||
    input.action->action || f.ball_y<96 || f.ball_y>=160 || (f.ball_x>=80 && f.ball_x<176))
  throw std::invalid_argument("Keeper-area setup requires an ordinary recovered ball in the goal area");
 auto state=input;auto& field=state.current.field;
 if(!state.recovery->ball_variant)state.recovery->ball_variant=(field.random_first&1u)+1;
 state.action->menu_route=(field.ball_x<80 ? 0 : 11)==field.side ? 2 : 1;
 field.selected_defenders=0;state.scheduling->selected_ids.fill(0);
 FlightDefenderSelectionInput selection{};
 selection.possession_side=field.side;selection.ball_cell=field.ball_cell;
 selection.random_first=field.random_first;selection.random_second=field.random_second;selection.threshold=255;
 for(unsigned i=0;i<field.motion.size();++i) {
  const auto& m=field.motion[i];selection.players[i]={static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8),m.status!=0};
 }
 const auto selected=select_flight_defenders(selection);
 field.selected_defenders=static_cast<std::uint8_t>(selected.size());
 for(unsigned i=0;i<selected.size();++i)state.scheduling->selected_ids[i]=selected[i];
 state.keeper_area=KeeperAreaContext{};state.phase=FieldSessionPhase::keeper_area_prepared;
 validate_phase_session(state);return state;
}
namespace {
void validate_selected_actions(const PhaseSessionState& input, const std::vector<RecoveryDefender>& selected) {
    const auto& progress = *input.shot_progress;
    for (std::size_t i=0;i<selected.size();++i) {
        const auto& choice = selected[i];
        if (choice.player_id != input.scheduling->selected_ids[i] || choice.defense_action > 6 ||
            (i < progress.cursor && choice.defense_action != 0 && choice.defense_action != 2) ||
            (i + 1 == progress.cursor && choice.defense_action != progress.defense_action))
            throw std::invalid_argument("Selected recovery actions must match the captured prefix");
    }
}
RecoverySessionResult recover(const PhaseSessionState& input,
    const std::vector<RecoveryDefender>& selected, bool retained_prefix, bool jitter,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement,
    FieldSessionPhase no_jitter_output=FieldSessionPhase::clearance_recovered_owner,
    FieldSessionPhase jitter_output=FieldSessionPhase::recovered_owner,
    bool clear_selection_on_boundary=false) {
    RecoverySessionResult result{input};
    auto& state = result.state;
    auto& field = state.current.field;
    auto& context = *state.recovery;
    if (jitter) {
        // Original9070 cleans the special owner; it is not a random-generator call.
        if ((context.special_owner_flags & 128u) && field.ball_owner == 20) {
            context.special_owner_flags = 0;
            field.roster[20].stamina &= 0xFF00u;
            result.special_owner_cleared = true;
        }
        const auto ball = jitter_loose_ball(field.ball_x, field.ball_y, field.random_first, field.random_second);
        field.ball_x = ball.x;
        field.ball_y = ball.y;
        context.x_jitter_control = ball.x_jitter_control;
        if (!ball.inside_field) {
            if(clear_selection_on_boundary) {
                field.selected_defenders=0;
                if(state.scheduling)state.scheduling->selected_ids.fill(0);
                state.kickoff_formation.reset();
            }
            state.phase = retained_prefix ? FieldSessionPhase::selected_boundary_restart : FieldSessionPhase::boundary_restart;
            validate_phase_session(state);
            return result;
        }
    }
    result.movement = advance_field_phase(field, primary, movement);
    field = result.movement->state;
    BallRecoveryInput choice{};
    choice.selected_defenders = selected;
    choice.ball_x = field.ball_x; choice.ball_y = field.ball_y;
    choice.ball_owner = field.ball_owner; choice.opposing_player = context.opposing_player;
    choice.random_e2 = field.random_first; choice.random_e3 = field.random_second;
    choice.ball_update_active = field.elapsed != 0;
    for (std::size_t i = 0; i < field.motion.size(); ++i) {
        const auto& actor = field.motion[i];
        choice.players[i] = {static_cast<std::uint8_t>(actor.x >> 8),
            static_cast<std::uint8_t>(actor.y >> 8), actor.status != 0};
    }
    result.recipient = recover_stationary_ball(choice);
    const auto& recovered = *result.recipient;
    result.request_possession_sound = field.side != recovered.possession_side;
    field.ball_owner = recovered.ball_owner;
    field.side = recovered.possession_side;
    auto& owner = field.motion[field_motion_index(field.ball_owner)];
    owner.x = static_cast<std::uint16_t>((owner.x & 255u) | (unsigned(field.ball_x) << 8));
    owner.y = static_cast<std::uint16_t>((owner.y & 255u) | (unsigned(field.ball_y) << 8));
    context.attack_subtype = recovered.attack_subtype;
    if (context.ball_variant == 0)
        context.ball_variant = static_cast<std::uint8_t>((field.random_first & 1u) + 1u);
    state.phase = !jitter ? no_jitter_output :
        retained_prefix ? FieldSessionPhase::selected_recovered_owner : jitter_output;
    validate_phase_session(state);
    return result;
}

}
RecoverySessionResult advance_session_keeper_punch_recovery(const PhaseSessionState& input,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_punch_recovery)
  throw std::invalid_argument("Keeper punch recovery requires prepared no-jitter handoff");
 auto state=input;state.phase=FieldSessionPhase::ball_recovery;
 // Prepared completion of the earlier shot animation; original EC3E clears0532
 // during the cooperative recovery wait. This does not implement its renderer.
 if(state.kickoff_formation)state.control->motion_requested=0;
 return recover(state,{},false,false,primary,movement,FieldSessionPhase::recovered_owner);
}
RecoverySessionResult advance_session_shot_loose_recovery(const PhaseSessionState& input,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::shot_loose_recovery)
  throw std::invalid_argument("Loose recovery requires prepared post-presentation shot entry");
 auto prepared=input;prepared.shot_flight.reset();prepared.ball_trajectory.reset();
 prepared.phase=FieldSessionPhase::ball_recovery;
 return recover(prepared,{},false,false,primary,movement,FieldSessionPhase::recovered_owner);
}
RecoverySessionResult advance_session_recovery(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::ball_recovery || !input.recovery)
        throw std::invalid_argument("Recovery requires ball-recovery phase and captured context");
    if (input.current.field.selected_defenders != 0)
        throw std::invalid_argument("Connected recovery requires zero selected defenders");
    return recover(input,{},false,true,primary,movement);
}
RecoverySessionResult advance_session_area_pass_blocked_recovery(const PhaseSessionState& input,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_pass_blocked_recovery_entry)
  throw std::invalid_argument("Blocked pass recovery needs its saved recovery entry");
 auto prepared=input;prepared.phase=FieldSessionPhase::ball_recovery;prepared.action=ActionContext{};
 prepared.area_ai_target.reset();prepared.area_contact.reset();prepared.area_pass_contact.reset();
 prepared.shot_flight.reset();prepared.ball_trajectory.reset();prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
 validate_phase_session(prepared);
 auto result=recover(prepared,{},false,true,primary,movement);auto& state=result.state;
 const bool boundary=state.phase==FieldSessionPhase::boundary_restart;
 state.action=input.action;state.area_ai_target=input.area_ai_target;state.area_contact=input.area_contact;state.area_pass_contact=input.area_pass_contact;
 state.shot_flight=input.shot_flight;state.ball_trajectory=input.ball_trajectory;state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 state.ball_trajectory->x=static_cast<std::uint16_t>((unsigned(state.current.field.ball_x)<<8)|(input.ball_trajectory->x&255u));
 state.ball_trajectory->y=static_cast<std::uint16_t>((unsigned(state.current.field.ball_y)<<8)|(input.ball_trajectory->y&255u));
 state.ball_trajectory->direction=state.recovery->x_jitter_control;
 state.phase=boundary ? FieldSessionPhase::area_pass_blocked_boundary : FieldSessionPhase::area_pass_blocked_recovered_owner;
 validate_phase_session(state);return result;
}
RecoverySessionResult advance_session_area_result_recovery(const PhaseSessionState& input,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::keeper_area_recovery_ready || !input.area_contact ||
       (input.area_contact->result!=2 && !(input.area_contact->result==3 && input.area_contact->defense_action==4)))
        throw std::invalid_argument("Area loose-ball recovery requires result2 or result3/action4");
    std::vector<RecoveryDefender> selected;
    selected.reserve(input.current.field.selected_defenders);
    for(unsigned i=0;i<input.current.field.selected_defenders;++i) {
        const auto action=input.area_defense->actions[i];
        if(action==255 || action>6)throw std::invalid_argument("Area recovery needs every selected defense action");
        selected.push_back({input.scheduling->selected_ids[i],action});
    }
    auto state=input;
    state.phase=FieldSessionPhase::ball_recovery;
    state.keeper_area.reset();state.area_defense.reset();state.area_ai_target.reset();state.area_contact.reset();
    state.area_defender_cursor=0;state.area_sequence_result=255;
    state.action=ActionContext{};
    validate_phase_session(state);
    return recover(state,selected,false,true,primary,movement,
        FieldSessionPhase::recovered_owner,FieldSessionPhase::keeper_area_recovered_owner,true);
}
RecoverySessionResult advance_session_selected_recovery(const PhaseSessionState& input,
    const std::vector<RecoveryDefender>& selected,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::selected_ball_recovery ||
        selected.size() != input.current.field.selected_defenders)
        throw std::invalid_argument("Selected recovery requires its complete saved prefix");
    validate_selected_actions(input,selected);
    return recover(input,selected,true,true,primary,movement);
}
RecoverySessionResult advance_session_area_action6_recovery(const PhaseSessionState& input,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action6_race_timeout || !input.area_defense)
  throw std::invalid_argument("Area action6 recovery needs retained defender choices after timeout");
 std::vector<RecoveryDefender> selected;selected.reserve(input.current.field.selected_defenders);
 for(unsigned i=0;i<input.current.field.selected_defenders;++i){
  const auto action=input.area_defense->actions[i];if(action>6)throw std::invalid_argument("Area action6 recovery needs every selected choice");
  selected.push_back({input.scheduling->selected_ids[i],action});
 }
 auto prepared=input;prepared.phase=FieldSessionPhase::ball_recovery;prepared.action=ActionContext{};
 prepared.area_ai_target.reset();prepared.area_defense.reset();prepared.clearance_flight.reset();prepared.clearance_race.reset();
 prepared.area_defender_cursor=0;prepared.area_sequence_result=255;validate_phase_session(prepared);
 auto result=recover(prepared,selected,false,false,primary,movement,FieldSessionPhase::keeper_area_recovered_owner);
 auto& state=result.state;state.action=input.action;state.area_ai_target=input.area_ai_target;state.area_defense=input.area_defense;
 state.clearance_flight=input.clearance_flight;state.clearance_race=input.clearance_race;
 state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 state.phase=FieldSessionPhase::area_action6_recovered_owner;validate_phase_session(state);return result;
}
PhaseSessionState continue_session_area_action6_recovery(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action6_recovered_owner || (facing!=0 && facing!=64))
  throw std::invalid_argument("Area action6 recovery needs a saved owner and acknowledged presentation");
 const auto& f=input.current.field;const auto& owner=f.motion[field_motion_index(f.ball_owner)];
 if((f.ball_y>=96 && f.ball_y<160 && (f.ball_x<80 || f.ball_x>=176)) ||
    (input.recovery->game_mode==4 && f.side==0 && f.roster[f.ball_owner].type!=1 && (owner.x>>8)>=128))
  throw std::invalid_argument("Area action6 recovery awaits keeper or story continuation");
 auto state=input;state.control->facing=facing;state.control->motion_requested=motion;
 state.phase=FieldSessionPhase::area_action6_recovered_control;validate_phase_session(state);return state;
}
RecoverySessionResult advance_session_clearance_recovery(const PhaseSessionState& input,
    const std::vector<RecoveryDefender>& selected,const PlayerProfileMappings& primary,const FieldPhaseContent& movement) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::clearance_recovery_ready || selected.size()!=input.current.field.selected_defenders)
        throw std::invalid_argument("Clearance recovery requires its saved complete prefix");
    validate_selected_actions(input,selected);
    return recover(input,selected,true,false,primary,movement);
}
PhaseSessionState continue_session_selected_recovery(const PhaseSessionState& input) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::selected_recovered_owner && input.phase != FieldSessionPhase::clearance_recovered_owner)
        throw std::invalid_argument("No selected recovered-owner handoff to continue");
    const auto& field=input.current.field;
    const auto& owner=field.motion[field_motion_index(field.ball_owner)];
    if ((field.ball_y >= 96 && field.ball_y < 160 && (field.ball_x < 80 || field.ball_x >= 176)) ||
        (input.recovery->game_mode == 4 && field.side == 0 &&
         field.roster[field.ball_owner].type != 1 && (owner.x >> 8) >= 128))
        throw std::invalid_argument("Selected recovery awaits keeper-area or story continuation");
    auto state=input;
    state.shot_progress.reset();state.clearance_flight.reset();state.clearance_race.reset();
    state.phase=FieldSessionPhase::player_control;
    validate_phase_session(state);
    return state;
}

PhaseSessionState continue_session_area_ordinary_recovery(const PhaseSessionState& input,std::uint8_t facing) {
    validate_phase_session(input);
    if (input.phase!=FieldSessionPhase::keeper_area_recovered_owner || (facing!=0 && facing!=64))
        throw std::invalid_argument("Area ordinary recovery needs a saved handoff and captured facing");
    const auto& field=input.current.field;
    const auto& owner=field.motion[field_motion_index(field.ball_owner)];
    if ((field.ball_y>=96 && field.ball_y<160 && (field.ball_x<80 || field.ball_x>=176)) ||
        (input.recovery->game_mode==4 && field.side==0 && field.roster[field.ball_owner].type!=1 && (owner.x>>8)>=128))
        throw std::invalid_argument("Area recovery awaits keeper-area or story continuation");
    auto state=input;
    state.control->facing=facing;
    state.phase=FieldSessionPhase::player_control;
    validate_phase_session(state);
    return state;
}

PhaseSessionState continue_session_area_story_recovery(const PhaseSessionState& input,std::uint8_t facing) {
    validate_phase_session(input);
    if (input.phase!=FieldSessionPhase::keeper_area_recovered_owner || (facing!=0 && facing!=64))
        throw std::invalid_argument("Area story recovery needs a saved handoff and captured facing");
    const auto& field=input.current.field;
    const auto& owner=field.motion[field_motion_index(field.ball_owner)];
    if ((field.ball_y>=96 && field.ball_y<160 && (field.ball_x<80 || field.ball_x>=176)) ||
        !(input.recovery->game_mode==4 && field.side==0 && field.roster[field.ball_owner].type!=1 && (owner.x>>8)>=128))
        throw std::invalid_argument("Area recovery is not at the story gate");
    auto state=input;
    state.control->facing=facing;
    state.phase=FieldSessionPhase::story_transition;
    validate_phase_session(state);
    return state;
}

PhaseSessionState continue_session_recovery(const PhaseSessionState& post_presentation) {
    validate_phase_session(post_presentation);
    const bool area = post_presentation.phase == FieldSessionPhase::keeper_area_recovered_owner;
    if ((post_presentation.phase != FieldSessionPhase::recovered_owner && !area) || !post_presentation.recovery)
        throw std::invalid_argument("No recovered-owner handoff to continue");
    auto state = post_presentation;
    const auto& field = state.current.field;
    if (field.ball_y >= 96 && field.ball_y < 160 && (field.ball_x < 80 || field.ball_x >= 176)) {
        state.phase = FieldSessionPhase::keeper_interaction;
        validate_phase_session(state);
        return state;
    }
    if (area) throw std::invalid_argument("Area recovery outside keeper-area needs a separate continuation");
    const auto& owner = field.motion[field_motion_index(field.ball_owner)];
    if (state.recovery->game_mode == 4 && field.side == 0 &&
        field.roster[field.ball_owner].type != 1 && (owner.x >> 8) >= 128) {
        state.phase = FieldSessionPhase::story_transition;
        validate_phase_session(state);
        return state;
    }
    state.phase = FieldSessionPhase::player_control;
    validate_phase_session(state);
    return state;
}
}

#include "captain/control_session.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>

namespace captain {
ControlSetupResult prepare_player2_one_two_control(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::player3_one_two_ball_recovered || input.current.field.random_first!=123 ||
    input.current.field.random_second!=65 || !input.control || input.control->facing!=64 || input.control->motion_requested)
  throw std::invalid_argument("Player2 one-two control needs its captured pre-setup boundary");
 auto state=input; state.phase=FieldSessionPhase::player_control;
 return prepare_session_control(state);
}
ControlSetupResult prepare_player3_recovered_control(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::player7_shot_ball_recovered || input.current.field.random_first!=95 ||
    input.current.field.random_second!=28 || !input.control || input.control->facing!=64 || input.control->motion_requested)
  throw std::invalid_argument("Player3 recovered control needs its captured pre-setup boundary");
 auto state=input; state.phase=FieldSessionPhase::player_control; state.area_contact.reset();
 state.area_defender_cursor=0; state.area_sequence_result=255;
 return prepare_session_control(state);
}
ControlSetupResult prepare_third_return_control(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::third_return_ball_recovered || input.current.field.random_first!=174 ||
    input.current.field.random_second!=34 || !input.control || input.control->facing!=64 || input.control->motion_requested)
  throw std::invalid_argument("Third-return control needs the captured pre-setup boundary");
 auto state=input;state.phase=FieldSessionPhase::player_control;state.area_defense.reset();
 state.area_defender_cursor=0;state.area_sequence_result=255;
 return prepare_session_control(state);
}
ControlSetupResult prepare_postgoal_return_control(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::postgoal_opponent_return_reception_wait || input.current.field.random_first!=82 ||
    input.current.field.random_second!=20 || input.control->facing || input.control->motion_requested)
  throw std::invalid_argument("Post-goal return control requires the observed pre-setup boundary");
 auto state=input;state.phase=FieldSessionPhase::player_control;state.area_ai_target.reset();state.area_contact.reset();
 state.shot_flight.reset();state.ball_trajectory.reset();
 state.area_defender_cursor=0;state.area_sequence_result=255;
 return prepare_session_control(state);
}
ControlSetupResult prepare_sampled_player5_result3_control(const PhaseSessionState& input) {
 validate_phase_session(input);
 const bool score24=input.phase==FieldSessionPhase::sampled_score24_dribble_possession;
 if(((input.phase!=FieldSessionPhase::attack_prepared || !input.human_defense_choices) && !score24) || !input.area_contact ||
    input.area_sequence_result!=3 || input.current.field.side!=11 || input.current.field.ball_owner!=(score24 ? 17 : 14) ||
    input.current.field.selected_defenders!=(score24 ? 1 : 5) || input.current.field.elapsed)
  throw std::invalid_argument("Player5 result3 control needs the observed opponent possession handoff");
 auto prepared=input;prepared.phase=FieldSessionPhase::player_control;
 prepared.human_defense_choices.reset();prepared.area_contact.reset();prepared.area_sequence_result=255;
 prepared.current.field.selected_defenders=0;prepared.scheduling->selected_ids.fill(0);
 return prepare_session_control(prepared);
}
ControlSetupResult prepare_session_area_action4_restart_pass_control(const PhaseSessionState& input) {
 validate_phase_session(input);
 const auto& f=input.current.field;
 if(input.phase!=FieldSessionPhase::area_pass_received || input.shot_flight->counter!=77 ||
    input.area_ai_target->recipient!=15 || f.ball_owner!=15 || f.side!=11 || f.ball_x!=124 || f.ball_y!=124 || f.ball_cell!=113)
  throw std::invalid_argument("Unsupported sampled restart pass control entry");
 auto state=input;state.phase=FieldSessionPhase::player_control;state.area_ai_target.reset();
 state.shot_flight.reset();state.ball_trajectory.reset();state.action=ActionContext{};
 state.area_defender_cursor=0;state.area_sequence_result=255;
 return prepare_session_control(state);
}
ControlSetupResult prepare_session_area_action4_control(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_control_entry)
  throw std::invalid_argument("Area action4 control setup requires its saved control entry");
 auto prepared=input;prepared.phase=FieldSessionPhase::player_control;
 prepared.area_ai_target.reset();prepared.area_defense.reset();prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
 auto result=prepare_session_control(prepared);auto& state=result.state;
 state.area_ai_target=input.area_ai_target;state.area_defense=input.area_defense;
 state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 state.phase=FieldSessionPhase::area_action4_control_ready;validate_phase_session(state);return result;
}
namespace {
ControlSetupResult prepare_area_pass_control(const PhaseSessionState& input,FieldSessionPhase expected,FieldSessionPhase output) {
    validate_phase_session(input);
    if(input.phase!=expected)throw std::invalid_argument("Area pass needs a saved control entry");
    auto prepared=input;prepared.phase=FieldSessionPhase::player_control;
    prepared.area_ai_target.reset();prepared.area_contact.reset();prepared.area_pass_contact.reset();
    prepared.shot_flight.reset();prepared.ball_trajectory.reset();prepared.action=ActionContext{};
    prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
    prepared.current.field.selected_defenders=0;prepared.scheduling->selected_ids.fill(0);
    auto result=prepare_session_control(prepared);auto& state=result.state;
    state.area_ai_target=input.area_ai_target;state.area_contact=input.area_contact;state.area_pass_contact=input.area_pass_contact;
    state.shot_flight=input.shot_flight;state.ball_trajectory=input.ball_trajectory;state.action=input.action;
    state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
    state.ball_trajectory->x=static_cast<std::uint16_t>((unsigned(state.current.field.ball_x)<<8)|(input.ball_trajectory->x&255u));
    state.ball_trajectory->y=static_cast<std::uint16_t>((unsigned(state.current.field.ball_y)<<8)|(input.ball_trajectory->y&255u));
    state.phase=output;validate_phase_session(state);return result;
}
}
ControlSetupResult prepare_session_area_pass_blocked_control(const PhaseSessionState& input) {
    return prepare_area_pass_control(input,FieldSessionPhase::area_pass_blocked_control_entry,FieldSessionPhase::area_pass_blocked_control_ready);
}
ControlSetupResult prepare_session_area_pass_capture_control(const PhaseSessionState& input) {
    return prepare_area_pass_control(input,FieldSessionPhase::area_pass_capture_control_entry,FieldSessionPhase::area_pass_capture_control_ready);
}
ControlSetupResult prepare_session_area_action6_control(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::area_action6_control_pending && input.phase!=FieldSessionPhase::area_action6_recovered_control)
        throw std::invalid_argument("Area action6 control setup needs a saved control handoff");
    auto prepared=input;prepared.phase=FieldSessionPhase::player_control;
    prepared.area_ai_target.reset();prepared.area_defense.reset();prepared.clearance_flight.reset();prepared.clearance_race.reset();
    prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
    auto result=prepare_session_control(prepared);auto& state=result.state;
    state.area_ai_target=input.area_ai_target;state.area_defense=input.area_defense;
    state.clearance_flight=input.clearance_flight;state.clearance_race=input.clearance_race;
    state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
    state.phase=FieldSessionPhase::area_action6_control_ready;validate_phase_session(state);return result;
}
ControlSetupResult prepare_session_control(const PhaseSessionState& input) {
    validate_phase_session(input);
    if ((input.phase != FieldSessionPhase::player_control && input.phase != FieldSessionPhase::ordinary_pass_control_entry && input.phase != FieldSessionPhase::kickoff_control_pending &&
         input.phase != FieldSessionPhase::sampled_field_contact_presentation) || !input.control ||
        !input.recovery || !input.experience)
        throw std::invalid_argument("Control setup requires a player-control handoff and captured contexts");
    const auto owner_index = field_motion_index(input.current.field.ball_owner);
    const auto& carrier = input.current.field.motion[owner_index];
    const auto x = static_cast<std::uint8_t>(carrier.x >> 8);
    const auto y = static_cast<std::uint8_t>(carrier.y >> 8);
    const auto cell = field_cell_at(x, y);
    if (cell == 255) throw std::invalid_argument("Control setup requires an interior field carrier");
    ControlSetupResult result{input};
    auto& state = result.state;
    auto& field = state.current.field;
    auto& recovery = *state.recovery;
    auto& control = *state.control;
    result.presentation = {{ControlPresentationKind::display, 0}, {ControlPresentationKind::display, 1},
        {ControlPresentationKind::script, 30}, {ControlPresentationKind::field_map, 0}};
    const auto status_requests = [&result](std::uint8_t side) {
        if (side == 0) result.presentation.push_back({ControlPresentationKind::display, 48});
        else {
            result.presentation.push_back({ControlPresentationKind::display, 49});
            result.presentation.push_back({ControlPresentationKind::display, 50});
        }
    };
    status_requests(field.side);
    state.current.clock.script_flags = 128;
    state.current.clock.ball_mode = 128;
    control.travel_accumulator = 0;
    control.display_parameter = 2;
    control.motion_requested = 0;
    // E233 writes0469, represented as the high byte of existing slot10.
    state.experience->totals[10] = static_cast<std::uint16_t>((state.experience->totals[10] & 255u) | 256u);
    control.decision_timer = 10;
    field.movement_region = 255;
    field.ball_x = x; field.ball_y = y; field.ball_cell = cell;
    recovery.ball_variant = 0;
    field.selected_defenders = 0;
    if(input.phase==FieldSessionPhase::sampled_field_contact_presentation)state.area_defense.reset();
    if (state.scheduling) state.scheduling->selected_ids.fill(0);
    field.side = field.ball_owner < 11 ? 0 : 11;
    control.facing = field.side == 0 ? 64 : 0;
    if (field.side == 0) {
        if (recovery.special_owner_flags & 128u) {
            recovery.special_owner_flags = 0;
            field.roster[20].stamina &= 0xFF00u;
            result.special_owner_cleared = true;
        }
    } else {
        recovery.opposing_player = 0;
        NearestFieldPlayerInput nearest{0, x, y, field.ball_owner, 0, {}};
        for (std::size_t i = 0; i < field.motion.size(); ++i) {
            const auto& actor = field.motion[i];
            nearest.players[i] = {static_cast<std::uint8_t>(actor.x >> 8),
                static_cast<std::uint8_t>(actor.y >> 8), actor.status != 0};
        }
        result.defender = find_nearest_field_player(nearest);
        control.controlled_player = result.defender->player_id;
        field.motion[owner_index].target_cell = 5;
        control.last_carrier_cell = field.ball_cell;
    }
    status_requests(field.side);
    state.phase = FieldSessionPhase::control_ready;
    state.kickoff_receiver.reset();
    validate_phase_session(state);
    return result;
}
}

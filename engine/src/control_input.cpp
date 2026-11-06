#include "captain/control_input.hpp"
#include "captain/defender_selection.hpp"
#include "captain/field_state_io.hpp"
#include <sstream>
#include <stdexcept>
#include <utility>

namespace captain {
PhaseSessionState capture_sampled_field_ai_choice(const PhaseSessionState& input, std::uint8_t action, std::uint8_t subtype) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::ai_cell_decision || action!=1 || subtype!=0 || !input.action || !input.score ||
    input.current.field.side!=11 || input.current.field.ball_owner!=16 || input.current.field.ball_x!=141 ||
    input.current.field.ball_y!=148 || input.current.field.ball_cell!=140 || input.current.field.roster[16].type!=23 ||
    input.current.field.selected_defenders || input.current.field.random_first || input.current.field.random_second ||
    input.experience->opponent_stage || input.current.clock.remaining!=443 || input.current.clock.extra_time!=5 ||
    input.control->last_carrier_cell!=140 || input.score->left_goals!=1 || input.score->right_goals!=0 ||
    input.score->kickoff!=4 || input.action->action!=0 || input.action->power!=22 || input.recovery->attack_subtype!=128)
  throw std::invalid_argument("Unsupported sampled opponent AI choice");
 auto state=input;state.action->action=action;state.recovery->attack_subtype=subtype;
 state.phase=FieldSessionPhase::sampled_field_ai_choice;validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_field_ai_preparation(const PhaseSessionState& input, std::uint8_t subtype, std::uint16_t power) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_field_ai_choice || subtype!=128 || power!=21)
  throw std::invalid_argument("Unsupported sampled opponent AI preparation");
 auto state=input;state.recovery->attack_subtype=subtype;state.action->power=power;
 state.phase=FieldSessionPhase::sampled_field_ai_prepared;validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_field_ai_presentation(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_field_ai_prepared)
  throw std::invalid_argument("Unsupported sampled opponent presentation");
 auto state=input;state.current.clock.script_flags=0;state.current.clock.ball_mode=0;
 state.phase=FieldSessionPhase::sampled_field_ai_presented;validate_phase_session(state);return state;
}
PhaseSessionState advance_sampled_field_ai_dispatch(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_field_ai_presented)
  throw std::invalid_argument("Unsupported sampled opponent dispatch");
 auto state=input;state.control->last_carrier_cell=0;
 state.phase=FieldSessionPhase::sampled_field_ai_dispatch;validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_field_ai_pass_presentation(const PhaseSessionState& input, const FieldPhaseState& captured) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_field_ai_dispatch || captured.elapsed!=0 || captured.movement_region!=140)
  throw std::invalid_argument("Unsupported captured opponent pass presentation");
 auto normalized=captured;normalized.motion=input.current.field.motion;normalized.elapsed=input.current.field.elapsed;
 normalized.movement_region=input.current.field.movement_region;
 std::ostringstream before(std::ios::out|std::ios::binary),after(std::ios::out|std::ios::binary);
 write_field_state(before,input.current.field);write_field_state(after,normalized);
 if(before.str()!=after.str())throw std::invalid_argument("Captured pass presentation changed unrelated field state");
 auto state=input;state.current.field=captured;state.phase=FieldSessionPhase::sampled_field_ai_pass_presented;
 validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_field_ai_pass_handoff(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_field_ai_pass_presented || input.recovery->special_owner_flags!=0 || input.recovery->ball_variant!=0 ||
    input.control->facing!=0 || input.control->display_parameter!=2 || input.control->motion_requested!=1)
  throw std::invalid_argument("Unsupported sampled opponent pass handoff");
 auto state=input;state.recovery->ball_variant=1;state.control->facing=64;
 state.control->last_carrier_cell=128;state.control->display_parameter=0;state.control->motion_requested=0;
 state.phase=FieldSessionPhase::sampled_field_ai_pass_handoff;validate_phase_session(state);return state;
}
PhaseSessionState capture_sampled_field_pass_target(const PhaseSessionState& input, std::uint8_t recipient, std::uint8_t destination, std::uint8_t counter, std::uint8_t distance) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::sampled_field_ai_pass_handoff || recipient!=19 || destination!=103 || counter!=255 || distance!=3)
  throw std::invalid_argument("Unsupported sampled opponent pass target");
 const auto& target=input.current.field.motion[field_motion_index(recipient)];
 if(field_cell_at(static_cast<std::uint8_t>(target.x>>8),static_cast<std::uint8_t>(target.y>>8))!=destination)
  throw std::invalid_argument("Captured pass destination disagrees with receiver position");
 auto state=input;state.sampled_field_pass=SampledFieldPassContext{recipient,destination,counter,distance};
 state.phase=FieldSessionPhase::sampled_field_ai_pass_target;validate_phase_session(state);return state;
}
PhaseSessionState acknowledge_sampled_opponent_contact_presentation(const PhaseSessionState& input) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::field_contact_pending || !input.score || !input.action || !input.kickoff_formation ||
    input.saved_ball_flight_flag!=1 || input.current.field.side!=11 || input.current.field.ball_owner!=19 ||
    input.current.field.ball_x!=112 || input.current.field.ball_y!=140 || input.current.field.ball_cell!=103 || input.current.field.elapsed!=1 ||
    input.current.field.selected_defenders!=3 || input.scheduling->selected_ids!=std::array<std::uint8_t,5>{3,7,8,0,0} ||
    input.current.field.random_first || input.current.field.random_second || input.experience->opponent_stage ||
    input.current.clock.remaining!=440 || input.current.clock.extra_time!=5 || input.current.clock.script_flags || input.current.clock.ball_mode ||
    input.score->left_goals!=1 || input.score->right_goals!=0 || input.score->kickoff!=4 ||
    input.control->display_parameter!=2 || input.control->motion_requested!=1 || input.action->action!=1 || input.action->power!=21)
  throw std::invalid_argument("Unsupported sampled opponent contact presentation");
 auto state=input;state.control->display_parameter=0;state.control->motion_requested=0;
 state.phase=FieldSessionPhase::sampled_opponent_contact_presented;validate_phase_session(state);return state;
}
ControlPollResult poll_session_area_action4_control(const PhaseSessionState& input,std::uint8_t held,std::uint8_t pressed,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement,const ControlMovementContent& control_content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action4_control_ready && input.phase!=FieldSessionPhase::area_action4_control_wait)
  throw std::invalid_argument("Area action4 polling requires a prepared input wait");
 auto prepared=input;prepared.phase=FieldSessionPhase::control_ready;
 prepared.area_ai_target.reset();prepared.area_defense.reset();prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
 auto result=poll_session_control(prepared,held,pressed,primary,movement,control_content);auto& state=result.state;
 if(state.phase==FieldSessionPhase::ai_cell_decision)
  throw std::invalid_argument("Area action4 AI cell interruption requires original tracing");
 state.area_ai_target=input.area_ai_target;state.area_defense=input.area_defense;
 state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 state.phase=result.decision_step ? FieldSessionPhase::area_action4_control_clock : FieldSessionPhase::area_action4_control_wait;
 validate_phase_session(state);return result;
}
ControlPollResult poll_session_area_pass_capture(const PhaseSessionState& input,std::uint8_t held,std::uint8_t pressed,
 const PlayerProfileMappings& primary,const FieldPhaseContent& movement,const ControlMovementContent& control_content) {
 validate_phase_session(input);
 const bool blocked=input.phase==FieldSessionPhase::area_pass_blocked_control_ready || input.phase==FieldSessionPhase::area_pass_blocked_control_wait;
 if((input.phase!=FieldSessionPhase::area_pass_capture_control_ready && input.phase!=FieldSessionPhase::area_pass_capture_control_wait && !blocked) ||
    (blocked && input.current.field.side!=0))
  throw std::invalid_argument("Captured pass polling needs prepared control");
 auto prepared=input;prepared.phase=FieldSessionPhase::control_ready;
 prepared.area_ai_target.reset();prepared.area_contact.reset();prepared.area_pass_contact.reset();
 prepared.shot_flight.reset();prepared.ball_trajectory.reset();prepared.area_defender_cursor=0;prepared.area_sequence_result=255;
 auto result=poll_session_control(prepared,held,pressed,primary,movement,control_content);auto& state=result.state;
 state.area_ai_target=input.area_ai_target;state.area_contact=input.area_contact;state.area_pass_contact=input.area_pass_contact;
 state.shot_flight=input.shot_flight;state.ball_trajectory=input.ball_trajectory;
 state.area_defender_cursor=input.area_defender_cursor;state.area_sequence_result=input.area_sequence_result;
 state.ball_trajectory->x=static_cast<std::uint16_t>((unsigned(state.current.field.ball_x)<<8)|(input.ball_trajectory->x&255u));
 state.ball_trajectory->y=static_cast<std::uint16_t>((unsigned(state.current.field.ball_y)<<8)|(input.ball_trajectory->y&255u));
 state.phase=blocked ? (result.menu_requested ? FieldSessionPhase::area_pass_blocked_action_menu :
  result.decision_step ? FieldSessionPhase::area_pass_blocked_clock_pending : FieldSessionPhase::area_pass_blocked_control_wait) :
  (result.menu_requested ? FieldSessionPhase::area_pass_capture_action_menu :
  result.decision_step ? FieldSessionPhase::area_pass_capture_clock_pending : FieldSessionPhase::area_pass_capture_control_wait);
 validate_phase_session(state);return result;
}
ControlButtonResult apply_control_buttons(const PhaseSessionState& input, std::uint8_t held, std::uint8_t pressed) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::control_ready && input.phase != FieldSessionPhase::field_round_pending)
        throw std::invalid_argument("Buttons require control-ready or field-round phase");
    ControlButtonResult result{input};
    auto& state = result.state;
    auto& field = state.current.field;
    auto& control = *state.control;
    control.motion_requested = 0;
    if (field.side == 0) {
        state.current.clock.script_flags |= 64u;
        if (held & 64u) {
            field.selected_defenders = 0;
            if (state.scheduling) state.scheduling->selected_ids.fill(0);
            state.current.clock.script_flags = 0;
            state.phase = FieldSessionPhase::action_menu_pending;
            result.menu_requested = true;
            return result;
        }
        if (held & 15u) {
            control.motion_requested = 1;
            control.facing = held & 2u ? 0 : 64;
            state.current.clock.script_flags &= 191u;
        }
    } else {
        control.motion_requested = 1;
        if (pressed & 192u) {
            auto selected = static_cast<std::uint8_t>(unsigned(control.controlled_player) +
                ((pressed & 128u) ? 1u : 255u));
            if (selected == 0) selected = 10;
            if (selected >= 11) selected = 1;
            control.controlled_player = selected;
            result.presentation = {{ControlPresentationKind::display, 49},
                                   {ControlPresentationKind::display, 50}};
        }
    }
    return result;
}

ControlPollResult poll_session_control(const PhaseSessionState& input,
    std::uint8_t held, std::uint8_t pressed, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ControlMovementContent& control_content) {
    if (input.phase != FieldSessionPhase::control_ready)
        throw std::invalid_argument("Input poll requires control-ready phase");
    auto buttons = apply_control_buttons(input, held, pressed);
    ControlPollResult result{std::move(buttons.state), std::move(buttons.presentation), {}, 0, false, buttons.menu_requested};
    if (result.menu_requested) return result;
    auto& state = result.state;
    auto& field = state.current.field;
    auto& control = *state.control;
    if (control.decision_timer != 0) {
        --control.decision_timer;
        return result;
    }
    control.decision_timer = 10;
    result.decision_step = true;
    if (held & 15u) {
        const auto id = field.side == 0 ? field.ball_owner : control.controlled_player;
        const auto& stats = field.roster[id];
        const auto profile = load_movement_profile({id, stats.type, static_cast<std::uint8_t>(stats.stamina),
            static_cast<std::uint8_t>(stats.stamina >> 8), stats.level}, field.side, primary, movement.profiles);
        const auto speed = field.side == 0 ? control_content.carrier_speeds[profile.speed_profile] : profile.speed;
        auto& actor = field.motion[field_motion_index(id)];
        const auto step = [speed](std::uint16_t value, std::uint8_t direction, FieldAxis axis) {
            const auto bits = direction & 3u;
            if (bits == 0) return value;
            const auto displacement = static_cast<std::uint16_t>((bits & 1u) ? unsigned(speed) : 0u - speed);
            return advance_field_coordinate(value, displacement, axis);
        };
        actor.x = step(actor.x, held, FieldAxis::horizontal);
        actor.y = step(actor.y, static_cast<std::uint8_t>(held >> 2), FieldAxis::vertical);
        result.moved_player = id;
        result.movement_speed = speed;
    }
    const auto& carrier = field.motion[field_motion_index(field.ball_owner)];
    field.ball_x = static_cast<std::uint8_t>(carrier.x >> 8);
    field.ball_y = static_cast<std::uint8_t>(carrier.y >> 8);
    field.ball_cell = field_cell_at(field.ball_x, field.ball_y);
    if (field.ball_owner >= 11 && field.ball_cell != control.last_carrier_cell) {
        control.last_carrier_cell = field.ball_cell;
        state.phase = FieldSessionPhase::ai_cell_decision;
    } else state.phase = FieldSessionPhase::control_clock_pending;
    validate_phase_session(state);
    return result;
}
}

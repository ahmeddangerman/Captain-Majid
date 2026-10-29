#include "captain/phase_session_io.hpp"
#include "captain/field_state_io.hpp"
#include <array>
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace captain {
namespace {
constexpr char magic[] = "CMJSESSION";
std::uint8_t byte(std::istream& input) {
    const auto value = input.get();
    if (value == std::char_traits<char>::eof()) throw std::invalid_argument("Truncated phase session");
    return static_cast<std::uint8_t>(value);
}
void byte(std::ostream& output, std::uint8_t value) { output.put(static_cast<char>(value)); }
}

PhaseSessionState load_phase_session(std::istream& input) {
    for (std::size_t i = 0; i < sizeof(magic) - 1; ++i)
        if (byte(input) != static_cast<std::uint8_t>(magic[i]))
            throw std::invalid_argument("Unsupported phase session format/version");
    const auto version = byte(input);
    if ((version < '1' || version > '9') && (version < 'A' || version > 'Z') && (version<'a' || version>'h'))
        throw std::invalid_argument("Unsupported phase session format/version");
    std::array<char, 239> field_bytes{};
    input.read(field_bytes.data(), field_bytes.size());
    if (input.gcount() != static_cast<std::streamsize>(field_bytes.size()))
        throw std::invalid_argument("Truncated phase session field section");
    std::istringstream field_input(std::string(field_bytes.data(), field_bytes.size()), std::ios::in | std::ios::binary);
    PhaseSessionState state{};
    state.current.field = load_field_state(field_input);
    const auto low = byte(input);
    state.current.clock.remaining = static_cast<std::uint16_t>(unsigned(low) | (unsigned(byte(input)) << 8));
    state.current.clock.extra_time = byte(input); state.current.clock.flags = byte(input);
    state.current.clock.script_flags = byte(input); state.current.clock.ball_mode = byte(input);
    state.current.shared_recovery_flags = byte(input); state.current.shared_recovery_timer = byte(input);
    state.phase = static_cast<FieldSessionPhase>(byte(input));
    if (version < '8' && static_cast<unsigned>(state.phase) > 20)
        throw std::invalid_argument("New shot phase requires session version8");
    if (version < '9' && static_cast<unsigned>(state.phase) > 22)
        throw std::invalid_argument("Staged shot phase requires session version9");
    if (version < 'A' && static_cast<unsigned>(state.phase) > 27)
        throw std::invalid_argument("Selected handoff phase requires session version10");
    if (version < 'B' && static_cast<unsigned>(state.phase) > 30)
        throw std::invalid_argument("Selected recovery phase requires session version11");
    if (version < 'C' && static_cast<unsigned>(state.phase) > 32)
        throw std::invalid_argument("Clearance flight phase requires session version12");
    if (version < 'D' && static_cast<unsigned>(state.phase) > 33)
        throw std::invalid_argument("Clearance race phase requires session version13");
    if (version < 'E' && static_cast<unsigned>(state.phase) > 35)
        throw std::invalid_argument("Clearance handoff requires session version14");
    if (version < 'F' && static_cast<unsigned>(state.phase) > 37)
        throw std::invalid_argument("Selected restart route requires session version15");
    if (version < 'G' && static_cast<unsigned>(state.phase) > 39)
        throw std::invalid_argument("Restart options require session version16");
    if (version < 'H' && static_cast<unsigned>(state.phase) > 42)
        throw std::invalid_argument("Nonterminal dispatch requires session version17");
    if (version < 'I' && static_cast<unsigned>(state.phase) > 43)
        throw std::invalid_argument("Attack clock requires session version18");
    if (version < 'J' && static_cast<unsigned>(state.phase) > 45)
        throw std::invalid_argument("Shot flight setup requires session version19");
    if (version < 'K' && static_cast<unsigned>(state.phase) > 46)
        throw std::invalid_argument("Numeric ball flight requires session version20");
    if (version < 'U' && static_cast<unsigned>(state.phase) > 63)
        throw std::invalid_argument("Kickoff handoff requires session version30");
    if (version < 'T' && static_cast<unsigned>(state.phase) > 62)
        throw std::invalid_argument("Kickoff ball requires session version29");
    if (version < 'S' && static_cast<unsigned>(state.phase) > 61)
        throw std::invalid_argument("Kickoff receiver requires session version28");
    if (version < 'R' && static_cast<unsigned>(state.phase) > 60)
        throw std::invalid_argument("Kickoff formation requires session version27");
    if (version < 'Q' && static_cast<unsigned>(state.phase) > 59)
        throw std::invalid_argument("Post-goal kickoff requires session version26");
    if (version < 'P' && static_cast<unsigned>(state.phase) > 58)
        throw std::invalid_argument("Scored goal requires session version25");
    if (version < 'O' && static_cast<unsigned>(state.phase) > 56)
        throw std::invalid_argument("Keeper save handoff requires session version24");
    if (version < 'N' && static_cast<unsigned>(state.phase) > 55)
        throw std::invalid_argument("Keeper comparison requires session version23");
    if (version < 'M' && static_cast<unsigned>(state.phase) > 52)
        throw std::invalid_argument("Keeper entry requires session version22");
    if (version < 'L' && static_cast<unsigned>(state.phase) > 50)
        throw std::invalid_argument("Flight return requires session version21");
    const auto pending = byte(input);
    const PendingTerminalContact contact{byte(input), byte(input), byte(input), byte(input)};
    if (pending > 1) throw std::invalid_argument("Unsupported pending-contact flag");
    if (pending) state.pending = contact;
    else if (contact.result_code || contact.defender_id || contact.defense_action || contact.defense_subtype)
        throw std::invalid_argument("Nonzero unused pending-contact data");
    if (version >= '2') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported experience presence flag");
        ExperienceState experience{};
        experience.campaign = byte(input); experience.opponent_stage = byte(input);
        bool unused = experience.campaign || experience.opponent_stage;
        for (auto& total : experience.totals) {
            const auto lo = byte(input);
            total = static_cast<std::uint16_t>(unsigned(lo) | (unsigned(byte(input)) << 8));
            unused = unused || total != 0;
        }
        if (present) state.experience = experience;
        else if (unused) throw std::invalid_argument("Nonzero unused experience data");
    }
    if (version >= '3') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported recovery presence flag");
        const RecoveryContext recovery{byte(input), byte(input), byte(input), byte(input), byte(input), byte(input)};
        if (present) state.recovery = recovery;
        else if (recovery.opposing_player || recovery.attack_subtype || recovery.special_owner_flags ||
                 recovery.ball_variant || recovery.game_mode || recovery.x_jitter_control)
            throw std::invalid_argument("Nonzero unused recovery data");
    }
    if (version >= '4') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported control presence flag");
        ControlContext control{};
        control.decision_timer = byte(input); control.facing = byte(input);
        control.controlled_player = byte(input); control.last_carrier_cell = byte(input);
        const auto low = byte(input);
        control.travel_accumulator = static_cast<std::uint16_t>(unsigned(low) | (unsigned(byte(input)) << 8));
        control.display_parameter = byte(input);
        if (version >= '5') control.motion_requested = byte(input);
        if (present) state.control = control;
        else if (control.decision_timer || control.facing || control.controlled_player ||
                 control.last_carrier_cell || control.travel_accumulator || control.display_parameter || control.motion_requested)
            throw std::invalid_argument("Nonzero unused control data");
    }
    if (version >= '6') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported scheduling presence flag");
        SchedulingContext schedule{};
        schedule.field_cycle = byte(input); schedule.energy_cycle = byte(input); schedule.special_flags = byte(input);
        for (auto& id : schedule.selected_ids) id = byte(input);
        if (present) state.scheduling = schedule;
        else if (schedule.field_cycle || schedule.energy_cycle || schedule.special_flags ||
                 std::any_of(schedule.selected_ids.begin(), schedule.selected_ids.end(), [](auto id) { return id != 0; }))
            throw std::invalid_argument("Nonzero unused scheduling data");
    }
    if (version >= '7') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported action-context presence flag");
        ActionContext action{};
        action.action = byte(input); action.menu_route = byte(input); action.cursor = byte(input);
        action.flash_counter = byte(input); action.auxiliary = byte(input);
        action.move_effect = byte(input); action.move_flags = byte(input);
        const auto power_low = byte(input);
        action.power = static_cast<std::uint16_t>(unsigned(power_low) | (unsigned(byte(input)) << 8));
        if (present) state.action = action;
        else if (action.action || action.menu_route || action.cursor || action.flash_counter ||
                 action.auxiliary || action.move_effect || action.move_flags || action.power)
            throw std::invalid_argument("Nonzero unused action data");
    }
    if (version >= '9') {
        const auto present = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported shot progress presence flag");
        const ShotContactProgress progress{byte(input),byte(input),byte(input),byte(input),byte(input)};
        if (present) state.shot_progress = progress;
        else if (progress.cursor || progress.result_code || progress.defender_id ||
                 progress.defense_action || progress.defense_subtype)
            throw std::invalid_argument("Nonzero unused shot progress");
    }
    if (version >= 'C') {
        const auto present = byte(input), distance = byte(input);
        if (present > 1) throw std::invalid_argument("Unsupported clearance presence flag");
        if (present) state.clearance_flight = ClearanceFlightContext{distance};
        else if (distance) throw std::invalid_argument("Nonzero unused clearance context");
    }
    if (version >= 'D') {
        const auto present=byte(input);
        if(present>1) throw std::invalid_argument("Unsupported race presence flag");
        const ClearanceRaceContext race{byte(input),byte(input),byte(input)};
        if(present) state.clearance_race=race;
        else if(race.remaining || race.active_candidate || race.opposing_candidate)
            throw std::invalid_argument("Nonzero unused race context");
    }
    if (version >= 'G') {
        const auto present = byte(input), goal = byte(input), cursor = byte(input);
        if (present > 1 || goal > 1) throw std::invalid_argument("Invalid restart options flags");
        if (present) state.restart_options = RestartOptionsContext{goal != 0,cursor};
        else if (goal || cursor) throw std::invalid_argument("Nonzero unused restart options");
    }
    if (version >= 'J') {
        const auto present=byte(input), distance=byte(input), destination=byte(input), counter=byte(input), loose=byte(input);
        if(present>1 || loose>1) throw std::invalid_argument("Unsupported flight setup flags");
        if(present) state.shot_flight=ShotFlightContext{distance,destination,counter,loose!=0};
        else if(distance || destination || counter || loose) throw std::invalid_argument("Nonzero unused shot flight context");
    }
    if(version >= 'K') {
        const auto present=byte(input);if(present>1)throw std::invalid_argument("Invalid trajectory presence");
        std::array<std::uint16_t,4> words{};
        for(auto& word:words){const auto low=byte(input);word=static_cast<std::uint16_t>(low|(unsigned(byte(input))<<8));}
        const auto direction=byte(input),flags=byte(input);
        if(present)state.ball_trajectory=BallTrajectoryContext{words[0],words[1],words[2],words[3],direction,flags};
        else if(words[0] || words[1] || words[2] || words[3] || direction || flags)
            throw std::invalid_argument("Nonzero unused trajectory context");
    }
    if(version >= 'M') {
        const auto present=byte(input),cursor=byte(input),result=byte(input);
        if(present>1)throw std::invalid_argument("Invalid keeper decision presence");
        if(present)state.keeper_decision=KeeperDecisionContext{cursor,result};
        else if(cursor || result)throw std::invalid_argument("Nonzero unused keeper decision");
    }
    if(version >= 'N') {
        const auto action=byte(input),subtype=byte(input),effect=byte(input);
        if(state.keeper_decision) {
            state.keeper_decision->defense_action=action;state.keeper_decision->defense_subtype=subtype;
            state.keeper_decision->effect=effect;
        } else if(action || subtype || effect)throw std::invalid_argument("Nonzero unused keeper move");
    }
    if(version >= 'P') {
        const auto present=byte(input),left=byte(input),right=byte(input),kickoff=byte(input);
        if(present>1)throw std::invalid_argument("Invalid score presence");
        if(present)state.score=MatchScoreContext{left,right,kickoff};
        else if(left || right || kickoff)throw std::invalid_argument("Nonzero unused score context");
    }
    if(version >= 'R') {
        const auto present=byte(input),left=byte(input),right=byte(input);
        if(present>1)throw std::invalid_argument("Invalid kickoff formation presence");
        if(present)state.kickoff_formation=KickoffFormationContext{{left,right}};
        else if(left || right)throw std::invalid_argument("Nonzero unused kickoff formation");
    }
    if(version >= 'S') {
        const auto present=byte(input),receiver=byte(input);
        if(present>1)throw std::invalid_argument("Invalid kickoff receiver presence");
        if(present)state.kickoff_receiver=receiver;
        else if(receiver)throw std::invalid_argument("Nonzero unused kickoff receiver");
    }
    if(version < 'V' && state.phase == FieldSessionPhase::control_ready && state.kickoff_formation)
        throw std::invalid_argument("Kickoff control setup requires session version31");
    if(version < 'W' && static_cast<unsigned>(state.phase)>=10 && static_cast<unsigned>(state.phase)<=12 && state.kickoff_formation)
        throw std::invalid_argument("Kickoff control polling requires session version32");
    if(version < 'X' && state.phase==FieldSessionPhase::field_round_pending && state.kickoff_formation)
        throw std::invalid_argument("Kickoff field round requires session version33");
    if(version < 'Y' && state.phase==FieldSessionPhase::human_action_menu && state.kickoff_formation)
        throw std::invalid_argument("Kickoff action menu requires session version34");
    if(version < 'Z' && (state.phase==FieldSessionPhase::attack_choice_pending ||
       state.phase==FieldSessionPhase::attack_prepared) && state.kickoff_formation)
        throw std::invalid_argument("Kickoff shot preparation requires session version35");
    const auto phase=static_cast<unsigned>(state.phase);
    if(version < 'a' && (phase==27 || phase==43 || phase==45 || phase==46) && state.kickoff_formation)
        throw std::invalid_argument("Kickoff shot approach requires session version36");
    if(version < 'b' && phase>=47 && phase<=49 && state.kickoff_formation)
        throw std::invalid_argument("Kickoff numeric flight requires session version37");
    if(version < 'c' && (phase==52 || phase==53) && state.kickoff_formation)
        throw std::invalid_argument("Kickoff keeper entry requires session version38");
    if(version < 'd' && phase==56 && state.kickoff_formation)
        throw std::invalid_argument("Kickoff keeper decision requires session version39");
    if(version < 'e' && phase==59 && state.kickoff_formation)
        throw std::invalid_argument("Kickoff scored goal requires session version40");
    if(version < 'f' && phase==60 && state.kickoff_formation)
        throw std::invalid_argument("Kickoff post-goal restart requires session version41");
    if(version < 'g' && (phase==2 || phase==57 || phase==58) && state.kickoff_formation)
        throw std::invalid_argument("Kickoff keeper save requires session version42");
    if(version < 'h' && phase==6 && state.kickoff_formation)
        throw std::invalid_argument("Kickoff recovered owner requires session version43");
    if (input.peek() != std::char_traits<char>::eof())
        throw std::invalid_argument("Trailing phase session data");
    if (input.bad()) throw std::invalid_argument("Unable to read phase session");
    validate_phase_session(state);
    return state;
}

void write_phase_session(std::ostream& output, const PhaseSessionState& state) {
    validate_phase_session(state);
    output.write(magic, sizeof(magic) - 1);
    byte(output, 'h');
    write_field_state(output, state.current.field);
    byte(output, static_cast<std::uint8_t>(state.current.clock.remaining));
    byte(output, static_cast<std::uint8_t>(state.current.clock.remaining >> 8));
    for (const auto value : {state.current.clock.extra_time, state.current.clock.flags,
         state.current.clock.script_flags, state.current.clock.ball_mode,
         state.current.shared_recovery_flags, state.current.shared_recovery_timer,
         static_cast<std::uint8_t>(state.phase), static_cast<std::uint8_t>(state.pending ? 1 : 0)})
        byte(output, value);
    const auto contact = state.pending.value_or(PendingTerminalContact{0, 0, 0, 0});
    for (const auto value : {contact.result_code, contact.defender_id, contact.defense_action, contact.defense_subtype})
        byte(output, value);
    byte(output, state.experience ? 1 : 0);
    const auto experience = state.experience.value_or(ExperienceState{});
    byte(output, experience.campaign); byte(output, experience.opponent_stage);
    for (const auto total : experience.totals) {
        byte(output, static_cast<std::uint8_t>(total));
        byte(output, static_cast<std::uint8_t>(total >> 8));
    }
    byte(output, state.recovery ? 1 : 0);
    const auto recovery = state.recovery.value_or(RecoveryContext{});
    for (const auto value : {recovery.opposing_player, recovery.attack_subtype, recovery.special_owner_flags,
         recovery.ball_variant, recovery.game_mode, recovery.x_jitter_control}) byte(output, value);
    byte(output, state.control ? 1 : 0);
    const auto control = state.control.value_or(ControlContext{});
    for (const auto value : {control.decision_timer, control.facing, control.controlled_player,
         control.last_carrier_cell, static_cast<std::uint8_t>(control.travel_accumulator),
         static_cast<std::uint8_t>(control.travel_accumulator >> 8), control.display_parameter,
         control.motion_requested}) byte(output, value);
    byte(output, state.scheduling ? 1 : 0);
    const auto schedule = state.scheduling.value_or(SchedulingContext{});
    byte(output, schedule.field_cycle); byte(output, schedule.energy_cycle); byte(output, schedule.special_flags);
    for (const auto id : schedule.selected_ids) byte(output, id);
    byte(output, state.action ? 1 : 0);
    const auto action = state.action.value_or(ActionContext{});
    for (const auto value : {action.action, action.menu_route, action.cursor, action.flash_counter,
         action.auxiliary, action.move_effect, action.move_flags}) byte(output, value);
    byte(output, static_cast<std::uint8_t>(action.power));
    byte(output, static_cast<std::uint8_t>(action.power >> 8));
    byte(output, state.shot_progress ? 1 : 0);
    const auto progress = state.shot_progress.value_or(ShotContactProgress{});
    for (const auto value : {progress.cursor,progress.result_code,progress.defender_id,
                            progress.defense_action,progress.defense_subtype}) byte(output,value);
    byte(output,state.clearance_flight ? 1 : 0);
    byte(output,state.clearance_flight ? state.clearance_flight->distance_adjustment : 0);
    byte(output,state.clearance_race ? 1 : 0);
    const auto race=state.clearance_race.value_or(ClearanceRaceContext{});
    byte(output,race.remaining);byte(output,race.active_candidate);byte(output,race.opposing_candidate);
    byte(output,state.restart_options ? 1 : 0);
    byte(output,state.restart_options && state.restart_options->goal_area ? 1 : 0);
    byte(output,state.restart_options ? state.restart_options->cursor : 0);
    byte(output,state.shot_flight ? 1 : 0);
    byte(output,state.shot_flight ? state.shot_flight->distance : 0);
    byte(output,state.shot_flight ? state.shot_flight->destination : 0);
    byte(output,state.shot_flight ? state.shot_flight->counter : 0);
    byte(output,state.shot_flight && state.shot_flight->loose_ball ? 1 : 0);
    byte(output,state.ball_trajectory ? 1 : 0);
    const auto trajectory=state.ball_trajectory.value_or(BallTrajectoryContext{});
    for(auto word:{trajectory.x,trajectory.y,trajectory.vx,trajectory.vy}){byte(output,word&255);byte(output,word>>8);}
    byte(output,trajectory.direction);byte(output,trajectory.flags);
    byte(output,state.keeper_decision ? 1 : 0);
    byte(output,state.keeper_decision ? state.keeper_decision->cursor : 0);
    byte(output,state.keeper_decision ? state.keeper_decision->result_code : 0);
    byte(output,state.keeper_decision ? state.keeper_decision->defense_action : 0);
    byte(output,state.keeper_decision ? state.keeper_decision->defense_subtype : 0);
    byte(output,state.keeper_decision ? state.keeper_decision->effect : 0);
    byte(output,state.score ? 1 : 0);
    byte(output,state.score ? state.score->left_goals : 0);
    byte(output,state.score ? state.score->right_goals : 0);
    byte(output,state.score ? state.score->kickoff : 0);
    byte(output,state.kickoff_formation ? 1 : 0);
    byte(output,state.kickoff_formation ? state.kickoff_formation->keeper_y[0] : 0);
    byte(output,state.kickoff_formation ? state.kickoff_formation->keeper_y[1] : 0);
    byte(output,state.kickoff_receiver ? 1 : 0);
    byte(output,state.kickoff_receiver.value_or(0));
    if (!output) throw std::invalid_argument("Unable to write phase session");
}
}

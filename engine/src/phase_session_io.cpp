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
    if ((version < '1' || version > '9') && (version < 'A' || version > 'Z') && (version<'a' || version>'z') && version!=128 && version!=129 && version!=130 && version!=131 && version!=132 && version!=133 && version!=134 && version!=135 && version!=136 && version!=137 && version!=138 && version!=139 && version!=140 && version!=141 && version!=142 && version!=143 && version!=144 && version!=145 && version!=146 && version!=147)
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
    const auto phase_low=byte(input);
    const auto phase_high=version==143 || version==144 || version==145 || version==146 || version==147 ? byte(input) : 0;
    state.phase = static_cast<FieldSessionPhase>(unsigned(phase_low) | (unsigned(phase_high)<<8));
    if((version==143 || version==144 || version==145 || version==146 || version==147) && static_cast<unsigned>(state.phase)<256)
        throw std::invalid_argument("Extended session version requires a two-byte phase");
    if((version==144)!=(state.phase==FieldSessionPhase::sampled_player14_pass_target))
        throw std::invalid_argument("Extended pass target requires session version144");
    if((version==145)!=((static_cast<unsigned>(state.phase)>=269 && static_cast<unsigned>(state.phase)<=275) ||
       (static_cast<unsigned>(state.phase)>=296 && static_cast<unsigned>(state.phase)<=302)))
        throw std::invalid_argument("Extended goalkeeper choice requires session version145");
    if((version==146)!=((static_cast<unsigned>(state.phase)>=276 && static_cast<unsigned>(state.phase)<=284) ||
       (static_cast<unsigned>(state.phase)>=310 && static_cast<unsigned>(state.phase)<=325)))
        throw std::invalid_argument("Extended human pass cursor requires session version146");
    if((version==147)!=(static_cast<unsigned>(state.phase)==326))throw std::invalid_argument("Player7 target requires session version147");
    if(version<133 && (static_cast<unsigned>(state.phase)==188 || static_cast<unsigned>(state.phase)==189))throw std::invalid_argument("Prepared sampled loose defender requires session version67");
    if((version==139)!=(static_cast<unsigned>(state.phase)==220 || static_cast<unsigned>(state.phase)==221 || static_cast<unsigned>(state.phase)==222 || static_cast<unsigned>(state.phase)==223 || static_cast<unsigned>(state.phase)==224 || static_cast<unsigned>(state.phase)==225 || static_cast<unsigned>(state.phase)==226))
        throw std::invalid_argument("Keeper menu selection requires session version73");
    if((version==138)!=(static_cast<unsigned>(state.phase)==200))
        throw std::invalid_argument("Sampled field pass target requires session version72");
    if((version==140)!=(static_cast<unsigned>(state.phase)==229 || static_cast<unsigned>(state.phase)==235))throw std::invalid_argument("Mixed pass target requires session version74");
    if((version==141)!=((static_cast<unsigned>(state.phase)==111 || static_cast<unsigned>(state.phase)==112) || (static_cast<unsigned>(state.phase)>=244 && static_cast<unsigned>(state.phase)<=255)))throw std::invalid_argument("Human pass cursor requires session version75");
    if(version==142 && state.phase!=FieldSessionPhase::attack_choice_pending && state.phase!=FieldSessionPhase::attack_prepared)
        throw std::invalid_argument("Five human defender choices require their supported phases");
    if(version<137 && static_cast<unsigned>(state.phase)==186)
        throw std::invalid_argument("Prepared area restart pass requires session version71");
    if(version<136 && ((static_cast<unsigned>(state.phase)>=181 && static_cast<unsigned>(state.phase)<=186) || static_cast<unsigned>(state.phase)==192 || static_cast<unsigned>(state.phase)==193))
        throw std::invalid_argument("Prepared area restart power requires session version70");
    if(version<135 && ((static_cast<unsigned>(state.phase)>=179 && static_cast<unsigned>(state.phase)<=186) || static_cast<unsigned>(state.phase)==192 || static_cast<unsigned>(state.phase)==193))
        throw std::invalid_argument("Captured area restart choice requires session version69");
    if(version<134 && ((static_cast<unsigned>(state.phase)>=160 && static_cast<unsigned>(state.phase)<=187) || static_cast<unsigned>(state.phase)==190 || static_cast<unsigned>(state.phase)==191 || static_cast<unsigned>(state.phase)==192 || static_cast<unsigned>(state.phase)==193))
        throw std::invalid_argument("Area action4 field entry requires session version68");
    if(version<133 && (static_cast<unsigned>(state.phase)>=150 && static_cast<unsigned>(state.phase)<=159))
        throw std::invalid_argument("Shot pass defenders require session version67");
    if(version<131 && static_cast<unsigned>(state.phase)>95)
        throw std::invalid_argument("Area flight requires session version65");
    if(version<130 && (state.phase==FieldSessionPhase::keeper_interaction || state.phase==FieldSessionPhase::story_transition) && state.current.field.selected_defenders)
        throw std::invalid_argument("Selected recovery continuation requires session version64");
    if(version<130 && static_cast<unsigned>(state.phase)>92)
        throw std::invalid_argument("Final area result handoffs require session version64");
    if(version<129 && static_cast<unsigned>(state.phase)>89)
        throw std::invalid_argument("Area prefix clock requires session version63");
    if(version<128 && static_cast<unsigned>(state.phase)>85)
        throw std::invalid_argument("Area defender sequence requires session version62");
    if(version<'z' && static_cast<unsigned>(state.phase)>82)
        throw std::invalid_argument("Area contact clock requires session version61");
    if(version<'y' && static_cast<unsigned>(state.phase)>81)
        throw std::invalid_argument("Area contact requires session version60");
    if(version<'x' && static_cast<unsigned>(state.phase)>80)
        throw std::invalid_argument("Area defender announcements require session version59");
    if(version<'w' && static_cast<unsigned>(state.phase)>79)
        throw std::invalid_argument("Area defense entry requires session version58");
    if(version<'v' && static_cast<unsigned>(state.phase)>78)
        throw std::invalid_argument("Prepared area attack requires session version57");
    if(version<'u' && static_cast<unsigned>(state.phase)>77)
        throw std::invalid_argument("Area AI choice requires session version56");
    if(version<'t' && static_cast<unsigned>(state.phase)>76)
        throw std::invalid_argument("Special redraw requires session version55");
    if(version<'s' && static_cast<unsigned>(state.phase)>75)
        throw std::invalid_argument("Special inspection requires session version54");
    if(version<'r' && static_cast<unsigned>(state.phase)>73)
        throw std::invalid_argument("Special chooser polling requires session version53");
    if(version<'q' && static_cast<unsigned>(state.phase)>72)
        throw std::invalid_argument("Special chooser requires session version52");
    if(version<'p' && static_cast<unsigned>(state.phase)>71)
        throw std::invalid_argument("Completed defensive choices require session version51");
    if(version<'o' && static_cast<unsigned>(state.phase)>69)
        throw std::invalid_argument("Defensive polling requires session version50");
    if(version<'n' && static_cast<unsigned>(state.phase)>68)
        throw std::invalid_argument("Defensive prompt requires session version49");
    if(version<'m' && static_cast<unsigned>(state.phase)>66)
        throw std::invalid_argument("Area defense initialization requires session version48");
    if(version<'k' && static_cast<unsigned>(state.phase)>65)
        throw std::invalid_argument("Keeper-area defensive entry requires session version46");
    if(version<'j' && static_cast<unsigned>(state.phase)>64)
        throw std::invalid_argument("Keeper-area setup requires session version45");
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
    if(version >= 'j') {
        const auto present=byte(input),counter=byte(input);
        if(present>1)throw std::invalid_argument("Invalid keeper-area presence");
        if(present)state.keeper_area=KeeperAreaContext{counter};
        else if(counter)throw std::invalid_argument("Nonzero unused keeper-area counter");
    }
    if(version >= 'm') {
        const auto present=byte(input);if(present>1)throw std::invalid_argument("Invalid area-defense presence");
        KeeperAreaDefenseContext defense{};
        for(auto& action:defense.actions)action=byte(input);
        for(auto& subtype:defense.subtypes)subtype=byte(input);
        defense.blink=byte(input);
        if(present)state.area_defense=defense;
        else if(defense.blink || std::any_of(defense.actions.begin(),defense.actions.end(),[](auto v){return v!=0;}) ||
                std::any_of(defense.subtypes.begin(),defense.subtypes.end(),[](auto v){return v!=0;}))
            throw std::invalid_argument("Nonzero unused area-defense data");
    }
    if(version >= 'o') {
        const auto pressed=byte(input);
        if(state.area_defense)state.area_defense->pressed=pressed;
        else if(pressed)throw std::invalid_argument("Nonzero unused defensive pressed input");
    }
    if(version >= 'q') {
        const auto present=byte(input),option=byte(input);
        if(present>1 || (!present && option))throw std::invalid_argument("Invalid special-option context");
        if(present)state.area_special_option=option;
    }
    if(version >= 'r') {state.area_special_cursor=byte(input);state.area_special_pressed=byte(input);}
    if(version >= 's') {
        const auto present=byte(input),status=byte(input),subtype=byte(input),low=byte(input),high=byte(input),effect=byte(input);
        if(present>1 || (!present && (status || subtype || low || high || effect)))
            throw std::invalid_argument("Invalid special decision context");
        if(present)state.area_special_decision=PhaseSessionState::SpecialDecision{status,subtype,static_cast<std::uint16_t>(unsigned(low)|(unsigned(high)<<8)),effect};
    }
    if(version>='u') {
        const auto present=byte(input),recipient=byte(input),cell=byte(input);
        if(present>1 || (!present && (recipient || cell)))throw std::invalid_argument("Invalid area AI target");
        if(present)state.area_ai_target=PhaseSessionState::AreaAiTarget{recipient,cell};
    }
    if(version>='v') {
        const auto low=byte(input),high=byte(input);const auto cost=static_cast<std::uint16_t>(unsigned(low)|(unsigned(high)<<8));
        if(state.area_ai_target)state.area_ai_target->cost=cost;
        else if(cost)throw std::invalid_argument("Unused area attack cost");
    }
    if(version>='y') {
        const auto present=byte(input);AreaContactContext c;
        c.cursor=byte(input);c.defense_action=byte(input);c.defense_subtype=byte(input);c.effect=byte(input);
        const auto power_low=byte(input),power_high=byte(input),cost_low=byte(input),cost_high=byte(input);
        c.defender_power=static_cast<std::uint16_t>(unsigned(power_low)|(unsigned(power_high)<<8));
        c.cost=static_cast<std::uint16_t>(unsigned(cost_low)|(unsigned(cost_high)<<8));
        c.strength=byte(input);c.loss=byte(input);c.score=byte(input);c.result=byte(input);
        const auto weakened=byte(input);c.weakened=bool(weakened);
        if(present>1 || weakened>1 || (!present && (c.cursor || c.defense_action || c.defense_subtype || c.effect ||
           c.defender_power || c.cost || c.strength || c.loss || c.score || c.result || c.weakened)))
            throw std::invalid_argument("Invalid area contact encoding");
        if(present)state.area_contact=c;
    }
    if(version>=128)state.area_defender_cursor=byte(input);
    if(version>=130)state.area_sequence_result=byte(input);
    else if((static_cast<unsigned>(state.phase)>=81 && static_cast<unsigned>(state.phase)<=86) || state.phase==FieldSessionPhase::keeper_area_terminal_result)
        state.area_sequence_result=state.area_contact->result;
    else if(state.phase==FieldSessionPhase::keeper_area_defenders_announced)
        state.area_sequence_result=0;
    if(version>=132 && version!=138) {
        const auto saved=byte(input),low=byte(input),high=byte(input);
        if(version==132 || saved || (static_cast<unsigned>(state.phase)>=152 && static_cast<unsigned>(state.phase)<=156) || state.phase==FieldSessionPhase::shot_pass_added_time || state.phase==FieldSessionPhase::shot_pass_period_end)state.area_pass_contact=AreaPassContactContext{saved,static_cast<std::uint16_t>(unsigned(low)|(unsigned(high)<<8))};
        else if(low || high)throw std::invalid_argument("Invalid absent area pass context");
    }
    if(version>=133 && version!=138)state.saved_ball_flight_flag=byte(input);
    if(version>=135 && version!=138 && version!=139 && version!=140 && version!=141 && version!=142 && version!=143 && version!=144 && version!=145 && version!=146 && version!=147)state.area_action4_restart=AreaAction4RestartContext{byte(input),byte(input),byte(input)};
    if(version>=136 && version!=138 && version!=139 && version!=140 && version!=141 && version!=142 && version!=143 && version!=144 && version!=145 && version!=146 && version!=147) {
        const auto cost_low=byte(input),cost_high=byte(input),power_low=byte(input),power_high=byte(input);
        state.area_action4_restart_power=AreaAction4RestartPowerContext{static_cast<std::uint16_t>(unsigned(cost_low)|(unsigned(cost_high)<<8)),static_cast<std::uint16_t>(unsigned(power_low)|(unsigned(power_high)<<8))};
    }
    if(version>=137 && version!=138 && version!=139 && version!=140 && version!=141 && version!=142 && version!=143 && version!=144 && version!=145 && version!=146 && version!=147)state.area_action4_restart_pass_counter=byte(input);
    if(version==138 || version==140 || version==144 || version==147 || (version==141 && state.phase==FieldSessionPhase::sampled_player3_empty_pass_destination))state.sampled_field_pass=SampledFieldPassContext{byte(input),byte(input),byte(input),byte(input)};
    if(version==139 || version==145)state.keeper_menu_action=byte(input);
    if(version==141 || version==146 || version==147) {state.human_pass_cursor=byte(input);state.human_pass_steps=byte(input);}
    if(version==142) {
        HumanDefenseChoices choices{};
        for(auto& action:choices.actions)action=byte(input);
        for(auto& subtype:choices.subtypes)subtype=byte(input);
        state.human_defense_choices=choices;
    }
    if(version<'k' && state.phase==FieldSessionPhase::human_action_menu && state.keeper_area)
        throw std::invalid_argument("Keeper-area menu requires session version46");
    if(version<'l' && state.phase==FieldSessionPhase::attack_choice_pending && state.keeper_area)
        throw std::invalid_argument("Keeper-area confirmation requires session version47");
    if(version < 'V' && state.phase == FieldSessionPhase::control_ready && state.kickoff_formation)
        throw std::invalid_argument("Kickoff control setup requires session version31");
    if(version < 'W' && static_cast<unsigned>(state.phase)>=10 && static_cast<unsigned>(state.phase)<=12 && state.kickoff_formation)
        throw std::invalid_argument("Kickoff control polling requires session version32");
    if(version < 'X' && (state.phase==FieldSessionPhase::field_round_pending || state.phase==FieldSessionPhase::field_contact_pending) && state.kickoff_formation)
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
    if(version < 'i' && (phase==1 || phase==7) && state.kickoff_formation)
        throw std::invalid_argument("Kickoff recovery continuation requires session version44");
    if (input.peek() != std::char_traits<char>::eof())
        throw std::invalid_argument("Trailing phase session data");
    if (input.bad()) throw std::invalid_argument("Unable to read phase session");
    validate_phase_session(state);
    if(version<131 && state.phase==FieldSessionPhase::keeper_area_attack_continuation && state.area_defense)
        throw std::invalid_argument("Retained area choices require session version65");
    if(version<'t' && state.area_defense && std::any_of(state.area_defense->subtypes.begin(),state.area_defense->subtypes.end(),[](auto v){return v!=0;}))
        throw std::invalid_argument("Defensive subtype requires session version55");
    if(version<'p' && state.phase==FieldSessionPhase::keeper_area_defense_poll &&
       state.action->cursor==state.current.field.selected_defenders)
        throw std::invalid_argument("Full defensive poll requires session version51");
    return state;
}

void write_phase_session(std::ostream& output, const PhaseSessionState& state) {
    validate_phase_session(state);
    output.write(magic, sizeof(magic) - 1);
    const bool extended=static_cast<unsigned>(state.phase)>=256;
    const bool action4_continuation=(static_cast<unsigned>(state.phase)>=160 && static_cast<unsigned>(state.phase)<=187) || static_cast<unsigned>(state.phase)==190 || static_cast<unsigned>(state.phase)==191 || static_cast<unsigned>(state.phase)==192 || static_cast<unsigned>(state.phase)==193;
    byte(output, extended ? (state.human_pass_cursor && state.sampled_field_pass ? 147 : state.human_pass_cursor ? 146 : state.keeper_menu_action ? 145 : state.sampled_field_pass ? 144 : 143) : state.human_defense_choices ? 142 : state.human_pass_cursor ? 141 : state.keeper_menu_action ? 139 : (state.phase==FieldSessionPhase::sampled_mixed_result4_target || state.phase==FieldSessionPhase::sampled_player12_pass_target) ? 140 : state.sampled_field_pass ? 138 : state.area_action4_restart_pass_counter ? 137 : state.area_action4_restart_power ? 136 : state.area_action4_restart ? 135 : action4_continuation ? 134 : (state.saved_ball_flight_flag || state.phase==FieldSessionPhase::shot_pass_defenders_pending || state.phase==FieldSessionPhase::shot_pass_defender_announced || (state.phase==FieldSessionPhase::sampled_loose_shot_defender_prepared || state.phase==FieldSessionPhase::sampled_loose_shot_defender_compared)) ? 133 : state.area_pass_contact ? 132 : (static_cast<unsigned>(state.phase)>=96 || (state.phase==FieldSessionPhase::keeper_area_attack_continuation && state.area_defense)) ? 131 : 130);
    write_field_state(output, state.current.field);
    byte(output, static_cast<std::uint8_t>(state.current.clock.remaining));
    byte(output, static_cast<std::uint8_t>(state.current.clock.remaining >> 8));
    for (const auto value : {state.current.clock.extra_time, state.current.clock.flags,
         state.current.clock.script_flags, state.current.clock.ball_mode,
         state.current.shared_recovery_flags, state.current.shared_recovery_timer})
        byte(output, value);
    byte(output,static_cast<std::uint8_t>(state.phase));
    if(extended)byte(output,static_cast<std::uint8_t>(static_cast<unsigned>(state.phase)>>8));
    byte(output,static_cast<std::uint8_t>(state.pending ? 1 : 0));
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
    byte(output,state.keeper_area ? 1 : 0);
    byte(output,state.keeper_area ? state.keeper_area->selection_counter : 0);
    byte(output,state.area_defense ? 1 : 0);
    for(unsigned i=0;i<4;++i)byte(output,state.area_defense ? state.area_defense->actions[i] : 0);
    for(unsigned i=0;i<4;++i)byte(output,state.area_defense ? state.area_defense->subtypes[i] : 0);
    byte(output,state.area_defense ? state.area_defense->blink : 0);
    byte(output,state.area_defense ? state.area_defense->pressed : 0);
    byte(output,state.area_special_option ? 1 : 0);
    byte(output,state.area_special_option.value_or(0));
    byte(output,state.area_special_cursor);byte(output,state.area_special_pressed);
    byte(output,state.area_special_decision ? 1 : 0);
    const auto decision=state.area_special_decision.value_or(PhaseSessionState::SpecialDecision{});
    byte(output,decision.status);byte(output,decision.subtype);byte(output,decision.cost&255);
    byte(output,decision.cost>>8);byte(output,decision.effect);
    byte(output,state.area_ai_target ? 1 : 0);
    byte(output,state.area_ai_target ? state.area_ai_target->recipient : 0);
    byte(output,state.area_ai_target ? state.area_ai_target->cell : 0);
    const auto area_cost=state.area_ai_target ? state.area_ai_target->cost : 0;
    byte(output,area_cost&255);byte(output,area_cost>>8);
    byte(output,state.area_contact ? 1 : 0);
    const auto area=state.area_contact.value_or(AreaContactContext{});
    for(const auto value:{area.cursor,area.defense_action,area.defense_subtype,area.effect,
        static_cast<std::uint8_t>(area.defender_power),static_cast<std::uint8_t>(area.defender_power>>8),
        static_cast<std::uint8_t>(area.cost),static_cast<std::uint8_t>(area.cost>>8),
        area.strength,area.loss,area.score,area.result,static_cast<std::uint8_t>(area.weakened)})byte(output,value);
    byte(output,state.area_defender_cursor);
    byte(output,state.area_sequence_result);
    const bool flight_flag_encoded=extended || state.saved_ball_flight_flag || state.human_pass_cursor || state.keeper_menu_action || action4_continuation ||
        state.phase==FieldSessionPhase::shot_pass_defenders_pending || state.phase==FieldSessionPhase::shot_pass_defender_announced ||
        state.phase==FieldSessionPhase::sampled_loose_shot_defender_prepared || state.phase==FieldSessionPhase::sampled_loose_shot_defender_compared;
    if(state.area_pass_contact) {
        byte(output,state.area_pass_contact->saved_ball_variant);
        byte(output,state.area_pass_contact->captured_cost&255);byte(output,state.area_pass_contact->captured_cost>>8);
    } else if(flight_flag_encoded) { byte(output,0);byte(output,0);byte(output,0); }
    if(flight_flag_encoded)byte(output,state.saved_ball_flight_flag);
    if(state.area_action4_restart) {byte(output,state.area_action4_restart->action);byte(output,state.area_action4_restart->subtype);byte(output,state.area_action4_restart->choice);}
    if(state.area_action4_restart_power) {byte(output,state.area_action4_restart_power->cost&255);byte(output,state.area_action4_restart_power->cost>>8);byte(output,state.area_action4_restart_power->power&255);byte(output,state.area_action4_restart_power->power>>8);}
    if(state.area_action4_restart_pass_counter)byte(output,*state.area_action4_restart_pass_counter);
    if(state.sampled_field_pass)for(const auto value:{state.sampled_field_pass->recipient,state.sampled_field_pass->destination,state.sampled_field_pass->counter,state.sampled_field_pass->distance})byte(output,value);
    if(state.keeper_menu_action)byte(output,*state.keeper_menu_action);
    if(state.human_pass_cursor) {byte(output,*state.human_pass_cursor);byte(output,state.human_pass_steps);}
    if(state.human_defense_choices) {
        for(auto action:state.human_defense_choices->actions)byte(output,action);
        for(auto subtype:state.human_defense_choices->subtypes)byte(output,subtype);
    }
    if (!output) throw std::invalid_argument("Unable to write phase session");
}
}

#include "captain/clearance_race.hpp"
#include "captain/ball_recovery.hpp"
#include "captain/defender_selection.hpp"
#include "captain/ball_step.hpp"
#include <stdexcept>
#include <utility>
namespace captain {
FieldLooseBallRaceResult resolve_field_loose_ball_race(const FieldPhaseState& input,
    std::uint8_t opposing_player,std::uint8_t distance,const PlayerProfileMappings& mappings,
    const FieldPhaseContent& movement,const ClearanceRaceContent& content) {
    auto field=input;
    field.elapsed=1;
    NearestFieldPlayerInput nearest{field.side,field.ball_x,field.ball_y,field.ball_owner,
        opposing_player,{}};
    for(std::size_t i=0;i<field.motion.size();++i) {
        const auto& p=field.motion[i];nearest.players[i]={static_cast<std::uint8_t>(p.x>>8),static_cast<std::uint8_t>(p.y>>8),p.status!=0};
    }
    const auto active=find_nearest_field_player(nearest).player_id;
    nearest.side ^= 11u;
    const auto other=find_nearest_field_player(nearest).player_id;
    field.motion[field_motion_index(active)].target_cell=field.ball_cell;
    field.motion[field_motion_index(other)].target_cell=field.ball_cell;
    auto remaining=distance;
    unsigned rounds=0;std::optional<std::uint8_t> winner;
    const auto move=[&](std::uint8_t id,unsigned selector) {
        const auto& stats=field.roster[id];
        const auto profile=load_movement_profile({id,stats.type,static_cast<std::uint8_t>(stats.stamina),
            static_cast<std::uint8_t>(stats.stamina>>8),stats.level},field.side,mappings,movement.profiles);
        auto& p=field.motion[field_motion_index(id)];
        p=advance_field_player({p,content.speeds[profile.speed_profile][selector],1,field.ball_cell},movement.geometry).player;
        return field_cell_at(static_cast<std::uint8_t>(p.x>>8),static_cast<std::uint8_t>(p.y>>8))==field.ball_cell;
    };
    do {
        ++rounds;
        if(move(active,0)) {winner=active;break;}
        if(move(other,1)) {winner=other;break;}
        remaining=static_cast<std::uint8_t>(unsigned(remaining)-1);
    } while(remaining!=0);
    return {std::move(field),rounds,remaining,active,other,winner};
}
PhaseSessionState begin_player2_same_cell_pass_race(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,const ClearanceRaceContent& content) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_pass_flight_target || !input.sampled_field_pass ||
       input.sampled_field_pass->recipient!=255 || input.sampled_field_pass->destination!=input.current.field.ball_cell ||
       !input.score || input.score->left_goals!=1 || input.score->right_goals || input.current.field.ball_owner!=2 ||
       input.current.field.ball_x!=104 || input.current.field.ball_y!=164 || input.current.field.ball_cell!=94 ||
       input.current.clock.remaining!=128 || input.ordinary_pass_defenses->count!=1 || input.sampled_field_pass->distance!=2 ||
       input.current.field.random_first!=56 || input.current.field.random_second!=51)
        throw std::invalid_argument("Unsupported player2 same-cell pass timeout");
    auto numeric=resolve_field_loose_ball_race(input.current.field,input.recovery->opposing_player,
        1,mappings,movement,content);
    if(numeric.rounds!=1 || numeric.remaining || numeric.active_candidate!=3 || numeric.opposing_candidate!=20 || numeric.winner)
        throw std::invalid_argument("Unobserved same-cell contest outcome");
    auto state=input; state.current.field=std::move(numeric.field);
    state.current.field.selected_defenders=0;state.scheduling->selected_ids.fill(0);
    state.area_defender_cursor=input.ordinary_pass_defenses->count;state.area_sequence_result=255;state.area_contact.reset();
    state.ordinary_pass_candidates.reset();state.ordinary_pass_recipient.reset();state.ordinary_pass_defenses.reset();
    state.sampled_field_pass.reset();state.human_pass_cursor.reset();state.human_pass_steps=0;
    state.saved_ball_flight_flag=1;state.clearance_race=ClearanceRaceContext{1,numeric.active_candidate,numeric.opposing_candidate};
    state.phase=FieldSessionPhase::player2_same_cell_pass_first_round;validate_phase_session(state);return state;
}
PhaseSessionState finish_player2_same_cell_pass_race(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,const ClearanceRaceContent& content) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::player2_same_cell_pass_first_round || input.current.field.random_first!=203 || input.current.field.random_second!=142)
        throw std::invalid_argument("The observed race random event must precede the second round");
    auto numeric=resolve_field_loose_ball_race(input.current.field,input.recovery->opposing_player,1,mappings,movement,content);
    if(numeric.rounds!=1 || numeric.remaining || numeric.active_candidate!=3 || numeric.opposing_candidate!=20 || numeric.winner)
        throw std::invalid_argument("Unobserved same-cell second round");
    auto state=input;state.current.field=std::move(numeric.field);state.clearance_race->remaining=0;
    state.phase=FieldSessionPhase::player2_same_cell_pass_timeout;validate_phase_session(state);return state;
}
PhaseSessionState resolve_ordinary_free_pass_contest(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,const ClearanceRaceContent& content) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::ordinary_pass_flight_ready || !input.area_ai_target || input.area_ai_target->recipient!=255 ||
       !input.ball_trajectory || !input.shot_flight || !input.recovery || input.current.field.selected_defenders)
        throw std::invalid_argument("Free pass contest requires an unopposed free-target flight");
    auto state=input;auto& trajectory=*state.ball_trajectory;
    const auto step=advance_ball_to_next_cell({trajectory.x,trajectory.y,{trajectory.vx,trajectory.vy},
        state.current.field.ball_cell,state.shot_flight->destination});
    if(step.event==BallStepEvent::cell_changed)throw std::invalid_argument("Free pass must first reach its destination");
    trajectory.x=step.x;trajectory.y=step.y;state.current.field.ball_x=step.x>>8;
    state.current.field.ball_y=step.y>>8;state.current.field.ball_cell=step.cell;
    auto numeric=resolve_field_loose_ball_race(state.current.field,state.recovery->opposing_player,state.shot_flight->distance,
        mappings,movement,content);
    if(numeric.rounds>2 || !numeric.winner || *numeric.winner<1 || *numeric.winner>10)
        throw std::invalid_argument("Free pass contest winner or timeout requires further evidence: rounds="+
            std::to_string(numeric.rounds)+" winner="+(numeric.winner ? std::to_string(*numeric.winner) : "none"));
    state.current.field=std::move(numeric.field);
    auto& field=state.current.field;field.ball_owner=*numeric.winner;
    auto& winner=field.motion[field_motion_index(*numeric.winner)];
    winner.x=static_cast<std::uint16_t>((winner.x&255u)|(unsigned(field.ball_x)<<8));
    winner.y=static_cast<std::uint16_t>((winner.y&255u)|(unsigned(field.ball_y)<<8));
    state.phase=FieldSessionPhase::ordinary_free_pass_contest_winner;
    validate_phase_session(state);return state;
}
PhaseSessionState resolve_sampled_player3_pass_race(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,
    const ClearanceRaceContent& content) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::sampled_player3_empty_pass_destination || input.clearance_race)
        throw std::invalid_argument("Player3 pass race requires the observed empty destination");
    auto field=input.current.field;
    // The same-cell flight clears the selected-defender count before the race.
    field.selected_defenders=0;
    auto numeric=resolve_field_loose_ball_race(field,input.recovery->opposing_player,
        input.sampled_field_pass->distance,mappings,movement,content);
    if(numeric.rounds!=1 || numeric.remaining!=2 || numeric.active_candidate!=5 ||
       numeric.opposing_candidate!=19 || numeric.winner!=5)
        throw std::invalid_argument("Unverified player3 pass contest outcome");
    auto state=input;
    state.current.field=std::move(numeric.field);
    state.scheduling->selected_ids.fill(0);
    state.clearance_race=ClearanceRaceContext{numeric.remaining,numeric.active_candidate,numeric.opposing_candidate};
    validate_phase_session(state);
    return state;
}
PhaseSessionState position_sampled_player3_pass_winner(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::sampled_player3_empty_pass_destination || !input.clearance_race ||
       input.current.field.ball_owner!=3)
        throw std::invalid_argument("Player3 pass winner must be positioned once after the contest");
    auto state=input;
    auto& field=state.current.field;
    field.ball_owner=5;
    auto& winner=field.motion[field_motion_index(5)];
    winner.x=static_cast<std::uint16_t>((winner.x&255u)|(unsigned(field.ball_x)<<8));
    winner.y=static_cast<std::uint16_t>((winner.y&255u)|(unsigned(field.ball_y)<<8));
    validate_phase_session(state);
    return state;
}
PhaseSessionState continue_sampled_player3_pass_winner(const PhaseSessionState& input) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::sampled_player3_empty_pass_destination || !input.clearance_race ||
       input.current.field.ball_owner!=5)
        throw std::invalid_argument("Player3 pass winner requires positioned possession and acknowledged presentation");
    auto state=input;
    state.clearance_race.reset();state.sampled_field_pass.reset();state.human_pass_cursor.reset();
    state.human_pass_steps=0;state.area_contact.reset();state.area_sequence_result=255;
    state.phase=FieldSessionPhase::player_control;
    validate_phase_session(state);
    return state;
}
namespace {
ClearanceRaceResult resolve_race(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,
    const ClearanceRaceContent& content,bool area) {
    validate_phase_session(input);
    if(input.phase!=(area ? FieldSessionPhase::area_action6_destination_pending : FieldSessionPhase::capture_clearance_flight))
        throw std::invalid_argument("Clearance race requires prepared flight setup");
    auto state=input;
    if(area)state.clearance_flight=ClearanceFlightContext{4};
    auto numeric=resolve_field_loose_ball_race(state.current.field,state.recovery->opposing_player,
        state.clearance_flight->distance_adjustment,mappings,movement,content);
    state.current.field=std::move(numeric.field);
    auto& field=state.current.field;
    const auto winner=numeric.winner;
    state.clearance_race=ClearanceRaceContext{numeric.remaining,numeric.active_candidate,numeric.opposing_candidate};
    bool music=false;std::uint8_t script=52;
    if(winner) {
        const auto side=static_cast<std::uint8_t>(*winner<11?0:11);
        music=side!=field.side;script=music?62:28;
        field.ball_owner=*winner;field.side=side;
        auto& owner=field.motion[field_motion_index(*winner)];
        owner.x=static_cast<std::uint16_t>((owner.x&255u)|(unsigned(field.ball_x)<<8));
        owner.y=static_cast<std::uint16_t>((owner.y&255u)|(unsigned(field.ball_y)<<8));
        state.phase=area ? FieldSessionPhase::area_action6_race_winner : FieldSessionPhase::clearance_race_winner;
    } else state.phase=area ? FieldSessionPhase::area_action6_race_timeout : FieldSessionPhase::clearance_race_timeout;
    validate_phase_session(state);
    return {std::move(state),numeric.rounds,winner,music,script};
}
}
ClearanceRaceResult resolve_session_clearance_race(const PhaseSessionState& input,const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,const ClearanceRaceContent& content) {
 return resolve_race(input,mappings,movement,content,false);
}
ClearanceRaceResult resolve_session_area_action6_race(const PhaseSessionState& input,const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,const ClearanceRaceContent& content) {
 return resolve_race(input,mappings,movement,content,true);
}
PhaseSessionState continue_session_area_action6_winner(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::area_action6_race_winner || (facing!=0 && facing!=64))
  throw std::invalid_argument("Area action6 winner needs acknowledged presentation");
 const auto& f=input.current.field;
 if(f.ball_y>=96 && f.ball_y<160 && (f.ball_x<80 || f.ball_x>=176))
  throw std::invalid_argument("Area action6 winner awaits keeper-area continuation");
 auto state=input;state.control->facing=facing;state.control->motion_requested=motion;
 state.phase=FieldSessionPhase::area_action6_control_pending;validate_phase_session(state);return state;
}
}

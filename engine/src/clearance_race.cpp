#include "captain/clearance_race.hpp"
#include "captain/ball_recovery.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>
#include <utility>
namespace captain {
namespace {
ClearanceRaceResult resolve_race(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,
    const ClearanceRaceContent& content,bool area) {
    validate_phase_session(input);
    if(input.phase!=(area ? FieldSessionPhase::area_action6_destination_pending : FieldSessionPhase::capture_clearance_flight))
        throw std::invalid_argument("Clearance race requires prepared flight setup");
    auto state=input;
    if(area)state.clearance_flight=ClearanceFlightContext{4};
    auto& field=state.current.field;
    field.elapsed=1;
    NearestFieldPlayerInput nearest{field.side,field.ball_x,field.ball_y,field.ball_owner,
        state.recovery->opposing_player,{}};
    for(std::size_t i=0;i<field.motion.size();++i) {
        const auto& p=field.motion[i];nearest.players[i]={static_cast<std::uint8_t>(p.x>>8),static_cast<std::uint8_t>(p.y>>8),p.status!=0};
    }
    const auto active=find_nearest_field_player(nearest).player_id;
    nearest.side ^= 11u;
    const auto other=find_nearest_field_player(nearest).player_id;
    field.motion[field_motion_index(active)].target_cell=field.ball_cell;
    field.motion[field_motion_index(other)].target_cell=field.ball_cell;
    auto remaining=state.clearance_flight->distance_adjustment;
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
    state.clearance_race=ClearanceRaceContext{remaining,active,other};
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
    return {std::move(state),rounds,winner,music,script};
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

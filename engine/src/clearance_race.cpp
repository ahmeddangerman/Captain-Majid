#include "captain/clearance_race.hpp"
#include "captain/ball_recovery.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>
#include <utility>
namespace captain {
ClearanceRaceResult resolve_session_clearance_race(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,
    const ClearanceRaceContent& content) {
    validate_phase_session(input);
    if(input.phase!=FieldSessionPhase::capture_clearance_flight)
        throw std::invalid_argument("Clearance race requires prepared flight setup");
    auto state=input;
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
        state.phase=FieldSessionPhase::clearance_race_winner;
    } else state.phase=FieldSessionPhase::clearance_race_timeout;
    validate_phase_session(state);
    return {std::move(state),rounds,winner,music,script};
}
}

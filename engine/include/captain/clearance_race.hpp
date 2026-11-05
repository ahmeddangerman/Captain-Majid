#pragma once
#include "captain/phase_session.hpp"
namespace captain {
struct ClearanceRaceContent { std::array<std::array<std::uint16_t,2>,256> speeds; };
struct ClearanceRaceResult {
    PhaseSessionState state;
    unsigned rounds = 0;
    std::optional<std::uint8_t> winner{};
    bool request_side_music = false;
    std::uint8_t pending_script = 0;
};
ClearanceRaceResult resolve_session_clearance_race(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,
    const ClearanceRaceContent& race);
ClearanceRaceResult resolve_session_area_action6_race(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,
    const ClearanceRaceContent& race);
PhaseSessionState continue_session_area_action6_winner(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion);
}

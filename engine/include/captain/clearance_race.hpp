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
}

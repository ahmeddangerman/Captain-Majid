#pragma once
#include "captain/phase_session.hpp"
namespace captain {
struct ClearanceRaceContent { std::array<std::array<std::uint16_t,2>,256> speeds; };
// Numeric contest up to the winner/timeout branch. Ownership and presentation
// are handled by the caller, and the input field is not modified.
struct FieldLooseBallRaceResult {
    FieldPhaseState field;
    unsigned rounds = 0;
    std::uint8_t remaining = 0, active_candidate = 0, opposing_candidate = 0;
    std::optional<std::uint8_t> winner{};
};
FieldLooseBallRaceResult resolve_field_loose_ball_race(const FieldPhaseState& input,
    std::uint8_t opposing_player, std::uint8_t distance,
    const PlayerProfileMappings& mappings, const FieldPhaseContent& movement,
    const ClearanceRaceContent& race);
// Guarded observed player2 empty pass, split at the original random event.
PhaseSessionState begin_player2_same_cell_pass_race(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings, const FieldPhaseContent& movement,
    const ClearanceRaceContent& race);
PhaseSessionState finish_player2_same_cell_pass_race(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings, const FieldPhaseContent& movement,
    const ClearanceRaceContent& race);
PhaseSessionState resolve_ordinary_free_pass_contest(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,const ClearanceRaceContent& race);
// The observed player3 same-cell pass, saved at the original winner branch.
PhaseSessionState resolve_sampled_player3_pass_race(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings, const FieldPhaseContent& movement,
    const ClearanceRaceContent& race);
PhaseSessionState position_sampled_player3_pass_winner(const PhaseSessionState& input);
// Caller acknowledges the original winner script before field control resumes.
PhaseSessionState continue_sampled_player3_pass_winner(const PhaseSessionState& input);
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

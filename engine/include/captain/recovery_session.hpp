#pragma once
#include "captain/phase_session.hpp"
#include "captain/ball_recovery.hpp"

namespace captain {
struct ClearanceRaceContent;
struct RecoverySessionResult {
    PhaseSessionState state;
    std::optional<FieldPhaseResult> movement{};
    std::optional<BallRecoveryResult> recipient{};
    bool special_owner_cleared = false;
    bool request_possession_sound = false;
};
// Calculates the observed same-cell timeout and recovery after the caller has
// acknowledged the original timeout script; no live presentation is implied.
RecoverySessionResult resolve_sampled_human_empty_pass_recovery(const PhaseSessionState& input,
    const PlayerProfileMappings& mappings,const FieldPhaseContent& movement,const ClearanceRaceContent& race);
// Supported original8BDF..8C31 sequence; selected-defender count must be zero.
// Uses staged inputs without synthesizing frame/NMI randomness.
RecoverySessionResult advance_session_keeper_punch_recovery(const PhaseSessionState& input,
    const PlayerProfileMappings& primary,const FieldPhaseContent& movement);
RecoverySessionResult advance_session_shot_loose_recovery(const PhaseSessionState& input,
    const PlayerProfileMappings& primary,const FieldPhaseContent& movement);
PhaseSessionState jitter_ordinary_return_blocked_recovery(const PhaseSessionState& input);
PhaseSessionState choose_ordinary_return_blocked_recovery_owner(const PhaseSessionState& input);
PhaseSessionState prepare_ordinary_return_goal_area(const PhaseSessionState& input);
PhaseSessionState resolve_ordinary_return_goal_area(const PhaseSessionState& input,const std::array<std::uint8_t,9>& keeper_cells);
PhaseSessionState open_ordinary_return_goal_area_menu(const PhaseSessionState& input);
RecoverySessionResult advance_session_recovery(const PhaseSessionState& input,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement);
RecoverySessionResult advance_session_selected_recovery(const PhaseSessionState& input,
    const std::vector<RecoveryDefender>& selected,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement);
RecoverySessionResult advance_session_clearance_recovery(const PhaseSessionState& input,
    const std::vector<RecoveryDefender>& selected,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement);
RecoverySessionResult advance_session_area_result_recovery(const PhaseSessionState& input,
    const PlayerProfileMappings& primary,const FieldPhaseContent& movement);
RecoverySessionResult advance_session_area_action6_recovery(const PhaseSessionState& input,
    const PlayerProfileMappings& primary,const FieldPhaseContent& movement);
RecoverySessionResult advance_session_area_pass_blocked_recovery(const PhaseSessionState& input,
    const PlayerProfileMappings& primary,const FieldPhaseContent& movement);
PhaseSessionState continue_session_area_pass_blocked_recovery(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion);
PhaseSessionState continue_session_area_action6_recovery(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion);
PhaseSessionState continue_session_selected_recovery(const PhaseSessionState& input);
// Explicit prepared continuation after ownership writes. Area recovery supports
// only the immediate keeper-area gate; ordinary/story area tails reject. Waits
// do not run here.
PhaseSessionState continue_session_recovery(const PhaseSessionState& post_presentation);
// Prepared ordinary-area handoff after host acknowledgement; facing is captured
// from that presentation. Keeper-area and story routes remain separate.
PhaseSessionState continue_session_area_ordinary_recovery(const PhaseSessionState& input, std::uint8_t facing);
PhaseSessionState continue_session_area_story_recovery(const PhaseSessionState& input, std::uint8_t facing);
// Numeric area setup before the original8B73 presentation/menu.
PhaseSessionState prepare_session_keeper_area(const PhaseSessionState& input);
PhaseSessionState prepare_area_pass_keeper(const PhaseSessionState& input);
PhaseSessionState continue_sampled_field_pass_control(const PhaseSessionState& input);
}

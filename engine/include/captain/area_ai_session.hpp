#pragma once
#include "captain/phase_session.hpp"
#include "captain/ai_action_candidate.hpp"
#include "captain/shot_content.hpp"
namespace captain {
struct AreaAction6DestinationContent { std::array<std::array<std::uint8_t,2>,8> positions{}; };
PhaseSessionState continue_session_area_action6(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion,const AreaAction6DestinationContent& content,const ExperienceContent* experience);
PhaseSessionState continue_session_area_action4(const PhaseSessionState& input,std::uint8_t facing,std::uint8_t motion,const ExperienceContent* experience);
struct AreaDispatchPreview { FieldPhaseResult movement; std::uint8_t route; };
struct AreaResultDispatch {
 PhaseSessionState state;
 FieldPhaseResult movement;
 std::optional<ExperienceRewardResult> reward{};
 std::uint8_t route=0;
 bool request_possession_sound=false;
};
// Before the original result dispatch, advance the ordinary field movement only.
// Route0 continues the attack, route2 enters loose-ball recovery, route3 enters
// the original terminal handler. Caller owns any later state transition.
AreaDispatchPreview preview_area_result_dispatch(const PhaseSessionState&,const ShotContent&);
AreaResultDispatch advance_session_area_result_dispatch(const PhaseSessionState&,const ShotContent&);
struct PreparedAreaDefender { FieldDefenderPreparationResult preparation; EnergyResult energy; };
PreparedAreaDefender prepare_area_defender(const FieldPlayerRecord&,std::uint8_t attack,
 std::uint8_t attack_subtype,std::uint8_t variant,std::uint8_t defense,
 std::uint8_t defense_subtype,const ShotContent&);
struct PreparedAreaContact { PreparedAreaDefender defender; DefenderContactResult comparison; };
PreparedAreaContact compare_area_defender(const FieldPlayerRecord&,std::uint8_t attack,
 std::uint8_t attack_subtype,std::uint8_t variant,std::uint8_t defense,
 std::uint8_t defense_subtype,std::uint16_t incoming,std::uint8_t first,
 std::uint8_t second,const ShotContent&);
// Numeric preview before original area result dispatch; elapsed is supplied by
// the original-derived action table. This does not advance the saved phase.
ContactPreludeResult calculate_area_prefix_clock(const PhaseSessionState&,std::uint8_t elapsed);
PhaseSessionResult advance_session_area_prefix_clock(const PhaseSessionState&,std::uint8_t elapsed);
PhaseSessionResult advance_profiled_area_prefix_clock(const PhaseSessionState&,const ShotContent&);
PhaseSessionState resume_session_area_prefix_clock(const PhaseSessionState&);
ActionMenuResult present_session_area_contact_result(const PhaseSessionState&);
ActionMenuResult advance_session_area_defender(const PhaseSessionState&);
ActionMenuResult skip_session_area_defender(const PhaseSessionState&);
PhaseSessionResult advance_session_area_contact_clock(const PhaseSessionState&,const ContactPreludeContent&);
PhaseSessionState resume_session_area_contact_clock(const PhaseSessionState&);
PhaseSessionState resolve_session_area_defender(const PhaseSessionState&,const ShotContent&);
ActionMenuResult announce_prepared_area_defenders(const PhaseSessionState&,std::uint8_t facing);
ActionMenuResult begin_prepared_area_defense(const PhaseSessionState&);
PhaseSessionState prepare_session_area_attack(const PhaseSessionState&,const ShotContent&);
PreparedAttackResult prepare_opponent_area_action(const FieldPlayerRecord& player,
 std::uint8_t action,std::uint8_t variant,std::uint8_t subtype,std::uint8_t first,
 std::uint8_t second,const ShotContent& content);
PhaseSessionState choose_session_area_action(const PhaseSessionState&,const AiAreaChoiceContent&);
}

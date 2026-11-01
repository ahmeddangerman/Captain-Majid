#pragma once
#include "captain/phase_session.hpp"
#include "captain/ai_action_candidate.hpp"
#include "captain/shot_content.hpp"
namespace captain {
ActionMenuResult announce_prepared_area_defenders(const PhaseSessionState&,std::uint8_t facing);
ActionMenuResult begin_prepared_area_defense(const PhaseSessionState&);
PhaseSessionState prepare_session_area_attack(const PhaseSessionState&,const ShotContent&);
PreparedAttackResult prepare_opponent_area_action(const FieldPlayerRecord& player,
 std::uint8_t action,std::uint8_t variant,std::uint8_t subtype,std::uint8_t first,
 std::uint8_t second,const ShotContent& content);
PhaseSessionState choose_session_area_action(const PhaseSessionState&,const AiAreaChoiceContent&);
}

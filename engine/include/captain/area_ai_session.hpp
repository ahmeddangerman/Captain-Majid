#pragma once
#include "captain/phase_session.hpp"
#include "captain/ai_action_candidate.hpp"
namespace captain {
PhaseSessionState choose_session_area_action(const PhaseSessionState&,const AiAreaChoiceContent&);
}

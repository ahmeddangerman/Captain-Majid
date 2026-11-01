#include "captain/area_ai_session.hpp"
#include "captain/player_movement.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState choose_session_area_action(const PhaseSessionState& input,const AiAreaChoiceContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_defense_complete)throw std::invalid_argument("Area AI requires completed defensive choices");
 const auto& f=input.current.field;const auto& player=f.roster[f.ball_owner];
 std::array<std::array<std::uint8_t,2>,10> positions{};
 for(unsigned id=12;id<=21;++id){const auto& m=f.motion[field_motion_index(id)];positions[id-12]={{static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8)}};}
 const auto plan=plan_ai_area_action(player.type,static_cast<std::uint8_t>(player.stamina),static_cast<std::uint8_t>(player.stamina>>8),f.ball_owner,f.ball_x,f.ball_y,f.random_first,f.random_second,input.recovery->ball_variant,positions,content);
 auto state=input;state.action->action=plan.action;state.recovery->attack_subtype=plan.subtype;state.current.field.random_first=plan.random_first;
 state.area_ai_target=PhaseSessionState::AreaAiTarget{plan.recipient.value_or(0),plan.cell.value_or(0)};
 state.phase=FieldSessionPhase::keeper_area_ai_chosen;validate_phase_session(state);return state;
}
}

#include "captain/area_ai_session.hpp"
#include "captain/player_movement.hpp"
#include <stdexcept>
namespace captain {
PhaseSessionState prepare_session_area_attack(const PhaseSessionState& input,const ShotContent& content) {
 validate_phase_session(input);
 if(input.phase!=FieldSessionPhase::keeper_area_ai_chosen)throw std::invalid_argument("Area attack requires a saved opponent choice");
 const auto id=input.current.field.ball_owner;const auto& p=input.current.field.roster[id];
 const auto result=prepare_opponent_area_action({id,p.type,static_cast<std::uint8_t>(p.stamina),static_cast<std::uint8_t>(p.stamina>>8),p.level},input.action->action,input.recovery->ball_variant,input.recovery->attack_subtype,input.current.field.random_first,input.current.field.random_second,content);
 auto state=input;state.action->power=result.action_power.power;state.action->move_effect=result.preparation.move_effect;
 state.action->move_flags=result.move_flags;state.recovery->attack_subtype=result.action_power.subtype;
 state.area_ai_target->cost=result.preparation.stamina_cost;state.phase=FieldSessionPhase::keeper_area_attack_prepared;
 validate_phase_session(state);return state;
}
PreparedAttackResult prepare_opponent_area_action(const FieldPlayerRecord& player,
 std::uint8_t action,std::uint8_t variant,std::uint8_t subtype,std::uint8_t first,
 std::uint8_t second,const ShotContent& content) {
 if(player.player_id<12 || player.player_id>21 || (variant!=1 && variant!=2) ||
    (action!=1 && action!=4 && action!=6) || (action==6 ? subtype>1 : subtype!=0))
  throw std::invalid_argument("Unsupported opponent area attack preparation");
 if(action==6 && !content.area_attack6)throw std::invalid_argument("Missing private area attack6 content");
 // Action6 uses its private selectors/moves in slot0 of the shared numeric loader.
 // Supported subtypes0/1 do not trigger the ordinary-shot subtype override.
 return prepare_attacking_action({player,static_cast<std::uint8_t>(action==6?0:action),variant,subtype,11,0,first,second,false},
  content.field_players,action==6?*content.area_attack6:content.attack);
}
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

#include "captain/complete_prepared_shot.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
namespace {
void req(bool b,const char*m){if(!b){std::cerr<<m<<'\n';std::exit(1);}}
template<class F>void rejects(F f){bool b=false;try{f();}catch(const std::invalid_argument&){b=true;}req(b,"invalid shot accepted");}
}
int main(){
 captain::FieldPlayerProfiles profiles{};profiles.skills.resize(1);
 captain::GoalkeeperProfiles kp{};kp.skills.resize(1);
 captain::FieldDefenseContent attack{},defense{};
 for(auto*c:{&attack,&defense}){for(auto&r:c->skill_selectors)r.fill(4);for(auto&m:c->moves)m={{7,0,30,0}};c->strength_curve.fill(240);}
 captain::GoalkeeperMoveContent kc{};kc.skill_selectors.fill(25);for(auto&m:kc.moves)m={{8,0,80,0}};kc.strength_curve.fill(1);
 captain::CompletePreparedShotInput in{};
 for(auto&p:in.field.roster)p={1,0,200};
 in.field.context={11,0,0,999,7};in.field.ball_owner=15;
 in.attack_random_first=100;in.attack_random_second=100;
 in.field.choices={{3,2,0,0,{999,999,100,100,100,0,false,false,0,13,99,{0,0,0,0}}}};
 in.field.continuation.keeper={999,{999,100,100,200,50,true,false,4,13,99},0,0,0,0,{200,180,100,0}};
 in.field.continuation.outcome.possession_side=0;in.field.continuation.outcome.ball_owner=255;
 in.field.continuation.outcome.left_goals=1;in.field.continuation.outcome.right_goals=2;
 in.field.continuation.outcome.defense_action=1;
 auto run=[&]{return captain::resolve_complete_prepared_shot(in,profiles,attack,defense,kp,kc);};
 auto r=run();
 req(r.attack.action_power.power==214&&r.flow.field.remaining_power==214&&r.keeper&&r.keeper->preparation.base_strength==1,"prepared power and keeper replace stale fixtures");
 req(r.flow.field.roster[15].stamina==200&&r.flow.field.roster[3].stamina==170&&r.flow.field.roster[0].stamina==120,"side11 shared roster charges defender/keeper only");
 req(r.flow.continuation->outcome->event==captain::ShotEvent::goal&&r.flow.continuation->outcome->right_goals==3&&r.flow.continuation->outcome->possession_side==0,"right-side goal and possession");
 req(in.field.roster[0].stamina==200&&in.field.context.incoming_power==999,"input unchanged");
 auto saved=in;in.field.context.side=0;in.field.ball_owner=7;in.field.choices.clear();r=run();
 req(r.flow.field.roster[7].stamina==170&&r.flow.field.roster[11].stamina==200&&r.flow.continuation->outcome->left_goals==2,"side0 charges attacker and skips keeper");
 in=saved;attack.strength_curve.fill(0);kc.strength_curve.fill(240);r=run();
 req(r.flow.continuation->outcome->event==captain::ShotEvent::keeper_catch&&r.flow.continuation->outcome->ball_owner==0,"chosen keeper catch overrides stale outcome defense action");
 in.keeper_action=1;in.field.continuation.outcome.punch_intervals.fill(3);r=run();
 req(r.flow.continuation->outcome->event==captain::ShotEvent::keeper_punch&&r.flow.continuation->outcome->ball_active==1&&r.flow.continuation->outcome->ball_x==80&&r.flow.continuation->outcome->ball_y==88,"chosen punch uses current keeper random bytes");
 in=saved;attack.strength_curve.fill(240);in.field.choices[0].comparison.thresholds={200,100,50,0};in.field.continuation.keeper.roll.offset=230;r=run();
 req(r.flow.field.remaining_power==191&&r.keeper->comparison.ratio==143&&r.flow.continuation->outcome->event==captain::ShotEvent::rebound,"field power loss reaches prepared keeper and rebound outcome");
 in=saved;attack.strength_curve.fill(240);in.field.choices[0].comparison.thresholds={255,220,180,0};kp.skills.clear();r=run();
 req(!r.keeper&&!r.flow.continuation&&r.flow.field_outcome.ball_owner==3&&r.flow.field.roster[0].stamina==200,"terminal field capture skips malformed keeper and its debit");
 in=saved;in.field.context.interruption=captain::FieldInterruptionContext{120,100,{}};in.field.context.interruption->state.status.fill(7);in.field.context.interruption->state.keeper_penalties={8,9};in.field.choices[0].comparison.random_first=0;r=run();
 req(r.flow.field.result_code==4&&r.flow.field.contacts[0].contact.result_code==0&&in.field.context.interruption->state.status[3]==7&&!r.keeper&&r.flow.field_outcome.route==captain::DefenderRoute::restart_decision&&r.flow.field.interruption_state->status[3]==0&&r.flow.field.interruption_state->keeper_penalties[1]==0&&r.flow.field.roster[3].stamina==170,"post-comparison interruption clears flags and skips malformed keeper after defender debit");
 in=saved;in.field.continuation.post_loop_active_defenders=1;r=run();req(!r.keeper&&r.flow.continuation->route==captain::ShotContinuationRoute::other_continuation,"diverted route skips keeper");
 in=saved;in.field.continuation.field_proximity_carry=true;r=run();req(!r.keeper&&r.flow.continuation->route==captain::ShotContinuationRoute::loose_ball,"loose-ball diversion skips keeper");
 in=saved;rejects(run);req(in.field.roster[3].stamina==200,"reached keeper failure leaves initial roster unchanged");
 kp.skills.resize(1);in.field.context.attack_action=1;rejects(run);
 in=saved;in.field.ball_owner=3;rejects(run);
 in.field.ball_owner=11;rejects(run);
 in.field.ball_owner=255;rejects(run);
 std::cout<<"Complete prepared shot checks passed\n";
}

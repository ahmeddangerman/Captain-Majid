#include "captain/prepared_field_shot.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
namespace {
void require(bool ok,const char* msg) { if(!ok) { std::cerr<<msg<<'\n'; std::exit(1); } }
template<class F> void rejects(F f) { bool threw=false; try { f(); }
    catch(const std::invalid_argument&) { threw=true; } require(threw,"invalid reached phase accepted"); }
captain::PreparedFieldShotInput fixture() {
    captain::PreparedFieldShotInput in{};
    for(auto& p: in.roster) p={1,0,200};
    in.context={11,0,0,300,7}; in.ball_owner=15;
    in.choices={{3,2,0,0,{999,999,100,100,100,0,false,false,0,0,99,{0,0,0,0}}}};
    in.continuation={99,99,0,false,0,999,99,
        {0,{256,100,100,200,50,false,false,4,11,99},240,10,3,4,{200,180,100,0}},
        {0,0,4,0,99,0,100,100,2,255,255,9,7,1,2,255,120,100,3,1,0,{2,4,5,7,8,9,10,11}}};
    return in;
}
}
int main() {
    captain::FieldPlayerProfiles profiles{}; profiles.skills.resize(1);
    captain::FieldDefenseContent defense{};
    for(auto& row: defense.skill_selectors) row.fill(4);
    for(auto& moves: defense.moves) moves={{9,0,80,0},{10,0,0,0}};
    defense.strength_curve.fill(240);
    auto in=fixture();
    auto r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.continuation && r.continuation->outcome &&
            r.continuation->outcome->event==captain::ShotEvent::goal &&
            r.continuation->outcome->right_goals==3 &&
            r.continuation->outcome->possession_side==0 &&
            r.continuation->outcome->ball_owner==15 && r.field.roster[3].stamina==120,
            "prepared defender energy and actual side/owner feed goal outcome");
    require(in.roster[3].stamina==200 && in.continuation.outcome.ball_owner==255,
            "input state and stale fixture fields remain unchanged");
    in.context.side=0; in.ball_owner=7;
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.continuation->outcome->left_goals==2 &&
            r.continuation->outcome->possession_side==11 && r.field.roster[3].stamina==200,
            "other scoring side and side-zero defensive energy skip");
    in=fixture(); in.context.incoming_power=140; in.context.attack_subtype=3;
    in.choices[0].subtype=129;
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.field.contacts[0].contact.subtype==129 && r.continuation->keeper->subtype==99 &&
            r.continuation->outcome->event==captain::ShotEvent::special_turnover,
            "attacking subtype stays separate from defender move subtype");
    in=fixture(); in.continuation.keeper.roll.defender_slot=3;
    in.continuation.keeper.critical_threshold=0;
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.continuation->keeper->special,
            "opposing keeper identity replaces stale field-player slot for special roll");
    in=fixture(); in.context.incoming_power=120;
    in.choices[0].comparison.thresholds={100,50,0,0};
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.field.result_code==1 && r.field.remaining_power==97 &&
            r.continuation->outcome->event==captain::ShotEvent::rebound,
            "contact power loss reaches goalkeeper rebound decision");
    in=fixture(); in.context.incoming_power=0;
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.continuation->outcome->event==captain::ShotEvent::keeper_catch &&
            r.continuation->outcome->ball_owner==0 && r.continuation->outcome->possession_side==0,
            "keeper catch transfers actual possession");
    in.continuation.outcome.defense_action=1;
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.continuation->outcome->event==captain::ShotEvent::keeper_punch &&
            r.continuation->outcome->ball_active==1 && r.continuation->outcome->possession_side==11,
            "keeper punch continues active play");
    in=fixture(); in.context.incoming_power=1;
    in.choices[0].comparison.thresholds={255,220,180,0};
    auto unvisited=in.choices[0]; unvisited.player_id=255; unvisited.action=0;
    in.choices.push_back(unvisited); in.continuation.keeper.roll.range=0;
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(!r.continuation && r.field.contacts.size()==1 &&
            r.field_outcome.route==captain::DefenderRoute::player_control &&
            r.field_outcome.ball_owner==3 && r.field_outcome.possession_side==0,
            "last visited defender/action controls capture; malformed unreachable keeper ignored");
    in.choices[0].action=1;
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.field_outcome.route==captain::DefenderRoute::defense_animation &&
            r.field_outcome.ball_owner==15 && !r.continuation,"animation handoff bypasses keeper");
    in.choices[0].comparison.thresholds={255,220,0,0};
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.field_outcome.route==captain::DefenderRoute::recover_ball &&
            !r.field_outcome.recovery && !r.continuation,"missing recovery retains unresolved route");
    captain::BallRecoveryInput recovery{};
    recovery.ball_x=120; recovery.ball_y=100; recovery.ball_owner=255;
    recovery.opposing_player=255; recovery.players.fill({120,100,false});
    recovery.random_e2=0;
    in.recovery=recovery;
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.field_outcome.recovery && r.field_outcome.ball_owner==1 &&
            r.field_outcome.recovery->ball_x==120 && !r.continuation,
            "field recovery joins outcome without visiting keeper");
    recovery.players.fill({0,0,true}); recovery.players[2]={120,100,false};
    recovery.players[3]={120,100,false}; recovery.players[10]={120,100,false};
    in.recovery=recovery;
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.field_outcome.ball_owner==4,
            "current visited defender replaces stale opposing player in recovery exclusion");
    in.recovery->ball_update_active=true;
    rejects([&]{ captain::resolve_prepared_field_shot(in,profiles,defense); });
    require(in.roster[3].stamina==200,"later recovery rejection preserves input stamina");
    for(auto route:{captain::ShotContinuationRoute::other_continuation,
                   captain::ShotContinuationRoute::loose_ball,
                   captain::ShotContinuationRoute::unsupported_action}) {
        in=fixture(); in.continuation.keeper.roll.range=0;
        if(route==captain::ShotContinuationRoute::other_continuation) in.continuation.post_loop_active_defenders=1;
        if(route==captain::ShotContinuationRoute::loose_ball) in.continuation.field_proximity_carry=true;
        if(route==captain::ShotContinuationRoute::unsupported_action) in.context.attack_action=1;
        r=captain::resolve_prepared_field_shot(in,profiles,defense);
        require(r.continuation && r.continuation->route==route && !r.continuation->keeper,
                "diverted continuation does not evaluate invalid keeper");
    }
    in=fixture(); in.continuation.opposing_keeper_status=1;
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.continuation->route==captain::ShotContinuationRoute::other_continuation,
            "keeper status gate retained");
    in=fixture(); in.choices.clear();
    r=captain::resolve_prepared_field_shot(in,profiles,defense);
    require(r.field.contacts.empty() && r.field.result_code==0 && r.continuation->outcome,
            "empty list uses original result-zero route without defender identity");
    in.continuation.keeper.roll.range=0;
    rejects([&]{ captain::resolve_prepared_field_shot(in,profiles,defense); });
    in=fixture(); in.context.side=1;
    rejects([&]{ captain::resolve_prepared_field_shot(in,profiles,defense); });
    std::cout<<"Prepared field shot checks passed\n";
}

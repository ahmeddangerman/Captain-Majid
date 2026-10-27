#include "captain/shot_sequence.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}

captain::Action0ShotContinuationInput make_action0_input() {
    return {
        0, 1, 0, false, 0, 300, 0,
        {0, {256, 100, 100, 200, 50, false, false, 4, 0, 0},
         240, 10, 3, 4, {200, 180, 100, 0}},
        {0, 0, 4, 0, 99, 0, 100, 100, 2, 255, 255, 9,
         7, 1, 2, 0, 30, 40, 3, 1, 0, {2, 4, 5, 7, 8, 9, 10, 11}}
    };
}

void require_diverted(const captain::Action0ShotContinuationInput& input,
                      captain::ShotContinuationRoute expected,
                      const char* message) {
    const auto result = captain::resolve_action0_shot_continuation(input);
    require(result.route == expected, message);
    require(!result.keeper && !result.outcome,
            "diverted route must not produce keeper or outcome results");
}
}

int main() {
    const captain::ShotSequenceInput direct_input{
        {100, 30, 1, 0, 0, 0, false},
        {128, 100, 100, true, 0, false},
        {0, {256, 100, 100, 200, 50, false, false, 4, 0, 0},
         240, 10, 3, 4, {200, 180, 100, 0}},
        {0, 0, 4, 0, 99, 0, 100, 100, 2, 255, 255, 9,
         7, 1, 2, 0, 30, 40, 3, 1, 0, {2, 4, 5, 7, 8, 9, 10, 11}}
    };
    const auto direct = captain::resolve_explicit_shot(direct_input);
    require(direct.energy.stamina == 70 && !direct.energy.weakened &&
            direct.action.power == 114 && direct.keeper.result_code == 2 &&
            direct.outcome.event == captain::ShotEvent::rebound &&
            direct.outcome.keeper_timer == 21 && direct.outcome.keeper_penalty == 4 &&
            direct.outcome.defenders == 0 && direct.outcome.possession_side == 0 &&
            direct.outcome.left_goals == 1 && direct.outcome.right_goals == 2,
            "direct shot still links energy, power, keeper, and rebound state");

    auto goal_input = make_action0_input();
    goal_input.subtype_after_contacts = 37;
    const auto goal = captain::resolve_action0_shot_continuation(goal_input);
    require(goal.route == captain::ShotContinuationRoute::keeper_attempt,
            "eligible result-1 action-0 route reaches keeper");
    require(goal.keeper && goal.outcome, "keeper route produces both calculation results");
    require(goal.keeper->result_code == 0 && goal.outcome->event == captain::ShotEvent::goal,
            "post-contact power flows into keeper comparison and goal handler");
    require(goal.keeper->subtype == 0,
            "keeper subtype remains separate from attacking subtype");
    auto subtype_input = make_action0_input();
    subtype_input.power_after_contacts = 140;
    subtype_input.subtype_after_contacts = 3;
    const auto subtype_result = captain::resolve_action0_shot_continuation(subtype_input);
    require(subtype_result.keeper->result_code == 1 && subtype_result.keeper->subtype == 0 &&
            subtype_result.outcome->event == captain::ShotEvent::special_turnover,
            "result-one outcome uses attacking subtype, not keeper move subtype");
    require(goal.outcome->left_goals == 2 && goal.outcome->possession_side == 11 &&
            goal.outcome->kickoff == 4,
            "goal handler updates score, possession, and kickoff");

    auto save_input = make_action0_input();
    save_input.field_result_code = 0;
    save_input.power_after_contacts = 100;
    const auto save = captain::resolve_action0_shot_continuation(save_input);
    require(save.route == captain::ShotContinuationRoute::keeper_attempt &&
            save.keeper && save.outcome,
            "eligible result-0 action-0 route reaches keeper");
    require(save.keeper->result_code == 2 && save.outcome->event == captain::ShotEvent::rebound,
            "lower post-contact power flows into keeper save outcome");

    auto catch_input = make_action0_input();
    catch_input.power_after_contacts = 0;
    const auto caught = captain::resolve_action0_shot_continuation(catch_input);
    require(caught.keeper && caught.keeper->result_code == 3 && caught.outcome &&
            caught.outcome->event == captain::ShotEvent::keeper_catch,
            "keeper result-3 catch is connected to the action-0 continuation");
    require(caught.outcome->possession_side == 11 && caught.outcome->ball_owner == 11,
            "keeper catch transfers possession and ball ownership");

    auto punch_input = make_action0_input();
    punch_input.power_after_contacts = 0;
    punch_input.outcome.defense_action = 1;
    const auto punched = captain::resolve_action0_shot_continuation(punch_input);
    require(punched.outcome && punched.outcome->event == captain::ShotEvent::keeper_punch &&
            punched.outcome->possession_side == 0 && punched.outcome->ball_active == 1,
            "keeper punch keeps play active without transferring possession");

    auto diverted = make_action0_input();
    diverted.action_code = 1;
    require_diverted(diverted, captain::ShotContinuationRoute::unsupported_action,
                     "other action codes remain outside this continuation");

    diverted = make_action0_input();
    diverted.post_loop_active_defenders = 1;
    require_diverted(diverted, captain::ShotContinuationRoute::other_continuation,
                     "remaining active defenders divert action-0 continuation");

    diverted = make_action0_input();
    diverted.field_proximity_carry = true;
    diverted.opposing_keeper_status = 1;
    require_diverted(diverted, captain::ShotContinuationRoute::loose_ball,
                     "field-proximity branch reaches loose-ball route before keeper check");

    diverted = make_action0_input();
    diverted.opposing_keeper_status = 1;
    require_diverted(diverted, captain::ShotContinuationRoute::other_continuation,
                     "nonzero opposing keeper status diverts the keeper attempt");

    for (const std::uint8_t result_code : {std::uint8_t{2}, std::uint8_t{3}}) {
        auto invalid = make_action0_input();
        invalid.field_result_code = result_code;
        bool rejected = false;
        try {
            (void)captain::resolve_action0_shot_continuation(invalid);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        require(rejected, "field result codes 2 and 3 must not reach action-0 keeper route");
    }

    std::cout << "Explicit shot and action-0 continuation checks passed\n";
}

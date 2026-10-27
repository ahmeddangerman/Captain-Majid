#include "captain/field_handoff.hpp"

#include <array>
#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}

captain::FieldContactHandoffInput make_code_three_input() {
    captain::FieldContactHandoffInput input{};
    input.contacts = {{256, 256, 100, 100, 100, 0, false, false, 0, 3, 0,
                       {255, 220, 180, 0}}};
    input.incoming_power = 256;
    input.possession_side = 0;
    input.ball_owner = 7;
    input.defender_id = 12;
    input.defense_action = 2;
    return input;
}
}

int main() {
    {
        captain::FieldContactHandoffInput input{};
        input.contacts = {{160, 256, 100, 100, 100, 0, false, false, 0, 3, 0,
                           {255, 150, 100, 0}}};
        input.incoming_power = 160;
        input.possession_side = 0;
        input.ball_owner = 22;
        auto recovery = captain::BallRecoveryInput{};
        recovery.ball_x = 120;
        recovery.ball_y = 100;
        recovery.ball_owner = 22;
        recovery.opposing_player = 22;
        recovery.random_e2 = 0x80;
        recovery.players.fill({120, 100, false});
        input.recovery = recovery;

        const auto result = captain::resolve_field_contact_handoff(input);
        require(result.field.result_code == 2 && result.field.stopped,
                "field contact result two reaches the handoff stage");
        require(result.outcome.route == captain::DefenderRoute::recover_ball &&
                result.outcome.recovery && result.outcome.possession_side == 11 &&
                result.outcome.ball_owner == 12,
                "result two runs ordinary recovery and transfers possession");
    }

    {
        const auto result = captain::resolve_field_contact_handoff(make_code_three_input());
        require(result.field.result_code == 3 && result.field.stopped,
                "field contact result three reaches the handoff stage");
        require(result.outcome.route == captain::DefenderRoute::player_control &&
                result.outcome.possession_side == 11 && result.outcome.ball_owner == 12,
                "result three applies field-player possession control");
    }

    {
        auto input = make_code_three_input();
        input.possession_side = 11;
        input.ball_owner = 7;
        input.defender_id = 3;
        input.defense_action = 1;
        const auto result = captain::resolve_field_contact_handoff(input);
        require(result.outcome.route == captain::DefenderRoute::defense_animation &&
                result.outcome.possession_side == 11 && result.outcome.ball_owner == 7,
                "result-three animation leaves possession for its later phase");
    }

    {
        captain::FieldContactHandoffInput input{};
        input.contacts = {{256, 256, 100, 100, 100, 0, false, false, 0, 1, 0,
                           {200, 100, 50, 0}}};
        input.incoming_power = 256;
        input.possession_side = 0;
        input.ball_owner = 7;
        const auto result = captain::resolve_field_contact_handoff(input);
        require(result.field.result_code == 1 && !result.field.stopped &&
                result.outcome.route == captain::DefenderRoute::continue_action &&
                result.outcome.possession_side == 0 && result.outcome.ball_owner == 7,
                "field result one continues without changing match ownership");
    }

    {
        captain::FieldContactHandoffInput input{};
        input.initial_result_code = 0;
        input.possession_side = 0;
        input.ball_owner = 7;
        const auto result = captain::resolve_field_contact_handoff(input);
        require(result.field.contacts.empty() && result.field.result_code == 0 &&
                result.outcome.route == captain::DefenderRoute::continue_action &&
                result.outcome.possession_side == 0 && result.outcome.ball_owner == 7,
                "empty result-zero sequence continues without changing ownership");
    }

    {
        captain::FieldContactHandoffInput input{};
        input.initial_result_code = 3;
        input.possession_side = 11;
        input.ball_owner = 7;
        input.defender_id = 0;
        const auto result = captain::resolve_field_contact_handoff(input);
        require(result.field.contacts.empty() && !result.field.stopped &&
                result.field.result_code == 3 &&
                result.outcome.route == captain::DefenderRoute::keeper_control &&
                result.outcome.possession_side == 0 && result.outcome.ball_owner == 0,
                "empty sequence dispatches its initial result to keeper control");
    }

    std::cout << "Field-contact handoff checks passed\n";
}

#include "captain/control_tick.hpp"
#include "captain/phase_session_io.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
std::string bytes(const captain::PhaseSessionState& state) {
    std::ostringstream output; captain::write_phase_session(output, state); return output.str();
}
}
int main() {
    using namespace captain;
    try {
        std::string records = "CMJWORLDOBS1\n";
        for (unsigned i = 0; i < 22; ++i) records += "1 0 2 3 1 1 160 3\n";
        std::istringstream record_stream(records);
        const auto parsed = load_observed_field_polls(record_stream);
        if (parsed[0].buttons.held != 1 || parsed[0].random_first != 2 ||
            !parsed[0].travel_observed || parsed[0].travel_speed != 928) return 1;
        for (const auto& invalid : {std::string("BAD\n"), records + "1\n", std::string("CMJWORLDOBS1\n1 0 2\n")}) {
            bool rejected = false;
            try { std::istringstream input(invalid); load_observed_field_polls(input); }
            catch (const std::invalid_argument&) { rejected = true; }
            if (!rejected) return 1;
        }
        const std::string original_record = "1 0 2 3 1 1 160 3";
        for (const std::string& bad_record : {"1 0 2 3 2 1 160 3", "1 0 2 3 0 1 160 3", "1 0 2 3 1 1 256 3"}) {
            auto invalid = records;
            invalid.replace(invalid.find(original_record), original_record.size(), bad_record);
            bool rejected = false;
            try { std::istringstream input(invalid); load_observed_field_polls(input); }
            catch (const std::invalid_argument&) { rejected = true; }
            if (!rejected) return 1;
        }
        PhaseSessionState state{}; state.phase = FieldSessionPhase::field_round_pending;
        state.experience = ExperienceState{}; state.recovery = RecoveryContext{};
        state.control = ControlContext{}; state.scheduling = SchedulingContext{};
        state.current.field.side = 0; state.current.field.ball_owner = 1;
        state.current.field.ball_x = 124; state.current.field.ball_y = 124;
        state.current.field.ball_cell = 113; state.current.field.elapsed = 1;
        state.current.field.movement_region = 7; state.control->facing = 64;
        for (auto& player : state.current.field.roster) player.type = 1;
        for (auto& motion : state.current.field.motion) { motion.x = 124u << 8; motion.y = 124u << 8; motion.target_cell = 113; }
        PlayerProfileMappings profiles{}; FieldPhaseContent movement{};
        movement.profiles.ordinary[1].speed_profile = 0;
        movement.profiles.speeds.push_back({0, 0});
        ControlMovementContent control{}; WorldStepContent world{};
        std::array<LogicalButtons, 22> buttons{};
        std::array<ObservedFieldPoll, 22> observed{};
        const auto legacy = advance_control_field_round(state, profiles, movement, control, world, buttons);
        const auto no_events = advance_observed_control_field_round(state, profiles, movement, control, world, observed);
        if (bytes(legacy.state) != bytes(no_events.state) || no_events.consumed_polls != 22) return 1;
        for (auto& event : observed) {
            event.random_first = 49; event.random_second = 55;
            event.travel_observed = true; event.travel_held = 1; event.travel_speed = 928;
        }
        const auto sampled = advance_observed_control_field_round(state, profiles, movement, control, world, observed);
        if (sampled.consumed_polls != 22 || sampled.state.control->travel_accumulator != 5104 ||
            sampled.state.current.field.random_first != 49 || sampled.state.current.field.random_second != 55 ||
            state.control->travel_accumulator != 0) return 1;
        observed[0].buttons.held = 64;
        const auto menu = advance_observed_control_field_round(state, profiles, movement, control, world, observed);
        if (menu.consumed_polls != 1 || menu.state.phase != FieldSessionPhase::action_menu_pending ||
            menu.state.control->travel_accumulator != 232) return 1;
        observed[21].travel_observed = false;
        const auto original = bytes(state);
        bool rejected = false;
        try { advance_observed_control_field_round(state, profiles, movement, control, world, observed); }
        catch (const std::invalid_argument&) { rejected = true; }
        if (!rejected || bytes(state) != original) return 1;
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

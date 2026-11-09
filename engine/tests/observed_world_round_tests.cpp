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
        std::string nmi_records = "CMJWORLDNMI1\n";
        for (unsigned i = 0; i < 22; ++i) nmi_records += "0 0 0 0 0 0 0 0 0 0\n";
        std::istringstream nmi_stream(nmi_records);
        const auto parsed_nmi = load_observed_nmi_field_polls(nmi_stream);
        if (parsed_nmi[0].random.carry || parsed_nmi[0].travel_observed) return 1;
        std::istringstream single_poll("CMJPOLLNMI1\n1 0 5 6 7 1 1 1 160 3\n");
        const auto parsed_poll = load_observed_nmi_poll(single_poll);
        if (parsed_poll.buttons.held != 1 || !parsed_poll.random.carry || parsed_poll.travel_speed != 928) return 1;
        bool bad_poll_rejected = false;
        try { std::istringstream bad("CMJPOLLNMI1\n1 0 5 6 7 1 1 1 160 3 4\n"); load_observed_nmi_poll(bad); }
        catch (const std::invalid_argument&) { bad_poll_rejected = true; }
        if (!bad_poll_rejected) return 1;
        std::istringstream clock_stream("CMJCLOCKNMI1\nR 1 2 3 1\nT 1 160 3\nS\nE\n");
        const auto parsed_clock = load_observed_clock_events(clock_stream);
        if (parsed_clock.size() != 4 || parsed_clock[0].kind != ObservedClockEventKind::random ||
            parsed_clock[1].speed != 928 || parsed_clock[3].kind != ObservedClockEventKind::energy) return 1;
        bool bad_clock_rejected = false;
        try { std::istringstream bad("CMJCLOCKNMI1\nR 1 2 3 2\nS\nE\n"); load_observed_clock_events(bad); }
        catch (const std::invalid_argument&) { bad_clock_rejected = true; }
        if (!bad_clock_rejected) return 1;
        std::istringstream sequence_stream("CMJNMISEQ1\nR 1 2 3 1\nT 1 160 3\n");
        const auto parsed_sequence = load_observed_nmi_events(sequence_stream);
        if (parsed_sequence.size() != 2 || parsed_sequence[1].speed != 928) return 1;
        const std::string good_nmi_record = "0 0 0 0 0 0 0 0 0 0";
        for (const std::string& bad_record : {"0 0 0 0 0 2 0 0 0 0", "0 0 0 0 0 0 0 1 0 0", "0 0 256 0 0 0 0 0 0 0"}) {
            auto invalid = nmi_records;
            invalid.replace(invalid.find(good_nmi_record), good_nmi_record.size(), bad_record);
            bool rejected = false;
            try { std::istringstream input(invalid); load_observed_nmi_field_polls(input); }
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
        auto event_input = state; event_input.phase = FieldSessionPhase::field_contact_pending;
        event_input.action = ActionContext{}; event_input.current.field.selected_defenders = 1;
        event_input.scheduling->selected_ids[0] = 12;
        const auto event_result = advance_observed_nmi_events(event_input, 255,
            {{ObservedClockEventKind::random, {0, 0, 0, false}}, {ObservedClockEventKind::travel, {}, 1, 928}});
        if (event_result.random_index != 0 || event_result.state.control->travel_accumulator != 232 ||
            event_input.control->travel_accumulator != 0) return 1;
        bool marker_rejected = false;
        try { advance_observed_nmi_events(event_input, 0, {{ObservedClockEventKind::schedule}}); }
        catch (const std::invalid_argument&) { marker_rejected = true; }
        if (!marker_rejected) return 1;
        for (auto& player : state.current.field.roster) player.type = 1;
        for (auto& motion : state.current.field.motion) { motion.x = 124u << 8; motion.y = 124u << 8; motion.target_cell = 113; }
        PlayerProfileMappings profiles{}; FieldPhaseContent movement{};
        movement.profiles.ordinary[1].speed_profile = 0;
        movement.profiles.speeds.push_back({0, 0});
        ControlMovementContent control{}; WorldStepContent world{};
        auto clock_input = state;
        clock_input.phase = FieldSessionPhase::control_clock_pending;
        clock_input.current.clock.remaining = 180;
        FieldPlayerProfiles field_profiles{}; field_profiles.skills.resize(1);
        GoalkeeperProfiles keeper_profiles{}; keeper_profiles.skills.resize(1);
        world.stamina_curve.fill(1000);
        const std::vector<ObservedClockEvent> clock_events = {
            {ObservedClockEventKind::random, {0, 0, 0, false}},
            {ObservedClockEventKind::travel, {}, 1, 928},
            {ObservedClockEventKind::schedule},
            {ObservedClockEventKind::energy}
        };
        const auto clock_result = advance_observed_nmi_control_clock(
            clock_input, field_profiles, keeper_profiles, world, 255, clock_events);
        if (clock_result.tick.state.phase != FieldSessionPhase::field_round_pending ||
            clock_result.tick.state.current.clock.remaining != 179 || clock_result.random_index != 0 ||
            clock_result.tick.state.control->travel_accumulator != 232 ||
            clock_result.tick.state.current.field.random_first != 255 ||
            clock_input.control->travel_accumulator != 0) return 1;
        bool invalid_clock_rejected = false;
        try { advance_observed_nmi_control_clock(clock_input, field_profiles, keeper_profiles, world, 0,
                                                  {{ObservedClockEventKind::energy}}); }
        catch (const std::invalid_argument&) { invalid_clock_rejected = true; }
        if (!invalid_clock_rejected) return 1;
        const auto plain_clock = advance_observed_nmi_control_clock(clock_input, field_profiles, keeper_profiles,
            world, 7, {{ObservedClockEventKind::schedule}, {ObservedClockEventKind::energy}});
        if (bytes(plain_clock.tick.state) != bytes(advance_control_tick(clock_input, field_profiles, keeper_profiles, world).state) ||
            plain_clock.random_index != 7) return 1;
        auto warning_clock = clock_input; warning_clock.current.clock.remaining = 30;
        bool warning_rejected = false;
        try { advance_observed_nmi_control_clock(warning_clock, field_profiles, keeper_profiles, world, 0,
                {{ObservedClockEventKind::schedule}, {ObservedClockEventKind::energy}}); }
        catch (const std::invalid_argument&) { warning_rejected = true; }
        if (!warning_rejected) return 1;
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
        std::array<ObservedNmiFieldPoll, 22> nmi{};
        auto nmi_result = advance_observed_nmi_field_round(state, profiles, movement, control, world, 250, nmi);
        ReferenceRandomState expected_random{250, 0, 0};
        for (unsigned i = 0; i < 22; ++i) expected_random = advance_reference_random(expected_random, {}).state;
        if (nmi_result.round.consumed_polls != 22 || nmi_result.random_index != 16 ||
            nmi_result.round.state.current.field.random_first != expected_random.first ||
            nmi_result.round.state.current.field.random_second != expected_random.second) return 1;
        auto changed_region = state; changed_region.current.field.movement_region = 0;
        movement.geometry.direction_thresholds.fill(65535);
        auto interleaved = advance_observed_nmi_field_round(changed_region, profiles, movement, control, world, 250, nmi);
        expected_random = {250, 0, 0};
        for (unsigned slot = 0; slot < 22; ++slot) {
            expected_random = advance_reference_random(expected_random, {}).state;
            if (slot != 0 && slot != 1 && slot != 11) expected_random.first >>= 1;
        }
        if (interleaved.random_index != expected_random.index ||
            interleaved.round.state.current.field.random_first != expected_random.first ||
            interleaved.round.state.current.field.random_second != expected_random.second) return 1;
        nmi[0].buttons.held = 64;
        const auto early = advance_observed_nmi_field_round(state, profiles, movement, control, world, 255, nmi);
        if (early.round.consumed_polls != 1 || early.random_index != 0 ||
            early.round.state.phase != FieldSessionPhase::action_menu_pending ||
            early.round.state.current.field.random_first != 255) return 1;
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

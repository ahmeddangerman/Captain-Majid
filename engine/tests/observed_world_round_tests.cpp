#include "captain/control_tick.hpp"
#include "captain/control_session.hpp"
#include "captain/action_menu.hpp"
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
        std::istringstream sampled_single_poll("CMJPOLLNMI2\n1 0 5 6 7 1 1 1 160 3 192\n");
        const auto sampled_single = load_observed_nmi_poll(sampled_single_poll);
        if (!sampled_single.ball_mode_sample || *sampled_single.ball_mode_sample != 192) return 1;
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
        std::istringstream motion_stream("CMJNMISEQ1\nM 1\n");
        const auto parsed_motion = load_observed_nmi_events(motion_stream);
        if (parsed_motion.size() != 1 || parsed_motion[0].kind != ObservedClockEventKind::motion_sample || parsed_motion[0].held != 1) return 1;
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
        auto player7_encounter = event_input;
        player7_encounter.score = MatchScoreContext{};
        player7_encounter.score->left_goals = 1;
        player7_encounter.score->kickoff = 4;
        player7_encounter.current.clock.remaining = 146;
        player7_encounter.current.field.ball_owner = 7;
        player7_encounter.current.field.ball_x = 120;
        player7_encounter.current.field.ball_y = 124;
        player7_encounter.current.field.ball_cell = 113;
        player7_encounter.scheduling->selected_ids[0] = 20;
        player7_encounter.control->display_parameter = 2;
        const auto encounter_presentation = advance_observed_nmi_events(player7_encounter, 0,
            {{ObservedClockEventKind::facing_sample, {}, 0}, {ObservedClockEventKind::display_sample, {}, 0}});
        if (encounter_presentation.state.control->facing != 0 ||
            encounter_presentation.state.control->display_parameter != 0 ||
            player7_encounter.control->facing != 64 || player7_encounter.control->display_parameter != 2) return 1;
        bool wrong_encounter_sample_rejected = false;
        try { advance_observed_nmi_events(player7_encounter, 0, {{ObservedClockEventKind::facing_sample, {}, 1}}); }
        catch (const std::invalid_argument&) { wrong_encounter_sample_rejected = true; }
        if (!wrong_encounter_sample_rejected) return 1;
        auto player3_encounter = player7_encounter;
        player3_encounter.current.clock.remaining = 139;
        player3_encounter.current.field.ball_owner = 3;
        player3_encounter.current.field.ball_y = 140;
        player3_encounter.current.field.ball_cell = 115;
        player3_encounter.current.field.selected_defenders = 3;
        player3_encounter.scheduling->selected_ids = {16,19,20,0,0};
        const auto player3_menu_presentation = advance_observed_nmi_events(player3_encounter, 0,
            {{ObservedClockEventKind::facing_sample, {}, 0}, {ObservedClockEventKind::display_sample, {}, 0}});
        if (player3_menu_presentation.state.control->facing || player3_menu_presentation.state.control->display_parameter ||
            player3_encounter.control->facing != 64) return 1;
        auto one_two_choice = player3_menu_presentation.state;
        one_two_choice.phase = FieldSessionPhase::attack_choice_pending;
        one_two_choice.action->action = 3;
        one_two_choice.recovery->opposing_player = 20;
        std::istringstream one_two_events("CMJNMISEQ1\nG 3\nM 23\nM 0\nB 130\nM 1\nB 0\nO 16\nO 19\nO 20\n");
        const auto one_two_wait = advance_observed_nmi_events(one_two_choice, 0, load_observed_nmi_events(one_two_events));
        if (one_two_wait.state.action->move_flags != 3 || one_two_wait.state.control->motion_requested != 1 ||
            one_two_wait.state.recovery->opposing_player != 20 || one_two_wait.state.current.clock.ball_mode ||
            one_two_choice.action->move_flags) return 1;
        bool incomplete_one_two_rejected = false;
        try { std::istringstream incomplete("CMJNMISEQ1\nG 3\nM 23\n"); advance_observed_nmi_events(one_two_choice, 0, load_observed_nmi_events(incomplete)); }
        catch (const std::invalid_argument&) { incomplete_one_two_rejected = true; }
        if (!incomplete_one_two_rejected) return 1;
        auto one_two_handoff = one_two_wait.state;
        one_two_handoff.current.field.roster[3].stamina = 310;
        one_two_handoff.current.field.random_first = 32; one_two_handoff.current.field.random_second = 42;
        one_two_handoff.action->cursor = 162; one_two_handoff.action->power = 5; one_two_handoff.saved_ball_flight_flag = 1;
        const auto one_two_defenders = capture_player3_one_two_defenders(one_two_handoff, {1,2,2});
        if (!one_two_defenders.human_defense_choices || one_two_handoff.human_defense_choices ||
            one_two_defenders.human_defense_choices->actions != std::array<std::uint8_t,5>{1,2,2,0,0}) return 1;
        std::istringstream one_two_snapshot(bytes(one_two_defenders));
        if (bytes(load_phase_session(one_two_snapshot)) != bytes(one_two_defenders)) return 1;
        bool wrong_one_two_defense_rejected = false;
        try { capture_player3_one_two_defenders(one_two_handoff, {1,2,1}); }
        catch (const std::invalid_argument&) { wrong_one_two_defense_rejected = true; }
        if (!wrong_one_two_defense_rejected) return 1;
        auto one_two_prepared = one_two_defenders;
        one_two_prepared.phase = FieldSessionPhase::attack_prepared;
        one_two_prepared.current.field.roster[3].stamina = 250;
        one_two_prepared.action->power = 8;
        one_two_prepared.control->last_carrier_cell = 128;
        std::istringstream defender_wait_events("CMJNMISEQ1\nL 0\nM 0\nF 64\n");
        const auto one_two_defender_wait = advance_observed_nmi_events(one_two_prepared, 0, load_observed_nmi_events(defender_wait_events));
        if (one_two_defender_wait.state.control->last_carrier_cell || one_two_defender_wait.state.control->motion_requested ||
            one_two_defender_wait.state.control->facing != 64 || one_two_prepared.control->last_carrier_cell != 128) return 1;
        auto first_defender_input = one_two_defender_wait.state;
        first_defender_input.current.field.random_first = 193; first_defender_input.current.field.random_second = 239;
        FieldPlayerProfiles one_two_profiles{}; one_two_profiles.team_types.resize(1); one_two_profiles.skills.resize(1);
        FieldDefenseContent one_two_defense{}; one_two_defense.skill_selectors[1][0] = 1;
        one_two_defense.moves[1].push_back({0,0,60,0}); one_two_defense.strength_curve.fill(8);
        const auto first_defender = prepare_player3_one_two_defender(first_defender_input, one_two_profiles, one_two_defense);
        if (!first_defender.area_contact || first_defender.recovery->opposing_player != 16 ||
            first_defender.area_contact->defender_power != 8 || first_defender_input.area_contact) return 1;
        const FieldComparisonSelection one_two_comparison{13,1,32,false,{200,150,100,0}};
        const auto first_compared = compare_player3_one_two_defender(first_defender, one_two_comparison);
        if (first_compared.area_contact->strength != 1 || first_compared.area_contact->score != 255 ||
            first_compared.area_sequence_result != 0 || first_defender.area_sequence_result != 255) return 1;
        std::istringstream first_compared_snapshot(bytes(first_compared));
        if (bytes(load_phase_session(first_compared_snapshot)) != bytes(first_compared)) return 1;
        bool wrong_one_two_table_rejected = false;
        try { auto wrong=one_two_comparison; wrong.table_index=12; compare_player3_one_two_defender(first_defender,wrong); }
        catch (const std::invalid_argument&) { wrong_one_two_table_rejected = true; }
        if (!wrong_one_two_table_rejected) return 1;
        auto one_two_contact_input = first_compared;
        one_two_contact_input.current.field.random_first = 128; one_two_contact_input.current.field.random_second = 175;
        one_two_contact_input.current.field.roster[3].type = 3;
        ContactPreludeContent one_two_prelude{}; one_two_prelude.field_elapsed[3] = 1; one_two_prelude.defender_status[1] = 6;
        ExperienceContent one_two_experience{}; one_two_experience.player_mappings[3] = 3;
        one_two_experience.stage_bases[0] = 128; one_two_experience.reward_factors[1] = 6;
        const auto one_two_contact = advance_player3_one_two_contact(one_two_contact_input,one_two_prelude,one_two_experience);
        if (one_two_contact.current.clock.remaining != 138 || one_two_contact.current.field.elapsed != 2 ||
            one_two_contact.current.field.motion[field_motion_index(16)].status != 6 || one_two_contact.experience->totals[3] != 3 ||
            one_two_contact_input.current.clock.remaining != 139) return 1;
        std::istringstream one_two_contact_snapshot(bytes(one_two_contact));
        if (bytes(load_phase_session(one_two_contact_snapshot)) != bytes(one_two_contact)) return 1;
        bool repeated_one_two_contact_rejected = false;
        try { advance_player3_one_two_contact(one_two_contact,one_two_prelude,one_two_experience); }
        catch (const std::invalid_argument&) { repeated_one_two_contact_rejected = true; }
        if (!repeated_one_two_contact_rejected) return 1;
        auto second_defender_input = one_two_contact;
        second_defender_input.current.field.random_first = 93; second_defender_input.current.field.random_second = 144;
        one_two_defense.skill_selectors[2][0] = 1; one_two_defense.moves[2].push_back({0,0,50,0});
        const auto second_defender = prepare_player3_one_two_next_defender(second_defender_input,one_two_profiles,one_two_defense);
        if (second_defender.recovery->opposing_player != 19 || second_defender.area_defender_cursor != 1 ||
            second_defender.area_contact->defense_action != 2 || second_defender_input.recovery->opposing_player != 16) return 1;
        const FieldComparisonSelection second_comparison{14,255,64,false,{200,100,50,0}};
        const auto second_compared = compare_player3_one_two_next_defender(second_defender,second_comparison);
        if (second_compared.area_contact->strength != 5 || second_compared.area_contact->score != 102 ||
            second_compared.area_sequence_result != 1 || second_defender.area_sequence_result != 255) return 1;
        std::istringstream second_compared_snapshot(bytes(second_compared));
        if (bytes(load_phase_session(second_compared_snapshot)) != bytes(second_compared)) return 1;
        bool wrong_second_table_rejected = false;
        try { auto wrong=second_comparison; wrong.table_index=13; compare_player3_one_two_next_defender(second_defender,wrong); }
        catch (const std::invalid_argument&) { wrong_second_table_rejected = true; }
        if (!wrong_second_table_rejected) return 1;
        const auto second_wait = advance_observed_nmi_events(second_compared, 0, {{ObservedClockEventKind::facing_sample,{},0}});
        auto second_contact_input = second_wait.state;
        second_contact_input.current.field.random_first = 10; second_contact_input.current.field.random_second = 211;
        one_two_prelude.field_elapsed[5] = 1; one_two_prelude.defender_status[2] = 6;
        const auto second_contact = advance_player3_one_two_next_contact(second_contact_input,one_two_prelude,one_two_experience);
        if (second_contact.current.clock.remaining != 137 || second_contact.current.field.elapsed != 3 ||
            second_contact.current.field.motion[field_motion_index(19)].status != 6 || second_contact.action->power != 7 ||
            second_contact.experience->totals[3] != 6 || second_contact_input.action->power != 8) return 1;
        std::istringstream second_contact_snapshot(bytes(second_contact));
        if (bytes(load_phase_session(second_contact_snapshot)) != bytes(second_contact)) return 1;
        bool repeated_second_contact_rejected = false;
        try { advance_player3_one_two_next_contact(second_contact,one_two_prelude,one_two_experience); }
        catch (const std::invalid_argument&) { repeated_second_contact_rejected = true; }
        if (!repeated_second_contact_rejected) return 1;
        auto player7_shot_choice = encounter_presentation.state;
        player7_shot_choice.phase = FieldSessionPhase::attack_choice_pending;
        player7_shot_choice.action->action = 0;
        player7_shot_choice.recovery->opposing_player = 5;
        const auto player7_defender = record_player7_shot_defender(player7_shot_choice, 20, 2, 0);
        if (player7_defender.recovery->opposing_player != 20 || player7_shot_choice.recovery->opposing_player != 5) return 1;
        bool wrong_shot_defender_rejected = false;
        try { record_player7_shot_defender(player7_shot_choice, 20, 1, 0); }
        catch (const std::invalid_argument&) { wrong_shot_defender_rejected = true; }
        if (!wrong_shot_defender_rejected) return 1;
        bool repeated_shot_defender_rejected = false;
        try { record_player7_shot_defender(player7_defender, 20, 2, 0); }
        catch (const std::invalid_argument&) { repeated_shot_defender_rejected = true; }
        if (!repeated_shot_defender_rejected) return 1;
        auto player7_prepared = player7_defender;
        player7_prepared.phase = FieldSessionPhase::attack_prepared;
        player7_prepared.current.field.roster[7].stamina = 305;
        player7_prepared.action->power = 5;
        const auto player7_shot_motion = advance_observed_nmi_events(player7_prepared, 0,
            {{ObservedClockEventKind::motion_sample, {}, 11}, {ObservedClockEventKind::motion_sample, {}, 0},
             {ObservedClockEventKind::motion_sample, {}, 2}});
        if (player7_shot_motion.state.control->motion_requested != 2 || player7_prepared.control->motion_requested != 0) return 1;
        bool wrong_shot_motion_rejected = false;
        try { advance_observed_nmi_events(player7_prepared, 0, {{ObservedClockEventKind::motion_sample, {}, 2}}); }
        catch (const std::invalid_argument&) { wrong_shot_motion_rejected = true; }
        if (!wrong_shot_motion_rejected) return 1;
        FieldPlayerProfiles shot_defender_profiles{};
        shot_defender_profiles.team_types.resize(1);
        shot_defender_profiles.skills.resize(1);
        FieldDefenseContent shot_defense{};
        shot_defense.skill_selectors[2][0] = 1;
        shot_defense.moves[2].push_back({0, 0, 50, 0});
        shot_defense.strength_curve.fill(8);
        const auto player7_defender_prepared = prepare_player7_shot_defender(player7_shot_motion.state, 1, shot_defender_profiles, shot_defense);
        if (player7_defender_prepared.phase != FieldSessionPhase::player7_shot_defender_prepared ||
            player7_defender_prepared.area_contact->defense_action != 2 || player7_defender_prepared.area_contact->defender_power != 8 ||
            player7_defender_prepared.area_contact->cost != 50 || player7_defender_prepared.control->last_carrier_cell != 128) return 1;
        std::istringstream defender_snapshot(bytes(player7_defender_prepared));
        if (bytes(load_phase_session(defender_snapshot)) != bytes(player7_defender_prepared)) return 1;
        bool wrong_shot_variant_rejected = false;
        try { prepare_player7_shot_defender(player7_shot_motion.state, 2, shot_defender_profiles, shot_defense); }
        catch (const std::invalid_argument&) { wrong_shot_variant_rejected = true; }
        if (!wrong_shot_variant_rejected) return 1;
        auto player7_compared = player7_defender_prepared;
        player7_compared.phase = FieldSessionPhase::player7_shot_defender_compared;
        player7_compared.area_contact->strength = 4;
        player7_compared.area_contact->loss = 1;
        player7_compared.area_contact->score = 80;
        player7_compared.area_contact->result = 2;
        player7_compared.area_sequence_result = 2;
        player7_compared.control->facing = 0;
        const auto terminal_wait = advance_observed_nmi_events(player7_compared, 0,
            {{ObservedClockEventKind::facing_sample, {}, 64}, {ObservedClockEventKind::motion_sample, {}, 0},
             {ObservedClockEventKind::facing_sample, {}, 0}});
        if (terminal_wait.state.control->facing || terminal_wait.state.control->motion_requested ||
            player7_compared.control->motion_requested != 2) return 1;
        bool invalid_terminal_wait_rejected = false;
        try { advance_observed_nmi_events(player7_compared, 0, {{ObservedClockEventKind::motion_sample, {}, 0}}); }
        catch (const std::invalid_argument&) { invalid_terminal_wait_rejected = true; }
        if (!invalid_terminal_wait_rejected) return 1;
        ContactPreludeContent player7_contact_content{};
        player7_contact_content.field_elapsed[4] = 2;
        const auto player7_contact = advance_player7_shot_contact(terminal_wait.state, player7_contact_content);
        if (player7_contact.phase != FieldSessionPhase::player7_shot_contact_clocked ||
            player7_contact.current.clock.remaining != 144 || player7_contact.current.field.elapsed != 3 ||
            player7_contact.current.field.motion[field_motion_index(7)].status != 5 ||
            terminal_wait.state.current.clock.remaining != 146) return 1;
        std::istringstream contact_snapshot(bytes(player7_contact));
        if (bytes(load_phase_session(contact_snapshot)) != bytes(player7_contact)) return 1;
        bool invalid_contact_rejected = false;
        try { advance_player7_shot_contact(player7_compared, player7_contact_content); }
        catch (const std::invalid_argument&) { invalid_contact_rejected = true; }
        if (!invalid_contact_rejected) return 1;
        auto player7_loose_input = player7_contact;
        player7_loose_input.current.field.random_first = 2;
        player7_loose_input.current.field.random_second = 56;
        const auto player7_loose = jitter_player7_shot_ball(player7_loose_input);
        if (player7_loose.phase != FieldSessionPhase::player7_shot_ball_jittered || player7_loose.current.field.ball_x != 120 ||
            player7_loose.current.field.ball_y != 140 || player7_loose.current.field.selected_defenders ||
            player7_loose.scheduling->selected_ids[0] || player7_loose_input.current.field.ball_y != 124) return 1;
        std::istringstream loose_snapshot(bytes(player7_loose));
        if (bytes(load_phase_session(loose_snapshot)) != bytes(player7_loose)) return 1;
        bool invalid_loose_random_rejected = false;
        try { jitter_player7_shot_ball(player7_contact); }
        catch (const std::invalid_argument&) { invalid_loose_random_rejected = true; }
        if (!invalid_loose_random_rejected) return 1;
        auto player7_loose_moved = player7_loose;
        player7_loose_moved.phase = FieldSessionPhase::player7_shot_recovery_field_moved;
        player7_loose_moved.current.field.elapsed = 0;
        player7_loose_moved.current.field.random_first = 11;
        player7_loose_moved.current.field.random_second = 105;
        for (auto& actor : player7_loose_moved.current.field.motion) { actor.x = 48u << 8; actor.y = 80u << 8; }
        auto& player3 = player7_loose_moved.current.field.motion[field_motion_index(3)];
        player3.x = static_cast<std::uint16_t>((120u << 8) | 17u); player3.y = static_cast<std::uint16_t>((140u << 8) | 19u);
        const auto player7_recovered = recover_player7_shot_ball(player7_loose_moved);
        if (player7_recovered.phase != FieldSessionPhase::player7_shot_ball_recovered || player7_recovered.current.field.ball_owner != 3 ||
            player7_recovered.current.field.motion[field_motion_index(3)].x != player3.x ||
            player7_recovered.current.field.motion[field_motion_index(3)].y != player3.y || player7_loose_moved.current.field.ball_owner != 7) return 1;
        std::istringstream recovered_snapshot(bytes(player7_recovered));
        if (bytes(load_phase_session(recovered_snapshot)) != bytes(player7_recovered)) return 1;
        bool early_recovery_rejected = false;
        try { recover_player7_shot_ball(player7_loose); }
        catch (const std::invalid_argument&) { early_recovery_rejected = true; }
        if (!early_recovery_rejected) return 1;
        const auto player3_wait = advance_observed_nmi_events(player7_recovered, 0,
            {{ObservedClockEventKind::facing_sample, {}, 64}, {ObservedClockEventKind::motion_sample, {}, 1},
             {ObservedClockEventKind::motion_sample, {}, 0}});
        if (player3_wait.state.control->facing != 64 || player3_wait.state.control->motion_requested ||
            player7_recovered.control->facing) return 1;
        bool wrong_player3_sample_rejected = false;
        try { advance_observed_nmi_events(player7_recovered, 0, {{ObservedClockEventKind::motion_sample, {}, 1}}); }
        catch (const std::invalid_argument&) { wrong_player3_sample_rejected = true; }
        if (!wrong_player3_sample_rejected) return 1;
        auto player3_setup_input = player3_wait.state;
        player3_setup_input.current.field.random_first = 95; player3_setup_input.current.field.random_second = 28;
        const auto player3_ready = prepare_player3_recovered_control(player3_setup_input);
        if (player3_ready.state.phase != FieldSessionPhase::control_ready || player3_ready.state.current.field.ball_owner != 3 ||
            player3_ready.state.current.field.ball_cell != 115 || player3_ready.state.area_contact ||
            player3_ready.state.area_sequence_result != 255 || player3_setup_input.phase != FieldSessionPhase::player7_shot_ball_recovered) return 1;
        bool early_player3_setup_rejected = false;
        try { prepare_player3_recovered_control(player3_wait.state); }
        catch (const std::invalid_argument&) { early_player3_setup_rejected = true; }
        if (!early_player3_setup_rejected) return 1;
        bool marker_rejected = false;
        try { advance_observed_nmi_events(event_input, 0, {{ObservedClockEventKind::schedule}}); }
        catch (const std::invalid_argument&) { marker_rejected = true; }
        if (!marker_rejected) return 1;
        auto confirmed_pass = event_input; confirmed_pass.phase = FieldSessionPhase::attack_choice_pending;
        confirmed_pass.action->action = 1;
        const auto waiting_pass = advance_observed_nmi_events(confirmed_pass, 255,
            {{ObservedClockEventKind::random, {0, 0, 0, false}}});
        if (waiting_pass.random_index != 0 || waiting_pass.state.phase != FieldSessionPhase::attack_choice_pending) return 1;
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
        auto captured_clock = clock_input;
        captured_clock.score = MatchScoreContext{1, 0, 4};
        captured_clock.current.clock.remaining = 160;
        captured_clock.current.clock.ball_mode = 128;
        captured_clock.current.field.side = 11;
        captured_clock.current.field.ball_owner = 18;
        captured_clock.control->controlled_player = 1;
        const auto sampled_clock = advance_observed_nmi_control_clock(captured_clock, field_profiles, keeper_profiles,
            world, 7, {{ObservedClockEventKind::ball_mode_sample, {}, 192},
                       {ObservedClockEventKind::schedule}, {ObservedClockEventKind::energy}});
        if (sampled_clock.tick.state.current.clock.ball_mode != 192) return 1;
        auto second_clock = captured_clock;
        second_clock.current.clock.remaining = 159;
        second_clock.current.clock.ball_mode = 192;
        const auto second_clock_result = advance_observed_nmi_control_clock(second_clock, field_profiles, keeper_profiles,
            world, 0, {{ObservedClockEventKind::ball_mode_sample, {}, 128},
                       {ObservedClockEventKind::schedule}, {ObservedClockEventKind::energy}});
        if (second_clock_result.tick.state.current.clock.ball_mode != 128) return 1;
        auto return_clock=second_clock;return_clock.current.clock.remaining=155;
        const auto return_clock_result=advance_observed_nmi_control_clock(return_clock,field_profiles,keeper_profiles,
            world,0,{{ObservedClockEventKind::ball_mode_sample,{},128},{ObservedClockEventKind::schedule},{ObservedClockEventKind::energy}});
        if(return_clock_result.tick.state.current.clock.remaining!=154 || return_clock_result.tick.state.current.clock.ball_mode!=128) return 1;
        auto stamina_input=clock_input;stamina_input.score=MatchScoreContext{1,0,4};
        stamina_input.current.clock.remaining=147;stamina_input.current.field.side=0;stamina_input.current.field.ball_owner=7;
        stamina_input.current.field.ball_x=120;stamina_input.current.field.ball_y=124;
        stamina_input.current.field.roster[7].type=9;stamina_input.current.field.roster[7].stamina=388;
        stamina_input.current.field.random_first=158;stamina_input.current.field.random_second=188;
        std::istringstream stamina_stream("CMJCLOCKNMI2\nS\nE\nR 87 15 1 1\nH 7 129 1\n");
        const auto stamina_events=load_observed_clock_events(stamina_stream);const auto stamina_before=bytes(stamina_input);
        const auto stamina_result=advance_observed_nmi_control_clock(stamina_input,field_profiles,keeper_profiles,world,30,stamina_events);
        if(stamina_result.tick.state.current.field.roster[7].stamina!=385 || stamina_result.random_index!=31 || bytes(stamina_input)!=stamina_before) return 1;
        auto missing_stamina=stamina_events;missing_stamina.pop_back();bool missing_sample_rejected=false;
        try { advance_observed_nmi_control_clock(stamina_input,field_profiles,keeper_profiles,world,30,missing_stamina); }
        catch(const std::invalid_argument&) { missing_sample_rejected=true; }
        if(!missing_sample_rejected || bytes(stamina_input)!=stamina_before) return 1;
        auto wrong_stamina=stamina_events;wrong_stamina.back().speed=384;bool wrong_sample_rejected=false;
        try { advance_observed_nmi_control_clock(stamina_input,field_profiles,keeper_profiles,world,30,wrong_stamina); }
        catch(const std::invalid_argument&) { wrong_sample_rejected=true; }
        if(!wrong_sample_rejected || bytes(stamina_input)!=stamina_before) return 1;
        auto early_stamina=stamina_events;std::swap(early_stamina[2],early_stamina[3]);bool early_sample_rejected=false;
        try { advance_observed_nmi_control_clock(stamina_input,field_profiles,keeper_profiles,world,30,early_stamina); }
        catch(const std::invalid_argument&) { early_sample_rejected=true; }
        if(!early_sample_rejected || bytes(stamina_input)!=stamina_before) return 1;
        bool old_format_rejected=false;
        try { std::istringstream old("CMJCLOCKNMI1\nH 7 129 1\n");load_observed_clock_events(old); }
        catch(const std::invalid_argument&) { old_format_rejected=true; }
        if(!old_format_rejected) return 1;
        auto third_return_clock=captured_clock;third_return_clock.current.clock.remaining=154;
        third_return_clock.current.clock.ball_mode=128;
        const auto third_return_clock_result=advance_observed_nmi_control_clock(third_return_clock,field_profiles,keeper_profiles,
            world,155,{{ObservedClockEventKind::ball_mode_sample,{},192},{ObservedClockEventKind::schedule},{ObservedClockEventKind::energy}});
        if(third_return_clock_result.tick.state.current.clock.remaining!=153 || third_return_clock_result.tick.state.current.clock.ball_mode!=192) return 1;
        std::istringstream sampled_clock_stream("CMJCLOCKNMI1\nB 192\nS\nE\n");
        if (load_observed_clock_events(sampled_clock_stream)[0].kind != ObservedClockEventKind::ball_mode_sample) return 1;
        std::string sampled_world_text = "CMJWORLDNMI2\n";
        for (unsigned i = 0; i < 22; ++i)
            sampled_world_text += i == 0 ? "0 0 0 0 0 0 0 0 0 0 128\n" : "0 0 0 0 0 0 0 0 0 0 256\n";
        std::istringstream sampled_world_stream(sampled_world_text);
        const auto sampled_world_events = load_observed_nmi_field_polls(sampled_world_stream);
        if (!sampled_world_events[0].ball_mode_sample || *sampled_world_events[0].ball_mode_sample != 128 ||
            sampled_world_events[1].ball_mode_sample) return 1;
        const auto sampled_world = advance_observed_nmi_field_round(sampled_clock.tick.state, profiles, movement,
            control, world, 7, sampled_world_events);
        if (sampled_world.round.state.current.clock.ball_mode != 128) return 1;
        bool stray_sample_rejected = false;
        try { advance_observed_nmi_control_poll(sampled_world.round.state, profiles, movement, control, 7, sampled_world_events[0]); }
        catch (const std::invalid_argument&) { stray_sample_rejected = true; }
        if (!stray_sample_rejected) return 1;
        auto second_world_events = sampled_world_events;
        second_world_events[0].ball_mode_sample = 192;
        const auto second_world = advance_observed_nmi_field_round(second_clock_result.tick.state, profiles, movement,
            control, world, 7, second_world_events);
        if (second_world.round.state.current.clock.ball_mode != 192) return 1;
        auto return_world_input=second_clock_result.tick.state;
        return_world_input.current.clock.remaining=155;
        const auto return_world=advance_observed_nmi_field_round(return_world_input,profiles,movement,control,world,7,second_world_events);
        if (return_world.round.state.current.clock.ball_mode!=192) return 1;
        auto third_return_world=third_return_clock_result.tick.state;
        const auto third_world=advance_observed_nmi_field_round(third_return_world,profiles,movement,control,world,158,sampled_world_events);
        if (third_world.round.state.current.clock.ball_mode!=128) return 1;
        return_world_input.current.clock.remaining=154;
        const auto second_return_world=advance_observed_nmi_field_round(return_world_input,profiles,movement,control,world,7,second_world_events);
        if (second_return_world.round.state.current.clock.ball_mode!=192) return 1;
        auto invalid_world_events = sampled_world_events;
        invalid_world_events[0].ball_mode_sample = 192;
        bool duplicate_mode_rejected = false;
        try { advance_observed_nmi_field_round(sampled_clock.tick.state, profiles, movement, control, world, 7, invalid_world_events); }
        catch (const std::invalid_argument&) { duplicate_mode_rejected = true; }
        if (!duplicate_mode_rejected) return 1;
        bool invalid_mode_rejected = false;
        try { std::istringstream invalid("CMJWORLDNMI2\n0 0 0 0 0 0 0 0 0 0 129\n"); load_observed_nmi_field_polls(invalid); }
        catch (const std::invalid_argument&) { invalid_mode_rejected = true; }
        if (!invalid_mode_rejected) return 1;
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

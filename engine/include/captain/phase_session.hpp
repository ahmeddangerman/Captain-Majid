#pragma once
#include "captain/contact_prelude.hpp"
#include "captain/terminal_flight_phase.hpp"
#include "captain/experience.hpp"
#include <vector>

namespace captain {
enum class FieldSessionPhase : std::uint16_t {
    field = 0, player_control = 1, ball_recovery = 2, added_time = 3, period_end = 4,
    boundary_restart = 5, recovered_owner = 6, keeper_interaction = 7, story_transition = 8,
    control_ready = 9, control_clock_pending = 10, action_menu_pending = 11, ai_cell_decision = 12,
    field_round_pending = 13, control_added_time = 14, field_contact_pending = 15,
    goal_area_action = 16, special_callback = 17, human_action_menu = 18,
    attack_choice_pending = 19, attack_prepared = 20,
    shot_field_complete = 21, shot_restart_pending = 22,
    shot_defender_next = 23, shot_contact_added_time = 24, shot_dispatch_clock = 25,
    shot_dispatch_added_time = 26, shot_dispatch_ready = 27,
    selected_ball_recovery = 28, selected_defense_animation = 29, selected_restart_entry = 30,
    selected_recovered_owner = 31, selected_boundary_restart = 32, capture_clearance_flight = 33, clearance_race_winner = 34, clearance_race_timeout = 35, clearance_recovery_ready = 36, clearance_recovered_owner = 37, selected_field_restart = 38, selected_goal_restart = 39, restart_options = 40, restart_tactics = 41, restart_options_done = 42, selected_attack_presentation = 43, attack_clock_added_time = 44, shot_approach_ready = 45, shot_flight_pending = 46, ball_advance_ready = 47, ball_cell_callback = 48, ball_arrival_callback = 49, ball_already_arrived = 50, shot_loose_recovery = 51, shot_keeper_entry = 52, keeper_presentation_pending = 53, stunned_keeper_pending = 54, stunned_keeper_decided = 55, keeper_decided = 56, keeper_possession = 57, keeper_punch_recovery = 58, goal_celebration_pending = 59, goal_kickoff_pending = 60, kickoff_formation_placed = 61, kickoff_receiver_selected = 62, kickoff_ball_placed = 63, kickoff_control_pending = 64, keeper_area_prepared = 65, keeper_area_defense_pending = 66, keeper_area_defense_initial = 67, keeper_area_choices_done = 68, keeper_area_defense_ready = 69, keeper_area_defense_poll = 70, keeper_area_defense_preparation = 71, keeper_area_defense_complete = 72, keeper_area_special_ready = 73, keeper_area_special_poll = 74, keeper_area_special_decided = 75, keeper_area_special_inspected = 76, keeper_area_special_redrawn = 77, keeper_area_ai_chosen = 78, keeper_area_attack_prepared = 79, keeper_area_defense_entry = 80, keeper_area_defenders_announced = 81, keeper_area_defender_compared = 82, keeper_area_contact_clock_done = 83, keeper_area_contact_added_time = 84, keeper_area_contact_period_end = 85, keeper_area_contact_presented = 86, keeper_area_defender_next = 87, keeper_area_prefix_done = 88, keeper_area_terminal_result = 89, keeper_area_prefix_clock_done = 90, keeper_area_prefix_added_time = 91, keeper_area_prefix_period_end = 92
    , keeper_area_attack_continuation = 93, keeper_area_recovery_ready = 94,
    keeper_area_recovered_owner = 95, area_ball_advance_ready = 96, area_ball_already_arrived = 97,
    area_ball_cell_callback = 98, area_ball_arrival_callback = 99, area_pass_received = 100,
    area_pass_keeper_entry = 101, area_action4_encounters_pending = 102, area_action4_control_pending = 103,
    area_action6_destination_pending = 104, area_pass_keeper_prepared = 105,
    area_action6_race_winner = 106, area_action6_race_timeout = 107,
    area_action6_control_pending = 108, area_action6_recovered_owner = 109,
    area_action6_recovered_control = 110, sampled_player3_pass_handoff = 111, sampled_player3_empty_pass_destination = 112, area_action6_control_ready = 113,
    area_pass_defenders_pending = 114, area_pass_defender_announced = 115,
    area_pass_defender_prepared = 116, area_pass_defender_compared = 117,
    area_pass_variant_restored = 118, area_pass_result_handler_entry = 119,
    area_pass_nonterminal_done = 120, area_pass_nonterminal_added_time = 121,
    area_pass_nonterminal_period_end = 122, area_pass_defenders_done = 123,
    area_pass_blocked_positioned = 124, area_pass_terminal_clock_done = 125,
    area_pass_terminal_added_time = 126, area_pass_terminal_period_end = 127,
    area_pass_defender_captured = 128, area_pass_capture_control_entry = 129,
    area_pass_capture_control_ready = 130, area_pass_capture_control_wait = 131,
    area_pass_capture_action_menu = 132, area_pass_capture_clock_pending = 133,
    area_pass_blocked_recovery_entry = 134, area_pass_blocked_recovered_owner = 135,
    area_pass_blocked_boundary = 136, area_pass_blocked_control_entry = 137,
    area_pass_blocked_control_ready = 138, area_pass_blocked_control_wait = 139,
    area_pass_blocked_action_menu = 140, area_pass_blocked_clock_pending = 141,
    area_pass_capture_field_round = 142, area_pass_capture_goal_area = 143,
    area_pass_capture_added_time = 144, area_pass_capture_period_end = 145,
    area_pass_blocked_field_round = 146, area_pass_blocked_goal_area = 147,
    area_pass_blocked_added_time = 148, area_pass_blocked_period_end = 149,
    shot_pass_defenders_pending = 150, shot_pass_defender_announced = 151,
    shot_pass_defender_prepared = 152, shot_pass_defender_compared = 153,
    shot_pass_variant_restored = 154, shot_pass_result_handler_entry = 155,
    shot_pass_nonterminal_done = 156, shot_pass_defenders_done = 157,
    shot_pass_added_time = 158, shot_pass_period_end = 159,
    area_action4_field_entry = 160, area_action4_control_entry = 161,
    area_action4_control_ready = 162, area_action4_control_wait = 163,
    area_action4_control_clock = 164, area_action4_field_round = 165,
    area_action4_goal_area = 166, area_action4_added_time = 167, area_action4_period_end = 168,
    area_action4_keeper_entry = 169, area_action4_keeper_presented = 170,
    area_action4_keeper_prepared = 171, area_action4_keeper_stamina_done = 172, area_action4_keeper_compared = 173, area_action4_keeper_dispatch_entry = 174, area_action4_keeper_result0_entry = 175, area_action4_keeper_result1_recovery = 176, area_action4_keeper_result1_boundary_check = 177, area_action4_keeper_result1_restart_entry = 178, area_action4_keeper_restart_prepared = 179, area_action4_keeper_restart_formation = 180, area_action4_keeper_restart_power = 181, area_action4_restart_clock_done = 182, area_action4_restart_added_time = 183, area_action4_restart_period_end = 184, area_action4_restart_action1_entry = 185, area_action4_restart_pass_entry = 186, area_action4_keeper_result0_goal = 187, sampled_loose_shot_defender_prepared = 188, sampled_loose_shot_defender_compared = 189, sampled_keeper_result4_restart = 190, sampled_keeper_result4_formation = 191, sampled_keeper_result4_action = 192, sampled_keeper_result4_extra_time = 193, sampled_field_ai_choice = 194, sampled_field_ai_prepared = 195, sampled_field_ai_presented = 196, sampled_field_ai_dispatch = 197, sampled_field_ai_pass_presented = 198, sampled_field_ai_pass_handoff = 199, sampled_field_ai_pass_target = 200, sampled_opponent_contact_presented = 201, sampled_field_defense_prompt = 202, sampled_field_defense_poll = 203, sampled_field_defense_preparation = 204, sampled_field_defense_complete = 205, sampled_field_contact_ai_choice = 206, sampled_field_contact_ai_prepared = 207, sampled_field_contact_presentation = 208, sampled_opponent_shot_choice = 209, sampled_opponent_shot_prepared = 210, sampled_opponent_shot_presented = 211, sampled_opponent_shot_clock = 212, sampled_opponent_shot_approach = 213, sampled_opponent_shot_handoff = 214, sampled_mixed_defense_ai_choice = 215, sampled_mixed_defense_ai_prepared = 216, sampled_mixed_defender_prepared = 217, sampled_mixed_defender_compared = 218, sampled_shot_keeper_prompt = 219, sampled_shot_keeper_selected = 220, sampled_shot_keeper_confirmed = 221, sampled_shot_keeper_precompare = 222, sampled_shot_keeper_compared = 223, sampled_opponent_goal_scored = 224, sampled_opponent_goal_presented = 225, sampled_opponent_goal_restart = 226, sampled_mixed_defender_result4 = 227, sampled_mixed_result4_pass = 228, sampled_mixed_result4_target = 229, sampled_player12_ai_choice = 230, sampled_player12_ai_prepared = 231, sampled_player12_ai_presented = 232, sampled_player12_ai_dispatch = 233, sampled_player12_pass_handoff = 234, sampled_player12_pass_target = 235, sampled_player19_shot_choice = 236, sampled_player19_shot_prepared = 237, sampled_home_shot_defender_choice = 238, sampled_home_shot_defender_resolved = 239, sampled_home_shot_contact_clock = 240, sampled_home_shot_dispatch_clock = 241, sampled_home_shot_handoff = 242, sampled_home_shot_target = 243, sampled_human_pass_target = 244, sampled_human_pass_empty_confirmed = 245, sampled_human_pass_cancel_pending = 246, sampled_human_pass_prepared = 247, sampled_human_pass_handoff = 248, sampled_player3_pass_target = 249, sampled_player3_empty_confirmed = 250, sampled_player3_pass_prepared = 251, sampled_player3_defender_prepared = 252, sampled_player3_defender_compared = 253, sampled_player3_contact_clock = 254, sampled_player3_dispatch_clock = 255
    , sampled_player14_ai_choice = 256, sampled_player14_ai_prepared = 257,
    sampled_player14_ai_presented = 258, sampled_player14_ai_dispatch = 259, sampled_player14_pass_handoff = 260,
    sampled_player14_pass_target = 261, sampled_player19_return_shot_choice = 262,
    sampled_player19_return_shot_prepared = 263, sampled_player19_return_shot_presented = 264,
    sampled_player19_return_shot_clock = 265, sampled_player19_return_shot_approach = 266,
    sampled_player19_return_shot_handoff = 267, sampled_player19_return_keeper_prompt = 268,
    sampled_player19_return_keeper_selected = 269, sampled_player19_return_keeper_confirmed = 270,
    sampled_player19_return_keeper_precompare = 271, sampled_player19_return_keeper_compared = 272,
    sampled_player19_return_goal_scored = 273, sampled_player19_return_goal_presented = 274,
    sampled_player19_return_goal_restart = 275, sampled_score23_pass_target = 276,
    sampled_score23_empty_confirmed = 277, sampled_score23_pass_cancel = 278,
    sampled_score23_pass_prepared = 279, sampled_score23_defender_prepared = 280,
    sampled_score23_defender_compared = 281, sampled_score23_contact_clock = 282,
    sampled_score23_dispatch_clock = 283, sampled_score23_pass_handoff = 284,
    sampled_score23_shot_choice = 285, sampled_score23_shot_prepared = 286,
    sampled_score23_shot_presented = 287, sampled_score23_shot_clock = 288,
    sampled_score23_shot_approach = 289, sampled_score23_shot_handoff = 290,
    sampled_score23_shot_target = 291, sampled_score23_ball_advance_ready = 292,
    sampled_score23_ball_cell_callback = 293, sampled_score23_ball_arrival_callback = 294,
    sampled_score23_keeper_prompt = 295, sampled_score23_keeper_selected = 296,
    sampled_score23_keeper_confirmed = 297, sampled_score23_keeper_precompare = 298,
    sampled_score23_keeper_compared = 299, sampled_score24_goal_scored = 300,
    sampled_score24_goal_presented = 301, sampled_score24_goal_restart = 302,
    sampled_score24_dribble_choice = 303, sampled_score24_dribble_prepared = 304,
    sampled_score24_dribble_defender = 305, sampled_score24_dribble_compared = 306,
    sampled_score24_dribble_clock = 307, sampled_score24_dribble_field = 308,
    sampled_score24_dribble_possession = 309, sampled_score25_pass_target = 310,
    sampled_score25_player7_confirmed = 311, sampled_score25_pass_cancel = 312,
    sampled_score25_pass_defenses = 313, sampled_score25_pass_prepared = 314,
    sampled_score25_first_defender_prepared = 315, sampled_score25_first_defender_compared = 316,
    sampled_score25_first_contact_clock = 317
};
struct PendingTerminalContact {
    std::uint8_t result_code, defender_id, defense_action, defense_subtype;
};
struct RecoveryContext {
    std::uint8_t opposing_player = 0, attack_subtype = 0, special_owner_flags = 0;
    std::uint8_t ball_variant = 0, game_mode = 0, x_jitter_control = 0;
};
struct ControlContext {
    std::uint8_t decision_timer = 0, facing = 0, controlled_player = 0, last_carrier_cell = 0;
    std::uint16_t travel_accumulator = 0;
    std::uint8_t display_parameter = 0;
    std::uint8_t motion_requested = 0;
};
struct SchedulingContext {
    std::uint8_t field_cycle = 0, energy_cycle = 0, special_flags = 0;
    std::array<std::uint8_t, 5> selected_ids{};
};
struct ActionContext {
    std::uint8_t action = 0, menu_route = 0, cursor = 0, flash_counter = 0, auxiliary = 0;
    std::uint8_t move_effect = 0, move_flags = 0;
    std::uint16_t power = 0;
};
struct ShotContactProgress {
    std::uint8_t cursor = 0, result_code = 0, defender_id = 0, defense_action = 0, defense_subtype = 0;
};
struct ClearanceFlightContext { std::uint8_t distance_adjustment = 0; };
struct ClearanceRaceContext { std::uint8_t remaining = 0, active_candidate = 0, opposing_candidate = 0; };
struct RestartOptionsContext { bool goal_area = false; std::uint8_t cursor = 4; };
struct ShotFlightContext { std::uint8_t distance = 0, destination = 0, counter = 255; bool loose_ball = false; };
struct BallTrajectoryContext { std::uint16_t x=0,y=0,vx=0,vy=0; std::uint8_t direction=0,flags=0; };
struct KeeperDecisionContext { std::uint8_t cursor=0, result_code=0, defense_action=0, defense_subtype=0, effect=0; };
struct MatchScoreContext { std::uint8_t left_goals=0,right_goals=0,kickoff=0; };
struct KickoffFormationContext { std::array<std::uint8_t,2> keeper_y{}; };
struct KeeperAreaContext { std::uint8_t selection_counter=255; };
struct KeeperAreaDefenseContext {
    std::array<std::uint8_t,4> actions{255,255,255,255},subtypes{};
    std::uint8_t blink=0,pressed=0;
};
struct AreaContactContext {
    std::uint8_t cursor=0,defense_action=0,defense_subtype=0,effect=0;
    std::uint16_t defender_power=0,cost=0;
    std::uint8_t strength=0,loss=0,score=0,result=0;
    bool weakened=false;
};
struct AreaAction4RestartContext { std::uint8_t action=0,subtype=0,choice=0; };
struct AreaAction4RestartPowerContext { std::uint16_t cost=0,power=0; };
struct SampledFieldPassContext { std::uint8_t recipient=0,destination=0,counter=0,distance=0; };
struct HumanDefenseChoices {
    std::array<std::uint8_t,5> actions{},subtypes{};
};
struct AreaPassContactContext { std::uint8_t saved_ball_variant=0; std::uint16_t captured_cost=0; };
struct PhaseSessionState {
    ContactPreludeState current;
    FieldSessionPhase phase = FieldSessionPhase::field;
    std::optional<PendingTerminalContact> pending{};
    std::optional<ExperienceState> experience{};
    std::optional<RecoveryContext> recovery{};
    std::optional<ControlContext> control{};
    std::optional<SchedulingContext> scheduling{};
    std::optional<ActionContext> action{};
    std::optional<ShotContactProgress> shot_progress{};
    std::optional<ClearanceFlightContext> clearance_flight{};
    std::optional<ClearanceRaceContext> clearance_race{};
    std::optional<RestartOptionsContext> restart_options{};
    std::optional<ShotFlightContext> shot_flight{};
    std::optional<SampledFieldPassContext> sampled_field_pass{};
    std::optional<HumanDefenseChoices> human_defense_choices{};
    std::optional<std::uint8_t> human_pass_cursor{};
    std::uint8_t human_pass_steps=0;
    std::optional<BallTrajectoryContext> ball_trajectory{};
    std::optional<KeeperDecisionContext> keeper_decision{};
    std::optional<std::uint8_t> keeper_menu_action{};
    std::optional<MatchScoreContext> score{};
    std::optional<KickoffFormationContext> kickoff_formation{};
    std::optional<std::uint8_t> kickoff_receiver{};
    std::optional<KeeperAreaContext> keeper_area{};
    std::optional<KeeperAreaDefenseContext> area_defense{};
    std::optional<std::uint8_t> area_special_option{};
    std::uint8_t area_special_cursor=0,area_special_pressed=0;
    struct SpecialDecision { std::uint8_t status=0, subtype=0; std::uint16_t cost=0; std::uint8_t effect=0; };
    std::optional<SpecialDecision> area_special_decision{};
    struct AreaAiTarget { std::uint8_t recipient=0,cell=0; std::uint16_t cost=0; };
    std::optional<AreaAiTarget> area_ai_target{};
    std::optional<AreaContactContext> area_contact{};
    std::optional<AreaAction4RestartContext> area_action4_restart{};
    std::optional<AreaAction4RestartPowerContext> area_action4_restart_power{};
    std::optional<std::uint8_t> area_action4_restart_pass_counter{};
    std::optional<AreaPassContactContext> area_pass_contact{};
    std::uint8_t area_defender_cursor=0;
    std::uint8_t area_sequence_result=255;
    std::uint8_t saved_ball_flight_flag=0;
};
struct PhaseSessionResult {
    PhaseSessionState state;
    std::optional<ContactPreludeResult> prelude{};
    std::optional<TerminalFlightPhaseResult> terminal{};
    std::vector<ExperienceRewardResult> rewards{};
};
void validate_phase_session(const PhaseSessionState& state);
// Input is after rewards/prior presentation, before original9095 status/time.
// Numeric clock pauses are retained; no presentation/NMI acknowledgement is implicit.
PhaseSessionResult resolve_session_terminal(const PhaseSessionState& input,
    PendingTerminalContact contact, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ContactPreludeContent& prelude);
// Caller explicitly supplies/accepts state after the added-time presentation
// and any original wait. The development lab keeps otherwise frozen inputs.
PhaseSessionResult resume_session_terminal(const PhaseSessionState& post_presentation,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement);
// Entry before the terminal reward caller. Result2 invokes that caller twice;
// result3 once. Resume never repeats these rewards.
PhaseSessionResult resolve_session_rewarded_terminal(const PhaseSessionState& input,
    PendingTerminalContact contact, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ContactPreludeContent& prelude,
    const ExperienceContent& experience);
}

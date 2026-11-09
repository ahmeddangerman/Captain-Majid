#include "captain/phase_session_io.hpp"
#include "captain/field_state_io.hpp"
#include "captain/shot_content.hpp"
#include "captain/area_ai_session.hpp"
#include "captain/recovery_session.hpp"
#include "captain/control_session.hpp"
#include "captain/control_travel.hpp"
#include "captain/session_shot.hpp"
#include "captain/staged_shot.hpp"
#include "captain/selected_dispatch.hpp"
#include "captain/capture_clearance.hpp"
#include "captain/clearance_race.hpp"
#include "captain/clearance_handoff.hpp"
#include "captain/restart_route.hpp"
#include "captain/restart_options.hpp"
#include "captain/attack_clock.hpp"
#include "captain/approach_session.hpp"
#include "captain/ball_flight_session.hpp"
#include "captain/pass_distance.hpp"
#include "captain/pass_target.hpp"
#include "captain/keeper_session.hpp"
#include "captain/keeper_save_session.hpp"
#include "captain/goal_session.hpp"
#include "captain/kickoff_session.hpp"
#include <iomanip>
#include <charconv>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
unsigned number(const std::string& token) {
    unsigned value = 0;
    const auto parsed = std::from_chars(token.data(), token.data() + token.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != token.data() + token.size() || value > 255)
        throw std::invalid_argument("Expected a byte from0 through255");
    return value;
}
const char* phase_name(captain::FieldSessionPhase phase) {
    switch (phase) {
    case captain::FieldSessionPhase::field: return "field";
    case captain::FieldSessionPhase::player_control: return "player control";
    case captain::FieldSessionPhase::ball_recovery: return "ball recovery";
    case captain::FieldSessionPhase::added_time: return "added-time presentation pending";
    case captain::FieldSessionPhase::period_end: return "period transition pending";
    case captain::FieldSessionPhase::boundary_restart: return "boundary restart pending";
    case captain::FieldSessionPhase::recovered_owner: return "recovered owner; presentation tail pending";
    case captain::FieldSessionPhase::keeper_interaction: return "keeper-area interaction pending";
    case captain::FieldSessionPhase::story_transition: return "story transition pending";
    case captain::FieldSessionPhase::control_ready: return "player control ready; input loop pending";
    case captain::FieldSessionPhase::control_clock_pending: return "control clock step pending";
    case captain::FieldSessionPhase::action_menu_pending: return "action menu pending";
    case captain::FieldSessionPhase::ai_cell_decision: return "opposing-carrier cell decision pending";
    case captain::FieldSessionPhase::sampled_player14_ai_choice: return "sampled player14 pass choice captured";
    case captain::FieldSessionPhase::sampled_player14_ai_prepared: return "sampled player14 pass prepared";
    case captain::FieldSessionPhase::sampled_player14_ai_presented: return "sampled player14 pass presented";
    case captain::FieldSessionPhase::sampled_player14_ai_dispatch: return "sampled player14 pass dispatch pending";
    case captain::FieldSessionPhase::sampled_player14_pass_handoff: return "sampled player14 pass handoff";
    case captain::FieldSessionPhase::sampled_player14_pass_target: return "sampled player14 pass target captured";
    case captain::FieldSessionPhase::sampled_player19_return_shot_choice: return "sampled player19 return shot choice captured";
    case captain::FieldSessionPhase::sampled_player19_return_shot_prepared: return "sampled player19 return shot prepared";
    case captain::FieldSessionPhase::sampled_player19_return_shot_presented: return "sampled player19 return shot presented";
    case captain::FieldSessionPhase::sampled_player19_return_shot_clock: return "sampled player19 return shot clock";
    case captain::FieldSessionPhase::sampled_player19_return_shot_approach: return "sampled player19 return shot approach";
    case captain::FieldSessionPhase::sampled_player19_return_shot_handoff: return "sampled player19 return shot handoff";
    case captain::FieldSessionPhase::sampled_player19_return_keeper_prompt: return "sampled player19 return goalkeeper prompt";
    case captain::FieldSessionPhase::sampled_player19_return_keeper_selected: return "sampled player19 return goalkeeper selected";
    case captain::FieldSessionPhase::sampled_player19_return_keeper_confirmed: return "sampled player19 return goalkeeper confirmed";
    case captain::FieldSessionPhase::sampled_player19_return_keeper_precompare: return "sampled player19 return goalkeeper comparison pending";
    case captain::FieldSessionPhase::sampled_player19_return_keeper_compared: return "sampled player19 return goalkeeper compared";
    case captain::FieldSessionPhase::sampled_player19_return_goal_scored: return "sampled player19 return goal scored";
    case captain::FieldSessionPhase::sampled_player19_return_goal_presented: return "sampled player19 return goal presented";
    case captain::FieldSessionPhase::sampled_player19_return_goal_restart: return "sampled player19 return goal restart";
    case captain::FieldSessionPhase::sampled_score23_pass_target: return "2-3 human pass target cursor";
    case captain::FieldSessionPhase::sampled_score23_empty_confirmed: return "2-3 empty pass confirmed";
    case captain::FieldSessionPhase::sampled_score23_pass_cancel: return "2-3 pass cancellation pending";
    case captain::FieldSessionPhase::sampled_score23_pass_prepared: return "2-3 weakened pass prepared";
    case captain::FieldSessionPhase::sampled_field_ai_choice: return "sampled opponent AI choice captured; preparation pending";
    case captain::FieldSessionPhase::sampled_field_ai_prepared: return "sampled opponent attack prepared; presentation pending";
    case captain::FieldSessionPhase::sampled_field_ai_presented: return "sampled opponent presentation acknowledged; field attack pending";
    case captain::FieldSessionPhase::sampled_field_ai_dispatch: return "sampled opponent pass dispatch pending";
    case captain::FieldSessionPhase::sampled_field_ai_pass_presented: return "sampled opponent pass presentation captured; pass handler pending";
    case captain::FieldSessionPhase::sampled_field_ai_pass_handoff: return "sampled opponent pass handoff; flight pending";
    case captain::FieldSessionPhase::sampled_field_ai_pass_target: return "sampled opponent pass target captured; flight setup pending";
    case captain::FieldSessionPhase::sampled_opponent_contact_presented: return "sampled opponent contact presented; action selection pending";
    case captain::FieldSessionPhase::sampled_field_defense_prompt: return "sampled field defense prompt; input pending";
    case captain::FieldSessionPhase::sampled_field_defense_poll: return "sampled field defense choices pending";
    case captain::FieldSessionPhase::sampled_field_defense_preparation: return "sampled field defender choice confirmed";
    case captain::FieldSessionPhase::sampled_field_defense_complete: return "sampled field defensive choices complete";
    case captain::FieldSessionPhase::sampled_field_contact_ai_choice: return "sampled field contact opponent choice captured";
    case captain::FieldSessionPhase::sampled_field_contact_ai_prepared: return "sampled field contact opponent preparation captured";
    case captain::FieldSessionPhase::sampled_field_contact_presentation: return "sampled contact presentation captured; control setup pending";
    case captain::FieldSessionPhase::sampled_opponent_shot_choice: return "sampled opponent shot choice captured";
    case captain::FieldSessionPhase::sampled_opponent_shot_prepared: return "sampled opponent shot prepared";
    case captain::FieldSessionPhase::sampled_opponent_shot_presented: return "sampled opponent shot presentation acknowledged";
    case captain::FieldSessionPhase::sampled_opponent_shot_clock: return "sampled opponent shot clock advanced";
    case captain::FieldSessionPhase::sampled_opponent_shot_approach: return "sampled opponent shot approach captured";
    case captain::FieldSessionPhase::sampled_opponent_shot_handoff: return "sampled opponent shot handoff";
    case captain::FieldSessionPhase::sampled_mixed_defense_ai_choice: return "sampled mixed defense opponent choice";
    case captain::FieldSessionPhase::sampled_mixed_defense_ai_prepared: return "sampled mixed defense opponent prepared";
    case captain::FieldSessionPhase::sampled_mixed_defender_prepared: return "sampled mixed defender prepared";
    case captain::FieldSessionPhase::sampled_mixed_defender_compared: return "sampled mixed defender compared";
    case captain::FieldSessionPhase::sampled_mixed_defender_result4: return "sampled mixed defender result4";
    case captain::FieldSessionPhase::sampled_mixed_result4_pass: return "sampled mixed result4 pass";
    case captain::FieldSessionPhase::sampled_mixed_result4_target: return "sampled mixed result4 target";
    case captain::FieldSessionPhase::sampled_player12_ai_choice: return "sampled player12 pass choice";
    case captain::FieldSessionPhase::sampled_player12_ai_prepared: return "sampled player12 pass prepared";
    case captain::FieldSessionPhase::sampled_player12_ai_presented: return "sampled player12 pass presented";
    case captain::FieldSessionPhase::sampled_player12_ai_dispatch: return "sampled player12 pass dispatch";
    case captain::FieldSessionPhase::sampled_player12_pass_handoff: return "sampled player12 pass handoff";
    case captain::FieldSessionPhase::sampled_player12_pass_target: return "sampled player12 pass target";
    case captain::FieldSessionPhase::sampled_player19_shot_choice: return "sampled player19 shot choice";
    case captain::FieldSessionPhase::sampled_player19_shot_prepared: return "sampled player19 shot prepared";
    case captain::FieldSessionPhase::sampled_home_shot_defender_choice: return "sampled home shot defender choice";
    case captain::FieldSessionPhase::sampled_home_shot_defender_resolved: return "sampled home shot defender resolved";
    case captain::FieldSessionPhase::sampled_home_shot_contact_clock: return "sampled home shot contact clock";
    case captain::FieldSessionPhase::sampled_home_shot_dispatch_clock: return "sampled home shot dispatch clock";
    case captain::FieldSessionPhase::sampled_home_shot_handoff: return "sampled home shot handoff";
    case captain::FieldSessionPhase::sampled_home_shot_target: return "sampled home shot target";
    case captain::FieldSessionPhase::sampled_shot_keeper_prompt: return "sampled home goalkeeper choice pending";
    case captain::FieldSessionPhase::sampled_shot_keeper_selected: return "sampled home goalkeeper choice selected";
    case captain::FieldSessionPhase::sampled_shot_keeper_confirmed: return "sampled home goalkeeper choice confirmed";
    case captain::FieldSessionPhase::sampled_shot_keeper_precompare: return "sampled home goalkeeper comparison pending";
    case captain::FieldSessionPhase::sampled_shot_keeper_compared: return "sampled home goalkeeper compared";
    case captain::FieldSessionPhase::sampled_opponent_goal_scored: return "sampled opponent goal registered";
    case captain::FieldSessionPhase::sampled_opponent_goal_presented: return "sampled opponent goal presentation acknowledged";
    case captain::FieldSessionPhase::sampled_opponent_goal_restart: return "sampled opponent goal restart pending";
    case captain::FieldSessionPhase::field_round_pending: return "field round pending";
    case captain::FieldSessionPhase::control_added_time: return "control added-time presentation pending";
    case captain::FieldSessionPhase::field_contact_pending: return "nearby field contact pending";
    case captain::FieldSessionPhase::goal_area_action: return "goal-area action pending";
    case captain::FieldSessionPhase::special_callback: return "special callback pending";
    case captain::FieldSessionPhase::human_action_menu: return "human action menu";
    case captain::FieldSessionPhase::sampled_human_pass_target: return "sampled human pass target cursor";
    case captain::FieldSessionPhase::ordinary_human_pass_target: return "ordinary human pass target initialized";
    case captain::FieldSessionPhase::ordinary_human_pass_confirmed: return "ordinary human pass target confirmed";
    case captain::FieldSessionPhase::ordinary_pass_defense_selection: return "ordinary pass defender choices pending";
    case captain::FieldSessionPhase::ordinary_pass_defenses_recorded: return "ordinary pass defender choices recorded";
    case captain::FieldSessionPhase::ordinary_pass_prepared: return "ordinary pass attacker prepared";
    case captain::FieldSessionPhase::ordinary_pass_defender_prepared: return "ordinary pass first defender prepared";
    case captain::FieldSessionPhase::ordinary_pass_defender_compared: return "ordinary pass first defender compared";
    case captain::FieldSessionPhase::ordinary_pass_first_contact_clock: return "ordinary pass first contact clock";
    case captain::FieldSessionPhase::ordinary_pass_next_defender_prepared: return "ordinary pass next defender prepared";
    case captain::FieldSessionPhase::ordinary_pass_next_defender_compared: return "ordinary pass next defender compared";
    case captain::FieldSessionPhase::ordinary_pass_next_contact_clock: return "ordinary pass next contact clock";
    case captain::FieldSessionPhase::ordinary_pass_dispatch_clock: return "ordinary pass dispatch clock";
    case captain::FieldSessionPhase::ordinary_pass_field_moved: return "ordinary pass field moved";
    case captain::FieldSessionPhase::ordinary_pass_flight_target: return "ordinary pass flight target";
    case captain::FieldSessionPhase::ordinary_pass_flight_ready: return "ordinary pass flight ready";
    case captain::FieldSessionPhase::ordinary_pass_flight_callback: return "ordinary pass flight callback";
    case captain::FieldSessionPhase::ordinary_pass_flight_arrival: return "ordinary pass flight arrival";
    case captain::FieldSessionPhase::ordinary_pass_received: return "ordinary pass received";
    case captain::FieldSessionPhase::ordinary_pass_recipient_positioned: return "ordinary pass recipient positioned";
    case captain::FieldSessionPhase::ordinary_pass_control_entry: return "ordinary pass control entry";
    case captain::FieldSessionPhase::ordinary_free_pass_contest_winner: return "ordinary free pass contest winner";
    case captain::FieldSessionPhase::ordinary_return_clock_pending: return "ordinary returned control clock pending";
    case captain::FieldSessionPhase::ordinary_return_field_round: return "ordinary returned control field round";
    case captain::FieldSessionPhase::ordinary_return_contact_pending: return "ordinary returned control encounter";
    case captain::FieldSessionPhase::ordinary_return_action_menu: return "ordinary returned action menu";
    case captain::FieldSessionPhase::ordinary_return_shot_choice: return "ordinary returned shot choice";
    case captain::FieldSessionPhase::ordinary_return_shot_defense: return "ordinary returned shot defense recorded";
    case captain::FieldSessionPhase::ordinary_return_shot_prepared: return "ordinary returned shot prepared";
    case captain::FieldSessionPhase::ordinary_return_shot_defender_prepared: return "ordinary returned shot defender prepared";
    case captain::FieldSessionPhase::ordinary_return_shot_defender_compared: return "ordinary returned shot defender compared";
    case captain::FieldSessionPhase::ordinary_return_shot_contact: return "ordinary returned shot contact clock";
    case captain::FieldSessionPhase::ordinary_return_shot_dispatch: return "ordinary returned shot dispatch clock";
    case captain::FieldSessionPhase::ordinary_return_shot_field_moved: return "ordinary returned shot field movement";
    case captain::FieldSessionPhase::ordinary_return_shot_target: return "ordinary returned shot target";
    case captain::FieldSessionPhase::ordinary_return_shot_flight_ready: return "ordinary returned shot flight ready";
    case captain::FieldSessionPhase::ordinary_return_shot_flight_callback: return "ordinary returned shot flight callback";
    case captain::FieldSessionPhase::ordinary_return_shot_flight_arrival: return "ordinary returned shot flight arrival";
    case captain::FieldSessionPhase::ordinary_return_shot_defenders_pending: return "ordinary returned shot defenders pending";
    case captain::FieldSessionPhase::ordinary_return_shot_flight_defender_prepared: return "ordinary returned shot flight defender prepared";
    case captain::FieldSessionPhase::ordinary_return_shot_flight_defender_compared: return "ordinary returned shot flight defender compared";
    case captain::FieldSessionPhase::ordinary_return_shot_blocked_positioned: return "ordinary returned shot blocked positioned";
    case captain::FieldSessionPhase::ordinary_return_shot_blocked_clocked: return "ordinary returned shot blocked clocked";
    case captain::FieldSessionPhase::ordinary_return_shot_blocked_recovery: return "ordinary returned shot blocked recovery";
    case captain::FieldSessionPhase::sampled_human_pass_empty_confirmed: return "sampled empty pass target confirmed";
    case captain::FieldSessionPhase::sampled_human_pass_cancel_pending: return "sampled human pass cancel pending";
    case captain::FieldSessionPhase::sampled_human_pass_prepared: return "sampled human pass prepared";
    case captain::FieldSessionPhase::sampled_human_pass_handoff: return "sampled human pass handoff";
    case captain::FieldSessionPhase::sampled_player3_pass_target: return "sampled player3 pass target";
    case captain::FieldSessionPhase::sampled_player3_empty_confirmed: return "sampled player3 empty target confirmed";
    case captain::FieldSessionPhase::sampled_player3_pass_prepared: return "sampled player3 pass prepared";
    case captain::FieldSessionPhase::sampled_player3_defender_prepared: return "sampled player3 defender prepared";
    case captain::FieldSessionPhase::sampled_player3_defender_compared: return "sampled player3 defender compared";
    case captain::FieldSessionPhase::sampled_score23_defender_prepared: return "sampled 2-3 pass defender prepared";
    case captain::FieldSessionPhase::sampled_score23_defender_compared: return "sampled 2-3 pass defender compared";
    case captain::FieldSessionPhase::sampled_score23_contact_clock: return "sampled 2-3 pass contact clock advanced";
    case captain::FieldSessionPhase::sampled_score23_dispatch_clock: return "sampled 2-3 pass dispatch clock advanced";
    case captain::FieldSessionPhase::sampled_score23_pass_handoff: return "sampled 2-3 pass field movement advanced";
    case captain::FieldSessionPhase::sampled_score23_shot_choice: return "sampled opponent shot choice";
    case captain::FieldSessionPhase::sampled_score23_shot_prepared: return "sampled opponent shot prepared";
    case captain::FieldSessionPhase::sampled_score23_shot_presented: return "sampled opponent shot presented";
    case captain::FieldSessionPhase::sampled_score23_shot_clock: return "sampled opponent shot clock";
    case captain::FieldSessionPhase::sampled_score23_shot_approach: return "sampled opponent shot approach";
    case captain::FieldSessionPhase::sampled_score23_shot_handoff: return "sampled opponent shot handoff";
    case captain::FieldSessionPhase::sampled_score23_shot_target: return "sampled opponent shot target";
    case captain::FieldSessionPhase::sampled_score23_ball_advance_ready: return "sampled opponent shot flight ready";
    case captain::FieldSessionPhase::sampled_score23_ball_cell_callback: return "sampled opponent shot cell callback";
    case captain::FieldSessionPhase::sampled_score23_ball_arrival_callback: return "sampled opponent shot arrival callback";
    case captain::FieldSessionPhase::sampled_score23_keeper_prompt: return "sampled goalkeeper prompt";
    case captain::FieldSessionPhase::sampled_score23_keeper_selected: return "sampled goalkeeper selected";
    case captain::FieldSessionPhase::sampled_score23_keeper_confirmed: return "sampled goalkeeper confirmed";
    case captain::FieldSessionPhase::sampled_score23_keeper_precompare: return "sampled goalkeeper comparison pending";
    case captain::FieldSessionPhase::sampled_score23_keeper_compared: return "sampled goalkeeper compared";
    case captain::FieldSessionPhase::sampled_score24_goal_scored: return "sampled opponent goal scored";
    case captain::FieldSessionPhase::sampled_score24_goal_presented: return "sampled opponent goal presented";
    case captain::FieldSessionPhase::sampled_score24_goal_restart: return "sampled opponent goal restart";
    case captain::FieldSessionPhase::sampled_score24_dribble_choice: return "sampled 2-4 dribble defender choice";
    case captain::FieldSessionPhase::sampled_score24_dribble_prepared: return "sampled 2-4 dribble prepared";
    case captain::FieldSessionPhase::sampled_score24_dribble_defender: return "sampled 2-4 dribble defender prepared";
    case captain::FieldSessionPhase::sampled_score24_dribble_compared: return "sampled 2-4 dribble defender compared";
    case captain::FieldSessionPhase::sampled_score24_dribble_clock: return "sampled 2-4 dribble contact clock";
    case captain::FieldSessionPhase::sampled_score24_dribble_field: return "sampled 2-4 dribble field movement";
    case captain::FieldSessionPhase::sampled_score24_dribble_possession: return "sampled 2-4 opponent possession";
    case captain::FieldSessionPhase::sampled_score25_pass_target: return "sampled 2-5 human pass target";
    case captain::FieldSessionPhase::sampled_score25_player7_confirmed: return "sampled player7 pass confirmed";
    case captain::FieldSessionPhase::sampled_score25_pass_cancel: return "sampled 2-5 pass cancel pending";
    case captain::FieldSessionPhase::sampled_score25_pass_defenses: return "sampled 2-5 player7 pass defender choices";
    case captain::FieldSessionPhase::sampled_score25_pass_prepared: return "sampled 2-5 player7 pass prepared";
    case captain::FieldSessionPhase::sampled_score25_first_defender_prepared: return "sampled 2-5 player7 pass first defender prepared";
    case captain::FieldSessionPhase::sampled_score25_first_defender_compared: return "sampled 2-5 player7 pass first defender compared";
    case captain::FieldSessionPhase::sampled_score25_first_contact_clock: return "sampled 2-5 player7 pass first contact clock";
    case captain::FieldSessionPhase::sampled_score25_second_defender_prepared: return "sampled 2-5 player7 pass second defender prepared";
    case captain::FieldSessionPhase::sampled_score25_second_defender_compared: return "sampled 2-5 player7 pass second defender compared";
    case captain::FieldSessionPhase::sampled_score25_second_contact_clock: return "sampled 2-5 player7 pass second contact clock";
    case captain::FieldSessionPhase::sampled_score25_third_defender_prepared: return "sampled player7 third defender prepared";
    case captain::FieldSessionPhase::sampled_score25_third_defender_compared: return "sampled player7 third defender compared";
    case captain::FieldSessionPhase::sampled_score25_third_contact_clock: return "sampled player7 third contact clock";
    case captain::FieldSessionPhase::sampled_score25_dispatch_clock: return "sampled player7 dispatch clock";
    case captain::FieldSessionPhase::sampled_score25_pass_handoff: return "sampled player7 pass handoff";
    case captain::FieldSessionPhase::sampled_score25_pass_target_captured: return "sampled player7 pass target";
    case captain::FieldSessionPhase::sampled_score25_shot_target: return "sampled player7 shot target";
    case captain::FieldSessionPhase::sampled_score25_shot_advance: return "sampled player7 shot flight";
    case captain::FieldSessionPhase::sampled_score25_shot_callback: return "sampled player7 shot callback";
    case captain::FieldSessionPhase::sampled_score25_shot_arrival: return "sampled player7 shot arrival";
    case captain::FieldSessionPhase::sampled_player3_contact_clock: return "sampled player3 contact clock advanced";
    case captain::FieldSessionPhase::sampled_player3_dispatch_clock: return "sampled player3 pass dispatch clock advanced";
    case captain::FieldSessionPhase::sampled_player3_pass_handoff: return "sampled player3 pass handoff";
    case captain::FieldSessionPhase::sampled_player3_empty_pass_destination: return "sampled player3 empty pass destination";
    case captain::FieldSessionPhase::attack_choice_pending: return "confirmed action pending";
    case captain::FieldSessionPhase::attack_prepared: return "ordinary shot prepared";
    case captain::FieldSessionPhase::shot_field_complete: return "shot passed selected defenses; flight pending";
    case captain::FieldSessionPhase::shot_restart_pending: return "shot interruption; restart pending";
    case captain::FieldSessionPhase::shot_defender_next: return "next selected shot defender";
    case captain::FieldSessionPhase::shot_contact_added_time: return "shot contact added-time presentation pending";
    case captain::FieldSessionPhase::shot_dispatch_clock: return "shot final dispatch clock pending";
    case captain::FieldSessionPhase::shot_dispatch_added_time: return "shot dispatch added-time presentation pending";
    case captain::FieldSessionPhase::shot_dispatch_ready: return "shot field result ready for dispatch";
    case captain::FieldSessionPhase::selected_ball_recovery: return "selected shot loose-ball recovery pending";
    case captain::FieldSessionPhase::selected_defense_animation: return "selected defense animation pending";
    case captain::FieldSessionPhase::selected_restart_entry: return "selected shot restart entry";
    case captain::FieldSessionPhase::selected_recovered_owner: return "selected recovered owner; presentation pending";
    case captain::FieldSessionPhase::selected_boundary_restart: return "selected recovery boundary restart pending";
    case captain::FieldSessionPhase::capture_clearance_flight: return "capture clearance prepared; flight pending";
    case captain::FieldSessionPhase::clearance_race_winner: return "clearance won; presentation pending";
    case captain::FieldSessionPhase::clearance_race_timeout: return "clearance race timeout; recovery pending";
    case captain::FieldSessionPhase::clearance_recovery_ready: return "clearance recovery ready; jitter already complete";
    case captain::FieldSessionPhase::clearance_recovered_owner: return "clearance recovered owner; presentation pending";
    case captain::FieldSessionPhase::selected_field_restart: return "selected field restart setup pending";
    case captain::FieldSessionPhase::selected_goal_restart: return "selected goal-area restart setup pending";
    case captain::FieldSessionPhase::selected_attack_presentation: return "selected shot attacking presentation pending";
    case captain::FieldSessionPhase::attack_clock_added_time: return "attack added-time presentation pending";
    case captain::FieldSessionPhase::shot_approach_ready: return "ordinary shot approach ready";
    case captain::FieldSessionPhase::shot_flight_pending: return "shot flight setup ready; scheduling pending";
    case captain::FieldSessionPhase::ball_already_arrived: return "ball was already at destination; return without arrival callback";
    case captain::FieldSessionPhase::shot_pass_defenders_pending: return "shot arrival queued pass defenders; defender announcement pending";
    case captain::FieldSessionPhase::shot_pass_defender_announced: return "shot pass defender announced; preparation pending";
    case captain::FieldSessionPhase::shot_pass_defender_prepared: return "shot pass defender prepared; comparison pending";
    case captain::FieldSessionPhase::shot_pass_defender_compared: return "shot pass defender compared; result handler pending";
    case captain::FieldSessionPhase::shot_pass_variant_restored: return "shot pass variant restored; result presentation pending";
    case captain::FieldSessionPhase::shot_pass_result_handler_entry: return "shot pass result acknowledged; nonterminal handler pending";
    case captain::FieldSessionPhase::shot_pass_nonterminal_done: return "shot pass defender result handled; next defender pending";
    case captain::FieldSessionPhase::shot_pass_defenders_done: return "shot pass defenders complete; arrival continuation pending";
    case captain::FieldSessionPhase::shot_pass_added_time: return "shot pass added-time presentation pending";
    case captain::FieldSessionPhase::shot_pass_period_end: return "shot pass period-end presentation pending";
    case captain::FieldSessionPhase::shot_loose_recovery: return "loose flight presentation/recovery pending";
    case captain::FieldSessionPhase::keeper_area_defense_poll: return "defensive input sampled; next poll pending";
    case captain::FieldSessionPhase::keeper_area_prefix_clock_done: return "area prefix clock complete; possession dispatch pending";
    case captain::FieldSessionPhase::keeper_area_prefix_added_time: return "area prefix added-time presentation pending";
    case captain::FieldSessionPhase::keeper_area_prefix_period_end: return "area prefix period ended; period transition pending";
    case captain::FieldSessionPhase::keeper_area_attack_continuation: return "area attack continuation pending";
    case captain::FieldSessionPhase::keeper_area_recovery_ready: return "area loose-ball recovery pending";
    case captain::FieldSessionPhase::keeper_area_recovered_owner: return "area recovered owner; presentation pending";
    case captain::FieldSessionPhase::area_ball_advance_ready: return "area ball flight initialized; next step pending";
    case captain::FieldSessionPhase::area_ball_already_arrived: return "area ball already at destination; return pending";
    case captain::FieldSessionPhase::area_ball_cell_callback: return "area ball cell callback pending";
    case captain::FieldSessionPhase::area_ball_arrival_callback: return "area ball arrival callback pending";
    case captain::FieldSessionPhase::area_pass_received: return "area pass recipient selected; handoff pending";
    case captain::FieldSessionPhase::area_pass_keeper_entry: return "area pass keeper interaction entry pending";
    case captain::FieldSessionPhase::area_action4_encounters_pending: return "area action4 defender encounters pending";
    case captain::FieldSessionPhase::area_action4_field_entry: return "area action4 field encounter setup pending";
    case captain::FieldSessionPhase::area_action4_control_entry: return "area action4 player control setup pending";
    case captain::FieldSessionPhase::area_action4_control_ready: return "area action4 player control prepared; input pending";
    case captain::FieldSessionPhase::area_action4_control_wait: return "area action4 control input wait pending";
    case captain::FieldSessionPhase::area_action4_control_clock: return "area action4 input accepted; control clock pending";
    case captain::FieldSessionPhase::area_action4_field_round: return "area action4 control clock complete; field round pending";
    case captain::FieldSessionPhase::area_action4_goal_area: return "area action4 goal-area action pending";
    case captain::FieldSessionPhase::area_action4_added_time: return "area action4 added-time presentation pending";
    case captain::FieldSessionPhase::area_action4_period_end: return "area action4 period-end presentation pending";
    case captain::FieldSessionPhase::area_action4_keeper_entry: return "area action4 goalkeeper entry; presentation pending";
    case captain::FieldSessionPhase::area_action4_keeper_presented: return "area action4 goalkeeper presentation acknowledged; decision pending";
    case captain::FieldSessionPhase::area_action4_keeper_prepared: return "area action4 goalkeeper preparation captured; stamina and comparison pending";
    case captain::FieldSessionPhase::area_action4_keeper_stamina_done: return "area action4 goalkeeper stamina applied; comparison pending";
    case captain::FieldSessionPhase::area_action4_keeper_compared: return "area action4 goalkeeper comparison saved; outcome presentation pending";
    case captain::FieldSessionPhase::area_action4_keeper_dispatch_entry: return "area action4 goalkeeper outcome dispatch entry; result handler pending";
    case captain::FieldSessionPhase::area_action4_keeper_result0_entry: return "area action4 goalkeeper result0 continuation entry";
    case captain::FieldSessionPhase::area_action4_keeper_result1_recovery: return "area action4 goalkeeper result1 loose recovery entry";
    case captain::FieldSessionPhase::area_action4_keeper_result1_boundary_check: return "area action4 goalkeeper result1 ball boundary check";
    case captain::FieldSessionPhase::area_action4_keeper_result1_restart_entry: return "area action4 goalkeeper result1 restart entry";
    case captain::FieldSessionPhase::area_action4_keeper_restart_prepared: return "area action4 goalkeeper restart choice captured";
    case captain::FieldSessionPhase::area_action4_keeper_restart_formation: return "area action4 goalkeeper restart formation captured";
    case captain::FieldSessionPhase::area_action4_keeper_restart_power: return "area action4 goalkeeper restart action power prepared";
    case captain::FieldSessionPhase::area_action4_restart_clock_done: return "area action4 restart clock advanced; action dispatch pending";
    case captain::FieldSessionPhase::area_action4_restart_added_time: return "area action4 restart added-time presentation pending";
    case captain::FieldSessionPhase::area_action4_restart_period_end: return "area action4 restart period end pending";
    case captain::FieldSessionPhase::area_action4_restart_action1_entry: return "area action4 restart action1 entry";
    case captain::FieldSessionPhase::sampled_keeper_result4_extra_time: return "sampled keeper result4 extra time updated";
    case captain::FieldSessionPhase::sampled_keeper_result4_action: return "sampled keeper result4 action captured";
    case captain::FieldSessionPhase::sampled_keeper_result4_formation: return "sampled keeper result4 formation captured";
    case captain::FieldSessionPhase::sampled_keeper_result4_restart: return "sampled keeper result4 restart branch";
    case captain::FieldSessionPhase::sampled_loose_shot_defender_compared: return "sampled loose-shot defender compared";
    case captain::FieldSessionPhase::sampled_loose_shot_defender_prepared: return "sampled loose-shot defender prepared";
    case captain::FieldSessionPhase::area_action4_keeper_result0_goal: return "area action4 keeper result0 goal scored";
    case captain::FieldSessionPhase::area_action4_restart_pass_entry: return "area action4 restart pass entry";
    case captain::FieldSessionPhase::area_action4_control_pending: return "area action4 control setup pending";
    case captain::FieldSessionPhase::area_action6_destination_pending: return "area action6 destination chosen; continuation pending";
    case captain::FieldSessionPhase::area_pass_keeper_prepared: return "area pass keeper interaction prepared; menu presentation pending";
    case captain::FieldSessionPhase::area_action6_race_winner: return "area action6 race winner; presentation pending";
    case captain::FieldSessionPhase::area_action6_race_timeout: return "area action6 race timeout; recovery pending";
    case captain::FieldSessionPhase::area_action6_control_pending: return "area action6 winner; control setup pending";
    case captain::FieldSessionPhase::area_action6_recovered_owner: return "area action6 recovered owner; continuation pending";
    case captain::FieldSessionPhase::area_action6_recovered_control: return "area action6 recovered control setup pending";
    case captain::FieldSessionPhase::area_action6_control_ready: return "area action6 control prepared; input polling pending";
    case captain::FieldSessionPhase::area_pass_defenders_pending: return "area pass defender encounters pending";
    case captain::FieldSessionPhase::area_pass_defender_announced: return "area pass defender announcement acknowledged";
    case captain::FieldSessionPhase::area_pass_defender_prepared: return "area pass defender prepared; contact pending";
    case captain::FieldSessionPhase::area_pass_defender_compared: return "area pass defender compared; outcome pending";
    case captain::FieldSessionPhase::area_pass_variant_restored: return "area pass ball variant restored; outcome pending";
    case captain::FieldSessionPhase::area_pass_result_handler_entry: return "area pass result handler entry";
    case captain::FieldSessionPhase::area_pass_nonterminal_done: return "area pass nonterminal result applied";
    case captain::FieldSessionPhase::area_pass_nonterminal_added_time: return "area pass added-time presentation pending";
    case captain::FieldSessionPhase::area_pass_nonterminal_period_end: return "area pass period ended";
    case captain::FieldSessionPhase::area_pass_defenders_done: return "area pass defender queue completed";
    case captain::FieldSessionPhase::area_pass_blocked_positioned: return "blocked area pass positioned at defender";
    case captain::FieldSessionPhase::area_pass_terminal_clock_done: return "area pass terminal clock complete";
    case captain::FieldSessionPhase::area_pass_terminal_added_time: return "area pass terminal added time pending";
    case captain::FieldSessionPhase::area_pass_terminal_period_end: return "area pass terminal period ended";
    case captain::FieldSessionPhase::area_pass_defender_captured: return "area pass captured by defender";
    case captain::FieldSessionPhase::area_pass_capture_control_entry: return "captured pass field movement complete; control pending";
    case captain::FieldSessionPhase::area_pass_capture_control_ready: return "captured pass control prepared; input polling pending";
    case captain::FieldSessionPhase::area_pass_capture_control_wait: return "captured pass control waiting";
    case captain::FieldSessionPhase::area_pass_capture_action_menu: return "captured pass action menu pending";
    case captain::FieldSessionPhase::area_pass_capture_clock_pending: return "captured pass control clock pending";
    case captain::FieldSessionPhase::area_pass_blocked_recovery_entry: return "blocked pass recovery entry";
    case captain::FieldSessionPhase::area_pass_blocked_recovered_owner: return "blocked pass recovered owner";
    case captain::FieldSessionPhase::area_pass_blocked_boundary: return "blocked pass boundary restart";
    case captain::FieldSessionPhase::area_pass_blocked_control_entry: return "blocked pass recovered-owner control entry";
    case captain::FieldSessionPhase::area_pass_blocked_control_ready: return "blocked pass control prepared; input polling pending";
    case captain::FieldSessionPhase::area_pass_blocked_control_wait: return "blocked pass control waiting";
    case captain::FieldSessionPhase::area_pass_blocked_action_menu: return "blocked pass action menu pending";
    case captain::FieldSessionPhase::area_pass_blocked_clock_pending: return "blocked pass control clock pending";
    case captain::FieldSessionPhase::area_pass_capture_field_round: return "captured pass field round pending";
    case captain::FieldSessionPhase::area_pass_capture_goal_area: return "captured pass goal area action pending";
    case captain::FieldSessionPhase::area_pass_capture_added_time: return "captured pass control added time";
    case captain::FieldSessionPhase::area_pass_capture_period_end: return "captured pass control period end";
    case captain::FieldSessionPhase::area_pass_blocked_field_round: return "blocked pass field round pending";
    case captain::FieldSessionPhase::area_pass_blocked_goal_area: return "blocked pass goal area action pending";
    case captain::FieldSessionPhase::area_pass_blocked_added_time: return "blocked pass control added time";
    case captain::FieldSessionPhase::area_pass_blocked_period_end: return "blocked pass control period end";
    case captain::FieldSessionPhase::keeper_area_contact_presented: return "area result presentation acknowledged; loop advance pending";
    case captain::FieldSessionPhase::keeper_area_defender_next: return "next area defender ready";
    case captain::FieldSessionPhase::keeper_area_prefix_done: return "area defenders exhausted; final clock pending";
    case captain::FieldSessionPhase::keeper_area_terminal_result: return "terminal area result; possession dispatch pending";
    case captain::FieldSessionPhase::keeper_area_contact_clock_done: return "area contact clock complete; result presentation pending";
    case captain::FieldSessionPhase::keeper_area_contact_added_time: return "area contact added-time presentation pending";
    case captain::FieldSessionPhase::keeper_area_contact_period_end: return "area contact period ended; period transition pending";
    case captain::FieldSessionPhase::keeper_area_defender_compared: return "area defender compared; result timing pending";
    case captain::FieldSessionPhase::keeper_area_defenders_announced: return "area defenders announced; contact calculation pending";
    case captain::FieldSessionPhase::keeper_area_defense_entry: return "area attack presentation acknowledged; defender sequence entry";
    case captain::FieldSessionPhase::keeper_area_attack_prepared: return "opponent area attack prepared; presentation pending";
    case captain::FieldSessionPhase::keeper_area_ai_chosen: return "opponent area action chosen; preparation pending";
    case captain::FieldSessionPhase::keeper_area_special_redrawn: return "defensive choices redrawn; input tail pending";
    case captain::FieldSessionPhase::keeper_area_special_inspected: return "special choice inspected; registration or redraw pending";
    case captain::FieldSessionPhase::keeper_area_special_poll: return "special chooser waiting for input";
    case captain::FieldSessionPhase::keeper_area_special_decided: return "special chooser decision pending continuation";
    case captain::FieldSessionPhase::keeper_area_special_ready: return "special defensive chooser prepared; input pending";
    case captain::FieldSessionPhase::keeper_area_defense_complete: return "defensive choices complete; area action pending";
    case captain::FieldSessionPhase::keeper_area_defense_preparation: return "defensive choice confirmed; preparation pending";
    case captain::FieldSessionPhase::keeper_area_defense_ready: return "defensive prompt prepared; input pending";
    case captain::FieldSessionPhase::keeper_area_defense_initial: return "defensive choices initialized; display pending";
    case captain::FieldSessionPhase::keeper_area_choices_done: return "no defensive choices; area action pending";
    case captain::FieldSessionPhase::keeper_area_defense_pending: return "keeper-area defensive choices pending";
    case captain::FieldSessionPhase::keeper_area_prepared: return "keeper-area route prepared; interaction menu pending";
    case captain::FieldSessionPhase::kickoff_control_pending: return "kickoff carrier set; control setup pending";
    case captain::FieldSessionPhase::kickoff_ball_placed: return "kickoff ball placed; presentation pending";
    case captain::FieldSessionPhase::kickoff_receiver_selected: return "kickoff receiver selected; ball setup pending";
    case captain::FieldSessionPhase::kickoff_formation_placed: return "kickoff formation placed; receiver selection pending";
    case captain::FieldSessionPhase::goal_kickoff_pending: return "post-goal kickoff preparation pending";
    case captain::FieldSessionPhase::goal_celebration_pending: return "goal scored; goal presentation and celebration pending";
    case captain::FieldSessionPhase::keeper_possession: return "keeper possession; keeper control pending";
    case captain::FieldSessionPhase::keeper_punch_recovery: return "keeper punch; no-jitter recovery pending";
    case captain::FieldSessionPhase::keeper_decided: return "keeper result saved; presentation9 pending";
    case captain::FieldSessionPhase::keeper_presentation_pending: return "keeper selected; presentation pending";
    case captain::FieldSessionPhase::stunned_keeper_pending: return "stunned keeper decision pending";
    case captain::FieldSessionPhase::stunned_keeper_decided: return "stunned keeper result saved; presentation12 pending";
    case captain::FieldSessionPhase::shot_keeper_entry: return "flight keeper entry pending";
    case captain::FieldSessionPhase::ball_advance_ready: return "numeric ball flight ready; frame acknowledgement pending";
    case captain::FieldSessionPhase::ball_cell_callback: return "ball entered next cell; callback pending";
    case captain::FieldSessionPhase::ball_arrival_callback: return "ball arrived; callback pending";
    case captain::FieldSessionPhase::restart_options: return "prepared restart options";
    case captain::FieldSessionPhase::restart_tactics: return "restart tactical UI pending";
    case captain::FieldSessionPhase::restart_options_done: return "restart options done; caller pending";
    }
    return "invalid";
}
void show(const captain::PhaseSessionState& state) {
    const auto& clock = state.current.clock;
    const auto& field = state.current.field;
    std::cout << "Session: " << phase_name(state.phase) << " | clock " << clock.remaining
              << " | extra " << unsigned(clock.extra_time) << " | flags " << unsigned(clock.flags)
              << " | script " << unsigned(clock.script_flags) << " | ball mode " << unsigned(clock.ball_mode)
              << " | shared recovery " << unsigned(state.current.shared_recovery_flags)
              << ',' << unsigned(state.current.shared_recovery_timer) << '\n';
    std::cout << "Field: side " << unsigned(field.side) << " | owner " << unsigned(field.ball_owner)
              << " | ball " << unsigned(field.ball_x) << ',' << unsigned(field.ball_y)
              << " | cell " << unsigned(field.ball_cell) << " | elapsed " << unsigned(field.elapsed)
              << " | selected " << unsigned(field.selected_defenders)
              << " | random " << unsigned(field.random_first) << ',' << unsigned(field.random_second) << '\n';
    if(state.kickoff_receiver)std::cout << "Kickoff receiver: " << unsigned(*state.kickoff_receiver) << '\n';
    if(state.kickoff_formation)std::cout << "Keeper Y: " << unsigned(state.kickoff_formation->keeper_y[0]) << ',' << unsigned(state.kickoff_formation->keeper_y[1]) << '\n';
    if(state.keeper_area)std::cout << "Area defender cursor: " << unsigned(state.area_defender_cursor)
        << " | last result " << unsigned(state.area_sequence_result) << '\n';
    if(state.area_contact)std::cout << "Area contact: cursor " << unsigned(state.area_contact->cursor)
        << " | result " << unsigned(state.area_contact->result) << " | strength " << unsigned(state.area_contact->strength)
        << " | cost " << state.area_contact->cost << " | weakened " << state.area_contact->weakened << '\n';
    if(state.keeper_area)std::cout << "Keeper-area selection counter: " << unsigned(state.keeper_area->selection_counter) << '\n';
    if(state.score)std::cout << "Score: " << unsigned(state.score->left_goals) << '-' << unsigned(state.score->right_goals) << " | kickoff " << unsigned(state.score->kickoff) << '\n';
    if(state.keeper_decision) std::cout << "Keeper result: " << unsigned(state.keeper_decision->result_code) << '\n';
    if(state.ball_trajectory) std::cout << "Trajectory: " << state.ball_trajectory->x << ',' << state.ball_trajectory->y
        << " | velocity " << state.ball_trajectory->vx << ',' << state.ball_trajectory->vy << '\n';
    if(state.restart_options) std::cout << "Restart options: goal area " << state.restart_options->goal_area
        << " | cursor " << unsigned(state.restart_options->cursor) << '\n';
    if(state.clearance_race) std::cout << "Race: remaining " << unsigned(state.clearance_race->remaining)
        << " | candidates " << unsigned(state.clearance_race->active_candidate) << ','
        << unsigned(state.clearance_race->opposing_candidate) << '\n';
    if (state.clearance_flight) std::cout << "Clearance distance adjustment "
        << unsigned(state.clearance_flight->distance_adjustment) << '\n';
    if (state.shot_progress) std::cout << "Shot progress: cursor " << unsigned(state.shot_progress->cursor)
        << " | result " << unsigned(state.shot_progress->result_code)
        << " | defender " << unsigned(state.shot_progress->defender_id) << '\n';
    if (state.pending) std::cout << "Pending: " << unsigned(state.pending->result_code) << ' '
        << unsigned(state.pending->defender_id) << ' ' << unsigned(state.pending->defense_action)
        << ' ' << unsigned(state.pending->defense_subtype) << '\n';
    if (state.experience) {
        std::cout << "Experience: campaign " << unsigned(state.experience->campaign)
                  << " | stage " << unsigned(state.experience->opponent_stage) << " | totals";
        for (const auto total : state.experience->totals) std::cout << ' ' << total;
        std::cout << '\n';
    } else std::cout << "Experience: context absent\n";
    if (state.recovery) {
        const auto& recovery = *state.recovery;
        std::cout << "Recovery: opponent " << unsigned(recovery.opposing_player)
                  << " | attack subtype " << unsigned(recovery.attack_subtype)
                  << " | owner flags " << unsigned(recovery.special_owner_flags)
                  << " | ball variant " << unsigned(recovery.ball_variant)
                  << " | game mode " << unsigned(recovery.game_mode)
                  << " | jitter control " << unsigned(recovery.x_jitter_control) << '\n';
    } else std::cout << "Recovery: context absent\n";
    if (state.control) {
        const auto& control = *state.control;
        std::cout << "Control: timer " << unsigned(control.decision_timer)
                  << " | facing " << unsigned(control.facing)
                  << " | controlled player " << unsigned(control.controlled_player)
                  << " | last carrier cell " << unsigned(control.last_carrier_cell)
                  << " | travel " << control.travel_accumulator
                  << " | display parameter " << unsigned(control.display_parameter)
                  << " | motion requested " << unsigned(control.motion_requested) << '\n';
    } else std::cout << "Control: context absent\n";
    if (state.scheduling) {
        const auto& schedule = *state.scheduling;
        std::cout << "Scheduling: field cycle " << unsigned(schedule.field_cycle)
                  << " | energy cycle " << unsigned(schedule.energy_cycle)
                  << " | special flags " << unsigned(schedule.special_flags) << " | selected";
        for (unsigned i = 0; i < field.selected_defenders; ++i) std::cout << ' ' << unsigned(schedule.selected_ids[i]);
        std::cout << '\n';
    } else std::cout << "Scheduling: context absent\n";
    if (state.action) {
        const auto& action = *state.action;
        std::cout << "Action: " << unsigned(action.action) << " | route " << unsigned(action.menu_route)
                  << " | cursor " << unsigned(action.cursor) << " | flash " << unsigned(action.flash_counter)
                  << " | auxiliary " << unsigned(action.auxiliary) << " | move " << unsigned(action.move_effect)
                  << ',' << unsigned(action.move_flags) << " | power " << action.power << '\n';
    } else std::cout << "Action: context absent\n";
    for (unsigned slot = 1; slot <= 21; ++slot) {
        if (slot == 11) continue;
        const auto& actor = field.motion[captain::field_motion_index(static_cast<std::uint8_t>(slot))];
        std::cout << "Player " << slot << " | xy " << actor.x << ',' << actor.y
                  << " | target " << unsigned(actor.target_cell) << " | status " << unsigned(actor.status) << '\n';
    }
}
void save(const std::string& path, const captain::PhaseSessionState& state) {
    if (std::filesystem::path(path).extension() != ".cmjsession")
        throw std::invalid_argument("Use a private .cmjsession output path");
    std::ostringstream output(std::ios::out | std::ios::binary);
    captain::write_phase_session(output, state);
    const auto bytes = output.str();
    auto* file = std::fopen(path.c_str(), "wbx");
    if (!file) throw std::invalid_argument("Unable to create new session; existing files are refused");
    const auto written = std::fwrite(bytes.data(), 1, bytes.size(), file);
    const auto closed = std::fclose(file);
    if (written != bytes.size() || closed != 0) throw std::invalid_argument("Unable to finish session write");
    std::cout << "Saved " << bytes.size() << " bytes to " << path << '\n';
}
void contact(captain::PhaseSessionState& state, const captain::ShotContent& content,
             unsigned code, unsigned defender, unsigned action, unsigned subtype, bool rewarded = false) {
    if (rewarded && !content.experience) throw std::invalid_argument("Rewarded contact requires version6 experience content");
    const captain::PendingTerminalContact input{
        static_cast<std::uint8_t>(code), static_cast<std::uint8_t>(defender),
        static_cast<std::uint8_t>(action), static_cast<std::uint8_t>(subtype)};
    const auto result = rewarded
        ? captain::resolve_session_rewarded_terminal(state, input, content.field_players,
            *content.movement, *content.contact, *content.experience)
        : captain::resolve_session_terminal(state, input, content.field_players, *content.movement, *content.contact);
    state = result.state;
    for (const auto& reward : result.rewards)
        std::cout << "Reward: " << (reward.slot ? "slot " + std::to_string(*reward.slot) : "ignored type")
                  << " | amount " << unsigned(reward.award) << " | saturated " << reward.saturated << '\n';
    std::cout << "Contact elapsed " << unsigned(result.prelude->elapsed_step)
              << " | clock display request " << result.prelude->clock.request_clock_display
              << " | warning cue request " << result.prelude->clock.request_warning_cue << '\n';
}
void resume(captain::PhaseSessionState& state, const captain::ShotContent& content) {
    const auto result = captain::resume_session_terminal(state, content.field_players, *content.movement);
    state = result.state;
    std::cout << "Resumed terminal tail with frozen development inputs; no NMI/presentation emulated.\n";
}
void advance(captain::PhaseSessionState& state, const captain::ShotContent& content, unsigned elapsed) {
    if (state.phase != captain::FieldSessionPhase::field && state.phase != captain::FieldSessionPhase::player_control)
        throw std::invalid_argument("Movement requires field or player-control phase");
    auto field = state.current.field;
    field.elapsed = static_cast<std::uint8_t>(elapsed);
    const auto result = captain::advance_field_phase(field, content.field_players, *content.movement);
    state.current.field = result.state;
    std::cout << "Advanced " << result.players.size() << " actors; clock unchanged.\n";
}
void recover(captain::PhaseSessionState& state, const captain::ShotContent& content) {
    const auto result = captain::advance_session_recovery(state, content.field_players, *content.movement);
    state = result.state;
    std::cout << "Recovery advanced " << (result.movement ? result.movement->players.size() : 0)
              << " actors | recipient " << (result.recipient ? std::to_string(result.recipient->ball_owner) : "pending")
              << " | owner cleanup " << result.special_owner_cleared
              << " | possession sound request " << result.request_possession_sound << '\n';
}
void recover_selected_file(captain::PhaseSessionState& state, const captain::ShotContent& content,
                           const std::string& path) {
    captain::validate_phase_session(state);
    if (state.phase != captain::FieldSessionPhase::selected_ball_recovery &&
        state.phase != captain::FieldSessionPhase::clearance_recovery_ready)
        throw std::invalid_argument("Selected recovery requires its saved handoff");
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open selected recovery action file");
    std::vector<captain::RecoveryDefender> choices;
    for (unsigned i=0;i<state.current.field.selected_defenders;++i) {
        unsigned action;
        if (!(input >> action) || action > 6) throw std::invalid_argument("Expected one action0..6 per saved selected player");
        choices.push_back({state.scheduling->selected_ids[i],static_cast<std::uint8_t>(action)});
    }
    input >> std::ws;
    if (input.peek() != std::char_traits<char>::eof() || input.bad())
        throw std::invalid_argument("Trailing or unreadable selected recovery actions");
    const auto result = state.phase == captain::FieldSessionPhase::clearance_recovery_ready
        ? captain::advance_session_clearance_recovery(state,choices,content.field_players,*content.movement)
        : captain::advance_session_selected_recovery(state,choices,content.field_players,*content.movement);
    state = result.state;
    std::cout << "Selected recovery | recipient " << (result.recipient ? std::to_string(result.recipient->ball_owner) : "pending")
              << " | direct selected capture " << (result.recipient && result.recipient->direct_defender_capture)
              << " | possession sound request " << result.request_possession_sound << '\n';
}
void continue_recovery(captain::PhaseSessionState& state) {
    state = captain::continue_session_recovery(state);
    std::cout << "Prepared recovery tail: " << phase_name(state.phase)
              << "; frozen development inputs, no presentation/NMI emulation.\n";
}
void setup_control(captain::PhaseSessionState& state) {
    const auto result = captain::prepare_session_control(state);
    state = result.state;
    std::cout << "Prepared control | selected defender "
              << (result.defender ? std::to_string(result.defender->player_id) : "unchanged")
              << " | special cleanup " << result.special_owner_cleared << '\n';
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind)
                  << " | id " << unsigned(request.id) << '\n';
}
void poll_control(captain::PhaseSessionState& state, const captain::ShotContent& content, unsigned held, unsigned pressed) {
    if (!content.control_movement) throw std::invalid_argument("Control polling requires version7 speed content");
    const auto result = (state.phase==captain::FieldSessionPhase::area_action4_control_ready || state.phase==captain::FieldSessionPhase::area_action4_control_wait)
        ? captain::poll_session_area_action4_control(state,static_cast<std::uint8_t>(held),static_cast<std::uint8_t>(pressed),content.field_players,*content.movement,*content.control_movement)
        : (state.phase==captain::FieldSessionPhase::area_pass_capture_control_ready || state.phase==captain::FieldSessionPhase::area_pass_capture_control_wait || state.phase==captain::FieldSessionPhase::area_pass_blocked_control_ready || state.phase==captain::FieldSessionPhase::area_pass_blocked_control_wait)
        ? captain::poll_session_area_pass_capture(state,static_cast<std::uint8_t>(held),static_cast<std::uint8_t>(pressed),content.field_players,*content.movement,*content.control_movement)
        : captain::poll_session_control(state, static_cast<std::uint8_t>(held),static_cast<std::uint8_t>(pressed), content.field_players, *content.movement, *content.control_movement);
    state = result.state;
    std::cout << "Input poll | decision step " << result.decision_step
              << " | moved player " << (result.moved_player ? std::to_string(*result.moved_player) : "none")
              << " | speed " << result.movement_speed << '\n';
    if (result.menu_requested) std::cout << "Action-menu script68 pending; handler not executed.\n";
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind)
                  << " | id " << unsigned(request.id) << '\n';
}
void advance_control(captain::PhaseSessionState& state, const captain::ShotContent& content, bool resume_time) {
    if (!content.world_step) throw std::invalid_argument("Control cycle requires version8 stamina content");
    const bool pass_clock=state.phase==captain::FieldSessionPhase::area_pass_capture_clock_pending || state.phase==captain::FieldSessionPhase::area_pass_blocked_clock_pending;
    const auto result = state.phase==captain::FieldSessionPhase::area_action4_added_time && resume_time
        ? captain::resume_area_action4_added_time(state,content.field_players,content.goalkeepers,*content.world_step)
        : state.phase==captain::FieldSessionPhase::area_action4_control_clock && !resume_time
        ? captain::advance_area_action4_control_tick(state,content.field_players,content.goalkeepers,*content.world_step)
        : pass_clock && !resume_time
        ? captain::advance_area_pass_control_tick(state,content.field_players,content.goalkeepers,*content.world_step)
        : resume_time
        ? captain::resume_control_added_time(state, content.field_players, content.goalkeepers, *content.world_step)
        : captain::advance_control_tick(state, content.field_players, content.goalkeepers, *content.world_step);
    state = result.state;
    if (result.clock) std::cout << "Control clock | display request " << result.clock->request_clock_display
        << " | warning cue request " << result.clock->request_warning_cue << '\n';
    else std::cout << "Prepared added-time continuation; no repeated clock/recovery.\n";
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << '\n';
}
void advance_world_inputs(captain::PhaseSessionState& state, const captain::ShotContent& content,
                         const std::array<captain::LogicalButtons, 22>& inputs) {
    if (!content.world_step || !content.control_movement)
        throw std::invalid_argument("World round requires version8 speed content");
    const bool pass_round=state.phase==captain::FieldSessionPhase::area_pass_capture_field_round || state.phase==captain::FieldSessionPhase::area_pass_blocked_field_round;
    const auto result = state.phase==captain::FieldSessionPhase::area_action4_field_round ? captain::advance_area_action4_field_round(state,content.field_players,*content.movement,*content.control_movement,*content.world_step,inputs) :
        pass_round ? captain::advance_area_pass_field_round(state,content.field_players,*content.movement,*content.control_movement,*content.world_step,inputs) :
        captain::advance_control_field_round(state, content.field_players,*content.movement,*content.control_movement,*content.world_step,inputs);
    state = result.state;
    std::cout << "World round | polls " << result.consumed_polls << " | moved " << result.moved_players
              << " | decayed statuses " << result.decayed_statuses << '\n';
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << '\n';
}
void advance_world(captain::PhaseSessionState& state, const captain::ShotContent& content, unsigned held, unsigned pressed) {
    std::array<captain::LogicalButtons, 22> inputs{};
    inputs.fill({static_cast<std::uint8_t>(held), static_cast<std::uint8_t>(pressed)});
    advance_world_inputs(state, content, inputs);
}
void advance_world_file(captain::PhaseSessionState& state, const captain::ShotContent& content, const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open world input file");
    std::array<captain::LogicalButtons, 22> inputs{};
    for (auto& buttons : inputs) {
        unsigned held, pressed;
        if (!(input >> held >> pressed) || held > 255 || pressed > 255)
            throw std::invalid_argument("World input file requires22held/pressed byte pairs");
        buttons = {static_cast<std::uint8_t>(held), static_cast<std::uint8_t>(pressed)};
    }
    input >> std::ws;
    if (input.peek() != std::char_traits<char>::eof() || input.bad())
        throw std::invalid_argument("Trailing or unreadable world input data");
    advance_world_inputs(state, content, inputs);
}
void advance_observed_world_file(captain::PhaseSessionState& state, const captain::ShotContent& content, const std::string& path) {
    if (!content.movement || !content.world_step || !content.control_movement)
        throw std::invalid_argument("Observed field round requires private movement and world content");
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open observed world file");
    const auto samples = captain::load_observed_field_polls(input);
    const auto result = captain::advance_observed_control_field_round(
        state, content.field_players, *content.movement, *content.control_movement, *content.world_step, samples);
    state = result.state;
    std::cout << "Observed world round | polls " << result.consumed_polls << " | moved " << result.moved_players
              << "; captured events supplied, no frame/NMI/audio emulation.\n";
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << '\n';
}
void advance_observed_nmi_world_file(captain::PhaseSessionState& state, const captain::ShotContent& content,
                                     const std::string& path, std::uint8_t initial_index, bool ordinary_return=false) {
    if (!content.movement || !content.world_step || !content.control_movement)
        throw std::invalid_argument("Observed NMI field round requires private movement and world content");
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open observed NMI world file");
    const auto samples = captain::load_observed_nmi_field_polls(input);
    const auto result = ordinary_return ? captain::advance_observed_ordinary_return_field_round(
        state, content.field_players, *content.movement, *content.control_movement, *content.world_step, initial_index, samples) :
        captain::advance_observed_nmi_field_round(state, content.field_players, *content.movement,
        *content.control_movement, *content.world_step, initial_index, samples);
    state = result.round.state;
    std::cout << "Observed NMI world round | polls " << result.round.consumed_polls
              << " | final random index " << unsigned(result.random_index)
              << "; captured inputs supplied, no frame/audio/presentation emulation.\n";
    for (const auto& request : result.round.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << '\n';
}
void advance_observed_nmi_poll_file(captain::PhaseSessionState& state, const captain::ShotContent& content,
                                    const std::string& path, std::uint8_t initial_index, bool ordinary_return=false) {
    if (!content.movement || !content.control_movement)
        throw std::invalid_argument("Observed NMI control poll requires private movement content");
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open observed NMI poll file");
    const auto sample = captain::load_observed_nmi_poll(input);
    const auto result = ordinary_return ? captain::advance_observed_ordinary_return_poll(
        state, content.field_players, *content.movement, *content.control_movement, initial_index, sample) :
        captain::advance_observed_nmi_control_poll(state, content.field_players, *content.movement, *content.control_movement, initial_index, sample);
    state = result.poll.state;
    std::cout << "Observed NMI control poll | final random index " << unsigned(result.random_index)
              << "; captured inputs supplied, no frame/audio/presentation emulation.\n";
    for (const auto& request : result.poll.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << '\n';
}
void advance_observed_nmi_clock_file(captain::PhaseSessionState& state, const captain::ShotContent& content,
                                     const std::string& path, std::uint8_t initial_index, bool ordinary_return=false) {
    if (!content.world_step) throw std::invalid_argument("Observed NMI clock requires private world content");
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open observed NMI clock file");
    const auto events = captain::load_observed_clock_events(input);
    const auto result = ordinary_return ? captain::advance_observed_ordinary_return_clock(
        state, content.field_players, content.goalkeepers, *content.world_step, initial_index, events) :
        captain::advance_observed_nmi_control_clock(state, content.field_players, content.goalkeepers, *content.world_step, initial_index, events);
    state = result.tick.state;
    std::cout << "Observed NMI control clock | final random index " << unsigned(result.random_index)
              << "; captured events supplied, no frame/audio/presentation emulation.\n";
    for (const auto& request : result.tick.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << '\n';
}
void advance_observed_nmi_sequence_file(captain::PhaseSessionState& state, const std::string& path,
                                        std::uint8_t initial_index) {
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open observed NMI sequence file");
    const auto events = captain::load_observed_nmi_events(input);
    const auto result = captain::advance_observed_nmi_events(state, initial_index, events);
    state = result.state;
    std::cout << "Observed NMI sequence | final random index " << unsigned(result.random_index)
              << "; captured events supplied, no frame/audio/presentation emulation.\n";
}
void continue_area_special(captain::PhaseSessionState& state,const captain::ShotContent& content) {
    if(!content.area_defense_menu)throw std::invalid_argument("Special continuation requires private menu content");
    const auto result=captain::continue_keeper_area_special(state,*content.area_defense_menu);state=result.state;
    for(const auto& request:result.presentation)
        std::cout << "Presentation request " << unsigned(request.kind) << " | id " << unsigned(request.id) << " | slot " << unsigned(request.slot) << '\n';
}
void redraw_area_special(captain::PhaseSessionState& state,const captain::ShotContent& content) {
    if(!content.area_defense_menu)throw std::invalid_argument("Special redraw requires private menu content");
    const auto result=captain::redraw_keeper_area_special(state,*content.area_defense_menu);state=result.state;
    for(const auto& request:result.presentation)
        std::cout << "Presentation request " << unsigned(request.kind) << " | id " << unsigned(request.id) << " | slot " << unsigned(request.slot) << '\n';
}
void resolve_area_special(captain::PhaseSessionState& state,const captain::ShotContent& content) {
    const auto result=captain::inspect_keeper_area_special(state,content.field_players,content.defense);state=result.state;
    for(const auto& request:result.presentation)
        std::cout << "Presentation request " << unsigned(request.kind) << " | id " << unsigned(request.id) << " | slot " << unsigned(request.slot) << '\n';
}
void poll_area_special(captain::PhaseSessionState& state,const captain::ShotContent& content,unsigned pressed) {
    if(!content.area_defense_menu)throw std::invalid_argument("Special chooser requires private menu content");
    const auto result=captain::poll_keeper_area_special(state,static_cast<std::uint8_t>(pressed),*content.area_defense_menu);state=result.state;
    if(result.special_cursor_y)std::cout << "Special cursor y " << unsigned(*result.special_cursor_y) << '\n';
}
void open_area_special(captain::PhaseSessionState& state,const captain::ShotContent& content) {
    if(!content.area_defense_menu)throw std::invalid_argument("Special chooser requires private menu content");
    const auto result=captain::open_keeper_area_special(state,*content.area_defense_menu);state=result.state;
    for(const auto& request:result.presentation)
        std::cout << "Presentation request " << unsigned(request.kind) << " | id " << unsigned(request.id) << " | slot " << unsigned(request.slot) << '\n';
    if(result.special_cursor)std::cout << "Defensive cursor | tile " << unsigned(result.special_cursor->tile) << " | x " << unsigned(result.special_cursor->x) << " | attribute " << unsigned(result.special_cursor->attribute) << '\n';
}
void inspect_area_defense(const captain::PhaseSessionState& state,const captain::ShotContent& content) {
    if(!content.area_defense_menu)throw std::invalid_argument("Defensive availability requires private menu content");
    const auto option=captain::keeper_area_defense_special_option(state,*content.area_defense_menu);
    std::cout << "Special defensive option: ";
    if(option)std::cout << unsigned(*option);else std::cout << "none";
    std::cout << '\n';
}
void prepare_area_defense(captain::PhaseSessionState& state,const captain::ShotContent& content) {
    if(!content.area_defense_menu)throw std::invalid_argument("Defensive registration requires private menu content");
    const auto result=captain::prepare_keeper_area_defense(state,*content.area_defense_menu);state=result.state;
    for(const auto& request:result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << " | slot " << unsigned(request.slot) << '\n';
}
void poll_area_defense(captain::PhaseSessionState& state,const captain::ShotContent& content,unsigned pressed) {
    if(!content.area_defense_menu || !content.area_defense_menu->field_icons)throw std::invalid_argument("Defensive input requires version11 private icon content");
    const auto result=captain::poll_keeper_area_defense(state,static_cast<std::uint8_t>(pressed),*content.area_defense_menu);
    state=result.state;
    for(const auto& request:result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << " | slot " << unsigned(request.slot) << '\n';
}
void open_area_defense_prompt(captain::PhaseSessionState& state,const captain::ShotContent& content) {
    if(!content.area_defense_menu)throw std::invalid_argument("Defensive prompt requires version10 private menu content");
    const auto result=captain::open_keeper_area_defense_prompt(state,*content.area_defense_menu);
    state=result.state;
    for(const auto& request:result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << " | slot " << unsigned(request.slot) << '\n';
}
void open_keeper_area_menu(captain::PhaseSessionState& state,const captain::ShotContent& content,unsigned facing) {
    if(!content.action_menu)throw std::invalid_argument("Keeper-area menu requires version9 menu content");
    const auto result=captain::open_prepared_keeper_area_menu(state,static_cast<std::uint8_t>(facing),*content.action_menu);
    state=result.state;
    for(const auto& request:result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind) << " | id " << unsigned(request.id) << '\n';
}
void open_action_menu(captain::PhaseSessionState& state, const captain::ShotContent& content) {
    if (!content.action_menu) throw std::invalid_argument("Action menu requires version9 menu content");
    const bool pass_menu=state.phase==captain::FieldSessionPhase::area_pass_capture_action_menu || state.phase==captain::FieldSessionPhase::area_pass_blocked_action_menu;
    const auto result = pass_menu ? captain::open_area_pass_human_action_menu(state,*content.action_menu) :
        state.phase==captain::FieldSessionPhase::ordinary_return_contact_pending ? captain::open_ordinary_return_action_menu(state,*content.action_menu) :
        captain::open_human_action_menu(state, *content.action_menu);
    state = result.state;
    std::cout << "Opened human attacking menu; no game-frame/NMI work performed.\n";
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind)
                  << " | id " << unsigned(request.id) << '\n';
}
void menu_input(captain::PhaseSessionState& state, const captain::ShotContent& content, unsigned pressed) {
    if (!content.action_menu) throw std::invalid_argument("Action menu polling requires version9 menu content");
    const auto result = state.phase==captain::FieldSessionPhase::ordinary_return_action_menu ?
        captain::poll_ordinary_return_action_menu(state,static_cast<std::uint8_t>(pressed),*content.action_menu) :
        captain::poll_human_action_menu(state, static_cast<std::uint8_t>(pressed), *content.action_menu);
    state = result.state;
    std::cout << "Menu input | confirmed " << result.confirmed << '\n';
    for (const auto& request : result.presentation)
        std::cout << "Presentation request " << static_cast<unsigned>(request.kind)
                  << " | id " << unsigned(request.id) << '\n';
}
void shot_defenders_file(captain::PhaseSessionState& state, const captain::ShotContent& content,
                         const std::string& path) {
    if (!content.experience) throw std::invalid_argument("Shot defenses need experience content");
    captain::validate_phase_session(state);
    if (state.phase != captain::FieldSessionPhase::attack_prepared)
        throw std::invalid_argument("Shot defenses require an already prepared shot");
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open shot defense choices");
    std::vector<captain::FieldDefenseChoice> choices;
    // One row for each saved selected defender: action subtype random1 random2.
    // No caller-supplied roster, player ID, power or comparison coefficients.
    for (unsigned i = 0; i < state.current.field.selected_defenders; ++i) {
        unsigned action, subtype, first, second;
        if (!(input >> action >> subtype >> first >> second) ||
            action > 255 || subtype > 255 || first > 255 || second > 255)
            throw std::invalid_argument("Expected action subtype random1 random2 bytes per selected defender");
        captain::FieldDefenseChoice choice{};
        choice.player_id = state.scheduling->selected_ids[i];
        choice.action = static_cast<std::uint8_t>(action);
        choice.subtype = static_cast<std::uint8_t>(subtype);
        choice.comparison.random_first = static_cast<std::uint8_t>(first);
        choice.comparison.random_second = static_cast<std::uint8_t>(second);
        choices.push_back(choice);
    }
    input >> std::ws;
    if (input.peek() != std::char_traits<char>::eof() || input.bad())
        throw std::invalid_argument("Trailing or unreadable shot defense data");
    const auto result = captain::resolve_session_shot_defenders(state,choices,false,
        content.field_players,content.defense,content.field_comparison,
        *content.movement,*content.contact,*content.experience);
    state = result.state;
    std::cout << "Shot defenses | reached " << result.defenses.contacts.size()
              << " | result " << unsigned(result.defenses.result_code)
              << " | retained power " << state.action->power << '\n';
    if (result.terminal) std::cout << "Terminal rewards " << result.terminal->rewards.size()
        << " | contact elapsed " << unsigned(result.terminal->prelude->elapsed_step) << '\n';
    if (result.restart) std::cout << "Restart route " << static_cast<unsigned>(result.restart->route) << " pending\n";
}
void staged_defense(captain::PhaseSessionState& state, const captain::ShotContent& content,
                    const std::array<unsigned,4>& values) {
    captain::validate_phase_session(state);
    const auto cursor = state.shot_progress ? state.shot_progress->cursor : 0;
    if (!state.scheduling || cursor >= state.current.field.selected_defenders)
        throw std::invalid_argument("No selected defender remains");
    captain::FieldDefenseChoice choice{};
    choice.player_id = state.scheduling->selected_ids[cursor];
    choice.action = static_cast<std::uint8_t>(values[0]);choice.subtype = static_cast<std::uint8_t>(values[1]);
    choice.comparison.random_first = static_cast<std::uint8_t>(values[2]);
    choice.comparison.random_second = static_cast<std::uint8_t>(values[3]);
    const auto result = captain::step_session_shot_defender(state,choice,false,
        content.field_players,content.defense,content.field_comparison,*content.contact);
    state = result.state;
    std::cout << "Selected defender stepped | clock " << state.current.clock.remaining
              << " | result " << unsigned(state.shot_progress->result_code) << '\n';
}
void dispatch_shot(captain::PhaseSessionState& state, unsigned elapsed) {
    const auto result = captain::dispatch_session_shot(state,static_cast<std::uint8_t>(elapsed));
    state = result.state;
    std::cout << "Final shot clock applied; dispatch handlers remain pending.\n";
}
void resolve_shot_result(captain::PhaseSessionState& state, const captain::ShotContent& content) {
    if (!content.experience) throw std::invalid_argument("Selected dispatch needs experience content");
    const auto result = captain::resolve_session_selected_result(state,content.field_players,
        *content.movement,*content.experience);
    state = result.state;
    std::cout << "Selected result dispatched | advanced actors " << result.movement.players.size()
              << " | reward " << (result.reward ? unsigned(result.reward->award) : 0u)
              << " | possession sound request " << result.request_possession_sound << '\n';
}
void prepare_clearance_file(captain::PhaseSessionState& state, const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::invalid_argument("Unable to open private clearance content");
    captain::CaptureClearanceContent content{};
    for (auto& pair : content.destinations) for (auto& axis : pair) {
        unsigned value;
        if (!(input >> value) || value > 255) throw std::invalid_argument("Expected sixteen clearance coordinate bytes");
        axis=static_cast<std::uint8_t>(value);
    }
    unsigned distance;
    if (!(input >> distance) || distance > 255) throw std::invalid_argument("Expected clearance distance byte");
    content.distance_adjustment=static_cast<std::uint8_t>(distance);
    input >> std::ws;
    if (input.peek()!=std::char_traits<char>::eof() || input.bad())
        throw std::invalid_argument("Trailing or unreadable clearance data");
    const auto result=captain::prepare_session_capture_clearance(state,content);
    state=result.state;
    std::cout << "Capture clearance prepared | side music request " << result.request_side_music
              << " | cooperative task request " << unsigned(result.requested_task) << '\n';
}
void clearance_race_file(captain::PhaseSessionState& state,const captain::ShotContent& content,const std::string& path,bool area=false,bool sampled_pass=false,bool human_recovery=false,bool ordinary_free=false) {
    std::ifstream input(path);
    if(!input) throw std::invalid_argument("Unable to open private race content");
    captain::ClearanceRaceContent race{};
    for(auto& pair:race.speeds) for(auto& speed:pair) {
        unsigned value;
        if(!(input>>value) || value>65535) throw std::invalid_argument("Expected512 unsigned race speed words");
        speed=static_cast<std::uint16_t>(value);
    }
    input>>std::ws;
    if(input.peek()!=std::char_traits<char>::eof() || input.bad()) throw std::invalid_argument("Trailing or unreadable race data");
    if(ordinary_free) {
        if(!content.movement)throw std::invalid_argument("Missing movement content");
        state=captain::resolve_ordinary_free_pass_contest(state,content.field_players,*content.movement,race);
        std::cout << "Ordinary free pass contest | winner " << unsigned(state.current.field.ball_owner) << '\n';
        return;
    }
    if(human_recovery) {
        const auto result=captain::resolve_sampled_human_empty_pass_recovery(state,content.field_players,*content.movement,race);
        state=result.state;std::cout << "Human empty pass | timeout acknowledged; player " <<
            unsigned(result.recipient->ball_owner) << " recovered | possession sound request " << result.request_possession_sound << '\n';
        return;
    }
    if(sampled_pass) {
        state=captain::resolve_sampled_player3_pass_race(state,content.field_players,*content.movement,race);
        std::cout << "Player3 pass contest | player5 reached the ball; winner handling pending\n";
        return;
    }
    const auto result=area ? captain::resolve_session_area_action6_race(state,content.field_players,*content.movement,race)
                           : captain::resolve_session_clearance_race(state,content.field_players,*content.movement,race);
    state=result.state;
    std::cout << "Clearance race | rounds " << result.rounds << " | winner "
        << (result.winner?std::to_string(*result.winner):"none") << " | pending script " << unsigned(result.pending_script)
        << " | side music request " << result.request_side_music << '\n';
}
void prepare_basic_shot(captain::PhaseSessionState& state, const captain::ShotContent& content) {
    if (!content.action_menu) throw std::invalid_argument("Shot preparation requires version9 menu content");
    const auto result = captain::prepare_session_basic_shot(state, content.field_players, content.attack);
    state = result.state;
    std::cout << "Prepared ordinary shot | stamina " << state.current.field.roster[state.current.field.ball_owner].stamina
              << " | raw cost " << result.preparation.preparation.stamina_cost
              << " | power " << result.preparation.action_power.power
              << " | subtype " << unsigned(result.preparation.action_power.subtype)
              << " | move " << unsigned(result.preparation.preparation.move_effect) << '\n';
}

void prepare_kickoff_formation(captain::PhaseSessionState& state,const std::string& file,unsigned side) {
 std::ifstream in(file);std::string magic,token;
 if(!(in>>magic) || magic!="CMJKICKFORMATION1")throw std::invalid_argument("Expected private CMJKICKFORMATION1 content");
 captain::KickoffFormationContent content{};
 for(auto* table:{&content.on_ball,&content.off_ball})for(auto& cell:*table){
  if(!(in>>token))throw std::invalid_argument("Expected88 formation cells");
  cell=number(token);
 }
 if(in>>token || in.bad())throw std::invalid_argument("Trailing or unreadable kickoff formation data");
 state=captain::prepare_session_kickoff_formation(state,side,content);
}
captain::BallVelocityContent read_ball_velocity(const std::string& file) {
 std::ifstream in(file);std::string magic,extra;
 if(!(in>>magic) || magic!="CMJBALL1")throw std::invalid_argument("Expected private CMJBALL1 projection content");
 captain::BallVelocityContent velocity{};
 for(auto& word:velocity.quarter_wave){
  std::string token;unsigned value=0;
  if(!(in>>token))throw std::invalid_argument("Expected64projection words");
  const auto parsed=std::from_chars(token.data(),token.data()+token.size(),value);
  if(parsed.ec!=std::errc{} || parsed.ptr!=token.data()+token.size() || value>65535)
   throw std::invalid_argument("Expected unsigned projection word0..65535");
  word=value;
 }
 if(in>>extra)throw std::invalid_argument("Trailing ball projection content");
 if(in.bad())throw std::invalid_argument("Unable to read ball projection content");
 return velocity;
}
void begin_ball_flight(captain::PhaseSessionState& state,const captain::ShotContent& content,
 const std::string& file,unsigned low_x,unsigned low_y) {
 const auto velocity=read_ball_velocity(file);
 state=captain::begin_prepared_session_ball_flight(state,low_x,low_y,content.movement->geometry,velocity);
}
void begin_area_flight(captain::PhaseSessionState& state,const captain::ShotContent& content,
 const std::string& file,const std::array<unsigned,9>& v) {
 const auto velocity=read_ball_velocity(file);
 const captain::AreaFlightAcknowledgement ack{static_cast<std::uint8_t>(v[0]),static_cast<std::uint8_t>(v[1]),
  static_cast<std::uint8_t>(v[2]),static_cast<std::uint8_t>(v[3]),static_cast<std::uint8_t>(v[4]),
  static_cast<std::uint16_t>(v[5]|(v[6]<<8)),static_cast<std::uint16_t>(v[7]|(v[8]<<8))};
 state=captain::begin_prepared_area_action_flight(state,ack,content.movement->geometry,velocity,content.experience ? &*content.experience : nullptr);
}
void continue_area_action6(captain::PhaseSessionState& state,const std::string& file,unsigned facing,unsigned motion,const captain::ShotContent& shot_content) {
 std::ifstream in(file);std::string magic,token;
 if(!(in>>magic) || magic!="CMJAREA6")throw std::invalid_argument("Expected private CMJAREA6 destination content");
 captain::AreaAction6DestinationContent content{};
 for(auto& pair:content.positions)for(auto& value:pair){if(!(in>>token))throw std::invalid_argument("Expected16 destination bytes");value=number(token);}
 if(in>>token || in.bad())throw std::invalid_argument("Trailing or unreadable area action6 content");
 state=captain::continue_session_area_action6(state,facing,motion,content,shot_content.experience ? &*shot_content.experience : nullptr);
}
void compare_area_pass(captain::PhaseSessionState& state,const std::string& file,unsigned low,unsigned high,unsigned context) {
 std::ifstream in(file);std::string token;if(!(in>>token) || token!="CMJPASS1")throw std::invalid_argument("Expected private CMJPASS1 content");
 captain::AreaPassComparisonContent content{};std::array<std::uint8_t*,7> values{&content.range,&content.offset,&content.eligibility,
  &content.thresholds[0],&content.thresholds[1],&content.thresholds[2],&content.thresholds[3]};
 for(auto* value:values){if(!(in>>token))throw std::invalid_argument("Expected seven comparison bytes");*value=number(token);}
 if(in>>token || in.bad())throw std::invalid_argument("Trailing or unreadable area pass comparison content");
 state=captain::compare_area_pass_defender(state,static_cast<std::uint16_t>(low|(high<<8)),context,content);
}
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: captain-phase-session PACK STATE [--contact CODE | --rewarded-contact CODE] --defender ID; "
                     "or [--resume | --recover | --continue-recovery | --setup-control | --poll-control HELD PRESSED | --begin-sampled-human-pass-target | --begin-ordinary-human-pass-target | --confirm-ordinary-human-pass-target | --begin-ordinary-pass-defenses | --prepare-ordinary-pass | --prepare-first-ordinary-pass-defender VARIANT | --compare-first-ordinary-pass-defender RANGE OFFSET ELIGIBLE T0 T1 T2 T3 | --advance-first-ordinary-pass-contact | --prepare-next-ordinary-pass-defender | --compare-next-ordinary-pass-defender RANGE OFFSET ELIGIBLE T0 T1 T2 T3 | --advance-next-ordinary-pass-contact | --advance-ordinary-pass-dispatch-clock | --advance-ordinary-pass-ball-cell | --resolve-ordinary-pass-ball-cell | --continue-ordinary-pass-reception | --position-ordinary-pass-recipient | --continue-ordinary-pass-control FACING | --record-ordinary-return-shot-defense DEFENDER ACTION SUBTYPE | --prepare-ordinary-return-shot | --prepare-ordinary-return-shot-defender VARIANT | --compare-ordinary-return-shot-defender RANGE OFFSET ELIGIBLE T0 T1 T2 T3 | --advance-ordinary-return-shot-contact | --advance-ordinary-return-shot-dispatch | --prepare-ordinary-return-flight-defender | --compare-ordinary-return-flight-defender | --position-ordinary-return-blocked-shot | --advance-ordinary-return-blocked-shot-clock | --record-ordinary-pass-defense DEFENDER ACTION SUBTYPE | --poll-sampled-human-pass-target HELD PRESSED | --poll-ordinary-human-pass-cursor HELD PRESSED | --capture-sampled-score25-pass-defenses A0 A1 A2 S0 S1 S2 | --prepare-sampled-score25-player7-pass | --prepare-sampled-human-empty-pass | --prepare-sampled-player3-defender | --compare-sampled-player3-defender RANGE OFFSET ELIGIBLE T0 T1 T2 T3 | --advance-sampled-player3-contact | --advance-sampled-player3-dispatch | --capture-sampled-player3-pass-handoff FILE | --ack-sampled-human-empty-pass | --capture-sampled-human-pass-recovery FILE | "
                     "--advance-control-travel HELD SPEED_LOW SPEED_HIGH | --advance-profiled-control-travel HELD | --advance-control | --resume-control-time | --advance-world HELD PRESSED | --advance-world-file FILE | --advance-observed-world-file FILE | --advance-observed-nmi-world-file FILE INITIAL_INDEX | --advance-observed-nmi-poll FILE INITIAL_INDEX | --advance-observed-nmi-clock FILE INITIAL_INDEX | --advance-observed-nmi-sequence FILE INITIAL_INDEX | --advance-observed-field-movement FILE INITIAL_INDEX | --advance-observed-pass-movement FILE INITIAL_INDEX | --resolve-ordinary-pass-flight-target DISTANCE_TABLE | --advance-observed-ordinary-return KIND FILE INDEX | "
                     "--open-action-menu | --menu-input PRESSED | --prepare-basic-shot | --shot-defenders-file FILE | --step-shot-defense ACTION SUBTYPE RANDOM1 RANDOM2 | --resume-shot-clock | --dispatch-shot ELAPSED | --resolve-shot-result | --recover-selected-file FILE | --continue-selected-recovery | --prepare-clearance-file FILE | --clearance-race-file FILE | --continue-clearance | --begin-ball-flight FILE LOWX LOWY | --continue-flight-arrival | --recover-shot-loose | --prepare-kickoff-formation FILE SIDE | --prepare-kickoff-receiver RANDOM C1 C2 C3 C4 C5 | --prepare-kickoff-ball | --continue-prepared-kickoff | --continue-prepared-goal-restart | --prepare-scored-goal LEFT RIGHT KICKOFF | --resolve-keeper-save | --recover-keeper-punch | --poll-keeper-area-special PRESSED | --advance-profiled-area-prefix-clock | --advance-area-prefix-clock ELAPSED | --resume-area-prefix-clock | --present-area-contact-result | --advance-area-defender | --skip-area-defender | --advance-area-contact-clock | --resume-area-contact-clock | --resolve-area-defender | --dispatch-area-result | --recover-area-result | --announce-prepared-area-defenders FACING | --begin-prepared-area-defense | --prepare-opponent-area-attack | --choose-keeper-area-action | --continue-keeper-area-special | --redraw-keeper-area-special | --resolve-keeper-area-special | --open-keeper-area-special | --inspect-keeper-area-defense | --prepare-keeper-area-defense | --poll-keeper-area-defense PRESSED | --open-keeper-area-defense | --begin-keeper-area-defense BLINK | --open-prepared-keeper-area-menu FACING | --prepare-keeper-area | --prepare-keeper-entry | --resolve-prepared-keeper ACTION SUBTYPE OPPONENT_TEAM | --resolve-stunned-keeper T0 T1 | --resolve-ball-cell | --advance-ball-cell | --prepare-shot-approach | --continue-unopposed-attack ACTION | --continue-prepared-attack | --resume-attack-clock | --route-selected-restart | --open-prepared-restart-options | --restart-options-input HELD PRESSED | --advance N] "
                     "[--defense-action N] [--defense-subtype N] [--capture NEW.cmjsession] [--interactive]\n"
                     "Area recovery: --continue-area-ordinary-recovery FACING | --continue-area-story-recovery FACING\n"
                     "Player3 pass contest: --resolve-sampled-player3-pass-race FILE | --position-sampled-player3-pass-winner | --continue-sampled-player3-pass-winner\n"
                     "Calculated human pass recovery: --resolve-sampled-human-pass-recovery FILE | --resolve-ordinary-free-pass-contest FILE\n"
                     "Extended movement: --advance-sampled-player14-pass-handoff | --advance-sampled-player19-return-shot-approach | --advance-sampled-score23-pass-handoff\n"
                     "Player5 dribble: --capture-sampled-score24-dribble-defense ACTION SUBTYPE | --capture-sampled-player5-defense A0 A1 A2 A3 A4 S0 S1 S2 S3 S4 | --prepare-sampled-player5-dribble | --begin-sampled-player5-defenders | --prepare-sampled-player5-defender | --compare-sampled-player5-defender RANGE OFFSET ELIGIBLE T0 T1 T2 T3 | --advance-sampled-player5-terminal | --advance-sampled-player5-terminal-field | --transfer-sampled-player5-possession | --prepare-sampled-player5-result3-control\n"
                     "Area actions: --continue-area-action4 FACING MOTION | --continue-area-action6 FILE FACING MOTION | --continue-area-pass-received | --prepare-area-pass-keeper\n"
                     "AI: --ack-sampled-unaffordable-shot | --begin-sampled-opponent-goal-restart | --ack-sampled-opponent-goal | --prepare-sampled-opponent-goal | --compare-sampled-home-goalkeeper | --ack-sampled-keeper-overlay | --poll-sampled-goalkeeper PRESSED ICON0 ICON1 | --ack-sampled-mixed-result4-pass | --advance-sampled-mixed-result4 | --compare-sampled-mixed-defender | --prepare-sampled-mixed-defender | --prepare-sampled-field-contact-attack | --poll-sampled-field-defense PRESSED | --ack-sampled-field-defense-prompt BLINK | --ack-sampled-opponent-contact | --continue-sampled-field-pass-control | --begin-sampled-field-pass-flight FILE XLOW YLOW | --begin-ordinary-pass-flight FILE XLOW YLOW | --capture-sampled-field-pass-target RECIPIENT DEST COUNTER DISTANCE | --calculate-sampled-field-pass-target TABLE RECIPIENT DEST COUNTER | --resolve-sampled-field-pass-target TABLE RECIPIENT PREVIOUS_CELL COUNTER | --ack-sampled-field-ai-pass-handoff | --ack-sampled-field-ai-pass-presentation FILE | --advance-sampled-field-ai-dispatch | --ack-sampled-field-ai-presentation | --capture-sampled-field-ai-preparation SUBTYPE POWER | --capture-sampled-home-shot-handoff FILE | --advance-sampled-home-shot-dispatch | --advance-sampled-home-shot-contact | --resolve-sampled-home-shot-defender | --capture-sampled-home-shot-defender ACTION SUBTYPE | --capture-sampled-field-ai-choice ACTION SUBTYPE\n"
                     "Area flight: --recover-sampled-keeper-result4-action | --advance-sampled-keeper-result4-extra-time | --prepare-sampled-keeper-result4-action ACTION SUBTYPE CONTEXT COST POWER | --ack-sampled-keeper-result4-formation FILE | --route-sampled-keeper-result4 CARRY SIDE | --recover-sampled-loose-shot-result2 | --compare-sampled-loose-shot-defender | --prepare-sampled-loose-shot-defender | --continue-area-action4-keeper-result0-goal | --prepare-area-action4-keeper-result0-goal | --begin-area-action4-restart-flight FILE | --prepare-area-action4-restart-pass-control | --begin-area-flight FILE LX LY DIST FACING MOTION VXLO VXHI VYLO VYHI | --advance-area-ball-cell | --resolve-area-ball-cell | --continue-area-already-arrived | --ack-area-pass-defender | --prepare-area-pass-defender COSTLO COSTHI | --compare-area-pass-defender FILE POWERLO POWERHI CONTEXT | --begin-area-action4-encounters | --continue-area-action4-control | --prepare-area-action4-control | --ack-area-action4-restart-pass | --resume-area-action4-restart-clock | --dispatch-area-action4-restart | --advance-area-action4-restart-clock | --prepare-area-action4-restart-power | --compute-area-action4-restart-formation TABLE | --ack-area-action4-restart-formation FIELD | --prepare-area-action4-keeper-restart OWNER ACTION SUBTYPE CHOICE | --route-area-action4-keeper-restart | --advance-area-action4-keeper-jitter | --continue-area-action4-keeper-result ACTION | --ack-area-action4-keeper-result | --compare-area-action4-keeper POWER ACTION SUBTYPE ADJUSTMENT | --spend-area-action4-keeper-stamina | --begin-area-action4-keeper | --ack-area-action4-keeper FACING MOTION | --prepare-area-action4-keeper COSTLO COSTHI POWERLO POWERHI\n";
        return 2;
    }
    try {
        bool interactive = false, run_resume = false, defense_options = false, rewarded = false;
        bool run_recovery = false, run_continue_recovery = false;
        bool run_setup_control = false;
        bool run_control_tick = false, resume_control_time = false;
        bool run_open_action_menu = false, run_prepare_basic_shot = false, run_reject_sampled_shot = false, resume_shot_clock = false, run_selected_result = false, run_continue_selected = false, run_continue_clearance = false, run_restart_route = false, run_restart_options = false, run_attack_clock = false, run_resume_attack = false, run_approach = false, run_ball_step=false, run_ball_cell=false, run_arrival=false, run_shot_recovery=false, run_keeper_entry=false, run_keeper_save=false, run_punch_recovery=false, run_goal_restart=false, run_kickoff_ball=false, run_kickoff_handoff=false, run_keeper_area=false,run_area_defense_prompt=false,run_prepare_area_defense=false,run_inspect_area_defense=false,run_open_area_special=false,run_resolve_area_special=false,run_redraw_area_special=false,run_continue_area_special=false,run_area_ai=false,run_area_attack=false,run_area_defense_entry=false;
        std::optional<std::array<std::uint8_t,2>> stunned_thresholds;
        std::optional<std::array<std::uint8_t,3>> keeper_choice,goal_baseline;
        std::optional<std::string> flight_file,kickoff_file,area_flight_file,restart_flight_file,area_action6_file,area_action6_race_file,area_action4_restart_field,area_action4_restart_table,sampled_result4_formation_file;
        std::array<unsigned,2> area_action6_ack{};
        std::array<unsigned,9> area_flight_input{};
        std::optional<unsigned> area_announce_facing,area_ordinary_facing,area_story_facing;
        std::optional<std::array<unsigned,2>> area_pass_cost;
        std::optional<std::array<unsigned,2>> area_pass_result_ack;
        std::optional<std::array<unsigned,2>> area_pass_capture_ack;
        std::optional<std::array<unsigned,2>> area_pass_recovery_ack;
        std::optional<std::string> area_pass_compare_file;std::array<unsigned,3> area_pass_compare_values{};
        bool run_area_pass_restore=false,run_area_pass_nonterminal=false,run_area_pass_next=false,run_area_pass_blocked=false,run_area_pass_terminal=false,run_area_pass_capture=false,run_area_pass_capture_control=false,run_area_pass_resume=false,run_area_pass_recovery=false,run_area_pass_recover=false,run_area_pass_blocked_control=false;
        bool run_area_contact=false,run_area_clock=false,run_area_clock_resume=false;
        bool run_area_result_dispatch=false,run_area_result_recovery=false,run_area_ball_step=false,run_area_ball_cell=false,run_area_pass_received=false,run_area_pass_keeper=false,run_prepare_area_pass_keeper=false,run_area_action6_recover=false,run_area_action6_control=false,run_area_pass_announcement=false;
        std::optional<unsigned> area_sequence_operation,area_prefix_elapsed;
        bool resume_area_prefix=false,run_profiled_area_prefix=false,run_area_action4_field_entry=false,run_area_action4_control_entry=false,run_area_action4_control_ready=false,run_area_action4_keeper=false,run_area_action4_keeper_stamina=false,run_area_action4_keeper_result_ack=false,run_area_action4_keeper_jitter=false,run_area_action4_keeper_restart=false,run_area_action4_restart_power=false,run_area_action4_restart_clock=false,run_area_action4_restart_resume=false,run_area_action4_restart_dispatch=false,run_area_action4_restart_pass=false,run_area_action4_restart_pass_control=false,run_area_action4_result0_goal=false,run_area_action4_postgoal=false,run_sampled_loose_defender=false,run_compare_sampled_loose_defender=false,run_recover_sampled_loose_result2=false,run_sampled_result4_extra_time=false,run_recover_sampled_result4_action=false;
        unsigned kickoff_side=0;
        std::optional<std::array<std::uint8_t,6>> kickoff_receiver;
        std::array<unsigned,2> flight_fractions{};
        std::optional<std::array<unsigned,2>> restart_input,area_action4_ack,area_action4_keeper_ack,sampled_keeper_result4,area_action6_winner_ack,area_action6_recovery_ack;
        std::optional<std::array<unsigned,5>> sampled_result4_action;
        std::optional<std::array<unsigned,4>> area_action4_keeper_prep,area_action4_keeper_compare,area_action4_keeper_restart_prep;
        std::optional<unsigned> area_action4_keeper_result_action;
        std::optional<std::string> selected_recovery_file, clearance_file, race_file, player3_race_file, human_race_recovery_file;
        std::optional<std::string> ordinary_free_race_file;
        std::optional<std::array<unsigned,4>> staged_input;
        std::optional<unsigned> dispatch_elapsed, unopposed_attack;
        std::optional<std::array<unsigned,2>> sampled_field_ai,sampled_field_ai_prep,sampled_home_shot_defender,score24_dribble_defense;
        std::optional<std::array<unsigned,10>> player5_defense;
        std::optional<std::array<unsigned,7>> compare_player5_defender;
        bool prepare_player5_dribble=false,begin_player5_defenders=false,prepare_player5_defender=false,advance_player5_terminal=false,advance_player5_terminal_field=false,transfer_player5_possession=false,prepare_player5_result3_control=false,advance_player14_handoff=false,advance_player19_return_approach=false,advance_score23_handoff=false;
        std::optional<std::array<unsigned,3>> sampled_keeper_poll;
        bool run_sampled_contact_attack=false,run_sampled_home_shot_defender=false,run_sampled_home_shot_contact=false,run_sampled_home_shot_dispatch=false,run_sampled_mixed_defender=false,run_sampled_mixed_compare=false,run_sampled_mixed_result4=false,run_sampled_mixed_pass=false,run_keeper_overlay=false,run_sampled_home_keeper=false,run_sampled_opponent_goal=false,run_sampled_goal_presentation=false,run_sampled_goal_restart=false;
        bool sampled_field_ai_ack=false,sampled_field_ai_dispatch=false,sampled_field_ai_pass_handoff=false,sampled_field_pass_control=false,sampled_opponent_contact_ack=false;
        std::optional<unsigned> sampled_field_defense_blink;
        std::optional<std::string> sampled_field_ai_pass_field,sampled_home_shot_handoff_field,human_pass_recovery_field,player3_handoff_field;
        std::optional<std::array<unsigned,4>> sampled_field_pass_target;
        std::optional<std::string> sampled_pass_distance_file;bool sampled_resolve_pass_target=false;
        std::optional<std::string> sampled_field_pass_flight_file;std::array<unsigned,2> sampled_field_pass_fractions{};
        std::optional<std::string> ordinary_pass_flight_file;std::array<unsigned,2> ordinary_pass_fractions{};
        std::optional<unsigned> code, defender, elapsed;
        std::optional<unsigned> menu_pressed,area_menu_facing,area_defense_blink,area_defense_pressed,area_special_pressed;
        std::optional<std::array<unsigned, 2>> poll_input, poll_human_pass, poll_ordinary_pass;
        std::optional<std::array<unsigned, 3>> control_travel_input;
        std::optional<unsigned> profiled_control_travel_input;
        bool begin_human_pass=false, begin_ordinary_human_pass=false, confirm_ordinary_human_pass=false, begin_ordinary_defenses=false, prepare_ordinary_pass=false, prepare_human_pass=false, ack_human_pass=false, prepare_player3_defender=false, advance_player3_contact=false, advance_player3_dispatch=false, position_player3_winner=false, continue_player3_winner=false, prepare_score25_pass=false;
        std::optional<std::array<unsigned,3>> ordinary_defense;
        std::optional<unsigned> ordinary_defender_variant;
        std::optional<std::array<unsigned,7>> compare_first_ordinary_defender;
        bool advance_first_ordinary_contact=false;
        bool prepare_next_ordinary_defender=false, advance_next_ordinary_contact=false;
        bool advance_ordinary_dispatch_clock=false;
        bool advance_ordinary_ball=false, resolve_ordinary_ball=false;
        bool continue_ordinary_reception=false,position_ordinary_recipient=false;
        std::optional<unsigned> ordinary_control_facing;
        std::optional<std::array<unsigned,3>> ordinary_return_shot_defense;
        bool prepare_ordinary_return_shot=false;
        std::optional<unsigned> ordinary_return_shot_variant;
        std::optional<std::array<unsigned,7>> ordinary_return_shot_comparison;
        bool ordinary_return_shot_contact=false,ordinary_return_shot_dispatch=false;
        bool ordinary_return_flight_prepare=false,ordinary_return_flight_compare=false;
        bool ordinary_return_blocked_position=false,ordinary_return_blocked_clock=false;
        std::optional<std::array<unsigned,7>> compare_next_ordinary_defender;
        std::optional<std::array<unsigned,6>> score25_pass_defenses;
        std::optional<std::array<unsigned,7>> compare_player3_defender;
        std::optional<std::array<unsigned, 2>> world_input;
        std::optional<std::string> world_file, observed_world_file, observed_nmi_world_file, observed_nmi_poll_file, observed_nmi_clock_file, observed_nmi_sequence_file, shot_file;
        unsigned observed_nmi_initial_index = 0;
        unsigned observed_nmi_poll_index = 0;
        unsigned observed_nmi_clock_index = 0;
        unsigned observed_nmi_sequence_index = 0;
        std::optional<std::string> observed_pass_movement_file;
        unsigned observed_pass_movement_index=0;
        std::optional<std::string> ordinary_return_file;
        std::string ordinary_return_kind;
        unsigned ordinary_return_index=0;
        std::optional<std::string> ordinary_pass_distance_file;
        unsigned action = 0, subtype = 0;
        std::string capture;
        for (int i = 3; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--interactive") interactive = true;
            else if (option == "--resume") run_resume = true;
            else if (option == "--recover") run_recovery = true;
            else if (option == "--continue-recovery") run_continue_recovery = true;
            else if (option == "--continue-area-ordinary-recovery") {
                if(++i>=argc || area_ordinary_facing)throw std::invalid_argument("Expected captured facing");
                area_ordinary_facing=number(argv[i]);
            }
            else if (option == "--continue-area-story-recovery") {
                if(++i>=argc || area_story_facing)throw std::invalid_argument("Expected captured facing");
                area_story_facing=number(argv[i]);
            }
            else if (option == "--begin-area-flight") {
                if(i+10>=argc || area_flight_file)throw std::invalid_argument("Expected area flight file and nine bytes");
                area_flight_file=argv[++i];for(auto& v:area_flight_input)v=number(argv[++i]);
            }
            else if (option == "--begin-area-action4-restart-flight") { if(i+1>=argc || restart_flight_file)throw std::invalid_argument("Expected restart flight velocity file");restart_flight_file=argv[++i]; }
            else if (option == "--advance-area-ball-cell") {if(run_area_ball_step)throw std::invalid_argument("Repeated area ball step");run_area_ball_step=true;}
            else if (option == "--resolve-area-ball-cell") {if(run_area_ball_cell)throw std::invalid_argument("Repeated area ball callback");run_area_ball_cell=true;}
            else if (option == "--ack-area-pass-defender") {if(run_area_pass_announcement)throw std::invalid_argument("Repeated area defender announcement");run_area_pass_announcement=true;}
            else if (option == "--restore-area-pass-variant") {if(run_area_pass_restore)throw std::invalid_argument("Repeated area pass variant restoration");run_area_pass_restore=true;}
            else if (option == "--advance-area-pass-nonterminal") {if(run_area_pass_nonterminal)throw std::invalid_argument("Repeated pass nonterminal handler");run_area_pass_nonterminal=true;}
            else if (option == "--advance-area-pass-defender") {if(run_area_pass_next)throw std::invalid_argument("Repeated pass defender continuation");run_area_pass_next=true;}
            else if (option == "--advance-area-pass-blocked") {if(run_area_pass_blocked)throw std::invalid_argument("Repeated blocked pass handler");run_area_pass_blocked=true;}
            else if (option == "--advance-area-pass-terminal") {if(run_area_pass_terminal)throw std::invalid_argument("Repeated pass terminal clock");run_area_pass_terminal=true;}
            else if (option == "--enter-area-pass-blocked-recovery") {if(run_area_pass_recovery)throw std::invalid_argument("Repeated pass recovery entry");run_area_pass_recovery=true;}
            else if (option == "--recover-area-pass-blocked") {if(run_area_pass_recover)throw std::invalid_argument("Repeated blocked pass recovery");run_area_pass_recover=true;}
            else if (option == "--resume-area-pass-added-time") {if(run_area_pass_resume)throw std::invalid_argument("Repeated pass added-time resume");run_area_pass_resume=true;}
            else if (option == "--capture-area-pass-defender") {if(run_area_pass_capture)throw std::invalid_argument("Repeated pass capture");run_area_pass_capture=true;}
            else if (option == "--prepare-area-pass-capture-control") {if(run_area_pass_capture_control)throw std::invalid_argument("Repeated pass control setup");run_area_pass_capture_control=true;}
            else if (option == "--prepare-area-pass-blocked-control") {if(run_area_pass_blocked_control)throw std::invalid_argument("Repeated blocked pass control setup");run_area_pass_blocked_control=true;}
            else if (option == "--continue-area-pass-capture") {
                if(i+2>=argc || area_pass_capture_ack)throw std::invalid_argument("Expected captured facing and motion");
                const auto facing=number(argv[++i]);const auto motion=number(argv[++i]);area_pass_capture_ack=std::array<unsigned,2>{facing,motion};
            }
            else if (option == "--continue-area-pass-blocked-recovery") {
                if(i+2>=argc || area_pass_recovery_ack)throw std::invalid_argument("Expected captured facing and motion");
                const auto facing=number(argv[++i]);const auto motion=number(argv[++i]);area_pass_recovery_ack=std::array<unsigned,2>{facing,motion};
            }
            else if (option == "--ack-area-pass-result") {
                if(i+2>=argc || area_pass_result_ack)throw std::invalid_argument("Expected captured facing and motion");
                const auto facing=number(argv[++i]);const auto motion=number(argv[++i]);area_pass_result_ack=std::array<unsigned,2>{facing,motion};
            }
            else if (option == "--prepare-area-pass-defender") {
                if(i+2>=argc || area_pass_cost)throw std::invalid_argument("Expected captured cost low and high bytes");
                const auto low=number(argv[++i]);const auto high=number(argv[++i]);area_pass_cost=std::array<unsigned,2>{low,high};
            }
            else if (option == "--compare-area-pass-defender") {
                if(i+4>=argc || area_pass_compare_file)throw std::invalid_argument("Expected comparison file, power low/high and context");
                area_pass_compare_file=argv[++i];for(auto& v:area_pass_compare_values)v=number(argv[++i]);
            }
            else if (option == "--continue-area-already-arrived" || option == "--continue-area-unopposed-arrival") {if(run_area_pass_received)throw std::invalid_argument("Repeated area pass handoff");run_area_pass_received=true;}
            else if (option == "--continue-area-pass-received") {if(run_area_pass_keeper)throw std::invalid_argument("Repeated area pass keeper entry");run_area_pass_keeper=true;}
            else if (option == "--prepare-area-pass-keeper") {if(run_prepare_area_pass_keeper)throw std::invalid_argument("Repeated area pass keeper setup");run_prepare_area_pass_keeper=true;}
            else if (option == "--continue-area-action4") {
                if(i+2>=argc || area_action4_ack)throw std::invalid_argument("Expected captured facing and motion");
                const auto facing=number(argv[++i]);const auto motion=number(argv[++i]);area_action4_ack=std::array<unsigned,2>{facing,motion};
            }
            else if (option == "--begin-area-action4-encounters") {if(run_area_action4_field_entry)throw std::invalid_argument("Repeated action4 field entry");run_area_action4_field_entry=true;}
            else if (option == "--continue-area-action4-control") {if(run_area_action4_control_entry)throw std::invalid_argument("Repeated action4 control entry");run_area_action4_control_entry=true;}
            else if (option == "--prepare-area-action4-control") {if(run_area_action4_control_ready)throw std::invalid_argument("Repeated action4 control setup");run_area_action4_control_ready=true;}
            else if (option == "--resume-area-action4-restart-clock") {if(run_area_action4_restart_resume)throw std::invalid_argument("Repeated restart added-time resume");run_area_action4_restart_resume=true;}
            else if (option == "--recover-sampled-loose-shot-result2") {if(run_recover_sampled_loose_result2)throw std::invalid_argument("Repeated loose-shot recovery entry");run_recover_sampled_loose_result2=true;}
            else if (option == "--compare-sampled-loose-shot-defender") {if(run_compare_sampled_loose_defender)throw std::invalid_argument("Repeated loose-shot comparison");run_compare_sampled_loose_defender=true;}
            else if (option == "--prepare-sampled-loose-shot-defender") {if(run_sampled_loose_defender)throw std::invalid_argument("Repeated loose-shot defender preparation");run_sampled_loose_defender=true;}
            else if (option == "--continue-area-action4-keeper-result0-goal") {if(run_area_action4_postgoal)throw std::invalid_argument("Repeated post-goal handoff");run_area_action4_postgoal=true;}
            else if (option == "--prepare-area-action4-keeper-result0-goal") {if(run_area_action4_result0_goal)throw std::invalid_argument("Repeated result0 goal");run_area_action4_result0_goal=true;}
            else if (option == "--prepare-area-action4-restart-pass-control") {if(run_area_action4_restart_pass_control)throw std::invalid_argument("Repeated restart pass control");run_area_action4_restart_pass_control=true;}
            else if (option == "--ack-area-action4-restart-pass") {if(run_area_action4_restart_pass)throw std::invalid_argument("Repeated restart pass acknowledgment");run_area_action4_restart_pass=true;}
            else if (option == "--dispatch-area-action4-restart") {if(run_area_action4_restart_dispatch)throw std::invalid_argument("Repeated restart action dispatch");run_area_action4_restart_dispatch=true;}
            else if (option == "--advance-area-action4-restart-clock") {if(run_area_action4_restart_clock)throw std::invalid_argument("Repeated restart clock advance");run_area_action4_restart_clock=true;}
            else if (option == "--prepare-area-action4-restart-power") {if(run_area_action4_restart_power)throw std::invalid_argument("Repeated restart power preparation");run_area_action4_restart_power=true;}
            else if (option == "--compute-area-action4-restart-formation") {
                if(i+1>=argc || area_action4_restart_table)throw std::invalid_argument("Expected private restart formation table");
                area_action4_restart_table=argv[++i];
            }
            else if (option == "--ack-area-action4-restart-formation") {
                if(i+1>=argc || area_action4_restart_field)throw std::invalid_argument("Expected captured restart field file");
                area_action4_restart_field=argv[++i];
            }
            else if (option == "--recover-sampled-keeper-result4-action") {if(run_recover_sampled_result4_action)throw std::invalid_argument("Repeated result4 recovery");run_recover_sampled_result4_action=true;}
            else if (option == "--advance-sampled-keeper-result4-extra-time") {if(run_sampled_result4_extra_time)throw std::invalid_argument("Repeated result4 extra-time update");run_sampled_result4_extra_time=true;}
            else if (option == "--prepare-sampled-keeper-result4-action") {
                if(i+5>=argc || sampled_result4_action)throw std::invalid_argument("Expected captured action, subtype, context, cost and power");
                std::array<unsigned,5> values{};for(auto& v:values)v=number(argv[++i]);
                if(values[0]>255 || values[1]>255 || values[2]>255 || values[3]>65535 || values[4]>65535)throw std::invalid_argument("Invalid captured action values");
                sampled_result4_action=values;
            }
            else if (option == "--ack-sampled-keeper-result4-formation") { if(i+1>=argc || sampled_result4_formation_file)throw std::invalid_argument("Expected captured result4 formation file");sampled_result4_formation_file=argv[++i]; }
            else if (option == "--route-sampled-keeper-result4") {
                if(i+2>=argc || sampled_keeper_result4)throw std::invalid_argument("Expected captured carry and side");
                const auto carry=number(argv[++i]),side=number(argv[++i]);if(carry>1 || side>255)throw std::invalid_argument("Invalid captured branch");
                sampled_keeper_result4=std::array<unsigned,2>{carry,side};
            }
            else if (option == "--prepare-area-action4-keeper-restart") {
                if(i+4>=argc || area_action4_keeper_restart_prep)throw std::invalid_argument("Expected captured restart recipient, action, subtype, choice");
                std::array<unsigned,4> values{};for(auto& value:values){value=number(argv[++i]);if(value>255)throw std::invalid_argument("Invalid captured restart value");}area_action4_keeper_restart_prep=values;
            }
            else if (option == "--route-area-action4-keeper-restart") {if(run_area_action4_keeper_restart)throw std::invalid_argument("Repeated keeper restart route");run_area_action4_keeper_restart=true;}
            else if (option == "--advance-area-action4-keeper-jitter") {if(run_area_action4_keeper_jitter)throw std::invalid_argument("Repeated keeper jitter");run_area_action4_keeper_jitter=true;}
            else if (option == "--continue-area-action4-keeper-result") {
                if(i+1>=argc || area_action4_keeper_result_action)throw std::invalid_argument("Expected captured attack action");
                const auto action=number(argv[++i]);if(action>255)throw std::invalid_argument("Invalid captured attack action");area_action4_keeper_result_action=action;
            }
            else if (option == "--ack-area-action4-keeper-result") {if(run_area_action4_keeper_result_ack)throw std::invalid_argument("Repeated keeper result acknowledgment");run_area_action4_keeper_result_ack=true;}
            else if (option == "--spend-area-action4-keeper-stamina") {if(run_area_action4_keeper_stamina)throw std::invalid_argument("Repeated keeper stamina");run_area_action4_keeper_stamina=true;}
            else if (option == "--compare-area-action4-keeper") {
                if(i+4>=argc || area_action4_keeper_compare)throw std::invalid_argument("Expected captured attack power, action, subtype, adjustment");
                std::array<unsigned,4> values{};for(auto& value:values)value=number(argv[++i]);area_action4_keeper_compare=values;
                if(values[0]>65535 || values[1]>255 || values[2]>255 || values[3]>255)throw std::invalid_argument("Invalid captured keeper comparison input");
            }
            else if (option == "--begin-area-action4-keeper") {if(run_area_action4_keeper)throw std::invalid_argument("Repeated action4 keeper entry");run_area_action4_keeper=true;}
            else if (option == "--ack-area-action4-keeper") {
                if(i+2>=argc || area_action4_keeper_ack)throw std::invalid_argument("Expected captured keeper facing and motion");
                const auto facing=number(argv[++i]);const auto motion=number(argv[++i]);area_action4_keeper_ack=std::array<unsigned,2>{facing,motion};
            }
            else if (option == "--prepare-area-action4-keeper") {
                if(i+4>=argc || area_action4_keeper_prep)throw std::invalid_argument("Expected captured keeper cost and power bytes");
                std::array<unsigned,4> values{};for(auto& value:values)value=number(argv[++i]);area_action4_keeper_prep=values;
            }
            else if (option == "--continue-area-action6") {
                if(i+3>=argc || area_action6_file)throw std::invalid_argument("Expected content file, facing and motion");
                area_action6_file=argv[++i];area_action6_ack[0]=number(argv[++i]);area_action6_ack[1]=number(argv[++i]);
            }
            else if (option == "--continue-area-action6-winner") {
                if(i+2>=argc || area_action6_winner_ack)throw std::invalid_argument("Expected captured facing and motion");
                const auto facing=number(argv[++i]);const auto motion=number(argv[++i]);area_action6_winner_ack=std::array<unsigned,2>{facing,motion};
            }
            else if (option == "--continue-area-action6-recovery") {
                if(i+2>=argc || area_action6_recovery_ack)throw std::invalid_argument("Expected captured facing and motion");
                const auto facing=number(argv[++i]);const auto motion=number(argv[++i]);area_action6_recovery_ack=std::array<unsigned,2>{facing,motion};
            }
            else if (option == "--area-action6-race-file") {
                if(++i>=argc || area_action6_race_file)throw std::invalid_argument("Expected private area action6 race file");
                area_action6_race_file=argv[i];
            }
            else if (option == "--recover-area-action6") {if(run_area_action6_recover)throw std::invalid_argument("Repeated area action6 recovery");run_area_action6_recover=true;}
            else if (option == "--prepare-area-action6-control") {if(run_area_action6_control)throw std::invalid_argument("Repeated area action6 control setup");run_area_action6_control=true;}
            else if (option == "--setup-control") run_setup_control = true;
            else if (option == "--advance-control") run_control_tick = true;
            else if (option == "--resume-control-time") resume_control_time = true;
            else if (option == "--open-action-menu") run_open_action_menu = true;
            else if (option == "--prepare-basic-shot") run_prepare_basic_shot = true;
            else if (option == "--ack-sampled-unaffordable-shot") run_reject_sampled_shot = true;
            else if (option == "--begin-sampled-human-pass-target") begin_human_pass = true;
            else if (option == "--begin-ordinary-human-pass-target") begin_ordinary_human_pass = true;
            else if (option == "--confirm-ordinary-human-pass-target") confirm_ordinary_human_pass = true;
            else if (option == "--begin-ordinary-pass-defenses") begin_ordinary_defenses = true;
            else if (option == "--prepare-ordinary-pass") prepare_ordinary_pass = true;
            else if (option == "--prepare-first-ordinary-pass-defender") {
                if (++i >= argc || ordinary_defender_variant) throw std::invalid_argument("Expected one observed ball variant");
                ordinary_defender_variant=number(argv[i]);
            }
            else if (option == "--compare-first-ordinary-pass-defender") {
                if (i+7>=argc || compare_first_ordinary_defender) throw std::invalid_argument("Expected direct ordinary pass comparison content");
                std::array<unsigned,7> values{};
                for (auto& value : values) { value=number(argv[++i]); if (value>255) throw std::invalid_argument("Comparison content must be bytes"); }
                if (values[2]>1) throw std::invalid_argument("Comparison eligibility must be zero or one");
                compare_first_ordinary_defender=values;
            }
            else if (option == "--advance-first-ordinary-pass-contact") advance_first_ordinary_contact=true;
            else if (option == "--prepare-next-ordinary-pass-defender") prepare_next_ordinary_defender=true;
            else if (option == "--advance-next-ordinary-pass-contact") advance_next_ordinary_contact=true;
            else if (option == "--advance-ordinary-pass-dispatch-clock") advance_ordinary_dispatch_clock=true;
            else if (option == "--advance-ordinary-pass-ball-cell") advance_ordinary_ball=true;
            else if (option == "--resolve-ordinary-pass-ball-cell") resolve_ordinary_ball=true;
            else if (option == "--continue-ordinary-pass-reception") continue_ordinary_reception=true;
            else if (option == "--position-ordinary-pass-recipient") position_ordinary_recipient=true;
            else if (option == "--continue-ordinary-pass-control") {
                if (++i>=argc || ordinary_control_facing) throw std::invalid_argument("Expected observed pass-control facing");
                ordinary_control_facing=number(argv[i]);
                if (*ordinary_control_facing>255) throw std::invalid_argument("Facing must be a byte");
            }
            else if (option == "--record-ordinary-return-shot-defense") {
                if(i+3>=argc || ordinary_return_shot_defense)throw std::invalid_argument("Expected defender, action and subtype");
                std::array<unsigned,3> values{};for(auto& value:values){value=number(argv[++i]);if(value>255)throw std::invalid_argument("Defense values must be bytes");}
                ordinary_return_shot_defense=values;
            }
            else if (option == "--prepare-ordinary-return-shot") prepare_ordinary_return_shot=true;
            else if (option == "--prepare-ordinary-return-shot-defender") {
                if(++i>=argc || ordinary_return_shot_variant)throw std::invalid_argument("Expected observed shot ball variant");
                ordinary_return_shot_variant=number(argv[i]);if(*ordinary_return_shot_variant>255)throw std::invalid_argument("Ball variant must be a byte");
            }
            else if (option == "--compare-ordinary-return-shot-defender") {
                if(i+7>=argc || ordinary_return_shot_comparison)throw std::invalid_argument("Expected direct shot comparison content");
                std::array<unsigned,7> values{};for(auto& value:values){value=number(argv[++i]);if(value>255)throw std::invalid_argument("Comparison content must be bytes");}
                if(values[2]>1)throw std::invalid_argument("Comparison eligibility must be zero or one");
                ordinary_return_shot_comparison=values;
            }
            else if (option == "--advance-ordinary-return-shot-contact") ordinary_return_shot_contact=true;
            else if (option == "--advance-ordinary-return-shot-dispatch") ordinary_return_shot_dispatch=true;
            else if (option == "--prepare-ordinary-return-flight-defender") ordinary_return_flight_prepare=true;
            else if (option == "--compare-ordinary-return-flight-defender") ordinary_return_flight_compare=true;
            else if (option == "--position-ordinary-return-blocked-shot") ordinary_return_blocked_position=true;
            else if (option == "--advance-ordinary-return-blocked-shot-clock") ordinary_return_blocked_clock=true;
            else if (option == "--compare-next-ordinary-pass-defender") {
                if (i+7>=argc || compare_next_ordinary_defender) throw std::invalid_argument("Expected direct ordinary pass comparison content");
                std::array<unsigned,7> values{};
                for (auto& value : values) { value=number(argv[++i]); if (value>255) throw std::invalid_argument("Comparison content must be bytes"); }
                if (values[2]>1) throw std::invalid_argument("Comparison eligibility must be zero or one");
                compare_next_ordinary_defender=values;
            }
            else if (option == "--record-ordinary-pass-defense") {
                if (i + 3 >= argc || ordinary_defense) throw std::invalid_argument("Expected one defender, action and subtype");
                std::array<unsigned,3> values{}; for (auto& value : values) value=number(argv[++i]);
                ordinary_defense=values;
            }
            else if (option == "--prepare-sampled-human-empty-pass") prepare_human_pass = true;
            else if (option == "--prepare-sampled-score25-player7-pass") prepare_score25_pass = true;
            else if (option == "--capture-sampled-score25-pass-defenses") {
                if(i+6>=argc || score25_pass_defenses)throw std::invalid_argument("Expected three defender actions and three subtypes");
                std::array<unsigned,6> values{};for(auto& value:values)value=number(argv[++i]);score25_pass_defenses=values;
            }
            else if (option == "--prepare-sampled-player3-defender") prepare_player3_defender = true;
            else if (option == "--advance-sampled-player3-contact") advance_player3_contact = true;
            else if (option == "--advance-sampled-player3-dispatch") advance_player3_dispatch = true;
            else if (option == "--position-sampled-player3-pass-winner") position_player3_winner = true;
            else if (option == "--continue-sampled-player3-pass-winner") continue_player3_winner = true;
            else if (option == "--capture-sampled-player5-defense") {
                if(i+10>=argc || player5_defense)throw std::invalid_argument("Expected five defender actions and five subtypes");
                std::array<unsigned,10> values{};for(auto& value:values)value=number(argv[++i]);
                player5_defense=values;
            }
            else if (option == "--capture-sampled-score24-dribble-defense") {
                if(i+2>=argc || score24_dribble_defense)throw std::invalid_argument("Expected one 2-4 defender action and subtype");
                std::array<unsigned,2> values{};for(auto& value:values){value=number(argv[++i]);if(value>255)throw std::invalid_argument("Defender choice must be bytes");}
                score24_dribble_defense=values;
            }
            else if (option == "--prepare-sampled-player5-dribble") prepare_player5_dribble=true;
            else if (option == "--begin-sampled-player5-defenders") begin_player5_defenders=true;
            else if (option == "--prepare-sampled-player5-defender") prepare_player5_defender=true;
            else if (option == "--advance-sampled-player5-terminal") advance_player5_terminal=true;
            else if (option == "--advance-sampled-player5-terminal-field") advance_player5_terminal_field=true;
            else if (option == "--transfer-sampled-player5-possession") transfer_player5_possession=true;
            else if (option == "--prepare-sampled-player5-result3-control") prepare_player5_result3_control=true;
            else if (option == "--advance-sampled-player14-pass-handoff") advance_player14_handoff=true;
            else if (option == "--advance-sampled-score23-pass-handoff") advance_score23_handoff=true;
            else if (option == "--advance-sampled-player19-return-shot-approach") advance_player19_return_approach=true;
            else if (option == "--compare-sampled-player5-defender") {
                if(i+7>=argc || compare_player5_defender)throw std::invalid_argument("Expected direct player5 comparison content");
                std::array<unsigned,7> values{};for(auto& value:values)value=number(argv[++i]);
                if(values[2]>1)throw std::invalid_argument("Comparison eligibility must be zero or one");
                compare_player5_defender=values;
            }
            else if (option == "--capture-sampled-player3-pass-handoff") {
                if(++i>=argc || player3_handoff_field)throw std::invalid_argument("Expected captured player3 pass handoff field");
                player3_handoff_field=argv[i];
            }
            else if (option == "--resolve-sampled-player3-pass-race") {
                if(++i>=argc || player3_race_file)throw std::invalid_argument("Expected private player3 race content");
                player3_race_file=argv[i];
            }
            else if (option == "--resolve-sampled-human-pass-recovery") {
                if(++i>=argc || human_race_recovery_file)throw std::invalid_argument("Expected private human pass race content");
                human_race_recovery_file=argv[i];
            }
            else if (option == "--resolve-ordinary-free-pass-contest") {
                if(++i>=argc || ordinary_free_race_file)throw std::invalid_argument("Expected private free-pass race content");
                ordinary_free_race_file=argv[i];
            }
            else if (option == "--compare-sampled-player3-defender") {
                if(i+7>=argc || compare_player3_defender)throw std::invalid_argument("Expected direct player3 comparison content");
                std::array<unsigned,7> values{};for(auto& value:values){value=number(argv[++i]);if(value>255)throw std::invalid_argument("Comparison content must be bytes");}
                if(values[2]>1)throw std::invalid_argument("Comparison eligibility must be zero or one");
                compare_player3_defender=values;
            }
            else if (option == "--ack-sampled-human-empty-pass") ack_human_pass = true;
            else if (option == "--capture-sampled-human-pass-recovery") {
                if(++i>=argc || human_pass_recovery_field)throw std::invalid_argument("Expected captured human pass recovery field");
                human_pass_recovery_field=argv[i];
            }
            else if (option == "--resume-shot-clock") resume_shot_clock = true;
            else if (option == "--resolve-shot-result") run_selected_result = true;
            else if (option == "--continue-selected-recovery") run_continue_selected = true;
            else if (option == "--continue-clearance") run_continue_clearance = true;
            else if (option == "--continue-unopposed-attack") {
                if (++i>=argc || unopposed_attack) throw std::invalid_argument("Expected one unopposed defending action");
                unopposed_attack=number(argv[i]);
            }
            else if (option == "--begin-ball-flight") {
                if(i+3>=argc || flight_file)throw std::invalid_argument("Expected begin-ball-flight FILE LOWX LOWY");
                flight_file=argv[++i];flight_fractions[0]=number(argv[++i]);flight_fractions[1]=number(argv[++i]);
            }
            else if (option == "--prepare-kickoff-formation") {
                if(i+2>=argc || kickoff_file)throw std::invalid_argument("Expected formation file and side");
                kickoff_file=argv[++i];kickoff_side=number(argv[++i]);
            }
            else if (option == "--continue-prepared-kickoff") run_kickoff_handoff=true;
            else if (option == "--prepare-kickoff-ball") run_kickoff_ball=true;
            else if (option == "--prepare-kickoff-receiver") {
                if(i+6>=argc || kickoff_receiver)throw std::invalid_argument("Expected random and five candidate slots");
                std::array<std::uint8_t,6> values{};
                for(auto& value:values)value=static_cast<std::uint8_t>(number(argv[++i]));
                kickoff_receiver=values;
            }
            else if (option == "--continue-prepared-goal-restart")run_goal_restart=true;
            else if (option == "--prepare-scored-goal") {
                if(i+3>=argc || goal_baseline)throw std::invalid_argument("Expected left score, right score and kickoff bytes");
                const auto left=number(argv[++i]),right=number(argv[++i]),kickoff=number(argv[++i]);
                goal_baseline=std::array<std::uint8_t,3>{static_cast<std::uint8_t>(left),static_cast<std::uint8_t>(right),static_cast<std::uint8_t>(kickoff)};
            }
            else if (option == "--resolve-keeper-save")run_keeper_save=true;
            else if (option == "--recover-keeper-punch")run_punch_recovery=true;
            else if (option == "--resolve-prepared-keeper") {
                if(i+3>=argc || keeper_choice)throw std::invalid_argument("Expected keeper action, subtype and opponent team");
                const auto action=number(argv[++i]),subtype=number(argv[++i]),team=number(argv[++i]);
                keeper_choice=std::array<std::uint8_t,3>{static_cast<std::uint8_t>(action),static_cast<std::uint8_t>(subtype),static_cast<std::uint8_t>(team)};
            }
            else if ((option == "--poll-keeper-area-defense" || option == "--poll-sampled-field-defense")) {
                if(++i>=argc || area_defense_pressed)throw std::invalid_argument("Expected sampled pressed byte");
                area_defense_pressed=number(argv[i]);
            }
            else if (option == "--poll-keeper-area-special") {
                if(++i>=argc || area_special_pressed)throw std::invalid_argument("Expected sampled pressed byte");
                area_special_pressed=number(argv[i]);
            }
            else if (option=="--advance-profiled-area-prefix-clock") {if(run_profiled_area_prefix)throw std::invalid_argument("Repeated area prefix clock");run_profiled_area_prefix=true;}
            else if (option=="--advance-area-prefix-clock") {if(++i>=argc || area_prefix_elapsed)throw std::invalid_argument("Expected area prefix elapsed byte");area_prefix_elapsed=number(argv[i]);}
            else if (option=="--resume-area-prefix-clock") {if(resume_area_prefix)throw std::invalid_argument("Repeated prefix resume");resume_area_prefix=true;}
            else if (option=="--present-area-contact-result" || option=="--advance-area-defender" || option=="--skip-area-defender") {
                if(area_sequence_operation)throw std::invalid_argument("Repeated area sequence operation");
                area_sequence_operation=option=="--present-area-contact-result"?1u:option=="--advance-area-defender"?2u:3u;
            }
            else if (option == "--advance-area-contact-clock") {if(run_area_clock)throw std::invalid_argument("Repeated area clock");run_area_clock=true;}
            else if (option == "--resume-area-contact-clock") {if(run_area_clock_resume)throw std::invalid_argument("Repeated area clock resume");run_area_clock_resume=true;}
            else if (option == "--resolve-area-defender") {if(run_area_contact)throw std::invalid_argument("Repeated area contact");run_area_contact=true;}
            else if (option == "--dispatch-area-result") {if(run_area_result_dispatch)throw std::invalid_argument("Repeated area result dispatch");run_area_result_dispatch=true;}
            else if (option == "--recover-area-result") {if(run_area_result_recovery)throw std::invalid_argument("Repeated area result recovery");run_area_result_recovery=true;}
            else if (option == "--announce-prepared-area-defenders") {
                if(++i>=argc || area_announce_facing)throw std::invalid_argument("Expected captured facing");
                area_announce_facing=number(argv[i]);
            }
            else if (option == "--begin-prepared-area-defense")run_area_defense_entry=true;
            else if (option == "--prepare-opponent-area-attack")run_area_attack=true;
            else if (option == "--choose-keeper-area-action")run_area_ai=true;
            else if (option == "--continue-keeper-area-special")run_continue_area_special=true;
            else if (option == "--redraw-keeper-area-special")run_redraw_area_special=true;
            else if (option == "--resolve-keeper-area-special")run_resolve_area_special=true;
            else if (option == "--open-keeper-area-special")run_open_area_special=true;
            else if (option == "--inspect-keeper-area-defense")run_inspect_area_defense=true;
            else if (option == "--prepare-keeper-area-defense")run_prepare_area_defense=true;
            else if (option == "--open-keeper-area-defense")run_area_defense_prompt=true;
            else if (option == "--begin-keeper-area-defense") {
                if(++i>=argc || area_defense_blink)throw std::invalid_argument("Expected captured blink counter");
                area_defense_blink=number(argv[i]);
            }
            else if (option == "--open-prepared-keeper-area-menu") {
                if(++i>=argc || area_menu_facing)throw std::invalid_argument("Expected captured facing");
                area_menu_facing=number(argv[i]);
            }
            else if (option == "--prepare-keeper-area")run_keeper_area=true;
            else if (option == "--prepare-keeper-entry") run_keeper_entry=true;
            else if (option == "--resolve-stunned-keeper") {
                if(i+2>=argc || stunned_thresholds)throw std::invalid_argument("Expected two stunned keeper thresholds");
                const auto t0=number(argv[++i]),t1=number(argv[++i]);
                stunned_thresholds=std::array<std::uint8_t,2>{static_cast<std::uint8_t>(t0),static_cast<std::uint8_t>(t1)};
            }
            else if (option == "--continue-flight-arrival") run_arrival=true;
            else if (option == "--recover-shot-loose") run_shot_recovery=true;
            else if (option == "--resolve-ball-cell") run_ball_cell=true;
            else if (option == "--advance-ball-cell") run_ball_step=true;
            else if (option == "--prepare-shot-approach") run_approach=true;
            else if (option == "--continue-prepared-attack") run_attack_clock = true;
            else if (option == "--resume-attack-clock") run_resume_attack = true;
            else if (option == "--route-selected-restart") run_restart_route = true;
            else if (option == "--open-prepared-restart-options") run_restart_options = true;
            else if (option == "--restart-options-input") {
                if (i+2 >= argc || restart_input) throw std::invalid_argument("Expected two restart input bytes");
                const auto held=number(argv[++i]); const auto pressed=number(argv[++i]);
                restart_input=std::array<unsigned,2>{held,pressed};
            }
            else if (option == "--clearance-race-file") {
                if(++i>=argc || race_file) throw std::invalid_argument("Expected one race content file");
                race_file=argv[i];
            }
            else if (option == "--prepare-clearance-file") {
                if (++i >= argc || clearance_file) throw std::invalid_argument("Expected one clearance content file");
                clearance_file=argv[i];
            }
            else if (option == "--recover-selected-file") {
                if (++i >= argc || selected_recovery_file) throw std::invalid_argument("Expected one selected recovery file");
                selected_recovery_file = argv[i];
            }
            else if (option == "--dispatch-shot") {
                if (++i >= argc || dispatch_elapsed) throw std::invalid_argument("Expected one dispatch elapsed byte");
                dispatch_elapsed = number(argv[i]);
            }
            else if (option == "--step-shot-defense") {
                if (i + 4 >= argc || staged_input) throw std::invalid_argument("Expected four defense bytes");
                std::array<unsigned,4> values{};
                for (auto& value : values) value = number(argv[++i]);
                staged_input = values;
            }
            else if (option == "--shot-defenders-file") {
                if (++i >= argc || shot_file) throw std::invalid_argument("Expected one shot defense file");
                shot_file = argv[i];
            }
            else if (option == "--menu-input") {
                if (++i >= argc || menu_pressed) throw std::invalid_argument("Expected one --menu-input PRESSED");
                menu_pressed = number(argv[i]);
            }
            else if (option == "--advance-world-file") {
                if (++i >= argc || world_file) throw std::invalid_argument("Expected one world input file");
                world_file = argv[i];
            }
            else if (option == "--advance-observed-world-file") {
                if (++i >= argc || observed_world_file) throw std::invalid_argument("Expected one observed world input file");
                observed_world_file = argv[i];
            }
            else if (option == "--advance-observed-nmi-world-file") {
                if (i + 2 >= argc || observed_nmi_world_file)
                    throw std::invalid_argument("Expected one observed NMI world file and initial index");
                observed_nmi_world_file = argv[++i];
                observed_nmi_initial_index = number(argv[++i]);
            }
            else if (option == "--advance-observed-nmi-poll") {
                if (i + 2 >= argc || observed_nmi_poll_file)
                    throw std::invalid_argument("Expected one observed NMI poll file and initial index");
                observed_nmi_poll_file = argv[++i];
                observed_nmi_poll_index = number(argv[++i]);
            }
            else if (option == "--advance-observed-nmi-clock") {
                if (i + 2 >= argc || observed_nmi_clock_file)
                    throw std::invalid_argument("Expected one observed NMI clock file and initial index");
                observed_nmi_clock_file = argv[++i];
                observed_nmi_clock_index = number(argv[++i]);
            }
            else if (option == "--advance-observed-nmi-sequence") {
                if (i + 2 >= argc || observed_nmi_sequence_file)
                    throw std::invalid_argument("Expected one observed NMI sequence file and initial index");
                observed_nmi_sequence_file = argv[++i];
                observed_nmi_sequence_index = number(argv[++i]);
            }
            else if ((option == "--advance-observed-pass-movement" || option == "--advance-observed-field-movement")) {
                if (i+2>=argc || observed_pass_movement_file) throw std::invalid_argument("Expected pass movement events and initial index");
                observed_pass_movement_file=argv[++i];
                observed_pass_movement_index=number(argv[++i]);
                if (observed_pass_movement_index>255) throw std::invalid_argument("Random index must be a byte");
            }
            else if (option == "--advance-observed-ordinary-return") {
                if(i+3>=argc || ordinary_return_file)throw std::invalid_argument("Expected returned-control kind, events and index");
                ordinary_return_kind=argv[++i];ordinary_return_file=argv[++i];ordinary_return_index=number(argv[++i]);
                if((ordinary_return_kind!="poll" && ordinary_return_kind!="clock" && ordinary_return_kind!="world") || ordinary_return_index>255)
                    throw std::invalid_argument("Invalid returned-control kind or random index");
            }
            else if (option == "--resolve-ordinary-pass-flight-target") {
                if (++i>=argc || ordinary_pass_distance_file) throw std::invalid_argument("Expected one private pass distance table");
                ordinary_pass_distance_file=argv[i];
            }
            else if (option == "--advance-world") {
                if (i + 2 >= argc || world_input) throw std::invalid_argument("Expected one --advance-world HELD PRESSED");
                const auto held = number(argv[++i]);
                const auto pressed = number(argv[++i]);
                world_input = std::array<unsigned, 2>{held, pressed};
            }
            else if (option == "--ack-sampled-field-defense-prompt") {if(++i>=argc || sampled_field_defense_blink)throw std::invalid_argument("Expected captured defensive blink");sampled_field_defense_blink=number(argv[i]);}
            else if (option == "--ack-sampled-mixed-result4-pass") {if(run_sampled_mixed_pass)throw std::invalid_argument("Duplicate sampled mixed pass acknowledgement");run_sampled_mixed_pass=true;}
            else if (option == "--advance-sampled-mixed-result4") {if(run_sampled_mixed_result4)throw std::invalid_argument("Duplicate sampled defender result");run_sampled_mixed_result4=true;}
            else if (option == "--compare-sampled-mixed-defender") {if(run_sampled_mixed_compare)throw std::invalid_argument("Duplicate sampled defender comparison");run_sampled_mixed_compare=true;}
            else if (option == "--begin-sampled-opponent-goal-restart") {if(run_sampled_goal_restart)throw std::invalid_argument("Duplicate sampled goal restart");run_sampled_goal_restart=true;}
            else if (option == "--ack-sampled-opponent-goal") {if(run_sampled_goal_presentation)throw std::invalid_argument("Duplicate sampled goal presentation");run_sampled_goal_presentation=true;}
            else if (option == "--prepare-sampled-opponent-goal") {if(run_sampled_opponent_goal)throw std::invalid_argument("Duplicate sampled goal");run_sampled_opponent_goal=true;}
            else if (option == "--compare-sampled-home-goalkeeper") {if(run_sampled_home_keeper)throw std::invalid_argument("Duplicate sampled keeper comparison");run_sampled_home_keeper=true;}
            else if (option == "--ack-sampled-keeper-overlay") {if(run_keeper_overlay)throw std::invalid_argument("Duplicate keeper overlay acknowledgement");run_keeper_overlay=true;}
            else if (option == "--poll-sampled-goalkeeper") {
                if(i+3>=argc || sampled_keeper_poll)throw std::invalid_argument("Expected goalkeeper input and two icon IDs");
                std::array<unsigned,3> values{};for(auto& v:values){v=number(argv[++i]);if(v>255)throw std::invalid_argument("Goalkeeper value exceeds byte");}sampled_keeper_poll=values;
            }
            else if (option == "--prepare-sampled-mixed-defender") {if(run_sampled_mixed_defender)throw std::invalid_argument("Duplicate sampled defender preparation");run_sampled_mixed_defender=true;}
            else if (option == "--prepare-sampled-field-contact-attack") {if(run_sampled_contact_attack)throw std::invalid_argument("Duplicate sampled contact preparation");run_sampled_contact_attack=true;}
            else if (option == "--ack-sampled-opponent-contact") {if(sampled_opponent_contact_ack)throw std::invalid_argument("Duplicate contact acknowledgement");sampled_opponent_contact_ack=true;}
            else if (option == "--continue-sampled-field-pass-control") {if(sampled_field_pass_control)throw std::invalid_argument("Duplicate pass control handoff");sampled_field_pass_control=true;}
            else if (option == "--begin-sampled-field-pass-flight") {
                if(i+3>=argc || sampled_field_pass_flight_file)throw std::invalid_argument("Expected velocity file and captured ball fractions");
                sampled_field_pass_flight_file=argv[++i];for(auto& value:sampled_field_pass_fractions)value=number(argv[++i]);
            }
            else if (option == "--begin-ordinary-pass-flight") {
                if(i+3>=argc || ordinary_pass_flight_file)throw std::invalid_argument("Expected velocity file and captured ball fractions");
                ordinary_pass_flight_file=argv[++i];
                for(auto& value:ordinary_pass_fractions) {value=number(argv[++i]);if(value>255)throw std::invalid_argument("Ball fractions must be bytes");}
            }
            else if (option == "--calculate-sampled-field-pass-target" || option == "--resolve-sampled-field-pass-target") {
                if (i+4>=argc || sampled_field_pass_target) throw std::invalid_argument("Expected distance table, recipient, target cell and counter");
                sampled_resolve_pass_target=option=="--resolve-sampled-field-pass-target";
                sampled_pass_distance_file=argv[++i];std::array<unsigned,4> values{};
                for(unsigned j=0;j<3;++j)values[j]=number(argv[++i]);
                sampled_field_pass_target=values;
            }
            else if (option == "--capture-sampled-field-pass-target") {
                if (i+4>=argc || sampled_field_pass_target) throw std::invalid_argument("Expected captured recipient, destination, counter and distance");
                std::array<unsigned,4> values{};for(auto& value:values)value=number(argv[++i]);sampled_field_pass_target=values;
            }
            else if (option == "--ack-sampled-field-ai-pass-handoff") { if(sampled_field_ai_pass_handoff)throw std::invalid_argument("Duplicate AI pass handoff");sampled_field_ai_pass_handoff=true; }
            else if (option == "--ack-sampled-field-ai-pass-presentation") {
                if (++i>=argc || sampled_field_ai_pass_field) throw std::invalid_argument("Expected captured pass field file");
                sampled_field_ai_pass_field=argv[i];
            }
            else if (option == "--advance-sampled-field-ai-dispatch") { if(sampled_field_ai_dispatch)throw std::invalid_argument("Duplicate AI dispatch");sampled_field_ai_dispatch=true; }
            else if (option == "--ack-sampled-field-ai-presentation") { if(sampled_field_ai_ack)throw std::invalid_argument("Duplicate AI presentation acknowledgement");sampled_field_ai_ack=true; }
            else if (option == "--capture-sampled-field-ai-preparation") {
                if (i + 2 >= argc || sampled_field_ai_prep) throw std::invalid_argument("Expected captured AI subtype and power");
                const auto subtype=number(argv[++i]);const auto power=number(argv[++i]);
                sampled_field_ai_prep=std::array<unsigned,2>{subtype,power};
            }
            else if (option == "--advance-sampled-home-shot-dispatch") {if(run_sampled_home_shot_dispatch)throw std::invalid_argument("Duplicate home shot dispatch");run_sampled_home_shot_dispatch=true;}
            else if (option == "--capture-sampled-home-shot-handoff") {if(i+1>=argc || sampled_home_shot_handoff_field)throw std::invalid_argument("Expected captured home shot field file");sampled_home_shot_handoff_field=argv[++i];}
            else if (option == "--advance-sampled-home-shot-contact") {if(run_sampled_home_shot_contact)throw std::invalid_argument("Duplicate home-shot contact clock");run_sampled_home_shot_contact=true;}
            else if (option == "--resolve-sampled-home-shot-defender") {if(run_sampled_home_shot_defender)throw std::invalid_argument("Duplicate home-shot defender resolution");run_sampled_home_shot_defender=true;}
            else if (option == "--capture-sampled-home-shot-defender") {
                if (i + 2 >= argc || sampled_home_shot_defender) throw std::invalid_argument("Expected home-shot defender action and subtype");
                const auto action=number(argv[++i]);const auto subtype=number(argv[++i]);
                if(action>255 || subtype>255)throw std::invalid_argument("Defender value exceeds byte");
                sampled_home_shot_defender=std::array<unsigned,2>{action,subtype};
            }
            else if (option == "--capture-sampled-field-ai-choice") {
                if (i + 2 >= argc || sampled_field_ai) throw std::invalid_argument("Expected captured AI action and subtype");
                const auto action=number(argv[++i]);const auto subtype=number(argv[++i]);
                sampled_field_ai=std::array<unsigned,2>{action,subtype};
            }
            else if (option == "--poll-control") {
                if (i + 2 >= argc || poll_input) throw std::invalid_argument("Expected one --poll-control HELD PRESSED");
                const auto held = number(argv[++i]);
                const auto pressed = number(argv[++i]);
                poll_input = std::array<unsigned, 2>{held, pressed};
            }
            else if (option == "--advance-control-travel") {
                if (i + 3 >= argc || control_travel_input)
                    throw std::invalid_argument("Expected one --advance-control-travel HELD SPEED_LOW SPEED_HIGH");
                const auto held = number(argv[++i]);
                const auto low = number(argv[++i]);
                const auto high = number(argv[++i]);
                control_travel_input = std::array<unsigned, 3>{held, low, high};
            }
            else if (option == "--advance-profiled-control-travel") {
                if (++i >= argc || profiled_control_travel_input)
                    throw std::invalid_argument("Expected one --advance-profiled-control-travel HELD");
                profiled_control_travel_input = number(argv[i]);
            }
            else if (option == "--poll-sampled-human-pass-target") {
                if(i+2>=argc || poll_human_pass)throw std::invalid_argument("Expected one human pass HELD PRESSED");
                const auto held=number(argv[++i]);const auto pressed=number(argv[++i]);
                if(held>255 || pressed>255)throw std::invalid_argument("Human pass inputs must be bytes");
                poll_human_pass=std::array<unsigned,2>{held,pressed};
            }
            else if (option == "--poll-ordinary-human-pass-cursor") {
                if (i + 2 >= argc || poll_ordinary_pass) throw std::invalid_argument("Expected one ordinary pass HELD PRESSED");
                const auto held = number(argv[++i]);
                const auto pressed = number(argv[++i]);
                poll_ordinary_pass = std::array<unsigned, 2>{held, pressed};
            }
            else if (option == "--contact" || option == "--rewarded-contact" || option == "--defender" || option == "--advance" ||
                     option == "--defense-action" || option == "--defense-subtype" || option == "--capture") {
                if (++i >= argc) throw std::invalid_argument("Missing option value");
                if (option == "--capture") capture = argv[i];
                else if (option == "--contact" || option == "--rewarded-contact") {
                    if (code) throw std::invalid_argument("Choose one contact operation");
                    code = number(argv[i]); rewarded = option == "--rewarded-contact";
                }
                else if (option == "--defender") defender = number(argv[i]);
                else if (option == "--advance") elapsed = number(argv[i]);
                else if (option == "--defense-action") { action = number(argv[i]); defense_options = true; }
                else { subtype = number(argv[i]); defense_options = true; }
            } else throw std::invalid_argument("Unknown session option");
        }
        if (unsigned(run_sampled_goal_restart) + unsigned(run_sampled_goal_presentation) + unsigned(run_sampled_opponent_goal) + unsigned(run_sampled_home_keeper) + unsigned(run_keeper_overlay) + unsigned(bool(sampled_keeper_poll)) + unsigned(run_sampled_mixed_compare) + unsigned(run_sampled_mixed_result4) + unsigned(run_sampled_mixed_pass) + unsigned(run_sampled_mixed_defender) + unsigned(run_sampled_contact_attack) + unsigned(bool(sampled_field_defense_blink)) + unsigned(sampled_opponent_contact_ack) + unsigned(sampled_field_pass_control) + unsigned(bool(sampled_field_pass_flight_file)) + unsigned(bool(ordinary_pass_flight_file)) + unsigned(bool(sampled_field_pass_target)) + unsigned(sampled_field_ai_pass_handoff) + unsigned(bool(sampled_field_ai_pass_field)) + unsigned(sampled_field_ai_dispatch) + unsigned(sampled_field_ai_ack) + unsigned(bool(sampled_field_ai_prep)) + unsigned(bool(sampled_field_ai)) + unsigned(bool(sampled_home_shot_defender)) + unsigned(run_sampled_home_shot_defender) + unsigned(run_sampled_home_shot_contact) + unsigned(run_sampled_home_shot_dispatch) + unsigned(bool(sampled_home_shot_handoff_field)) + unsigned(bool(code)) + unsigned(run_resume) + unsigned(bool(elapsed)) + unsigned(run_recovery) +
            unsigned(run_continue_recovery) + unsigned(bool(area_ordinary_facing)) + unsigned(bool(area_story_facing)) + unsigned(bool(area_flight_file)) + unsigned(bool(restart_flight_file)) + unsigned(run_area_ball_step) + unsigned(run_area_ball_cell) + unsigned(run_area_pass_received) + unsigned(run_area_pass_keeper) + unsigned(run_area_pass_announcement) + unsigned(run_area_pass_restore) + unsigned(run_area_pass_nonterminal) + unsigned(run_area_pass_next) + unsigned(run_area_pass_blocked) + unsigned(run_area_pass_terminal) + unsigned(run_area_pass_capture) + unsigned(run_area_pass_capture_control) + unsigned(run_area_pass_blocked_control) + unsigned(run_area_pass_resume) + unsigned(run_area_pass_recovery) + unsigned(run_area_pass_recover) + unsigned(bool(area_pass_capture_ack)) + unsigned(bool(area_pass_recovery_ack)) + unsigned(bool(area_pass_cost)) + unsigned(bool(area_pass_result_ack)) + unsigned(bool(area_pass_compare_file)) + unsigned(run_prepare_area_pass_keeper) + unsigned(bool(area_action4_ack)) + unsigned(run_area_action4_field_entry) + unsigned(run_area_action4_control_entry) + unsigned(run_area_action4_control_ready) + unsigned(run_area_action4_keeper) + unsigned(run_area_action4_keeper_stamina) + unsigned(run_area_action4_keeper_result_ack) + unsigned(run_area_action4_keeper_jitter) + unsigned(run_area_action4_keeper_restart) + unsigned(bool(area_action4_keeper_restart_prep)) + unsigned(bool(area_action4_restart_field)) + unsigned(bool(area_action4_restart_table)) + unsigned(run_area_action4_restart_power) + unsigned(run_area_action4_restart_clock) + unsigned(run_area_action4_restart_resume) + unsigned(run_area_action4_restart_dispatch) + unsigned(run_area_action4_restart_pass) + unsigned(run_area_action4_restart_pass_control) + unsigned(run_area_action4_result0_goal) + unsigned(run_area_action4_postgoal) + unsigned(run_sampled_loose_defender) + unsigned(run_compare_sampled_loose_defender) + unsigned(run_recover_sampled_loose_result2) + unsigned(run_sampled_result4_extra_time) + unsigned(run_recover_sampled_result4_action) + unsigned(bool(sampled_keeper_result4)) + unsigned(bool(sampled_result4_formation_file)) + unsigned(bool(sampled_result4_action)) + unsigned(bool(area_action4_keeper_result_action)) + unsigned(bool(area_action4_keeper_compare)) + unsigned(bool(area_action4_keeper_ack)) + unsigned(bool(area_action4_keeper_prep)) + unsigned(bool(area_action6_file)) + unsigned(bool(area_action6_race_file)) + unsigned(bool(area_action6_winner_ack)) + unsigned(run_area_action6_recover) + unsigned(bool(area_action6_recovery_ack)) + unsigned(run_area_action6_control) + unsigned(run_setup_control) + unsigned(bool(control_travel_input)) + unsigned(bool(profiled_control_travel_input)) + unsigned(bool(poll_input)) +
            unsigned(run_control_tick) + unsigned(resume_control_time) + unsigned(bool(world_input)) +
            unsigned(bool(world_file)) + unsigned(bool(observed_world_file)) + unsigned(bool(observed_nmi_world_file)) + unsigned(bool(observed_nmi_poll_file)) + unsigned(bool(observed_nmi_clock_file)) + unsigned(bool(observed_nmi_sequence_file)) + unsigned(bool(observed_pass_movement_file)) + unsigned(bool(ordinary_pass_distance_file)) + unsigned(bool(ordinary_return_file)) + unsigned(bool(human_pass_recovery_field)) + unsigned(bool(human_race_recovery_file)) + unsigned(bool(ordinary_free_race_file)) + unsigned(bool(player3_handoff_field)) + unsigned(bool(player3_race_file)) + unsigned(position_player3_winner) + unsigned(continue_player3_winner) + unsigned(bool(player5_defense)) + unsigned(bool(score24_dribble_defense)) + unsigned(prepare_player5_dribble) + unsigned(begin_player5_defenders) + unsigned(prepare_player5_defender) + unsigned(advance_player5_terminal) + unsigned(advance_player5_terminal_field) + unsigned(transfer_player5_possession) + unsigned(prepare_player5_result3_control) + unsigned(advance_player14_handoff) + unsigned(advance_player19_return_approach) + unsigned(advance_score23_handoff) + unsigned(bool(compare_player5_defender)) + unsigned(prepare_player3_defender) + unsigned(bool(compare_player3_defender)) + unsigned(advance_player3_contact) + unsigned(advance_player3_dispatch) + unsigned(run_open_action_menu) + unsigned(bool(menu_pressed)) +
            unsigned(run_prepare_basic_shot) + unsigned(run_reject_sampled_shot) + unsigned(begin_human_pass) + unsigned(begin_ordinary_human_pass) + unsigned(confirm_ordinary_human_pass) + unsigned(begin_ordinary_defenses) + unsigned(bool(ordinary_defense)) + unsigned(prepare_ordinary_pass) + unsigned(bool(ordinary_defender_variant)) + unsigned(bool(compare_first_ordinary_defender)) + unsigned(advance_first_ordinary_contact) + unsigned(prepare_next_ordinary_defender) + unsigned(bool(compare_next_ordinary_defender)) + unsigned(advance_next_ordinary_contact) + unsigned(advance_ordinary_dispatch_clock) + unsigned(advance_ordinary_ball) + unsigned(resolve_ordinary_ball) + unsigned(continue_ordinary_reception) + unsigned(position_ordinary_recipient) + unsigned(bool(ordinary_control_facing)) + unsigned(bool(ordinary_return_shot_defense)) + unsigned(prepare_ordinary_return_shot) + unsigned(bool(ordinary_return_shot_variant)) + unsigned(bool(ordinary_return_shot_comparison)) + unsigned(ordinary_return_shot_contact) + unsigned(ordinary_return_shot_dispatch) + unsigned(ordinary_return_flight_prepare) + unsigned(ordinary_return_flight_compare) + unsigned(ordinary_return_blocked_position) + unsigned(ordinary_return_blocked_clock) + unsigned(prepare_human_pass) + unsigned(prepare_score25_pass) + unsigned(bool(score25_pass_defenses)) + unsigned(ack_human_pass) + unsigned(bool(poll_human_pass)) + unsigned(bool(poll_ordinary_pass)) + unsigned(bool(shot_file)) + unsigned(bool(staged_input)) + unsigned(bool(dispatch_elapsed)) + unsigned(resume_shot_clock) + unsigned(run_selected_result) + unsigned(run_continue_selected) + unsigned(bool(selected_recovery_file)) + unsigned(bool(clearance_file)) + unsigned(bool(race_file)) + unsigned(run_continue_clearance) + unsigned(run_restart_route) + unsigned(run_restart_options) + unsigned(bool(restart_input)) + unsigned(run_attack_clock) + unsigned(run_resume_attack) + unsigned(bool(unopposed_attack)) + unsigned(run_approach) + unsigned(run_ball_step) + unsigned(bool(flight_file)) + unsigned(run_ball_cell) + unsigned(run_arrival) + unsigned(run_shot_recovery) + unsigned(run_keeper_entry) + unsigned(bool(stunned_thresholds)) + unsigned(bool(keeper_choice)) + unsigned(run_keeper_save) + unsigned(run_punch_recovery) + unsigned(bool(goal_baseline)) + unsigned(run_goal_restart) + unsigned(bool(kickoff_file)) + unsigned(bool(kickoff_receiver)) + unsigned(run_kickoff_ball) + unsigned(run_kickoff_handoff) + unsigned(run_keeper_area) + unsigned(bool(area_menu_facing)) + unsigned(bool(area_defense_blink)) + unsigned(run_area_defense_prompt) + unsigned(bool(area_defense_pressed)) + unsigned(run_prepare_area_defense) + unsigned(run_inspect_area_defense) + unsigned(run_open_area_special) + unsigned(bool(area_special_pressed)) + unsigned(run_resolve_area_special) + unsigned(run_redraw_area_special) + unsigned(run_continue_area_special) + unsigned(run_area_ai) + unsigned(run_area_attack) + unsigned(run_area_defense_entry) + unsigned(bool(area_announce_facing)) + unsigned(run_area_contact) + unsigned(run_area_clock) + unsigned(run_area_clock_resume) + unsigned(run_area_result_dispatch) + unsigned(run_area_result_recovery) + unsigned(bool(area_sequence_operation)) + unsigned(bool(area_prefix_elapsed)) + unsigned(resume_area_prefix) + unsigned(run_profiled_area_prefix) > 1)
            throw std::invalid_argument("Choose one initial operation");
        if (bool(code) != bool(defender) || (defense_options && !code))
            throw std::invalid_argument("Contact needs --contact and --defender");
        std::ifstream pack(argv[1], std::ios::binary), snapshot(argv[2], std::ios::binary);
        if (!pack || !snapshot) throw std::invalid_argument("Unable to open private content/session");
        const auto content = captain::load_shot_content(pack);
        if (!content.movement || !content.contact) throw std::invalid_argument("Session requires version5 contact/movement content");
        auto state = captain::load_phase_session(snapshot);
        std::cout << "Captain Majid native phase session; development checkpoint, incomplete match.\n";
        if (code) contact(state, content, *code, *defender, action, subtype, rewarded);
        if (run_resume) resume(state, content);
        if (run_recovery) recover(state, content);
        if (run_continue_recovery) continue_recovery(state);
        if (area_ordinary_facing) state=captain::continue_session_area_ordinary_recovery(state,*area_ordinary_facing);
        if (area_story_facing) state=captain::continue_session_area_story_recovery(state,*area_story_facing);
        if (area_flight_file) begin_area_flight(state,content,*area_flight_file,area_flight_input);
        if (restart_flight_file) { if(!content.movement)throw std::invalid_argument("Restart flight requires movement geometry"); state=captain::begin_session_area_action4_restart_flight(state,15,0,0,4,content.movement->geometry,read_ball_velocity(*restart_flight_file)); }
        if (run_area_ball_step) state=captain::advance_prepared_area_ball_cell(state).state;
        if (run_area_ball_cell) state=captain::resolve_session_area_ball_cell(state);
        if (run_area_pass_received) state=captain::continue_area_already_arrived(state);
        if (run_area_pass_announcement) state=captain::acknowledge_area_pass_defender_announcement(state);
        if (area_pass_cost) state=captain::prepare_area_pass_defender(state,static_cast<std::uint16_t>((*area_pass_cost)[0]|((*area_pass_cost)[1]<<8)));
        if (area_pass_compare_file) compare_area_pass(state,*area_pass_compare_file,area_pass_compare_values[0],area_pass_compare_values[1],area_pass_compare_values[2]);
        if (run_area_pass_restore) state=captain::restore_area_pass_ball_variant(state);
        if (area_pass_result_ack) state=captain::acknowledge_area_pass_result(state,(*area_pass_result_ack)[0],(*area_pass_result_ack)[1]);
        if (run_area_pass_nonterminal) {
            if(!content.contact || !content.experience)throw std::invalid_argument("Pass handler needs contact and experience content");
            state=captain::advance_area_pass_nonterminal(state,*content.contact,*content.experience);
        }
        if (run_area_pass_next) state=captain::advance_area_pass_defender(state);
        if (run_area_pass_resume) state=captain::resume_area_pass_added_time(state);
        if (run_area_pass_recovery) state=captain::enter_area_pass_blocked_recovery(state);
        if (run_area_pass_recover) {
            if(!content.movement)throw std::invalid_argument("Blocked pass recovery needs movement content");
            const auto recovered=captain::advance_session_area_pass_blocked_recovery(state,content.field_players,*content.movement);state=recovered.state;
            std::cout<<"Possession sound request "<<recovered.request_possession_sound<<'\n';
        }
        if (area_pass_recovery_ack) state=captain::continue_session_area_pass_blocked_recovery(state,(*area_pass_recovery_ack)[0],(*area_pass_recovery_ack)[1]);
        if (run_area_pass_blocked) {
            if(!content.experience)throw std::invalid_argument("Blocked pass requires experience content");
            state=captain::advance_area_pass_blocked(state,*content.experience);
        }
        if (run_area_pass_terminal) {
            if(!content.contact || !content.experience)throw std::invalid_argument("Pass terminal clock needs contact and experience content");
            state=captain::advance_area_pass_terminal(state,*content.contact,*content.experience);
        }
        if (run_area_pass_capture) {const auto captured=captain::capture_area_pass_defender(state);state=captured.state;std::cout<<"Possession sound request "<<captured.request_possession_sound<<'\n';}
        if (area_pass_capture_ack) {
            if(!content.movement)throw std::invalid_argument("Captured pass continuation needs movement content");
            state=captain::continue_area_pass_capture(state,content.field_players,*content.movement,(*area_pass_capture_ack)[0],(*area_pass_capture_ack)[1]);
        }
        if (run_area_pass_capture_control) state=captain::prepare_session_area_pass_capture_control(state).state;
        if (run_area_pass_blocked_control) state=captain::prepare_session_area_pass_blocked_control(state).state;
        if (run_area_pass_keeper) state=captain::continue_area_pass_received(state);
        if (run_prepare_area_pass_keeper) state=captain::prepare_area_pass_keeper(state);
        if (area_action4_ack) state=captain::continue_session_area_action4(state,(*area_action4_ack)[0],(*area_action4_ack)[1],content.experience ? &*content.experience : nullptr);
        if (run_area_action4_field_entry) state=captain::begin_session_area_action4_encounters(state);
        if (run_area_action4_control_entry) state=captain::continue_session_area_action4_control(state);
        if (run_area_action4_control_ready) state=captain::prepare_session_area_action4_control(state).state;
        if (run_area_action4_keeper) state=captain::begin_session_area_action4_keeper(state);
        if (run_area_action4_keeper_stamina) state=captain::spend_session_area_action4_keeper_stamina(state);
        if (run_area_action4_keeper_result_ack) state=captain::acknowledge_session_area_action4_keeper_result(state);
        if (run_area_action4_keeper_jitter) state=captain::advance_session_area_action4_keeper_result1_jitter(state);
        if (run_area_action4_keeper_restart) state=captain::route_session_area_action4_keeper_result1_restart(state);
        if (run_area_action4_restart_power) state=captain::prepare_session_area_action4_restart_power(state,true,content);
        if (run_area_action4_restart_clock) state=captain::advance_session_area_action4_restart_clock(state).state;
        if (run_area_action4_restart_resume) state=captain::resume_session_area_action4_restart_added_time(state);
        if (run_area_action4_restart_dispatch) state=captain::dispatch_session_area_action4_restart(state);
        if (run_area_action4_restart_pass) state=captain::acknowledge_session_area_action4_restart_pass(state);
        if (run_area_action4_restart_pass_control) state=captain::prepare_session_area_action4_restart_pass_control(state).state;
        if (run_recover_sampled_loose_result2) { if(!content.contact || !content.experience)throw std::invalid_argument("Missing private clock or experience content");state=captain::recover_session_sampled_loose_shot_result2(state,*content.contact,*content.experience); }
        if (run_compare_sampled_loose_defender) { if(!content.flight)throw std::invalid_argument("Missing private flight comparison content");state=captain::compare_session_sampled_loose_shot_defender(state,19,*content.flight); }
        if (run_sampled_loose_defender) state=captain::prepare_session_sampled_loose_shot_defender(state);
        if (run_area_action4_postgoal) state=captain::continue_session_area_action4_keeper_result0_goal(state);
        if (run_area_action4_result0_goal) { if(!content.experience)throw std::invalid_argument("Result0 goal requires experience data");state=captain::prepare_session_area_action4_keeper_result0_goal(state,*content.experience); }
        if (area_action4_restart_table) {
            std::ifstream table(*area_action4_restart_table,std::ios::binary);if(!table)throw std::invalid_argument("Cannot open private restart formation table");
            captain::AreaAction4RestartFormationContent formation{};
            for(auto& cell:formation.cells){const auto value=table.get();if(value==std::char_traits<char>::eof())throw std::invalid_argument("Truncated restart formation table");cell=static_cast<std::uint8_t>(value);}
            const auto owner=table.get();if(owner==std::char_traits<char>::eof() || table.peek()!=std::char_traits<char>::eof())throw std::invalid_argument("Invalid restart formation table length");
            formation.owner_cell=static_cast<std::uint8_t>(owner);state=captain::compute_session_area_action4_restart_formation(state,formation);
        }
        if (area_action4_restart_field) {
            std::ifstream captured(*area_action4_restart_field,std::ios::binary);
            if(!captured)throw std::invalid_argument("Cannot open captured restart field");
            state=captain::acknowledge_session_area_action4_restart_formation(state,captain::load_field_state(captured));
        }
        if (run_recover_sampled_result4_action) {if(!content.experience)throw std::invalid_argument("Missing private experience content");state=captain::recover_session_sampled_keeper_result4_action(state,*content.experience);}
        if (run_sampled_result4_extra_time) state=captain::advance_session_sampled_keeper_result4_extra_time(state);
        if (sampled_result4_action) state=captain::prepare_session_sampled_keeper_result4_action(state,(*sampled_result4_action)[0],(*sampled_result4_action)[1],(*sampled_result4_action)[2],(*sampled_result4_action)[3],(*sampled_result4_action)[4]);
        if (sampled_result4_formation_file) {
            std::ifstream captured(*sampled_result4_formation_file,std::ios::binary);
            if(!captured)throw std::invalid_argument("Cannot open captured result4 formation");
            state=captain::acknowledge_session_sampled_keeper_result4_formation(state,captain::load_field_state(captured));
        }
        if (sampled_keeper_result4) state=captain::route_session_sampled_keeper_result4(state,(*sampled_keeper_result4)[0]!=0,(*sampled_keeper_result4)[1]);
        if (area_action4_keeper_restart_prep) state=captain::prepare_session_area_action4_keeper_restart(state,(*area_action4_keeper_restart_prep)[0],(*area_action4_keeper_restart_prep)[1],(*area_action4_keeper_restart_prep)[2],(*area_action4_keeper_restart_prep)[3]);
        if (area_action4_keeper_result_action) {
            if(!content.experience)throw std::invalid_argument("Keeper result handler requires experience content");
            state=captain::continue_session_area_action4_keeper_result(state,*area_action4_keeper_result_action,*content.experience);
        }
        if (area_action4_keeper_compare) state=captain::compare_session_area_action4_keeper(state,(*area_action4_keeper_compare)[0],(*area_action4_keeper_compare)[1],(*area_action4_keeper_compare)[2],(*area_action4_keeper_compare)[3]);
        if (area_action4_keeper_ack) state=captain::acknowledge_session_area_action4_keeper(state,(*area_action4_keeper_ack)[0],(*area_action4_keeper_ack)[1]);
        if (area_action4_keeper_prep) state=captain::prepare_session_area_action4_keeper(state,(*area_action4_keeper_prep)[0] | ((*area_action4_keeper_prep)[1]<<8),(*area_action4_keeper_prep)[2] | ((*area_action4_keeper_prep)[3]<<8));
        if (area_action6_file) continue_area_action6(state,*area_action6_file,area_action6_ack[0],area_action6_ack[1],content);
        if (area_action6_race_file) clearance_race_file(state,content,*area_action6_race_file,true);
        if (area_action6_winner_ack) state=captain::continue_session_area_action6_winner(state,(*area_action6_winner_ack)[0],(*area_action6_winner_ack)[1]);
        if (run_area_action6_recover) state=captain::advance_session_area_action6_recovery(state,content.field_players,*content.movement).state;
        if (area_action6_recovery_ack) state=captain::continue_session_area_action6_recovery(state,(*area_action6_recovery_ack)[0],(*area_action6_recovery_ack)[1]);
        if (run_area_action6_control) state=captain::prepare_session_area_action6_control(state).state;
        if (run_setup_control) setup_control(state);
        if (control_travel_input) {
            const auto& values = *control_travel_input;
            state = captain::advance_session_control_travel(state, static_cast<std::uint16_t>(values[1] | (values[2] << 8)),
                                                            static_cast<std::uint8_t>(values[0]));
            std::cout << "Advanced control travel only; no frame, random, audio or presentation work performed.\n";
        }
        if (profiled_control_travel_input) {
            const auto held = static_cast<std::uint8_t>(*profiled_control_travel_input);
            if (state.current.field.side == 0 && !(held & 15u)) {
                state = captain::advance_session_control_travel(state, 0, held);
                std::cout << "Control travel unchanged; no speed loaded.\n";
            } else {
                if (!content.movement || !content.control_movement)
                    throw std::invalid_argument("Profiled control travel requires private movement content");
                const auto result = captain::advance_profiled_session_control_travel(
                    state, held, content.field_players, *content.movement, *content.control_movement);
                state = result.state;
                std::cout << "Advanced control travel | loaded speed " << *result.loaded_speed
                          << "; no frame, random, audio or presentation work performed.\n";
            }
        }
        if (poll_input) poll_control(state, content, (*poll_input)[0], (*poll_input)[1]);
        if (run_control_tick || resume_control_time) advance_control(state, content, resume_control_time);
        if (world_input) advance_world(state, content, (*world_input)[0], (*world_input)[1]);
        if (world_file) advance_world_file(state, content, *world_file);
        if (observed_world_file) advance_observed_world_file(state, content, *observed_world_file);
        if (observed_nmi_world_file) advance_observed_nmi_world_file(
            state, content, *observed_nmi_world_file, static_cast<std::uint8_t>(observed_nmi_initial_index));
        if (observed_nmi_poll_file) advance_observed_nmi_poll_file(
            state, content, *observed_nmi_poll_file, static_cast<std::uint8_t>(observed_nmi_poll_index));
        if (observed_nmi_clock_file) advance_observed_nmi_clock_file(
            state, content, *observed_nmi_clock_file, static_cast<std::uint8_t>(observed_nmi_clock_index));
        if (observed_nmi_sequence_file) advance_observed_nmi_sequence_file(
            state, *observed_nmi_sequence_file, static_cast<std::uint8_t>(observed_nmi_sequence_index));
        if (observed_pass_movement_file) {
            if (!content.movement) throw std::invalid_argument("Missing field movement content");
            std::ifstream input(*observed_pass_movement_file);
            if (!input) throw std::invalid_argument("Unable to open observed pass movement events");
            const auto events=captain::load_observed_nmi_events(input);
            const auto moved=captain::advance_observed_pass_field_movement(state,content.field_players,*content.movement,
                static_cast<std::uint8_t>(observed_pass_movement_index),events);
            state=moved.state;
            std::cout << "Observed field movement | final random index " << unsigned(moved.random_index)
                      << "; captured events supplied, no frame/audio/presentation emulation.\n";
        }
        if(ordinary_return_file) {
            if(ordinary_return_kind=="poll") advance_observed_nmi_poll_file(state,content,*ordinary_return_file,static_cast<std::uint8_t>(ordinary_return_index),true);
            else if(ordinary_return_kind=="clock") advance_observed_nmi_clock_file(state,content,*ordinary_return_file,static_cast<std::uint8_t>(ordinary_return_index),true);
            else advance_observed_nmi_world_file(state,content,*ordinary_return_file,static_cast<std::uint8_t>(ordinary_return_index),true);
        }
        if (ordinary_pass_distance_file) {
            std::ifstream input(*ordinary_pass_distance_file,std::ios::binary);
            if (!input) throw std::invalid_argument("Unable to open pass distance table");
            state=captain::resolve_ordinary_pass_flight_target(state,captain::read_pass_distance_content(input));
        }
        if (run_open_action_menu) open_action_menu(state, content);
        if (menu_pressed) menu_input(state, content, *menu_pressed);
        if (run_prepare_basic_shot) prepare_basic_shot(state, content);
        if (run_reject_sampled_shot) state=captain::acknowledge_sampled_unaffordable_shot(state,content.field_players,content.attack);
        if(begin_human_pass)state=captain::begin_sampled_human_pass_target(state);
        if(begin_ordinary_human_pass)state=captain::begin_ordinary_human_pass_target(state);
        if(confirm_ordinary_human_pass) {
            state=captain::confirm_ordinary_human_pass_target(state);
            std::cout << "Ordinary pass target confirmed | recipient " << unsigned(*state.ordinary_pass_recipient)
                      << " | cell " << unsigned(*state.human_pass_cursor) << "; no chooser, frame or presentation simulated.\n";
        }
        if(begin_ordinary_defenses) state=captain::begin_ordinary_pass_defense_selection(state);
        if(ordinary_defense) state=captain::record_ordinary_pass_defense(state,
            static_cast<std::uint8_t>((*ordinary_defense)[0]), static_cast<std::uint8_t>((*ordinary_defense)[1]),
            static_cast<std::uint8_t>((*ordinary_defense)[2]));
        if(prepare_ordinary_pass) state=captain::prepare_ordinary_pass(state,content.field_players,content.attack);
        if(ordinary_defender_variant) state=captain::prepare_first_ordinary_pass_defender(state,
            static_cast<std::uint8_t>(*ordinary_defender_variant),content.field_players,content.defense);
        if(compare_first_ordinary_defender) {
            const auto& c=*compare_first_ordinary_defender;
            state=captain::compare_first_ordinary_pass_defender(state,captain::FieldComparisonSelection{
                5,static_cast<std::uint8_t>(c[0]),static_cast<std::uint8_t>(c[1]),bool(c[2]),
                {static_cast<std::uint8_t>(c[3]),static_cast<std::uint8_t>(c[4]),static_cast<std::uint8_t>(c[5]),static_cast<std::uint8_t>(c[6])}});
        }
        if(advance_first_ordinary_contact) {
            if (!content.contact || !content.experience) throw std::invalid_argument("Missing contact or experience content");
            state=captain::advance_first_ordinary_pass_contact(state,*content.contact,*content.experience);
        }
        if(prepare_next_ordinary_defender) state=captain::prepare_next_ordinary_pass_defender(state,content.field_players,content.defense);
        if(compare_next_ordinary_defender) {
            const auto& c=*compare_next_ordinary_defender;
            state=captain::compare_next_ordinary_pass_defender(state,captain::FieldComparisonSelection{
                5,static_cast<std::uint8_t>(c[0]),static_cast<std::uint8_t>(c[1]),bool(c[2]),
                {static_cast<std::uint8_t>(c[3]),static_cast<std::uint8_t>(c[4]),static_cast<std::uint8_t>(c[5]),static_cast<std::uint8_t>(c[6])}});
        }
        if(advance_next_ordinary_contact) {
            if (!content.contact || !content.experience) throw std::invalid_argument("Missing contact or experience content");
            state=captain::advance_next_ordinary_pass_contact(state,*content.contact,*content.experience);
        }
        if(advance_ordinary_dispatch_clock) state=captain::advance_ordinary_pass_dispatch_clock(state);
        if(advance_ordinary_ball) state=captain::advance_ordinary_pass_ball_cell(state).state;
        if(resolve_ordinary_ball) state=captain::resolve_ordinary_pass_ball_cell(state);
        if(continue_ordinary_reception) state=captain::continue_ordinary_pass_reception(state);
        if(position_ordinary_recipient) state=captain::position_ordinary_pass_recipient(state);
        if(ordinary_control_facing) state=captain::continue_ordinary_pass_control(state,static_cast<std::uint8_t>(*ordinary_control_facing));
        if(ordinary_return_shot_defense) state=captain::record_ordinary_return_shot_defense(state,
            static_cast<std::uint8_t>((*ordinary_return_shot_defense)[0]),static_cast<std::uint8_t>((*ordinary_return_shot_defense)[1]),static_cast<std::uint8_t>((*ordinary_return_shot_defense)[2]));
        if(prepare_ordinary_return_shot) state=captain::prepare_ordinary_return_shot(state,content.field_players,content.attack).state;
        if(ordinary_return_shot_variant) state=captain::prepare_ordinary_return_shot_defender(state,
            static_cast<std::uint8_t>(*ordinary_return_shot_variant),content.field_players,content.defense);
        if(ordinary_return_shot_comparison) {
            const auto& c=*ordinary_return_shot_comparison;
            state=captain::compare_ordinary_return_shot_defender(state,captain::FieldComparisonSelection{1,
                static_cast<std::uint8_t>(c[0]),static_cast<std::uint8_t>(c[1]),bool(c[2]),
                {static_cast<std::uint8_t>(c[3]),static_cast<std::uint8_t>(c[4]),static_cast<std::uint8_t>(c[5]),static_cast<std::uint8_t>(c[6])}});
        }
        if(ordinary_return_shot_contact) {
            if(!content.contact || !content.experience)throw std::invalid_argument("Missing contact or experience content");
            state=captain::advance_ordinary_return_shot_contact(state,*content.contact,*content.experience);
        }
        if(ordinary_return_shot_dispatch) state=captain::advance_ordinary_return_shot_dispatch(state);
        if(ordinary_return_flight_prepare)state=captain::prepare_ordinary_return_flight_defender(state,content.field_players,content.defense);
        if(ordinary_return_flight_compare){if(!content.flight)throw std::invalid_argument("Missing private flight comparison content");state=captain::compare_ordinary_return_flight_defender(state,*content.flight);}
        if(ordinary_return_blocked_position){if(!content.experience)throw std::invalid_argument("Missing experience content");state=captain::position_ordinary_return_blocked_shot(state,*content.experience);}
        if(ordinary_return_blocked_clock){if(!content.experience || !content.contact)throw std::invalid_argument("Missing clock or experience content");state=captain::advance_ordinary_return_blocked_shot_clock(state,*content.contact,*content.experience);}
        if(prepare_human_pass)state=captain::prepare_sampled_human_empty_pass(state,content.field_players,content.attack);
        if(score25_pass_defenses) {
            std::array<std::uint8_t,3> actions{},subtypes{};
            for(unsigned j=0;j<3;++j) {actions[j]=static_cast<std::uint8_t>((*score25_pass_defenses)[j]);subtypes[j]=static_cast<std::uint8_t>((*score25_pass_defenses)[j+3]);}
            state=captain::capture_sampled_score25_pass_defenses(state,actions,subtypes);
        }
        if(prepare_score25_pass)state=captain::prepare_sampled_score25_player7_pass(state,content.field_players,content.attack);
        if(prepare_player3_defender)state=captain::prepare_sampled_player3_pass_defender(state,content.field_players,content.defense);
        if(compare_player3_defender) {
            const auto& c=*compare_player3_defender;
            const captain::FieldComparisonSelection direct{5,static_cast<std::uint8_t>(c[0]),static_cast<std::uint8_t>(c[1]),bool(c[2]),
                {static_cast<std::uint8_t>(c[3]),static_cast<std::uint8_t>(c[4]),static_cast<std::uint8_t>(c[5]),static_cast<std::uint8_t>(c[6])}};
            state=captain::compare_sampled_player3_pass_defender(state,direct);
        }
        if(advance_player3_contact) {
            if(!content.contact || !content.experience)throw std::invalid_argument("Missing contact or experience content");
            state=captain::advance_sampled_player3_pass_contact(state,*content.contact,*content.experience);
        }
        if(advance_player3_dispatch)state=captain::advance_sampled_player3_pass_dispatch(state);
        if(position_player3_winner)state=captain::position_sampled_player3_pass_winner(state);
        if(continue_player3_winner)state=captain::continue_sampled_player3_pass_winner(state);
        if(player5_defense) {
            std::array<std::uint8_t,5> actions{},subtypes{};
            for(unsigned j=0;j<5;++j) {actions[j]=static_cast<std::uint8_t>((*player5_defense)[j]);subtypes[j]=static_cast<std::uint8_t>((*player5_defense)[j+5]);}
            state=captain::capture_sampled_player5_defense_choices(state,actions,subtypes);
        }
        if(score24_dribble_defense)state=captain::capture_sampled_score24_dribble_defense(state,
            static_cast<std::uint8_t>((*score24_dribble_defense)[0]),static_cast<std::uint8_t>((*score24_dribble_defense)[1]));
        if(prepare_player5_dribble)state=captain::prepare_sampled_player5_dribble(state,content.field_players,content.attack);
        if(begin_player5_defenders)state=captain::begin_sampled_player5_dribble_defenders(state);
        if(prepare_player5_defender)state=captain::prepare_sampled_player5_dribble_defender(state,content.field_players,content.defense);
        if(advance_player5_terminal) {
            if(!content.experience)throw std::invalid_argument("Player5 terminal needs experience content");
            state=captain::advance_sampled_player5_dribble_terminal(state,*content.contact,*content.experience);
        }
        if(advance_player5_terminal_field)state=captain::advance_sampled_player5_terminal_field(state,content.field_players,*content.movement);
        if(transfer_player5_possession) {
            const auto result=captain::transfer_sampled_player5_dribble_possession(state);
            state=result.state;std::cout << "Player " << unsigned(state.current.field.ball_owner) << " possession | side music request " << result.request_side_music << '\n';
        }
        if(prepare_player5_result3_control)state=captain::prepare_sampled_player5_result3_control(state).state;
        if(advance_player14_handoff)state=captain::advance_sampled_player14_pass_handoff(state,content.field_players,*content.movement);
        if(advance_score23_handoff)state=captain::advance_sampled_score23_pass_handoff(state,content.field_players,*content.movement);
        if(advance_player19_return_approach)state=captain::advance_sampled_player19_return_shot_approach(state,content.field_players,*content.movement);
        if(compare_player5_defender) {
            const auto& v=*compare_player5_defender;
            const captain::FieldComparisonSelection selected{static_cast<std::uint8_t>(state.action && state.action->action==0 ? 1 : 9),static_cast<std::uint8_t>(v[0]),static_cast<std::uint8_t>(v[1]),bool(v[2]),
                {static_cast<std::uint8_t>(v[3]),static_cast<std::uint8_t>(v[4]),static_cast<std::uint8_t>(v[5]),static_cast<std::uint8_t>(v[6])}};
            state=captain::compare_sampled_player5_dribble_defender(state,selected);
        }
        if(player3_handoff_field) {
            std::ifstream captured(*player3_handoff_field,std::ios::binary);
            if(!captured)throw std::invalid_argument("Cannot open captured player3 pass handoff field");
            state=captain::capture_sampled_player3_pass_handoff(state,captain::load_field_state(captured));
        }
        if(ack_human_pass)state=captain::acknowledge_sampled_human_empty_pass_handoff(state);
        if(human_pass_recovery_field) {
            std::ifstream captured(*human_pass_recovery_field,std::ios::binary);
            if(!captured)throw std::invalid_argument("Cannot open captured human pass recovery field");
            state=captain::capture_sampled_human_empty_pass_recovery(state,captain::load_field_state(captured));
        }
        if(poll_human_pass)state=captain::poll_sampled_human_pass_target(state,static_cast<std::uint8_t>((*poll_human_pass)[0]),static_cast<std::uint8_t>((*poll_human_pass)[1]));
        if (poll_ordinary_pass) {
            const auto result = captain::poll_ordinary_human_pass_cursor(state,
                static_cast<std::uint8_t>((*poll_ordinary_pass)[0]), static_cast<std::uint8_t>((*poll_ordinary_pass)[1]));
            state = result.state;
            std::cout << "Ordinary pass cursor | changed " << result.cursor_changed
                      << "; cursor presentation acknowledged, no frame or input timing simulated.\n";
        }
        if (shot_file) shot_defenders_file(state, content, *shot_file);
        if (staged_input) staged_defense(state,content,*staged_input);
        if (dispatch_elapsed) dispatch_shot(state,*dispatch_elapsed);
        if (resume_shot_clock) state = captain::resume_session_shot_clock(state);
        if (run_selected_result) resolve_shot_result(state,content);
        if (selected_recovery_file) recover_selected_file(state,content,*selected_recovery_file);
        if (run_continue_selected) state = captain::continue_session_selected_recovery(state);
        if (clearance_file) prepare_clearance_file(state,*clearance_file);
        if (race_file) clearance_race_file(state,content,*race_file);
        if (player3_race_file) clearance_race_file(state,content,*player3_race_file,false,true);
        if (human_race_recovery_file) clearance_race_file(state,content,*human_race_recovery_file,false,false,true);
        if (ordinary_free_race_file) clearance_race_file(state,content,*ordinary_free_race_file,false,false,false,true);
        if (run_continue_clearance) state=captain::continue_session_clearance(state);
        if (unopposed_attack) state=captain::continue_prepared_session_attack(state,*content.contact,*unopposed_attack).state;
        if(flight_file)begin_ball_flight(state,content,*flight_file,flight_fractions[0],flight_fractions[1]);
        if(run_arrival)state=captain::continue_session_unopposed_arrival(state);
        if(kickoff_file)prepare_kickoff_formation(state,*kickoff_file,kickoff_side);
        if(kickoff_receiver)state=captain::prepare_session_kickoff_receiver(state,(*kickoff_receiver)[0],
            {(*kickoff_receiver)[1],(*kickoff_receiver)[2],(*kickoff_receiver)[3],(*kickoff_receiver)[4],(*kickoff_receiver)[5]});
        if(run_kickoff_handoff)state=captain::continue_prepared_session_kickoff(state);
        if(run_kickoff_ball)state=captain::prepare_session_kickoff_ball(state);
        if(run_goal_restart)state=captain::continue_prepared_goal_restart(state);
        if(goal_baseline)state=captain::prepare_session_scored_goal(state,{(*goal_baseline)[0],(*goal_baseline)[1],(*goal_baseline)[2]},content);
        if(run_keeper_save)state=captain::resolve_session_keeper_save(state,content).state;
        if(run_punch_recovery)state=captain::advance_session_keeper_punch_recovery(state,content.field_players,*content.movement).state;
        if(keeper_choice)state=captain::resolve_prepared_session_keeper(state,(*keeper_choice)[0],(*keeper_choice)[1],(*keeper_choice)[2],content);
        if(area_special_pressed)poll_area_special(state,content,*area_special_pressed);
        if(run_profiled_area_prefix)state=captain::advance_profiled_area_prefix_clock(state,content).state;
        if(area_prefix_elapsed)state=captain::advance_session_area_prefix_clock(state,*area_prefix_elapsed).state;
        if(resume_area_prefix)state=captain::resume_session_area_prefix_clock(state);
        if(area_sequence_operation){const auto result=*area_sequence_operation==1?captain::present_session_area_contact_result(state):*area_sequence_operation==2?captain::advance_session_area_defender(state):captain::skip_session_area_defender(state);state=result.state;for(const auto& r:result.presentation)std::cout<<"Presentation request "<<unsigned(r.kind)<<" | id "<<unsigned(r.id)<<" | slot "<<unsigned(r.slot)<<'\n';}
        if(run_area_clock){if(!content.contact)throw std::invalid_argument("Missing contact timing content");state=captain::advance_session_area_contact_clock(state,*content.contact).state;}
        if(run_area_clock_resume)state=captain::resume_session_area_contact_clock(state);
        if(run_area_contact)state=captain::resolve_session_area_defender(state,content);
        if(run_area_result_dispatch){const auto result=captain::advance_session_area_result_dispatch(state,content);state=result.state;std::cout<<"Area result dispatched | route "<<unsigned(result.route)<<" | reward "<<(result.reward?unsigned(result.reward->award):0u)<<" | possession sound request "<<result.request_possession_sound<<'\n';}
        if(run_area_result_recovery){const auto result=captain::advance_session_area_result_recovery(state,content.field_players,*content.movement);state=result.state;std::cout<<"Area recovery | owner "<<unsigned(state.current.field.ball_owner)<<" | side "<<unsigned(state.current.field.side)<<"\n";}
        if(area_announce_facing){const auto result=captain::announce_prepared_area_defenders(state,*area_announce_facing);state=result.state;for(const auto& r:result.presentation){std::cout<<"Presentation request "<<unsigned(r.kind)<<" | id "<<unsigned(r.id)<<" | slot "<<unsigned(r.slot)<<'\n';}}
        if(run_area_defense_entry){const auto entry=captain::begin_prepared_area_defense(state);state=entry.state;for(const auto& r:entry.presentation)std::cout<<"Presentation request "<<unsigned(r.kind)<<" | id "<<unsigned(r.id)<<" | slot "<<unsigned(r.slot)<<'\n';}
        if(run_area_attack)state=captain::prepare_session_area_attack(state,content);
        if(sampled_field_ai)state=captain::capture_sampled_field_ai_choice(state,(*sampled_field_ai)[0],(*sampled_field_ai)[1]);
        if(sampled_home_shot_defender)state=captain::capture_sampled_home_shot_defender(state,(*sampled_home_shot_defender)[0],(*sampled_home_shot_defender)[1]);
        if(run_sampled_home_shot_defender)state=captain::resolve_sampled_home_shot_defender(state,content.field_players,content.defense,content.field_comparison);
        if(run_sampled_home_shot_contact) {
            if(!content.contact || !content.experience)throw std::invalid_argument("Missing contact or experience content");
            state=captain::advance_sampled_home_shot_contact(state,*content.contact,*content.experience);
        }
        if(run_sampled_home_shot_dispatch)state=captain::advance_sampled_home_shot_dispatch(state);
        if(sampled_home_shot_handoff_field) {
            std::ifstream captured(*sampled_home_shot_handoff_field,std::ios::binary);
            if(!captured)throw std::invalid_argument("Unable to open captured home shot field");
            state=captain::capture_sampled_home_shot_handoff(state,captain::load_field_state(captured));
        }
        if(sampled_field_ai_prep)state=captain::capture_sampled_field_ai_preparation(state,(*sampled_field_ai_prep)[0],(*sampled_field_ai_prep)[1]);
        if(sampled_field_ai_ack)state=captain::acknowledge_sampled_field_ai_presentation(state);
        if(sampled_field_ai_dispatch)state=captain::advance_sampled_field_ai_dispatch(state);
        if(sampled_field_ai_pass_handoff)state=captain::acknowledge_sampled_field_ai_pass_handoff(state);
        if(sampled_field_pass_control)state=captain::continue_sampled_field_pass_control(state);
        if(run_sampled_contact_attack)state=captain::prepare_sampled_field_contact_attack(state,content.field_players,content.attack);
        if(run_sampled_mixed_defender)state=captain::prepare_sampled_mixed_defender(state,content.field_players,content.defense);
        if(run_sampled_mixed_compare)state=captain::compare_sampled_mixed_defender(state,content.field_comparison);
        if(run_sampled_mixed_result4)state=captain::advance_sampled_mixed_result4(state);
        if(run_sampled_mixed_pass)state=captain::acknowledge_sampled_mixed_result4_pass(state);
        if(run_sampled_home_keeper)state=captain::compare_sampled_home_goalkeeper(state,content);
        if(run_sampled_opponent_goal)state=captain::prepare_sampled_opponent_goal(state,content);
        if(run_sampled_goal_presentation)state=captain::acknowledge_sampled_opponent_goal_presentation(state);
        if(run_sampled_goal_restart)state=captain::begin_sampled_opponent_goal_restart(state);
        if(run_keeper_overlay) {const auto r=captain::acknowledge_sampled_goalkeeper_overlay(state);state=r.state;
          for(const auto& q:r.presentation)std::cout<<"Presentation request "<<unsigned(q.kind)<<" | id "<<unsigned(q.id)<<" | slot "<<unsigned(q.slot)<<'\n';}
        if(sampled_keeper_poll) {const auto r=captain::poll_sampled_goalkeeper_prompt(state,static_cast<std::uint8_t>((*sampled_keeper_poll)[0]),
          {static_cast<std::uint8_t>((*sampled_keeper_poll)[1]),static_cast<std::uint8_t>((*sampled_keeper_poll)[2])});state=r.state;
          for(const auto& q:r.presentation)std::cout<<"Presentation request "<<unsigned(q.kind)<<" | id "<<unsigned(q.id)<<" | slot "<<unsigned(q.slot)<<'\n';}
        if(sampled_opponent_contact_ack)state=captain::acknowledge_sampled_opponent_contact_presentation(state);
        if(sampled_field_defense_blink)state=captain::acknowledge_sampled_field_defense_prompt(state,*sampled_field_defense_blink);
        if(sampled_field_pass_target) {
            auto t=*sampled_field_pass_target;
            if(sampled_pass_distance_file) {
                std::ifstream table_file(*sampled_pass_distance_file,std::ios::binary);
                if(!table_file)throw std::invalid_argument("Unable to open pass distance table");
                const auto table=captain::read_pass_distance_content(table_file);
                if(sampled_resolve_pass_target) {
                    std::uint8_t x=0,y=0;
                    if(t[0]!=255) {
                        const auto& recipient=state.current.field.motion[captain::field_motion_index(static_cast<std::uint8_t>(t[0]))];
                        x=static_cast<std::uint8_t>(recipient.x>>8);y=static_cast<std::uint8_t>(recipient.y>>8);
                    }
                    t[1]=captain::resolve_pass_target_cell(static_cast<std::uint8_t>(t[0]),static_cast<std::uint8_t>(t[1]),x,y);
                }
                t[3]=captain::calculate_pass_distance(state.current.field.ball_x,state.current.field.ball_y,static_cast<std::uint8_t>(t[1]),table);
            }
            state=(state.phase==captain::FieldSessionPhase::sampled_player3_pass_handoff || state.phase==captain::FieldSessionPhase::sampled_score25_pass_handoff)
                ? captain::capture_sampled_player3_pass_target(state,t[0],t[1],t[2],t[3])
                : captain::capture_sampled_field_pass_target(state,t[0],t[1],t[2],t[3]);
        }
        if(sampled_field_pass_flight_file) {
            if(!content.movement)throw std::invalid_argument("Missing movement geometry");
            const auto velocity=read_ball_velocity(*sampled_field_pass_flight_file);
            state=captain::begin_sampled_field_pass_flight(state,sampled_field_pass_fractions[0],sampled_field_pass_fractions[1],content.movement->geometry,velocity);
        }
        if(ordinary_pass_flight_file) {
            if(!content.movement)throw std::invalid_argument("Missing movement geometry");
            const auto velocity=read_ball_velocity(*ordinary_pass_flight_file);
            state=captain::begin_ordinary_pass_flight(state,ordinary_pass_fractions[0],ordinary_pass_fractions[1],content.movement->geometry,velocity);
        }
        if(sampled_field_ai_pass_field) {
            std::ifstream captured(*sampled_field_ai_pass_field,std::ios::binary);
            if(!captured)throw std::invalid_argument("Cannot open captured pass field");
            state=captain::acknowledge_sampled_field_ai_pass_presentation(state,captain::load_field_state(captured));
        }
        if(run_area_ai){if(!content.ai_area_choice)throw std::invalid_argument("Missing private AI content");state=captain::choose_session_area_action(state,*content.ai_area_choice);}
        if(run_continue_area_special)continue_area_special(state,content);
        if(run_redraw_area_special)redraw_area_special(state,content);
        if(run_resolve_area_special)resolve_area_special(state,content);
        if(run_open_area_special)open_area_special(state,content);
        if(run_inspect_area_defense)inspect_area_defense(state,content);
        if(run_prepare_area_defense)prepare_area_defense(state,content);
        if(area_defense_pressed)poll_area_defense(state,content,*area_defense_pressed);
        if(run_area_defense_prompt)open_area_defense_prompt(state,content);
        if(area_defense_blink)state=captain::begin_keeper_area_defense(state,static_cast<std::uint8_t>(*area_defense_blink));
        if(area_menu_facing)open_keeper_area_menu(state,content,*area_menu_facing);
        if(run_keeper_area)state=captain::prepare_session_keeper_area(state);
        if(run_keeper_entry)state=captain::prepare_session_keeper_entry(state);
        if(stunned_thresholds)state=captain::resolve_session_stunned_keeper(state,*stunned_thresholds);
        if(run_shot_recovery)state=captain::advance_session_shot_loose_recovery(state,content.field_players,*content.movement).state;
        if(run_ball_cell)state=captain::resolve_session_ball_cell(state);
        if(run_ball_step)state=captain::advance_prepared_session_ball_cell(state).state;
        if (run_approach) {
            if (!content.approach) throw std::invalid_argument("Missing shot approach content");
            state=captain::prepare_session_shot_approach(state,*content.approach).state;
        }
        if (run_attack_clock) state=captain::continue_prepared_session_attack(state,*content.contact).state;
        if (run_resume_attack) state=captain::resume_session_attack_clock(state);
        if (run_restart_options) state=captain::open_prepared_restart_options(state).state;
        if (restart_input) state=captain::poll_prepared_restart_options(state,(*restart_input)[0],(*restart_input)[1]).state;
        if (run_restart_route) state=captain::route_session_selected_restart(state).state;
        if (elapsed) advance(state, content, *elapsed);
        show(state);
        if (!capture.empty()) save(capture, state);
        if (!interactive) return 0;
        std::cout << "Commands: state, contact/rewarded-contact CODE DEFENDER ACTION SUBTYPE, resume, recover, continue-recovery, setup-control, poll-control HELD PRESSED, advance-control, resume-control-time, advance-world HELD PRESSED, advance-world-file FILE, open-action-menu, menu-input PRESSED, prepare-basic-shot, shot-defenders-file FILE, step-shot-defense ACTION SUBTYPE RANDOM1 RANDOM2, resume-shot-clock, dispatch-shot ELAPSED, resolve-shot-result, recover-selected-file FILE, continue-selected-recovery, prepare-clearance-file FILE, clearance-race-file FILE, continue-clearance, begin-ball-flight FILE LOWX LOWY, continue-flight-arrival, recover-shot-loose, prepare-kickoff-formation FILE SIDE, continue-prepared-goal-restart, prepare-scored-goal LEFT RIGHT KICKOFF, resolve-keeper-save, recover-keeper-punch, poll-keeper-area-special PRESSED, open-keeper-area-special, inspect-keeper-area-defense, prepare-keeper-area-defense, poll-keeper-area-defense PRESSED, open-keeper-area-defense, begin-keeper-area-defense BLINK, open-prepared-keeper-area-menu FACING, prepare-keeper-area, prepare-keeper-entry, resolve-prepared-keeper ACTION SUBTYPE OPPONENT_TEAM, resolve-stunned-keeper T0 T1, resolve-ball-cell, advance-ball-cell, prepare-shot-approach, continue-unopposed-attack ACTION, continue-prepared-attack, resume-attack-clock, dispatch-area-result, recover-area-result, route-selected-restart, open-prepared-restart-options, restart-options-input HELD PRESSED, advance N, save NEW.cmjsession, quit\n";
        std::cout << "Area flight: begin-area-flight FILE LX LY DIST FACING MOTION VXLO VXHI VYLO VYHI; advance-area-ball-cell; resolve-area-ball-cell; continue-area-already-arrived; ack-area-pass-defender; prepare-area-pass-defender COSTLO COSTHI; compare-area-pass-defender FILE POWERLO POWERHI CONTEXT; begin-area-action4-encounters; continue-area-action4-control; prepare-area-action4-control\n"
                     "Area actions: continue-area-action4 FACING MOTION; continue-area-action6 FILE FACING MOTION; continue-area-pass-received; prepare-area-pass-keeper\n";
        std::string line;
        while (std::cout << "> " && std::getline(std::cin, line)) {
            try {
                std::istringstream input(line);
                std::string command, value, extra;
                if (!(input >> command)) continue;
                if (command == "state" || command == "quit" || command == "resume" || command == "dispatch-area-result" || command == "recover-area-result" ||
                    command == "recover" || command == "continue-recovery" || command == "setup-control" ||
                    command == "advance-control" || command == "resume-control-time" || command == "resume-shot-clock" || command == "resolve-shot-result" || command == "continue-selected-recovery" || command == "continue-clearance" || command == "route-selected-restart" || command == "open-prepared-restart-options" || command == "continue-prepared-attack" || command == "resume-attack-clock" || command == "prepare-shot-approach" || command == "advance-ball-cell" || command == "resolve-ball-cell" || command == "continue-flight-arrival" || command == "recover-shot-loose" || command == "prepare-keeper-entry" || command == "resolve-keeper-save" || command == "recover-keeper-punch" || command == "continue-prepared-goal-restart") {
                    if (input >> extra) throw std::invalid_argument("Command takes no arguments");
                    if (command == "quit") break;
                    if(command=="dispatch-area-result")state=captain::advance_session_area_result_dispatch(state,content).state;
                    if(command=="recover-area-result")state=captain::advance_session_area_result_recovery(state,content.field_players,*content.movement).state;
                    if(command=="continue-prepared-goal-restart")state=captain::continue_prepared_goal_restart(state);
                    if(command=="resolve-keeper-save")state=captain::resolve_session_keeper_save(state,content).state;
                    if(command=="recover-keeper-punch")state=captain::advance_session_keeper_punch_recovery(state,content.field_players,*content.movement).state;
                    if(command=="prepare-keeper-entry")state=captain::prepare_session_keeper_entry(state);
                    if(command=="continue-flight-arrival")state=captain::continue_session_unopposed_arrival(state);
                    if(command=="recover-shot-loose")state=captain::advance_session_shot_loose_recovery(state,content.field_players,*content.movement).state;
                    if(command=="resolve-ball-cell")state=captain::resolve_session_ball_cell(state);
                    if(command=="advance-ball-cell")state=captain::advance_prepared_session_ball_cell(state).state;
                    if (command == "prepare-shot-approach") {
                        if(!content.approach) throw std::invalid_argument("Missing shot approach content");
                        state=captain::prepare_session_shot_approach(state,*content.approach).state;
                    }
                    if (command == "continue-prepared-attack") state=captain::continue_prepared_session_attack(state,*content.contact).state;
                    if (command == "resume-attack-clock") state=captain::resume_session_attack_clock(state);
                    if (command == "open-prepared-restart-options") state=captain::open_prepared_restart_options(state).state;
                    if (command == "route-selected-restart") state=captain::route_session_selected_restart(state).state;
                    if (command == "continue-clearance") state=captain::continue_session_clearance(state);
                    if (command == "continue-selected-recovery") state = captain::continue_session_selected_recovery(state);
                    if (command == "resolve-shot-result") resolve_shot_result(state,content);
                    if (command == "resume-shot-clock") state = captain::resume_session_shot_clock(state);
                    if (command == "resume") resume(state, content);
                    if (command == "recover") recover(state, content);
                    if (command == "continue-recovery") continue_recovery(state);
                    if (command == "setup-control") setup_control(state);
                    if (command == "advance-control" || command == "resume-control-time")
                        advance_control(state, content, command == "resume-control-time");
                    show(state);
                } else if (command == "poll-keeper-area-special") {
                    if(!(input>>value)||(input>>extra))throw std::invalid_argument("Expected sampled pressed byte");
                    poll_area_special(state,content,number(value));show(state);
                } else if(command=="advance-profiled-area-prefix-clock") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::advance_profiled_area_prefix_clock(state,content).state;show(state);
                } else if(command=="advance-area-prefix-clock") {
                    if(!(input>>value) || (input>>extra))throw std::invalid_argument("Expected area prefix elapsed byte");
                    state=captain::advance_session_area_prefix_clock(state,number(value)).state;show(state);
                } else if(command=="resume-area-prefix-clock") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::resume_session_area_prefix_clock(state);show(state);
                } else if(command=="present-area-contact-result" || command=="advance-area-defender" || command=="skip-area-defender") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    const auto result=command=="present-area-contact-result"?captain::present_session_area_contact_result(state):command=="advance-area-defender"?captain::advance_session_area_defender(state):captain::skip_session_area_defender(state);
                    state=result.state;for(const auto& r:result.presentation)std::cout<<"Presentation request "<<unsigned(r.kind)<<" | id "<<unsigned(r.id)<<" | slot "<<unsigned(r.slot)<<'\n';show(state);
                } else if (command == "advance-area-contact-clock") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    if(!content.contact)throw std::invalid_argument("Missing contact timing content");
                    state=captain::advance_session_area_contact_clock(state,*content.contact).state;show(state);
                } else if (command == "resume-area-contact-clock") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::resume_session_area_contact_clock(state);show(state);
                } else if (command == "resolve-area-defender") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::resolve_session_area_defender(state,content);show(state);
                } else if (command == "prepare-area-action6-control") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::prepare_session_area_action6_control(state).state;show(state);
                } else if (command == "continue-area-action6-recovery") {
                    std::string second;
                    if(!(input>>value>>second) || (input>>extra))throw std::invalid_argument("Expected captured facing and motion");
                    state=captain::continue_session_area_action6_recovery(state,number(value),number(second));show(state);
                } else if (command == "recover-area-action6") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::advance_session_area_action6_recovery(state,content.field_players,*content.movement).state;show(state);
                } else if (command == "continue-area-action6-winner") {
                    std::string second;
                    if(!(input>>value>>second) || (input>>extra))throw std::invalid_argument("Expected captured facing and motion");
                    state=captain::continue_session_area_action6_winner(state,number(value),number(second));show(state);
                } else if (command == "area-action6-race-file") {
                    std::string file;
                    if(!(input>>std::quoted(file)) || (input>>extra))throw std::invalid_argument("Expected private area action6 race file");
                    clearance_race_file(state,content,file,true);show(state);
                } else if (command == "continue-area-action6") {
                    std::string file,first,second;
                    if(!(input>>std::quoted(file)>>first>>second) || (input>>extra))throw std::invalid_argument("Expected content file, facing and motion");
                    continue_area_action6(state,file,number(first),number(second),content);show(state);
                } else if (command == "continue-area-action4") {
                    std::string second;
                    if(!(input>>value>>second) || (input>>extra))throw std::invalid_argument("Expected captured facing and motion");
                    state=captain::continue_session_area_action4(state,number(value),number(second),content.experience ? &*content.experience : nullptr);show(state);
                } else if (command == "begin-area-action4-encounters") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::begin_session_area_action4_encounters(state);show(state);
                } else if (command == "continue-area-action4-control") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::continue_session_area_action4_control(state);show(state);
                } else if (command == "prepare-area-action4-control") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::prepare_session_area_action4_control(state).state;show(state);
                } else if (command == "begin-area-action4-keeper") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::begin_session_area_action4_keeper(state);show(state);
                } else if (command == "ack-area-action4-keeper") {
                    std::string second;
                    if(!(input>>value>>second) || (input>>extra))throw std::invalid_argument("Expected captured keeper facing and motion");
                    state=captain::acknowledge_session_area_action4_keeper(state,number(value),number(second));show(state);
                } else if (command == "prepare-area-action4-keeper") {
                    std::string second,third,fourth;
                    if(!(input>>value>>second>>third>>fourth) || (input>>extra))throw std::invalid_argument("Expected captured keeper cost and power bytes");
                    state=captain::prepare_session_area_action4_keeper(state,number(value) | (number(second)<<8),number(third) | (number(fourth)<<8));show(state);
                } else if (command == "prepare-area-pass-keeper") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::prepare_area_pass_keeper(state);show(state);
                } else if (command == "continue-area-pass-received") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::continue_area_pass_received(state);show(state);
                } else if (command == "prepare-area-pass-blocked-control") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::prepare_session_area_pass_blocked_control(state).state;show(state);
                } else if (command == "prepare-area-pass-capture-control") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::prepare_session_area_pass_capture_control(state).state;show(state);
                } else if (command == "continue-area-pass-capture") {
                    std::string second;if(!(input>>value>>second) || (input>>extra) || !content.movement)throw std::invalid_argument("Expected captured facing, motion and movement content");
                    state=captain::continue_area_pass_capture(state,content.field_players,*content.movement,number(value),number(second));show(state);
                } else if (command == "capture-area-pass-defender") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    const auto captured=captain::capture_area_pass_defender(state);state=captured.state;std::cout<<"Possession sound request "<<captured.request_possession_sound<<'\n';show(state);
                } else if (command == "continue-area-pass-blocked-recovery") {
                    std::string second;if(!(input>>value>>second) || (input>>extra))throw std::invalid_argument("Expected captured facing and motion");
                    state=captain::continue_session_area_pass_blocked_recovery(state,number(value),number(second));show(state);
                } else if (command == "recover-area-pass-blocked") {
                    if(input>>extra || !content.movement)throw std::invalid_argument("Blocked pass recovery needs movement content");
                    const auto recovered=captain::advance_session_area_pass_blocked_recovery(state,content.field_players,*content.movement);state=recovered.state;
                    std::cout<<"Possession sound request "<<recovered.request_possession_sound<<'\n';show(state);
                } else if (command == "enter-area-pass-blocked-recovery") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::enter_area_pass_blocked_recovery(state);show(state);
                } else if (command == "resume-area-pass-added-time") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::resume_area_pass_added_time(state);show(state);
                } else if (command == "advance-area-pass-terminal") {
                    if(input>>extra || !content.contact || !content.experience)throw std::invalid_argument("Pass terminal clock needs contact and experience content");
                    state=captain::advance_area_pass_terminal(state,*content.contact,*content.experience);show(state);
                } else if (command == "advance-area-pass-blocked") {
                    if(input>>extra || !content.experience)throw std::invalid_argument("Blocked pass requires experience content");
                    state=captain::advance_area_pass_blocked(state,*content.experience);show(state);
                } else if (command == "advance-area-pass-defender") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::advance_area_pass_defender(state);show(state);
                } else if (command == "advance-area-pass-nonterminal") {
                    if(input>>extra || !content.contact || !content.experience)throw std::invalid_argument("Pass handler needs contact and experience content");
                    state=captain::advance_area_pass_nonterminal(state,*content.contact,*content.experience);show(state);
                } else if (command == "ack-area-pass-result") {
                    std::string second;if(!(input>>value>>second) || (input>>extra))throw std::invalid_argument("Expected captured facing and motion");
                    state=captain::acknowledge_area_pass_result(state,number(value),number(second));show(state);
                } else if (command == "restore-area-pass-variant") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::restore_area_pass_ball_variant(state);show(state);
                } else if (command == "compare-area-pass-defender") {
                    std::string file,low,high,context;
                    if(!(input>>std::quoted(file)>>low>>high>>context) || (input>>extra))throw std::invalid_argument("Expected file, power low/high and context");
                    compare_area_pass(state,file,number(low),number(high),number(context));show(state);
                } else if (command == "prepare-area-pass-defender") {
                    std::string second;if(!(input>>value>>second) || (input>>extra))throw std::invalid_argument("Expected captured cost low and high bytes");
                    state=captain::prepare_area_pass_defender(state,static_cast<std::uint16_t>(number(value)|(number(second)<<8)));show(state);
                } else if (command == "ack-area-pass-defender") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::acknowledge_area_pass_defender_announcement(state);show(state);
                } else if (command == "continue-area-already-arrived" || command == "continue-area-unopposed-arrival") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::continue_area_already_arrived(state);show(state);
                } else if (command == "resolve-area-ball-cell") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::resolve_session_area_ball_cell(state);show(state);
                } else if (command == "advance-area-ball-cell") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::advance_prepared_area_ball_cell(state).state;show(state);
                } else if (command == "begin-area-flight") {
                    std::string file,token;
                    if(!(input>>std::quoted(file)))throw std::invalid_argument("Expected area flight file and nine bytes");
                    std::array<unsigned,9> v{};
                    for(auto& item:v){if(!(input>>token))throw std::invalid_argument("Expected area flight file and nine bytes");item=number(token);}
                    if(input>>extra)throw std::invalid_argument("Unexpected area flight argument");
                    begin_area_flight(state,content,file,v);show(state);
                } else if (command == "continue-area-ordinary-recovery") {
                    if(!(input>>value) || (input>>extra))throw std::invalid_argument("Expected captured facing");
                    state=captain::continue_session_area_ordinary_recovery(state,number(value));show(state);
                } else if (command == "continue-area-story-recovery") {
                    if(!(input>>value) || (input>>extra))throw std::invalid_argument("Expected captured facing");
                    state=captain::continue_session_area_story_recovery(state,number(value));show(state);
                } else if (command == "announce-prepared-area-defenders") {
                    if(!(input>>value) || (input>>extra))throw std::invalid_argument("Expected captured facing");
                    const auto result=captain::announce_prepared_area_defenders(state,number(value));state=result.state;
                    for(const auto& r:result.presentation){std::cout<<"Presentation request "<<unsigned(r.kind)<<" | id "<<unsigned(r.id)<<" | slot "<<unsigned(r.slot)<<'\n';}show(state);
                } else if (command == "begin-prepared-area-defense") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    const auto result=captain::begin_prepared_area_defense(state);state=result.state;
                    for(const auto& r:result.presentation){std::cout<<"Presentation request "<<unsigned(r.kind)<<" | id "<<unsigned(r.id)<<" | slot "<<unsigned(r.slot)<<'\n';}
                    show(state);
                } else if (command == "prepare-opponent-area-attack") {
                    if(input>>extra)throw std::invalid_argument("Expected no arguments");
                    state=captain::prepare_session_area_attack(state,content);show(state);
                } else if (command == "choose-keeper-area-action") {
                    if(input>>extra || !content.ai_area_choice)throw std::invalid_argument("Expected private AI content and no arguments");
                    state=captain::choose_session_area_action(state,*content.ai_area_choice);show(state);
                } else if (command == "continue-keeper-area-special") {
                    if(input>>extra)throw std::invalid_argument("Expected continue-keeper-area-special without arguments");
                    continue_area_special(state,content);show(state);
                } else if (command == "redraw-keeper-area-special") {
                    if(input>>extra)throw std::invalid_argument("Expected redraw-keeper-area-special without arguments");
                    redraw_area_special(state,content);show(state);
                } else if (command == "resolve-keeper-area-special") {
                    if(input>>extra)throw std::invalid_argument("Expected resolve-keeper-area-special without arguments");
                    resolve_area_special(state,content);show(state);
                } else if (command == "open-keeper-area-special") {
                    if(input>>extra)throw std::invalid_argument("Expected open-keeper-area-special without arguments");
                    open_area_special(state,content);show(state);
                } else if (command == "inspect-keeper-area-defense") {
                    if(input>>extra)throw std::invalid_argument("Expected inspect-keeper-area-defense without arguments");
                    inspect_area_defense(state,content);show(state);
                } else if (command == "prepare-keeper-area-defense") {
                    if(input>>extra)throw std::invalid_argument("Expected prepare-keeper-area-defense without arguments");
                    prepare_area_defense(state,content);show(state);
                } else if (command == "poll-keeper-area-defense") {
                    if(!(input>>value)||(input>>extra))throw std::invalid_argument("Expected sampled pressed byte");
                    poll_area_defense(state,content,number(value));show(state);
                } else if (command == "open-keeper-area-defense") {
                    if(input>>extra)throw std::invalid_argument("Expected open-keeper-area-defense without arguments");
                    open_area_defense_prompt(state,content);show(state);
                } else if (command == "begin-keeper-area-defense") {
                    if(!(input>>value)||(input>>extra))throw std::invalid_argument("Expected captured blink counter");
                    state=captain::begin_keeper_area_defense(state,number(value));show(state);
                } else if (command == "open-prepared-keeper-area-menu") {
                    if(!(input>>value)||(input>>extra))throw std::invalid_argument("Expected captured facing");
                    open_keeper_area_menu(state,content,number(value));show(state);
                } else if (command == "prepare-keeper-area") {
                    if(input>>extra)throw std::invalid_argument("Expected prepare-keeper-area without arguments");
                    state=captain::prepare_session_keeper_area(state);show(state);
                } else if (command == "prepare-kickoff-formation") {
                    std::string file,side;
                    if(!(input>>std::quoted(file)>>side)||(input>>extra))throw std::invalid_argument("Expected prepare-kickoff-formation FILE SIDE");
                    prepare_kickoff_formation(state,file,number(side));show(state);
                } else if (command == "prepare-scored-goal") {
                    std::string left,right,kickoff;
                    if(!(input>>left>>right>>kickoff)||(input>>extra))throw std::invalid_argument("Expected prepare-scored-goal LEFT RIGHT KICKOFF");
                    state=captain::prepare_session_scored_goal(state,{static_cast<std::uint8_t>(number(left)),static_cast<std::uint8_t>(number(right)),static_cast<std::uint8_t>(number(kickoff))},content);show(state);
                } else if (command == "resolve-prepared-keeper") {
                    std::string action,subtype,team;
                    if(!(input>>action>>subtype>>team)||(input>>extra))throw std::invalid_argument("Expected resolve-prepared-keeper ACTION SUBTYPE OPPONENT_TEAM");
                    state=captain::resolve_prepared_session_keeper(state,number(action),number(subtype),number(team),content);show(state);
                } else if (command == "resolve-stunned-keeper") {
                    std::string t0,t1;
                    if(!(input>>t0>>t1)||(input>>extra))throw std::invalid_argument("Expected resolve-stunned-keeper T0 T1");
                    state=captain::resolve_session_stunned_keeper(state,{static_cast<std::uint8_t>(number(t0)),static_cast<std::uint8_t>(number(t1))});show(state);
                } else if (command == "begin-ball-flight") {
                    std::string file,lx,ly;
                    if(!(input>>std::quoted(file)>>lx>>ly)||(input>>extra))throw std::invalid_argument("Expected begin-ball-flight FILE LOWX LOWY");
                    begin_ball_flight(state,content,file,number(lx),number(ly));show(state);
                } else if (command == "continue-unopposed-attack") {
                    if (!(input >> value) || (input >> extra)) throw std::invalid_argument("Expected continue-unopposed-attack ACTION");
                    state=captain::continue_prepared_session_attack(state,*content.contact,number(value)).state;
                    show(state);
                } else if (command == "poll-control" || command == "advance-world" || command == "restart-options-input") {
                    std::string pressed;
                    if (!(input >> value >> pressed) || (input >> extra))
                        throw std::invalid_argument("Expected " + command + " HELD PRESSED");
                    if (command == "restart-options-input") state=captain::poll_prepared_restart_options(state,number(value),number(pressed)).state;
                    else if (command == "poll-control") poll_control(state, content, number(value), number(pressed));
                    else advance_world(state, content, number(value), number(pressed));
                    show(state);
                } else if (command == "open-action-menu" || command == "prepare-basic-shot") {
                    if (input >> extra) throw std::invalid_argument("Command takes no arguments");
                    if (command == "open-action-menu") open_action_menu(state, content);
                    else prepare_basic_shot(state, content);
                    show(state);
                } else if (command == "step-shot-defense") {
                    std::array<unsigned,4> values{};
                    for (auto& entry : values) {
                        if (!(input >> value)) throw std::invalid_argument("Expected four defense bytes");
                        entry = number(value);
                    }
                    if (input >> extra) throw std::invalid_argument("Expected exactly four defense bytes");
                    staged_defense(state,content,values);show(state);
                } else if (command == "dispatch-shot") {
                    if (!(input >> value) || (input >> extra)) throw std::invalid_argument("Expected dispatch-shot ELAPSED");
                    dispatch_shot(state,number(value));show(state);
                } else if (command == "menu-input") {
                    if (!(input >> value) || (input >> extra)) throw std::invalid_argument("Expected menu-input PRESSED");
                    menu_input(state, content, number(value));
                    show(state);
                } else if (command == "advance" || command == "save" || command == "advance-world-file" || command == "shot-defenders-file" || command == "recover-selected-file" || command == "prepare-clearance-file" || command == "clearance-race-file") {
                    if (!(input >> value) || (input >> extra)) throw std::invalid_argument("Expected command and one value");
                    if (command == "save") save(value, state);
                    else if (command == "clearance-race-file") { clearance_race_file(state,content,value); show(state); }
                    else if (command == "prepare-clearance-file") { prepare_clearance_file(state,value); show(state); }
                    else if (command == "recover-selected-file") { recover_selected_file(state,content,value); show(state); }
                    else if (command == "shot-defenders-file") { shot_defenders_file(state, content, value); show(state); }
                    else if (command == "advance-world-file") { advance_world_file(state, content, value); show(state); }
                    else { advance(state, content, number(value)); show(state); }
                } else if (command == "contact" || command == "rewarded-contact") {
                    std::string id, defense, kind;
                    if (!(input >> value >> id >> defense >> kind) || (input >> extra))
                        throw std::invalid_argument("Expected contact CODE DEFENDER ACTION SUBTYPE");
                    contact(state, content, number(value), number(id), number(defense), number(kind), command == "rewarded-contact");
                    show(state);
                } else throw std::invalid_argument("Unknown session command");
            } catch (const std::exception& error) { std::cout << "Error: " << error.what() << '\n'; }
        }
        return std::cin.bad() ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << "Phase session stopped: " << error.what() << '\n';
        return 1;
    }
}

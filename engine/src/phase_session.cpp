#include "captain/phase_session.hpp"
#include "captain/defender_selection.hpp"
#include "captain/defense_affordability.hpp"
#include "captain/pass_target.hpp"
#include <stdexcept>
#include <string>
#include <utility>

namespace captain {
namespace {
void validate_contact(const FieldPhaseState& field, const PendingTerminalContact& contact) {
    if ((contact.result_code != 2 && contact.result_code != 3) ||
        (contact.defense_action != 0 && contact.defense_action != 2) ||
        (contact.defense_subtype != 0 && contact.defense_subtype != 128))
        throw std::invalid_argument("Unsupported terminal contact");
    field_motion_index(contact.defender_id);
    if ((contact.defender_id < 11 ? 0 : 11) == field.side)
        throw std::invalid_argument("Terminal contact requires an opposing field player");
}
PhaseSessionResult finish(PhaseSessionState state, PendingTerminalContact contact,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement) {
    state.pending.reset();
    TerminalFlightPhaseResult terminal;
    if (contact.result_code == 2) {
        // Placement happened before the clock. Do not re-read the defender's
        // position after a deferred presentation; preserve that earlier write.
        state.current.field.selected_defenders = 0;
        if (state.scheduling) state.scheduling->selected_ids.fill(0);
        terminal = {state.current.field, TerminalFlightNextPhase::ball_recovery, std::nullopt};
        state.phase = FieldSessionPhase::ball_recovery;
    } else {
        terminal = resolve_prepared_terminal_flight({state.current.field, contact.result_code,
            contact.defender_id, contact.defense_action, contact.defense_subtype}, primary, movement);
        state.current.field = terminal.state;
        state.phase = FieldSessionPhase::player_control;
    }
    return {std::move(state), std::nullopt, std::move(terminal)};
}
}

void validate_phase_session(const PhaseSessionState& state) {
    if(state.saved_ball_flight_flag>1)throw std::invalid_argument("Unsupported saved ball flight flag");
    if ((state.current.field.side != 0 && state.current.field.side != 11) ||
        state.current.field.ball_owner > 21)
        throw std::invalid_argument("Unsupported phase session state");
    const auto phase = static_cast<unsigned>(state.phase);
    const bool player14_pass=phase>=256 && phase<=261;
    const bool player19_return_shot=phase>=262 && phase<=267;
    const bool player19_return_keeper=phase>=268 && phase<=275;
    const bool player19_goal_possession=phase==274 || phase==275;
    const bool score23_human_pass=phase>=276 && phase<=284;
    const bool score23_defender=phase>=280 && phase<=284;
    const bool score23_shot_preflight=phase>=285 && phase<=290;
    const bool score23_shot_flight=phase>=291 && phase<=294;
    const bool score23_keeper=phase>=295 && phase<=302;
    const bool score23_opponent_shot=score23_shot_preflight || score23_shot_flight || score23_keeper;
    const bool score24_dribble=phase>=303 && phase<=309;
    const bool score25_human_pass=phase>=310 && phase<=326;
    const bool score36_human_pass=score25_human_pass && state.score && state.score->left_goals==3 && state.score->right_goals==6;
    const bool score25_shot=phase>=327 && phase<=330;
    const bool ordinary_human_target=phase>=331 && phase<=344;
    if (state.ordinary_pass_candidates && !ordinary_human_target)
        throw std::invalid_argument("Ordinary pass candidate context requires the target phase");
    if (bool(state.ordinary_pass_recipient) != (phase>=332 && phase<=344))
        throw std::invalid_argument("Ordinary pass recipient requires the confirmed target phase");
    if (bool(state.ordinary_pass_defenses) != (phase>=333 && phase<=344))
        throw std::invalid_argument("Ordinary pass defense context requires its selection phase");
    const bool score35_opponent_shot=state.score && state.score->left_goals==3 && (state.score->right_goals==5 || (phase>=300 && state.score->right_goals==6)) && state.current.field.ball_owner==19;
    const bool player17_score24=state.score && state.score->left_goals==2 && state.score->right_goals==4 && state.current.field.ball_owner==17;
    const bool player17_keeper=score23_keeper && state.current.field.ball_owner==17;
    const bool postgoal_opponent_action=phase>=392 && phase<=414;
    const bool extended_opponent=player14_pass || player19_return_shot || player19_return_keeper || score23_opponent_shot || postgoal_opponent_action;
    if(phase>255 && !(phase>=415 && phase<=427) && !(phase>=345 && phase<=391) && !extended_opponent && !score23_human_pass && !score24_dribble && !score25_human_pass && !score25_shot && !ordinary_human_target)throw std::invalid_argument("Unsupported extended phase session state");
    if (ordinary_human_target) {
        const auto& field = state.current.field;
        if (!state.action || !state.recovery || !state.control || !state.experience || !state.scheduling ||
            !state.human_pass_cursor || state.human_pass_steps || field.side != 0 || field.ball_owner < 1 || field.ball_owner > 10 || field.selected_defenders > 5 ||
            state.action->action != 1 || state.action->menu_route != 0 || state.current.clock.ball_mode != (phase>=333 ? 0 : 129) ||
            (phase>=335 ? (state.recovery->attack_subtype != 0 && state.recovery->attack_subtype != 128) : state.recovery->attack_subtype != 0) || *state.human_pass_cursor >= 240 ||
            field_cell_at(field.ball_x, field.ball_y) != field.ball_cell ||
            (field.motion[field_motion_index(field.ball_owner)].x >> 8) != field.ball_x ||
            (field.motion[field_motion_index(field.ball_owner)].y >> 8) != field.ball_y)
            throw std::invalid_argument("Invalid ordinary human pass target boundary");
        for (unsigned i = 0; i < field.selected_defenders; ++i)
            if (state.scheduling->selected_ids[i] < 12 || state.scheduling->selected_ids[i] > 21)
                throw std::invalid_argument("Ordinary pass target requires opposing field defenders");
        if (state.ordinary_pass_candidates) {
            const auto& context = *state.ordinary_pass_candidates;
            const auto& candidates = context.candidates;
            if (candidates.count > 4 || candidates.allied_count > candidates.count ||
                (!context.scanned && (candidates.count || candidates.allied_count)))
                throw std::invalid_argument("Invalid ordinary pass candidate counts");
            for (unsigned i = 0; i < 4; ++i) {
                const auto id = candidates.ids[i];
                if (i >= candidates.count) {
                    if (id) throw std::invalid_argument("Unused pass candidates must be zero");
                } else if (id == field.ball_owner || id == 0 || id == 11 || id > 21 ||
                           (i && id <= candidates.ids[i-1]) ||
                           ((i < candidates.allied_count) != (id < 11)))
                    throw std::invalid_argument("Invalid ordinary pass candidate prefix");
            }
            if (candidates.allied_count && candidates.allied_count != candidates.count)
                throw std::invalid_argument("Allied pass candidates exclude opponents");
        }
        if (phase>=332) {
            if (!state.ordinary_pass_candidates || state.ordinary_pass_candidates->candidates.allied_count > 1)
                throw std::invalid_argument("Confirmed ordinary pass requires a known single or empty target");
            const auto& candidates = state.ordinary_pass_candidates->candidates;
            const auto expected = candidates.allied_count == 1 ? candidates.ids[0] : 255;
            if (*state.ordinary_pass_recipient != expected)
                throw std::invalid_argument("Confirmed ordinary pass recipient disagrees with saved candidates");
        }
        if (state.ordinary_pass_defenses) {
            const auto& defense = *state.ordinary_pass_defenses;
            if (!field.selected_defenders || defense.count > field.selected_defenders ||
                (phase==333 && defense.count >= field.selected_defenders) ||
                (phase>=334 && defense.count != field.selected_defenders) ||
                (phase>=336 && state.area_defender_cursor>=defense.count) ||
                (defense.count && state.recovery->opposing_player != state.scheduling->selected_ids[phase>=336 ? state.area_defender_cursor : defense.count-1]))
                throw std::invalid_argument("Invalid ordinary pass defender selection cursor");
            for (unsigned i=0; i<5; ++i)
                if (defense.subtypes[i] || defense.actions[i] != (i < defense.count ? 1 : 0))
                    throw std::invalid_argument("Only explicitly captured ordinary action-one defenses are supported");
        }
        if ((phase>=336 && phase<=344) && (!state.area_contact || state.area_defender_cursor>=field.selected_defenders ||
            (phase<342 && ((phase<=338) != (state.area_defender_cursor==0))) || state.area_sequence_result!=((phase==336 || phase==339) ? 255 : 0) ||
            state.area_contact->cursor!=state.area_defender_cursor || state.area_contact->defense_action!=1 || state.area_contact->defense_subtype ||
            ((phase==336 || phase==339) && (state.area_contact->strength || state.area_contact->loss || state.area_contact->score)) ||
            ((phase==337 || phase==338 || phase==340 || phase==341 || phase==342 || phase==343 || phase==344) && state.area_contact->loss!=state.area_contact->strength/4) ||
            state.area_contact->result || state.area_contact->weakened || state.control->facing!=64 ||
            state.control->last_carrier_cell!=128 || state.control->motion_requested ||
            (state.recovery->ball_variant!=1 && state.recovery->ball_variant!=2)))
            throw std::invalid_argument("Invalid ordinary pass defender preparation");
    }
    if(score25_shot && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       !state.shot_flight || state.shot_flight->distance!=99 || state.shot_flight->destination!=233 || state.shot_flight->counter!=255 || state.shot_flight->loose_ball ||
       !((state.score->left_goals==2 && state.score->right_goals==5 && state.current.clock.remaining==304 && state.area_defender_cursor==5) ||
         (state.score->left_goals==3 && state.score->right_goals==6 && state.current.clock.remaining==263 && state.area_defender_cursor==4)) || state.score->kickoff!=4 || state.current.clock.extra_time!=13 ||
       state.current.field.side!=0 || state.current.field.ball_owner!=7 || state.current.field.elapsed || state.current.field.movement_region!=135 ||
       (phase==327 && (state.current.field.ball_cell!=103 || state.current.field.ball_x!=(state.score->left_goals==3 ? 113 : 117) || state.current.field.ball_y!=(state.score->left_goals==3 ? 143 : 139) || state.current.field.selected_defenders!=(state.score->left_goals==3 ? 4 : 5))) ||
       (phase>=328 && state.current.field.selected_defenders) || state.action->action!=0 || state.action->power!=22 ||
       state.recovery->attack_subtype!=128 || state.saved_ball_flight_flag!=(phase==327 ? 1 : 0) || state.human_defense_choices))
        throw std::invalid_argument("Invalid player7 shot target or flight");
    if(score25_human_pass && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       !state.human_pass_cursor || state.human_pass_steps>5 ||
       (state.human_pass_steps<=5 && *state.human_pass_cursor!=std::array<std::uint8_t,6>{87,99,100,101,102,103}[state.human_pass_steps]) ||
       ((phase==311 || phase>=313) && state.human_pass_steps!=5) || (phase==312 && state.human_pass_steps) ||
       state.current.field.side!=0 || state.current.field.ball_owner!=5 || state.current.field.ball_x!=108 || state.current.field.ball_y!=108 ||
       state.current.field.ball_cell!=87 || state.current.field.elapsed!=(phase>=325 ? 0 : phase==324 ? 5 : phase==323 ? 4 : phase>=320 ? 3 : phase>=317 ? 2 : 1) || state.current.field.selected_defenders!=3 ||
       state.scheduling->selected_ids!=std::array<std::uint8_t,5>{17,19,20,0,0} || state.current.field.roster[5].type!=23 || state.current.field.roster[5].stamina!=(score36_human_pass && phase<=313 ? 10 : 0) ||
       state.current.field.random_first || state.current.field.random_second || state.current.clock.remaining!=(score36_human_pass ? (phase>=324 ? 279 : phase==323 ? 280 : phase>=320 ? 281 : phase>=317 ? 282 : 283) : phase>=324 ? 321 : phase==323 ? 322 : phase>=320 ? 323 : phase>=317 ? 324 : 325) || state.current.clock.extra_time!=13 ||
       state.current.clock.ball_mode!=(phase>=313 ? 0 : 129) || (!score36_human_pass && (state.score->left_goals!=2 || state.score->right_goals!=5)) || state.score->kickoff!=4 ||
       state.action->action!=1 || state.action->cursor!=225 || state.action->menu_route || state.action->power!=(phase>=314 ? 0 : 22) ||
       state.recovery->opposing_player!=(phase>=321 ? 20 : phase>=318 ? 19 : phase>=315 ? 17 : phase>=313 ? 20 : 4) || state.recovery->attack_subtype || state.saved_ball_flight_flag || state.area_defender_cursor!=(phase>=324 ? 3 : phase>=321 ? 2 : phase>=318 ? 1 : 0) ||
       state.human_defense_choices || state.keeper_menu_action || bool(state.sampled_field_pass)!=(phase==326) || state.area_action4_restart ||
       state.area_action4_restart_power || state.area_action4_restart_pass_counter))
        throw std::invalid_argument("Invalid sampled 2-5 human pass target");
    if(phase>=315 && phase<=317 && (state.current.field.roster[17].type!=23 || state.current.field.roster[17].stamina!=100 ||
       state.recovery->ball_variant!=1 || state.control->facing!=64 || !state.area_contact || state.area_contact->cursor ||
       state.area_contact->defense_action!=1 || state.area_contact->defense_subtype || state.area_contact->effect ||
       state.area_contact->defender_power!=18 || state.area_contact->cost!=60 || state.area_contact->strength ||
       state.area_contact->loss || state.area_contact->score!=(phase>=316 ? 255 : 0) || state.area_contact->result || state.area_contact->weakened ||
       state.area_sequence_result!=(phase>=316 ? 0 : 255) || state.current.field.motion[field_motion_index(17)].status!=(phase==317 ? 6 : 0)))
        throw std::invalid_argument("Invalid sampled player7 first defender preparation");
    if(phase>=318 && phase<=326 && (!state.area_contact || state.area_contact->cursor!=(phase>=321 ? 2 : 1) || state.area_contact->defense_action!=1 ||
       state.area_contact->defense_subtype || state.area_contact->effect || state.area_contact->defender_power!=18 ||
       state.area_contact->cost!=60 || state.area_contact->strength || state.area_contact->loss || state.area_contact->score!=((phase==318 || phase==321) ? 0 : 255) ||
       state.area_contact->result || state.area_contact->weakened || state.area_sequence_result!=0 ||
       state.recovery->ball_variant!=1 || state.control->facing!=64 || (phase<=324 && (state.current.field.motion[field_motion_index(17)].status!=6 ||
       state.current.field.motion[field_motion_index(19)].status!=(phase>=320 ? 6 : 0) ||
       (phase>=321 && state.current.field.motion[field_motion_index(20)].status!=(phase>=323 ? 6 : 0))))))
        throw std::invalid_argument("Invalid sampled player7 second defender preparation");
    if(score25_human_pass && phase<=324 && (phase==311 || phase>=313) &&
       field_cell_at(static_cast<std::uint8_t>(state.current.field.motion[field_motion_index(7)].x>>8),
                     static_cast<std::uint8_t>(state.current.field.motion[field_motion_index(7)].y>>8))!=103)
        throw std::invalid_argument("Sampled player7 recipient is not in the observed target cell");
    if(score24_dribble && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       state.current.field.side!=(phase==309 ? 11 : 0) || state.current.field.ball_owner!=(phase==309 ? 17 : 5) || state.current.field.ball_x!=108 || state.current.field.ball_y!=108 ||
       state.current.field.ball_cell!=87 || state.current.field.elapsed!=(phase>=308 ? 0 : phase==307 ? 2 : 1) || state.current.field.selected_defenders!=1 ||
       state.scheduling->selected_ids!=std::array<std::uint8_t,5>{17,0,0,0,0} || state.current.field.roster[5].type!=23 ||
       state.current.field.roster[5].stamina || state.current.field.roster[17].type!=23 || state.current.field.roster[17].stamina!=100 ||
       state.current.field.random_first || state.current.field.random_second || state.current.clock.remaining!=(phase>=307 ? 341 : 342) || state.current.clock.extra_time!=13 ||
       state.score->left_goals!=2 || state.score->right_goals!=4 || state.score->kickoff!=4 || state.action->action!=2 ||
       state.action->cursor!=227 || state.action->menu_route || state.action->power!=(phase==303 ? 22 : 0) ||
       state.recovery->opposing_player!=17 || state.recovery->attack_subtype || state.saved_ball_flight_flag || state.area_defender_cursor ||
       (phase>=305 && state.control->last_carrier_cell) || (phase>=307 && (state.control->facing!=64 || state.current.field.motion[field_motion_index(5)].status!=5)) ||
       (phase>=308 && state.current.field.movement_region!=129) || state.human_defense_choices || state.human_pass_cursor || state.keeper_menu_action ||
       state.sampled_field_pass || state.area_action4_restart || state.area_action4_restart_power || state.area_action4_restart_pass_counter))
        throw std::invalid_argument("Invalid sampled 2-4 dribble state");
    if(phase>=305 && phase<=309 && (!state.area_contact || state.area_contact->cursor || state.area_contact->defense_action!=1 ||
       state.area_contact->defense_subtype || state.area_contact->effect || state.area_contact->defender_power!=18 || state.area_contact->cost!=60 ||
       state.area_contact->strength!=(phase>=306 ? 4 : 0) || state.area_contact->loss!=(phase>=306 ? 1 : 0) ||
       state.area_contact->score || state.area_contact->result!=(phase>=306 ? 3 : 0) || state.area_contact->weakened ||
       (phase>=306 && state.area_sequence_result!=3)))
        throw std::invalid_argument("Invalid sampled 2-4 dribble defender comparison");
    if(score23_human_pass && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       !state.human_pass_cursor || state.human_pass_steps>1 ||
       (state.human_pass_steps==0 && *state.human_pass_cursor!=87) ||
       (state.human_pass_steps==1 && *state.human_pass_cursor!=99 && *state.human_pass_cursor!=75 && *state.human_pass_cursor!=88 && *state.human_pass_cursor!=86) ||
       (phase!=276 && state.human_pass_steps) || state.current.field.side!=0 || state.current.field.ball_owner!=5 ||
       state.current.field.ball_x!=108 || state.current.field.ball_y!=108 || state.current.field.ball_cell!=87 || state.current.field.elapsed!=(phase==284 ? 0 : phase==283 ? 3 : phase==282 ? 2 : 1) ||
       state.current.field.selected_defenders!=1 || state.scheduling->selected_ids!=std::array<std::uint8_t,5>{17,0,0,0,0} ||
       state.current.field.roster[5].type!=23 || state.current.field.roster[5].stamina || state.current.field.random_first || state.current.field.random_second ||
       state.current.clock.remaining!=(phase>=283 ? 353 : phase==282 ? 354 : 355) || state.current.clock.extra_time!=13 || state.current.clock.ball_mode!=(phase>=279 ? 0 : 129) ||
       state.score->left_goals!=2 || state.score->right_goals!=3 || state.score->kickoff!=4 || state.saved_ball_flight_flag ||
       state.action->action!=1 || state.action->cursor!=225 || state.action->menu_route || state.action->power!=(phase>=279 ? 0 : 22) ||
       state.recovery->attack_subtype || state.recovery->opposing_player!=(phase>=279 ? 17 : 5) || state.human_defense_choices ||
       state.sampled_field_pass || state.keeper_menu_action || state.area_action4_restart || state.area_action4_restart_power || state.area_action4_restart_pass_counter))
        throw std::invalid_argument("Invalid sampled 2-3 human pass state");
    if(score23_defender && (state.current.field.roster[17].stamina!=100 || state.recovery->ball_variant!=1 || state.control->facing!=64 ||
       !state.area_contact || state.area_contact->cursor || state.area_contact->defense_action!=1 || state.area_contact->defense_subtype ||
       state.area_contact->effect || state.area_contact->defender_power!=18 || state.area_contact->cost!=60 || state.area_contact->strength ||
       state.area_contact->loss || state.area_contact->weakened || state.area_contact->score!=(phase>=281 ? 255 : 0) ||
       state.area_contact->result || state.area_sequence_result!=(phase>=281 ? 0 : 255) ||
       state.current.field.motion[field_motion_index(17)].status!=(phase>=282 ? 6 : 0) || state.area_defender_cursor!=(phase>=283 ? 1 : 0) ||
       (phase==284 && state.current.field.movement_region!=129)))
        throw std::invalid_argument("Invalid sampled 2-3 defender comparison at phase "+std::to_string(phase)+
            ", status "+std::to_string(state.current.field.motion[field_motion_index(17)].status)+
            ", cursor "+std::to_string(state.area_defender_cursor));
    if(score23_shot_preflight && !score35_opponent_shot && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       state.current.field.side!=11 || state.current.field.ball_owner!=(player17_score24 ? 17 : 19) || state.current.field.ball_x!=94 || state.current.field.ball_y!=(player17_score24 ? 106 : 108) ||
       state.current.field.ball_cell!=63 || state.current.field.elapsed!=(phase>=289 ? 0 : phase==288 ? 2 : 1) || state.current.field.selected_defenders ||
       state.current.field.roster[player17_score24 ? 17 : 19].type!=23 || state.current.field.roster[player17_score24 ? 17 : 19].stamina!=100 || state.current.field.random_first || state.current.field.random_second ||
       state.experience->opponent_stage || state.current.clock.remaining!=(player17_score24 ? (phase>=288 ? 335 : 336) : (phase>=288 ? 348 : 349)) || state.current.clock.extra_time!=13 ||
       state.current.clock.script_flags!=(phase>=287 ? 0 : 128) || state.current.clock.ball_mode!=(phase>=287 ? 0 : 128) ||
       state.score->left_goals!=2 || state.score->right_goals!=(player17_score24 ? 4 : 3) || state.score->kickoff!=4 || state.action->action ||
       state.action->power!=(phase==285 ? 0 : 22) || state.recovery->attack_subtype!=(phase==285 ? 0 : 128) ||
       state.recovery->opposing_player || state.recovery->ball_variant!=(phase==290 ? 1 : 0) || state.control->controlled_player!=(player17_score24 ? 9 : 5) ||
       state.control->last_carrier_cell!=(phase==290 ? 128 : phase==289 ? 0 : 63) || state.control->facing ||
       (phase>=289 && state.current.field.movement_region!=129) ||
       (phase==290 && (state.control->display_parameter || state.control->motion_requested!=2)) ||
       state.area_defender_cursor!=(player17_score24 ? 0 : 1) || state.saved_ball_flight_flag!=(player17_score24 ? 0 : 1) ||
       state.human_defense_choices || state.human_pass_cursor || state.sampled_field_pass || state.keeper_menu_action ||
       state.area_action4_restart || state.area_action4_restart_power || state.area_action4_restart_pass_counter))
        throw std::invalid_argument("Invalid sampled 2-3 opponent shot state");
    if(score23_shot_preflight && score35_opponent_shot && (!state.action || !state.recovery || !state.control || !state.experience || !state.scheduling || !state.kickoff_formation ||
       state.current.field.side!=11 || state.current.field.ball_x!=85 || state.current.field.ball_y!=157 || state.current.field.ball_cell!=57 ||
       state.current.field.elapsed!=(phase>=289 ? 0 : phase==288 ? 2 : 1) || state.current.field.selected_defenders ||
       state.current.field.roster[19].type!=23 || state.current.field.roster[19].stamina!=100 || state.current.field.random_first || state.current.field.random_second ||
       state.experience->opponent_stage || state.current.clock.remaining!=(phase>=288 ? 293 : 294) || state.current.clock.extra_time!=13 ||
       state.current.clock.script_flags!=(phase>=287 ? 0 : 128) || state.current.clock.ball_mode!=(phase>=287 ? 0 : 128) || state.score->kickoff!=4 || state.action->action ||
       state.action->power!=(phase==285 ? 21 : 22) || state.recovery->attack_subtype!=(phase==285 ? 0 : 128) || state.recovery->opposing_player ||
       state.recovery->ball_variant!=(phase==290 ? 1 : 0) || state.control->controlled_player!=2 ||
       state.control->last_carrier_cell!=(phase==290 ? 128 : phase==289 ? 0 : 57) || state.control->facing ||
       (phase>=289 && state.current.field.movement_region!=139) || (phase==290 && (state.control->display_parameter || state.control->motion_requested!=2)) ||
       state.area_defender_cursor || state.saved_ball_flight_flag!=1 || state.human_defense_choices || state.human_pass_cursor || state.sampled_field_pass || state.keeper_menu_action ||
       state.area_action4_restart || state.area_action4_restart_power || state.area_action4_restart_pass_counter))
        throw std::invalid_argument("Invalid sampled 3-5 opponent shot state");
    if(score23_shot_flight && !score35_opponent_shot && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       !state.shot_flight || state.current.field.side!=11 || state.current.field.ball_owner!=(player17_score24 ? 17 : 19) || state.current.field.selected_defenders ||
       state.current.field.elapsed || state.current.field.movement_region!=129 || state.current.field.random_first || state.current.field.random_second ||
       state.current.clock.remaining!=(player17_score24 ? 335 : 348) || state.current.clock.extra_time!=13 || state.current.clock.script_flags || state.current.clock.ball_mode ||
       state.score->left_goals!=2 || state.score->right_goals!=(player17_score24 ? 4 : 3) || state.score->kickoff!=4 || state.action->action || state.action->power!=22 ||
       state.recovery->attack_subtype!=128 || state.recovery->opposing_player || state.recovery->ball_variant!=1 || state.control->controlled_player!=(player17_score24 ? 9 : 5) ||
       state.control->last_carrier_cell!=128 || state.control->display_parameter || state.control->motion_requested!=2 || state.control->facing ||
       state.area_defender_cursor!=(player17_score24 ? 0 : 1) || state.saved_ball_flight_flag!=((phase==291 && !player17_score24) ? 1 : 0) || state.shot_flight->distance!=45 ||
       state.shot_flight->destination!=5 || state.shot_flight->counter!=255 || state.shot_flight->loose_ball ||
       (phase==291 && (state.current.field.ball_x!=94 || state.current.field.ball_y!=(player17_score24 ? 106 : 108) || state.current.field.ball_cell!=63)) ||
       (phase==292 && state.current.field.ball_cell==5) || (phase==293 && state.current.field.ball_cell==5) ||
       (phase==294 && state.current.field.ball_cell!=5)))
        throw std::invalid_argument("Invalid sampled 2-3 numeric shot flight");
    if(score23_shot_flight && score35_opponent_shot && (!state.action || !state.recovery || !state.control || !state.experience || !state.scheduling || !state.kickoff_formation ||
       !state.shot_flight || state.current.field.side!=11 || state.current.field.selected_defenders || state.current.field.elapsed ||
       state.current.field.movement_region!=139 || state.current.field.random_first || state.current.field.random_second ||
       state.current.clock.remaining!=293 || state.current.clock.extra_time!=13 || state.current.clock.script_flags || state.current.clock.ball_mode ||
       state.score->kickoff!=4 || state.action->action || state.action->power!=22 || state.recovery->attack_subtype!=128 || state.recovery->opposing_player ||
       state.recovery->ball_variant!=1 || state.control->controlled_player!=2 || state.control->last_carrier_cell!=128 || state.control->display_parameter ||
       state.control->motion_requested!=2 || state.control->facing || state.area_defender_cursor || state.saved_ball_flight_flag!=(phase==291 ? 1 : 0) ||
       state.shot_flight->distance!=36 || state.shot_flight->destination!=5 || state.shot_flight->counter!=255 || state.shot_flight->loose_ball ||
       (phase==291 && (state.current.field.ball_x!=85 || state.current.field.ball_y!=157 || state.current.field.ball_cell!=57)) ||
       ((phase==292 || phase==293) && state.current.field.ball_cell==5) || (phase==294 && state.current.field.ball_cell!=5)))
        throw std::invalid_argument("Invalid sampled 3-5 opponent numeric shot flight");
    if(score23_keeper && score35_opponent_shot && (!state.action || !state.recovery || !state.control || !state.experience || !state.scheduling || !state.kickoff_formation ||
       state.current.field.side!=(phase>=301 ? 0 : 11) || state.current.field.ball_x!=52 || state.current.field.ball_y!=124 || state.current.field.ball_cell!=5 ||
       state.current.field.elapsed || state.current.field.selected_defenders!=1 || state.current.field.roster[0].stamina!=(phase>=299 ? 64 : 84) ||
       state.current.field.random_first || state.current.field.random_second || state.current.clock.remaining!=293 || state.current.clock.extra_time!=13 ||
       state.current.clock.script_flags || state.current.clock.ball_mode || state.score->kickoff!=4 || state.score->right_goals!=(phase>=300 ? 6 : 5) || state.action->action || state.action->power!=22 ||
       state.action->menu_route!=4 || state.action->cursor!=(phase==295 ? 0 : 224) || state.recovery->attack_subtype!=128 || state.recovery->opposing_player!=(phase>=300 ? 4 : 0) ||
       state.recovery->ball_variant!=1 || state.recovery->x_jitter_control!=159 || state.control->controlled_player!=2 ||
       state.control->last_carrier_cell!=128 || state.control->display_parameter || state.control->motion_requested!=(phase==300 ? 3 : 0) || state.control->facing!=(phase>=300 ? 64 : 0) ||
       state.area_defender_cursor || state.saved_ball_flight_flag || (phase==295 ? bool(state.keeper_menu_action) : state.keeper_menu_action!=0) || state.shot_flight || state.ball_trajectory))
        throw std::invalid_argument("Invalid sampled 3-5 goalkeeper prompt");
    if(score23_keeper && !score35_opponent_shot && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       state.current.field.side!=(phase>=301 ? 0 : 11) || state.current.field.ball_owner!=(player17_keeper ? 17 : 19) || state.current.field.ball_x!=52 || state.current.field.ball_y!=124 ||
       state.current.field.ball_cell!=5 || state.current.field.elapsed || state.current.field.selected_defenders!=1 ||
       state.current.field.roster[0].stamina!=(player17_keeper ? (phase>=299 ? 24 : 44) : (phase>=299 ? 22 : 42)) || state.current.field.random_first || state.current.field.random_second ||
       state.current.clock.remaining!=(player17_keeper ? 335 : 348) || state.current.clock.extra_time!=13 || state.current.clock.script_flags || state.current.clock.ball_mode ||
       state.score->left_goals!=2 || state.score->right_goals!=(player17_keeper ? (phase>=300 ? 5 : 4) : (phase>=300 ? 4 : 3)) || state.score->kickoff!=4 || state.action->action || state.action->power!=22 ||
       state.action->menu_route!=4 || state.action->cursor!=(phase==295 ? 0 : 224) || state.recovery->attack_subtype!=128 || state.recovery->opposing_player!=(phase>=300 ? (player17_keeper ? 4 : 1) : 0) ||
       state.recovery->ball_variant!=1 || state.recovery->x_jitter_control!=(player17_keeper ? 111 : 113) || state.control->controlled_player!=(player17_keeper ? 9 : 5) ||
       state.control->last_carrier_cell!=128 || state.control->display_parameter || state.control->motion_requested!=(phase==300 ? 3 : 0) || state.control->facing ||
       state.area_defender_cursor!=((phase>=300 || player17_keeper) ? 0 : 1) || state.saved_ball_flight_flag ||
       (phase==295 ? bool(state.keeper_menu_action) : state.keeper_menu_action!=0) || state.shot_flight || state.ball_trajectory))
        throw std::invalid_argument("Invalid sampled 2-3 goalkeeper prompt");
    if(player19_return_keeper && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       state.current.field.side!=(player19_goal_possession ? 0 : 11) || state.current.field.ball_owner!=19 || state.current.field.ball_x!=52 || state.current.field.ball_y!=124 ||
       state.current.field.ball_cell!=5 || state.current.field.elapsed || state.current.field.selected_defenders!=1 ||
       state.current.field.roster[0].type!=1 || state.current.field.roster[0].stamina!=(phase>=272 ? 24 : 44) || state.scheduling->selected_ids!=std::array<std::uint8_t,5>{} ||
       state.current.field.random_first || state.current.field.random_second || state.experience->opponent_stage ||
       state.current.clock.remaining!=360 || state.current.clock.extra_time!=13 || state.current.clock.script_flags || state.current.clock.ball_mode ||
       state.score->left_goals!=2 || state.score->right_goals!=(phase>=273 ? 3 : 2) || state.score->kickoff!=4 || state.action->action || state.action->power!=22 ||
       state.action->menu_route!=4 || state.action->cursor!=(phase==268 ? 0 : 224) || state.recovery->attack_subtype!=128 || state.recovery->opposing_player!=(phase>=273 ? 5 : 0) ||
       state.recovery->ball_variant!=1 || state.recovery->x_jitter_control!=110 || state.control->last_carrier_cell!=128 ||
       state.control->motion_requested!=(phase==273 ? 3 : 0) || state.control->facing || state.saved_ball_flight_flag ||
       (phase==268 ? bool(state.keeper_menu_action) : (state.keeper_menu_action!=0 || state.action->flash_counter!=13)) ||
       state.human_defense_choices || state.human_pass_cursor || state.sampled_field_pass || state.area_action4_restart ||
       state.area_action4_restart_power || state.area_action4_restart_pass_counter))
        throw std::invalid_argument("Invalid sampled player19 return keeper prompt");
    if(player19_return_shot && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       state.current.field.side!=11 || state.current.field.ball_owner!=19 || state.current.field.ball_x!=85 || state.current.field.ball_y!=109 ||
       state.current.field.ball_cell!=51 || state.current.field.elapsed!=(phase>=266 ? 0 : phase==265 ? 2 : 1) || state.current.field.selected_defenders ||
       state.current.field.roster[19].type!=23 || state.current.field.roster[19].stamina!=100 || state.current.field.random_first || state.current.field.random_second ||
       state.experience->opponent_stage || state.current.clock.remaining!=(phase>=265 ? 360 : 361) || state.current.clock.extra_time!=13 ||
       state.current.clock.script_flags!=(phase>=264 ? 0 : 128) || state.current.clock.ball_mode!=(phase>=264 ? 0 : 128) || state.score->left_goals!=2 || state.score->right_goals!=2 || state.score->kickoff!=4 ||
       state.control->controlled_player!=7 || state.control->last_carrier_cell!=(phase==267 ? 128 : phase>=265 ? 0 : 51) || state.control->display_parameter!=(phase==267 ? 0 : 2) || state.control->motion_requested!=(phase==267 ? 2 : 1) ||
       state.action->action!=0 || state.action->power!=(phase==262 ? 21 : 22) || state.recovery->attack_subtype!=(phase==262 ? 0 : 128) ||
       state.saved_ball_flight_flag!=1 || state.human_defense_choices || state.human_pass_cursor || state.sampled_field_pass ||
       state.area_action4_restart || state.area_action4_restart_power || state.area_action4_restart_pass_counter || state.keeper_menu_action))
        throw std::invalid_argument("Invalid sampled player19 return shot state");
    if(player19_return_shot && phase>=266 && (state.current.field.movement_region!=129 || state.control->facing!=0 || state.recovery->ball_variant!=(phase==267 ? 1 : 0)))
        throw std::invalid_argument("Invalid sampled player19 return shot handoff");
    if(player14_pass && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       state.current.field.side!=11 || state.current.field.ball_owner!=14 || state.current.field.ball_x!=100 || state.current.field.ball_y!=107 ||
       state.current.field.ball_cell!=75 || state.current.field.elapsed!=(phase>=260 ? 0 : 1) || state.current.field.selected_defenders ||
       state.current.field.roster[14].type!=23 || state.current.field.roster[14].stamina!=100 ||
       state.current.field.random_first || state.current.field.random_second || state.experience->opponent_stage ||
       state.current.clock.remaining!=363 || state.current.clock.extra_time!=13 || state.current.clock.script_flags!=(phase>=258 ? 0 : 128) || state.current.clock.ball_mode!=(phase>=258 ? 0 : 128) ||
       state.score->left_goals!=2 || state.score->right_goals!=2 || state.score->kickoff!=4 || state.control->controlled_player!=9 ||
       state.control->last_carrier_cell!=(phase>=260 ? 128 : phase==259 ? 0 : 75) || state.control->display_parameter!=(phase>=260 ? 0 : 2) || state.control->motion_requested!=(phase>=260 ? 0 : 1) ||
       state.action->action!=1 || state.action->power!=(phase==256 ? 0 : 21) || state.recovery->attack_subtype!=(phase==256 ? 0 : 128) ||
       state.saved_ball_flight_flag!=1 || state.human_defense_choices || state.human_pass_cursor || bool(state.sampled_field_pass)!=(phase==261) ||
       state.area_action4_restart || state.area_action4_restart_power || state.area_action4_restart_pass_counter || state.keeper_menu_action))
        throw std::invalid_argument("Invalid sampled player14 pass state");
    if(phase>=260 && phase<=261 && (state.current.field.movement_region!=129 || state.recovery->ball_variant!=1 || state.control->facing!=0))
        throw std::invalid_argument("Invalid sampled player14 pass handoff");
    const bool shot_pass_context=(phase>=152 && phase<=156) || phase==158 || phase==159;
    const bool shot_pass_compared=(phase>=153 && phase<=156) || phase==158 || phase==159;
    const bool area_flight=(phase>=96 && phase<=101) || phase==105 || (phase>=114 && phase<=149) || (phase>=345 && phase<=349) || phase==351;
    const bool pass_capture_tick=phase>=142 && phase<=145;
    const bool pass_blocked_tick=phase>=146 && phase<=149;
    const bool area_action4=phase==102 || phase==103 || (phase>=160 && phase<=187) || phase==190 || phase==191 || phase==192 || phase==193;
    if(bool(state.area_action4_restart)!=((phase>=179 && phase<=186) || phase==192 || phase==193) ||
       (state.area_action4_restart && ((phase==192 || phase==193) ? (state.area_action4_restart->action!=9 || state.area_action4_restart->subtype!=128 || state.area_action4_restart->choice!=5 || state.current.field.ball_owner!=20) :
       (state.area_action4_restart->action!=1 || state.area_action4_restart->subtype || state.area_action4_restart->choice!=6 || state.current.field.ball_owner!=14))))
        throw std::invalid_argument("Invalid captured action4 restart choice");
    if(bool(state.area_action4_restart_power)!=((phase>=181 && phase<=186) || phase==192 || phase==193) ||
       (state.area_action4_restart_power && (state.area_action4_restart_power->cost!=((phase==192 || phase==193) ? 40 : 20) || state.area_action4_restart_power->power!=21)))
        throw std::invalid_argument("Invalid action4 restart action power");
    if(bool(state.area_action4_restart_pass_counter)!=(phase==186) ||
       (state.area_action4_restart_pass_counter && *state.area_action4_restart_pass_counter!=77))
        throw std::invalid_argument("Invalid action4 restart pass counter");
    const bool area_action6=phase==104 || (phase>=106 && phase<=110) || phase==113;
    const bool area_action6_race=(phase>=106 && phase<=110) || phase==113;
    const bool area_setup=phase==65 || phase==105;
    const bool area_menu=(phase>=66 && phase<=94) || ((phase==18 || phase==19) && state.keeper_area);
    const bool area_state=area_setup || area_menu;
    if((((phase>=65 && phase<=94) || phase==105) && !state.keeper_area) || (state.keeper_area && !area_state))
        throw std::invalid_argument("Keeper-area setup needs exactly one captured selection counter");
    if(state.keeper_area && (!state.action || !state.recovery || !state.scheduling ||
       state.keeper_area->selection_counter!=255 || !state.recovery->ball_variant ||
       state.current.field.ball_y<96 || state.current.field.ball_y>=160 ||
       (state.current.field.ball_x>=80 && state.current.field.ball_x<176) ||
       state.action->menu_route!=(((state.current.field.ball_x<80 ? 0 : 11)==state.current.field.side)?2:1)))
        throw std::invalid_argument("Keeper-area setup requires its original area and route (phase " + std::to_string(phase) + ", ball " + std::to_string(state.current.field.ball_x) + "," + std::to_string(state.current.field.ball_y) + ", side " + std::to_string(state.current.field.side) + ", route " + std::to_string(state.action ? state.action->menu_route : 255) + ")");
    if(area_menu && (state.action->menu_route!=2 || (phase>=66 ? state.current.field.side!=11 : state.current.field.side!=0)))
        throw std::invalid_argument("Prepared own-area menu requires its matching attack or defense side");
    if(bool(state.area_defense)!=(((phase>=415 && phase<=420) || (phase>=238 && phase<=243) || ((phase>=202 && phase<=208) || ((phase>=215 && phase<=218) || phase==227 || (phase==228 || phase==229))) || phase==67 || (phase>=69 && phase<=92) || phase==94 || area_action4 || ((phase==93 || area_action6) && state.action && (state.action->action==4 || state.action->action==6) && state.area_defense))))
        throw std::invalid_argument("Initial area defense requires its captured choice arrays");
    if((phase>=415 && phase<=420) && (!state.action || !state.recovery || !state.control || !state.score || !state.scheduling || !state.area_defense ||
       state.current.field.side!=(phase==420 ? 0 : 11) || state.current.field.ball_owner!=(phase==420 ? 7 : 18) || state.current.field.ball_cell!=87 ||
       state.current.field.ball_x!=110 || state.current.field.ball_y!=(phase>=418 ? 124 : 108) || state.current.field.selected_defenders!=(phase>=418 ? 0 : 1) ||
       state.scheduling->selected_ids!=(phase>=418 ? std::array<std::uint8_t,5>{0,0,0,0,0} : std::array<std::uint8_t,5>{5,0,0,0,0}) || state.current.clock.remaining!=(phase>=417 ? 151 : 153) || state.current.clock.extra_time ||
       state.score->left_goals!=1 || state.score->right_goals || state.score->kickoff!=4 || state.action->action!=2 || state.action->power!=4 ||
       state.current.field.roster[5].type!=7 || state.current.field.roster[5].stamina!=330 || state.recovery->opposing_player!=5 ||
       state.area_defense->actions!=std::array<std::uint8_t,4>{0,255,255,255}))
        throw std::invalid_argument("Invalid third-return defender comparison");
    if((phase==415 && state.area_sequence_result!=255) || ((phase>=416 && phase<=420) && state.area_sequence_result!=2))
        throw std::invalid_argument("Invalid third-return defender classification");
    if((phase>=417 && phase<=420) && (state.current.field.elapsed!=(phase>=419 ? 0 : 3) || state.current.field.motion[field_motion_index(18)].status!=5 ||
       !state.control || state.control->facing!=64 || (phase==420 ? state.control->motion_requested>1 : state.control->motion_requested!=0)))
        throw std::invalid_argument("Invalid third-return contact clock");
    if(phase==420 && (state.recovery->ball_variant!=2 ||
       (state.current.field.motion[field_motion_index(7)].x>>8)!=110 || (state.current.field.motion[field_motion_index(7)].y>>8)!=124))
        throw std::invalid_argument("Invalid third-return recovered owner");
    if((phase>=418 && phase<=420) && state.recovery->x_jitter_control!=0)throw std::invalid_argument("Invalid third-return loose ball jitter");
    if(phase==93 && state.area_defense)for(unsigned i=0;i<state.current.field.selected_defenders;++i)
        if(state.area_defense->actions[i]>6)throw std::invalid_argument("Area continuation needs all captured defender choices");
    if(area_action4 && (!state.action || state.action->action!=4 || !state.recovery || state.recovery->ball_variant || state.current.field.side!=11 || ((phase>=162 && phase<=168) ? state.current.field.selected_defenders!=0 : !state.current.field.selected_defenders) || state.current.field.selected_defenders>4))
        throw std::invalid_argument("Area action4 boundary needs its retained defender choices");
    if(phase==160 && state.action->menu_route!=0)
        throw std::invalid_argument("Area action4 field entry needs its reset comparison route");
    if(phase==165 && state.current.field.elapsed!=1)
        throw std::invalid_argument("Area action4 field round requires elapsed1");
    if(phase==166 && (state.current.field.elapsed!=1 || state.current.clock.ball_mode || state.current.clock.script_flags))
        throw std::invalid_argument("Area action4 goal-area boundary requires its cleared control flags");
    if(phase==167 && (state.current.clock.remaining || !state.current.clock.extra_time || state.current.clock.ball_mode || (state.current.clock.script_flags&64u)))
        throw std::invalid_argument("Invalid area action4 added-time boundary");
    if(phase==168 && state.current.clock.remaining)throw std::invalid_argument("Invalid area action4 period-end boundary");
    if(((phase>=169 && phase<=187) || phase==190 || phase==191 || phase==192 || phase==193) && (!state.scheduling || state.current.field.selected_defenders!=1 || state.scheduling->selected_ids[0]!=0 ||
       state.recovery->opposing_player!=0 || state.action->menu_route!=3 || state.current.field.elapsed!=((phase>=182 && phase<=186)?19:1)))
        throw std::invalid_argument("Area action4 keeper entry requires the selected side0 goalkeeper");
    if(phase==170 && (!state.control || (state.control->facing!=0 && state.control->facing!=64)))
        throw std::invalid_argument("Area action4 keeper presentation requires captured facing");
    if(phase==67 || phase==69) {
        if(!state.current.field.selected_defenders || state.current.field.selected_defenders>4 || state.action->cursor || state.area_defense->pressed)
            throw std::invalid_argument("Area defense initialization requires a nonempty prefix and cursor0");
        for(unsigned i=0;i<4;++i)if(state.area_defense->actions[i]!=255 || state.area_defense->subtypes[i])
            throw std::invalid_argument("Initial defense actions must be unselected with unused subtypes zero");
    }
    if((phase>=70 && phase<=92) || phase==94) {
        const auto count=state.current.field.selected_defenders;
        if(!count || count>4 || state.action->cursor>count || ((phase==71 || (phase>=73 && phase<=76)) && state.action->cursor==count))
            throw std::invalid_argument("Defensive poll requires a valid selected cursor");
        for(unsigned i=0;i<4;++i) {
            const auto action=state.area_defense->actions[i];
            if((action!=255 && action>6) || (state.area_defense->subtypes[i] && (state.area_defense->subtypes[i]!=1 || action!=2)) || (i>=count && action!=255))
                throw std::invalid_argument("Defensive poll supports ordinary field actions in the active prefix");
        }
        if(((phase==70 || phase==72 || (phase>=73 && phase<=77)) && state.area_defense->pressed) || (phase==71 && !(state.area_defense->pressed&128u)))
            throw std::invalid_argument("Pending defensive preparation requires the captured confirm input");
        if(phase==72 && state.action->cursor!=count)
            throw std::invalid_argument("Completed defense requires the full selected prefix");
        if(state.action->cursor==count)for(unsigned i=0;i<count;++i)
            if(state.area_defense->actions[i]==255)
                throw std::invalid_argument("Full defensive prefix requires selected actions");
        const bool advanced=phase==77 && state.area_special_decision && state.area_special_decision->status==0;
        if(advanced && !state.action->cursor)throw std::invalid_argument("Accepted special redraw requires advanced cursor");
        const auto chosen=state.action->cursor-(advanced?1u:0u);
        if((phase==71 || (phase>=73 && phase<=77)) && (chosen>=count || state.area_defense->actions[chosen]==255 ||
           state.recovery->opposing_player!=state.scheduling->selected_ids[chosen]))
            throw std::invalid_argument("Pending defensive preparation requires a confirmed selected field player");
    }
    if(bool(state.area_special_option)!=(phase>=73 && phase<=77) ||
       (state.area_special_option && (*state.area_special_option>127 ||
        state.area_defense->actions[state.action->cursor-((phase==77 && state.area_special_decision && state.area_special_decision->status==0)?1u:0u)]!=2)))
        throw std::invalid_argument("Special chooser requires its captured option and action2");
    if((phase<73 && (state.area_special_cursor || state.area_special_pressed)) ||
       (phase>=73 && (state.area_special_cursor>1 || (phase==73 && state.area_special_cursor) ||
        (phase!=75 && phase!=76 && phase!=77 && state.area_special_pressed) || ((phase==75 || phase==76 || phase==77) && !(state.area_special_pressed&192u)))))
        throw std::invalid_argument("Invalid special chooser cursor or sampled input");
    if(bool(state.area_special_decision)!=(phase==76 || phase==77))
        throw std::invalid_argument("Special inspection needs its decision context");
    if(state.area_special_decision) {
        const auto& d=*state.area_special_decision;
        const bool cancel=!(state.area_special_pressed&128u);
        const auto subtype=state.area_special_cursor ? *state.area_special_option : 0;
        if(d.status>2 || (cancel ? (d.status!=2 || d.subtype || d.cost || d.effect) :
           (d.status==2 || d.subtype!=subtype || (d.subtype==0 && d.status!=0))))
            throw std::invalid_argument("Invalid special inspection decision");
        if(phase==77 && d.status==0 && state.area_defense->subtypes[state.action->cursor-1]!=d.subtype)
            throw std::invalid_argument("Accepted special redraw must retain chosen subtype");
        if(!cancel && (state.recovery->opposing_player>21 || d.cost>1023 ||
           (d.status==0)!=can_select_defense_move(state.current.field.roster[state.recovery->opposing_player].stamina,d.cost,d.subtype)))
            throw std::invalid_argument("Special inspection decision disagrees with stamina");
    }
    if(bool(state.area_ai_target)!=((phase>=78 && phase<=94) || area_flight || area_action4 || area_action6 || postgoal_opponent_action))throw std::invalid_argument("Area AI choice needs saved target context");
    if(postgoal_opponent_action && (!state.action || !state.score || !state.recovery || !state.control || !state.scheduling || !state.experience || !state.kickoff_formation ||
       state.score->left_goals!=1 || state.score->right_goals!=0 || state.score->kickoff!=4 ||
       state.current.clock.remaining!=(phase>=411 ? 156 : phase>=396 ? 157 : 158) || state.current.clock.extra_time ||
       (phase>=395 ? (state.current.clock.ball_mode || state.current.clock.script_flags) :
        (state.current.clock.script_flags!=128 || (state.current.clock.ball_mode!=128 && !(phase==394 && state.current.clock.ball_mode==192)))) ||
       state.current.field.side!=11 || state.current.field.ball_owner!=(phase>=413 ? 18 : phase>=404 ? 14 : 18) ||
       (phase<400 && (state.current.field.ball_x!=141 || state.current.field.ball_y!=108 || state.current.field.ball_cell!=135)) ||
       state.current.field.roster[18].type || state.current.field.roster[18].stamina || state.current.field.roster[18].level ||
       state.current.field.elapsed!=(phase==411 ? 1 : phase>=398 ? 0 : phase>=396 ? 2 : 1) ||
       (phase<405 ? state.current.field.selected_defenders!=0 : state.current.field.selected_defenders>1) ||
       (phase>=405 && state.current.field.selected_defenders && state.scheduling->selected_ids[0]!=8) ||
       (phase<=393 && (state.current.field.random_first!=93 || state.current.field.random_second!=128)) ||
       (phase==395 && (state.current.field.random_first!=191 || state.current.field.random_second!=238)) ||
       (phase==397 && (state.current.field.random_first!=204 || state.current.field.random_second!=253)) ||
       (phase>=403 && ((phase<=404 && (state.current.field.random_first!=236 || state.current.field.random_second!=69)) ||
        (state.current.field.motion[field_motion_index(18)].x>>8)!=117)) ||
       ((phase==399 || phase>=400) && ((phase==399 && (state.current.field.random_first!=176 || state.current.field.random_second!=26)) ||
        (phase<=409 ? state.control->facing!=64 : (state.control->facing!=64 && state.control->facing!=0)) || state.control->display_parameter || (phase==414 ? state.control->motion_requested>1 : state.control->motion_requested!=0) || state.recovery->ball_variant!=1)) ||
       state.control->controlled_player!=10 || state.control->last_carrier_cell!=(phase>=399 ? 128 : phase>=397 ? 0 : 135) || state.action->action!=3 || state.action->move_effect || state.recovery->attack_subtype ||
       state.area_ai_target->recipient!=(phase>=404 ? 18 : 14) || state.area_ai_target->cell!=(phase>=405 ? 99 : phase>=400 ? 148 : 161) || state.area_ai_target->cost!=(phase>=393 ? 60 : 0) ||
       state.action->power!=(phase>=393 ? 12 : 11) || state.action->move_flags!=(phase>=393 ? 3 : 0)))
        throw std::invalid_argument("Invalid captured post-goal opponent action");
    if(phase>=400 && phase<=404 && (!state.shot_flight || !state.ball_trajectory || state.shot_flight->distance!=18 ||
       state.shot_flight->destination!=148 || state.shot_flight->counter!=255 || state.shot_flight->loose_ball ||
       state.ball_trajectory->direction!=34 || state.ball_trajectory->vx!=162 || state.ball_trajectory->vy!=185 ||
       state.ball_trajectory->flags!=1 || state.saved_ball_flight_flag!=1 ||
       (phase==400 && state.current.field.ball_cell!=135 && state.current.field.ball_cell!=147) ||
       (phase==401 && state.current.field.ball_cell!=147) || (phase>=402 && state.current.field.ball_cell!=148)))
        throw std::invalid_argument("Invalid captured post-goal first flight");
    if(phase>=405 && phase<=414 && (!state.shot_flight || !state.ball_trajectory || state.shot_flight->distance!=18 ||
       state.shot_flight->destination!=99 || state.shot_flight->counter!=255 || state.shot_flight->loose_ball ||
       state.ball_trajectory->direction!=137 || state.ball_trajectory->vx!=65290 || state.ball_trajectory->vy!=65487 ||
       state.ball_trajectory->flags!=1 || state.saved_ball_flight_flag!=1 ||
       (phase==405 && state.current.field.ball_cell!=148 && state.current.field.ball_cell!=136 && state.current.field.ball_cell!=124 && state.current.field.ball_cell!=112 && state.current.field.ball_cell!=111) ||
       (phase==406 && state.current.field.ball_cell!=136 && state.current.field.ball_cell!=124 && state.current.field.ball_cell!=112 && state.current.field.ball_cell!=111) ||
       (phase>=407 && (state.current.field.ball_cell!=99 || state.current.field.selected_defenders!=(phase>=412 ? 0 : 1) ||
        (phase==407 && (state.current.field.random_first!=19 || state.current.field.random_second!=116)) ||
        state.current.field.ball_x!=(phase>=413 ? 117 : phase==407 ? 119 : 116) || state.current.field.ball_y!=(phase==407 ? 110 : 108) ||
        (state.ball_trajectory->x&255u)!=248 || (state.ball_trajectory->y&255u)!=152))))
        throw std::invalid_argument("Invalid captured post-goal return flight");
    if((phase>=409 && phase<=414) && (!state.area_contact || state.area_contact->cursor || state.area_contact->defense_action!=2 ||
       state.area_contact->defense_subtype || state.area_contact->effect || state.area_contact->defender_power!=8 ||
       state.area_contact->cost!=50 || state.area_contact->strength!=4 || state.area_contact->loss!=1 ||
       state.area_contact->score!=192 || state.area_contact->result || state.area_contact->weakened ||
       state.area_sequence_result!=0 || state.recovery->opposing_player!=8 ||
       state.area_defender_cursor!=(phase>=412 ? 1 : 0) ||
       (phase>=411 && ((phase<=413 && state.control->facing!=0) || state.current.field.motion[field_motion_index(8)].status!=6)) ||
       ((phase==412 || phase==413) && (state.current.field.random_first!=62 || state.current.field.random_second!=45 || state.scheduling->selected_ids[0])) ||
       (phase==409 && (state.current.field.random_first!=104 || state.current.field.random_second!=186))))
        throw std::invalid_argument("Invalid captured post-goal defender comparison");
    if(area_action6 && (!state.action || state.action->action!=6 || !state.area_ai_target || state.area_ai_target->recipient || state.area_ai_target->cell!=state.current.field.ball_cell || ((phase==104 || phase==107) && state.current.field.side!=11) || state.current.field.selected_defenders>4))
        throw std::invalid_argument("Area action6 destination needs its captured target cell");
    const bool sampled_player7_flight=area_flight && state.score && ((state.score->left_goals==2 && state.score->right_goals==5 && state.current.clock.remaining==321) ||
        (state.score->left_goals==3 && state.score->right_goals==6 && state.current.clock.remaining==279)) && state.current.field.side==0 &&
        (state.current.field.ball_owner==5 || state.current.field.ball_owner==7) && state.area_defender_cursor==3 &&
        state.saved_ball_flight_flag==1 && state.area_ai_target && state.area_ai_target->recipient==7 && state.area_ai_target->cell==103;
    const bool ordinary_home_flight=((phase>=345 && phase<=349) || phase==351) && state.current.field.side==0 && state.saved_ball_flight_flag==1 && state.area_ai_target &&
        (state.area_ai_target->recipient==255 || (state.area_ai_target->recipient>=1 && state.area_ai_target->recipient<=10)) && !state.area_ai_target->cost;
    if(area_flight && (!state.action || state.action->action!=1 || !state.area_ai_target || (!sampled_player7_flight && !ordinary_home_flight && (state.area_ai_target->recipient<12 || state.area_ai_target->recipient>21)) || state.area_ai_target->cell>=240 || state.area_ai_target->cost>1023 || (!sampled_player7_flight && !ordinary_home_flight && phase<135 && state.current.field.side!=((phase>=128 && phase<=133) ? 0 : 11)) || ((pass_capture_tick || pass_blocked_tick) && state.current.field.side!=0) || state.current.field.selected_defenders>4 || (phase<114 && state.area_defender_cursor && !sampled_player7_flight && !ordinary_home_flight) || (ordinary_home_flight && state.area_defender_cursor>5) || (((phase>=117 && phase<=122) || (phase>=124 && phase<=149)) ? state.area_sequence_result>((phase>=120 && phase<=122) ? 1 : 3) : state.area_sequence_result!=255)))
        throw std::invalid_argument("Area flight requires its captured recipient and cleared sequence");
    if (phase>=345 && phase<=349 && (!ordinary_home_flight || state.current.field.selected_defenders ||
        (phase>=347 && state.area_ai_target->recipient==255) || (phase>=348 && state.current.field.ball_owner!=state.area_ai_target->recipient)))
        throw std::invalid_argument("Unsupported ordinary pass flight context");
    if (phase==351 && (!ordinary_home_flight || !state.shot_flight || state.area_ai_target->recipient!=255 || state.current.field.selected_defenders ||
        state.current.field.ball_owner<1 || state.current.field.ball_owner>10 || state.current.field.elapsed!=1 ||
        state.current.field.ball_cell!=state.shot_flight->destination))
        throw std::invalid_argument("Invalid free pass contest winner");
    if (phase==350 && (!state.action || !state.recovery || !state.control || !state.scheduling ||
        state.current.field.side!=0 || state.current.field.ball_owner<1 || state.current.field.ball_owner>10 ||
        state.current.field.selected_defenders || state.area_defender_cursor>5 || state.area_sequence_result!=255 ||
        state.saved_ball_flight_flag!=1 || state.action->action!=1))
        throw std::invalid_argument("Invalid ordinary pass control entry");
    if ((phase==352 || phase==353) && (!state.action || !state.recovery || !state.control || !state.scheduling ||
        state.current.field.side!=0 || state.current.field.ball_owner<1 || state.current.field.ball_owner>10 ||
        state.current.field.selected_defenders || !state.area_defender_cursor || state.area_defender_cursor>5 ||
        state.area_sequence_result!=255 || state.saved_ball_flight_flag!=1 || state.action->action!=1))
        throw std::invalid_argument("Invalid ordinary returned control boundary");
    if (phase==354 && (!state.action || !state.recovery || !state.control || !state.scheduling ||
        state.current.field.side!=0 || state.current.field.ball_owner<1 || state.current.field.ball_owner>10 ||
        !state.current.field.selected_defenders || state.current.field.selected_defenders>5 || !state.area_defender_cursor ||
        state.area_defender_cursor>5 || state.area_sequence_result!=255 || state.saved_ball_flight_flag!=1 ||
        state.current.clock.ball_mode || state.current.clock.script_flags || state.action->action!=1))
        throw std::invalid_argument("Invalid ordinary returned encounter boundary");
    if ((phase==355 || phase==356) && (!state.action || !state.recovery || !state.control || !state.scheduling ||
        state.current.field.side!=0 || state.current.field.ball_owner<1 || state.current.field.ball_owner>10 ||
        !state.current.field.selected_defenders || state.current.field.selected_defenders>5 || !state.area_defender_cursor ||
        state.area_defender_cursor>5 || state.area_sequence_result!=255 || state.saved_ball_flight_flag!=1 ||
        state.current.clock.ball_mode || state.current.clock.script_flags || state.action->action>=10 || state.action->menu_route ||
        (phase==356 && (state.action->action!=0 || !(state.action->cursor&128u))) ||
        state.control->facing || state.control->display_parameter || state.control->motion_requested))
        throw std::invalid_argument("Invalid ordinary returned action menu");
    if ((phase==357 || phase==358) && (!state.action || !state.recovery || !state.control || !state.scheduling || !state.area_contact ||
        state.current.field.side!=0 || state.current.field.ball_owner<1 || state.current.field.ball_owner>10 ||
        state.current.field.selected_defenders!=1 || state.recovery->opposing_player!=state.scheduling->selected_ids[0] ||
        (phase==357 && !state.area_defender_cursor) || state.area_defender_cursor>5 || state.area_sequence_result!=255 || state.saved_ball_flight_flag!=1 ||
        state.current.clock.ball_mode || state.current.clock.script_flags || state.action->action || state.action->menu_route ||
        state.area_contact->cursor || state.area_contact->defense_action!=1 || state.area_contact->defense_subtype ||
        state.area_contact->effect || state.area_contact->defender_power || state.area_contact->cost || state.area_contact->strength ||
        state.area_contact->loss || state.area_contact->score || state.area_contact->result || state.area_contact->weakened))
        throw std::invalid_argument("Invalid ordinary returned shot defense or preparation");
    if (phase==427 && (!state.score || !state.action || !state.recovery || !state.control || !state.scheduling || state.area_contact || state.human_defense_choices ||
        state.score->left_goals!=1 || state.score->right_goals || state.score->kickoff!=4 || state.current.clock.remaining!=135 || state.current.clock.extra_time ||
        state.current.field.side || state.current.field.ball_owner!=3 || state.current.field.ball_x!=104 || state.current.field.ball_y!=164 ||
        state.current.field.selected_defenders || state.scheduling->selected_ids!=std::array<std::uint8_t,5>{} || state.recovery->opposing_player!=20 ||
        state.recovery->ball_variant!=2 || state.recovery->x_jitter_control!=130 || state.current.field.elapsed!=5 ||
        state.current.field.motion[field_motion_index(3)].status!=5 || state.action->action!=3 || state.action->power!=7 || state.control->facing ||
        state.control->motion_requested || state.area_defender_cursor!=2 || state.area_sequence_result!=2 || state.saved_ball_flight_flag!=1))
        throw std::invalid_argument("Invalid player3 one-two loose ball");
    if ((phase>=421 && phase<=426) && (!state.score || !state.action || !state.recovery || !state.control || !state.scheduling || !state.area_contact ||
        state.score->left_goals!=1 || state.score->right_goals || state.score->kickoff!=4 || state.current.clock.remaining!=(phase>=423 ? 144 : 146) || state.current.clock.extra_time ||
        state.current.field.side || state.current.field.ball_owner!=(phase==426 ? 3 : 7) || state.current.field.ball_x!=120 || state.current.field.ball_y!=(phase>=424 ? 140 : 124) ||
        state.current.field.selected_defenders!=(phase>=424 ? 0 : 1) || state.scheduling->selected_ids[0]!=(phase>=424 ? 0 : 20) || state.recovery->opposing_player!=20 ||
        state.current.field.roster[7].stamina!=305 || state.action->action || state.action->power!=5 ||
        (phase==421 ? state.control->motion_requested!=2 : phase>=423 ? state.control->motion_requested!=0 : (state.control->motion_requested!=0 && state.control->motion_requested!=2)) ||
        (phase>=423 && (state.current.field.elapsed!=(phase>=425 ? 0 : 3) || state.current.field.motion[field_motion_index(7)].status!=5 ||
          (phase==426 ? (state.control->facing!=0 && state.control->facing!=64) : state.control->facing!=0))) ||
        (phase==426 && ((state.current.field.motion[field_motion_index(3)].x>>8)!=120 || (state.current.field.motion[field_motion_index(3)].y>>8)!=140)) ||
        state.control->last_carrier_cell!=128 || state.recovery->ball_variant!=1 || state.area_defender_cursor || state.area_sequence_result!=(phase==421 ? 255 : 2) ||
        state.area_contact->cursor || state.area_contact->defense_action!=2 || state.area_contact->defense_subtype ||
        (phase==421 && (state.area_contact->strength || state.area_contact->loss || state.area_contact->score || state.area_contact->result)) ||
        (phase>=422 && (state.area_contact->strength!=4 || state.area_contact->loss!=1 || state.area_contact->score!=80 || state.area_contact->result!=2)) || state.area_contact->weakened))
        throw std::invalid_argument("Invalid player7 shot defender preparation");
    if ((phase>=359 && phase<=364) && (!state.action || !state.recovery || !state.control || !state.scheduling || !state.area_contact ||
        state.current.field.side!=0 || state.current.field.ball_owner<1 || state.current.field.ball_owner>10 ||
        state.current.field.selected_defenders!=1 || state.recovery->opposing_player!=state.scheduling->selected_ids[0] ||
        state.area_defender_cursor!=(phase>=362 ? 1 : 0) || state.area_sequence_result!=(phase==359 ? 255 : 0) ||
        (phase>=363 && state.current.field.elapsed) ||
        state.saved_ball_flight_flag!=1 || state.current.clock.ball_mode || state.current.clock.script_flags || state.action->action || state.action->menu_route ||
        state.area_contact->cursor || state.area_contact->defense_action!=1 || state.area_contact->defense_subtype ||
        state.area_contact->result || state.area_contact->weakened || (phase==359 && (state.area_contact->strength || state.area_contact->loss || state.area_contact->score)) ||
        (phase>=360 && state.area_contact->loss!=state.area_contact->strength/4) || state.control->last_carrier_cell!=128 ||
        state.control->motion_requested!=2 || (state.recovery->ball_variant!=1 && state.recovery->ball_variant!=2)))
        throw std::invalid_argument("Invalid ordinary returned shot defender comparison");
    if ((phase>=365 && phase<=391) && (!state.action || !state.recovery || !state.control || !state.scheduling || !state.shot_flight || !state.ball_trajectory ||
        state.current.field.side!=0 || state.current.field.ball_owner<1 || state.current.field.ball_owner>10 || state.current.field.selected_defenders>5 ||
        state.current.field.elapsed!=((phase>=372 && phase<=374) ? 2 : (phase==383 || phase==385 || phase==386 || phase==387) ? 1 : 0) || state.area_defender_cursor!=(phase>=368 ? 0 : 1) || (phase>=368 && phase<=372 && !state.current.field.selected_defenders) || (phase>=373 && phase<=376 && state.current.field.selected_defenders) ||
        state.area_sequence_result!=((phase>=370 && phase<=381) ? 2 : 0) || state.saved_ball_flight_flag || state.action->action || state.action->menu_route!=((phase>=377 && phase<=385) ? 1 : 0) ||
        bool(state.area_contact)!=(phase>=369) || state.shot_flight->loose_ball || (phase>=379 && phase<=387 && state.current.field.selected_defenders) || (phase>=380 && !(state.action->cursor&128u)) || (phase>=381 && state.recovery->attack_subtype!=1) ||
        (phase>=369 && ((phase<373 && state.recovery->opposing_player!=state.scheduling->selected_ids[0]) ||
          state.recovery->ball_variant!=(phase==369 ? 0 : 1) || state.area_contact->cursor || state.area_contact->defense_action ||
          state.area_contact->defense_subtype || state.area_contact->weakened || state.area_contact->result!=(phase>=370 ? 2 : 0) ||
          (phase==369 && (state.area_contact->strength || state.area_contact->loss || state.area_contact->score)) ||
          (phase>=370 && state.area_contact->loss!=state.area_contact->strength/4)))))
        throw std::invalid_argument("Invalid ordinary returned shot flight setup or defender");
    if(((phase>=114 && phase<=122) || (phase>=124 && phase<=129)) && !state.current.field.selected_defenders)
        throw std::invalid_argument("Defended area arrival needs a selected defender");
    if(((phase>=114 && phase<=122) || (phase>=124 && phase<=129)) && state.area_defender_cursor>=state.current.field.selected_defenders)
        throw std::invalid_argument("Invalid queued pass defender cursor");
    if(phase==123 && (state.current.field.selected_defenders || !state.area_defender_cursor || state.area_defender_cursor>4 || state.current.field.elapsed))
        throw std::invalid_argument("Completed pass defenders need cleared selection and time");
    if(bool(state.area_pass_contact)!=((phase>=116 && phase<=122) || (phase>=124 && phase<=149) || shot_pass_context))throw std::invalid_argument("Prepared pass defender needs saved context");
    if(shot_pass_context && (!state.recovery || !state.scheduling || state.area_pass_contact->saved_ball_variant!=0 ||
       state.recovery->ball_variant!=0 || state.area_defender_cursor>=state.current.field.selected_defenders ||
       state.recovery->opposing_player!=state.scheduling->selected_ids[state.area_defender_cursor]))
        throw std::invalid_argument("Shot pass defender preparation needs its zero stacked variant and selected defender");
    if(((phase>=116 && phase<=122) || (phase>=124 && phase<=149)) && (!state.recovery || !state.scheduling || state.area_pass_contact->saved_ball_variant<1 || state.area_pass_contact->saved_ball_variant>2 ||
       (phase>=118 ? (((phase>=130 && phase<=133) || phase>=138) ? state.recovery->ball_variant!=0 : state.recovery->ball_variant!=state.area_pass_contact->saved_ball_variant) : state.recovery->ball_variant!=0) || (((phase>=130 && phase<=133) || pass_capture_tick) ? state.recovery->opposing_player!=state.current.field.ball_owner : phase>=138 ? state.recovery->opposing_player>10 : phase>=134 ? (state.recovery->opposing_player<1 || state.recovery->opposing_player>10) : state.recovery->opposing_player!=state.scheduling->selected_ids[state.area_defender_cursor])))
        throw std::invalid_argument("Invalid prepared pass defender context");
    if(phase>=78 && phase<=94) {
        const auto& target=*state.area_ai_target;
        const auto subtype=phase>=79 ? state.recovery->attack_subtype&127u : state.recovery->attack_subtype;
        const bool terminal_presentation=(phase==86 || phase==89 || phase==94) && state.area_contact && state.area_contact->result>=2;
        if((phase>=81 && phase<=94) && (state.control->motion_requested!=(terminal_presentation?0:16) ||
           (terminal_presentation ? state.control->facing!=0 : (state.control->facing!=0 && state.control->facing!=64))))throw std::invalid_argument("Invalid announced area presentation state");
        if(phase==80 && state.control->motion_requested!=(state.recovery->ball_variant==1 ? 2 : 0))throw std::invalid_argument("Area defense entry needs acknowledged motion request");
        if(target.cost>1023 || (phase==78 && target.cost))throw std::invalid_argument("Invalid prepared area cost");
        if(state.action->cursor!=state.current.field.selected_defenders ||
           (state.action->action!=1 && state.action->action!=4 && state.action->action!=6) ||
           (state.recovery->ball_variant!=1 && state.recovery->ball_variant!=2) ||
           (state.action->action==1 ? (target.recipient<12 || target.recipient>21 || subtype) :
            (target.recipient || target.cell || (state.action->action==4 ? subtype!=0 : subtype>1))))
            throw std::invalid_argument("Unsupported saved area AI choice");
        if(state.action->action==1) {
            const auto& m=state.current.field.motion[field_motion_index(target.recipient)];
            if(target.cell!=field_cell_at(static_cast<std::uint8_t>(m.x>>8),static_cast<std::uint8_t>(m.y>>8)))
                throw std::invalid_argument("Area AI target cell disagrees with saved player position");
        }
    }
    const bool player5_dribble_contact=phase==20 && state.human_defense_choices && state.area_contact && state.action && state.action->action==2;
    const bool player7_shot_contact=phase==20 && state.human_defense_choices && state.area_contact && state.action && state.action->action==0;
    const bool player3_one_two_contact=phase==20 && state.human_defense_choices && state.area_contact && state.action && state.action->action==3;
    const bool player5_dribble_possession=player5_dribble_contact && state.area_sequence_result==3 &&
        state.current.clock.remaining==366 && !state.current.field.elapsed &&
        state.current.field.side==11 && state.current.field.ball_owner==14;
    if(bool(state.area_contact)!=((phase>=421 && phase<=426) || (phase>=409 && phase<=414) || (phase>=336 && phase<=344) || phase==357 || phase==358 || phase==359 || phase==360 || phase==361 || phase==362 || phase==363 || phase==364 || phase==369 || phase==370 || phase==371 || phase==372 || phase==373 || phase==374 || phase==375 || phase==376 || phase==377 || phase==378 || phase==379 || phase==380 || phase==381 || phase==382 || phase==383 || phase==384 || phase==385 || phase==386 || phase==387 || phase==388 || (phase==389 || phase==390 || phase==391) || (phase>=82 && phase<=86) || phase==89 || phase==94 || (phase==111 || phase==112) || (phase>=117 && phase<=122) || (phase>=124 && phase<=149) || shot_pass_compared || player7_shot_contact || player3_one_two_contact || phase==171 || phase==172 || phase==173 || phase==174 || phase==175 || phase==176 || phase==177 || phase==178 || phase==179 || phase==180 || phase==181 || phase==182 || phase==183 || phase==184 || phase==185 || phase==186 || phase==187 || phase==189 || phase==190 || phase==191 || phase==192 || phase==193 || (phase>=252 && phase<=255) || (phase>=305 && phase<=309) || (phase>=315 && phase<=326) || score23_defender || player5_dribble_contact))throw std::invalid_argument("Area comparison requires saved contact context");
    if((phase==171 || phase==172) && (state.area_contact->cursor || state.area_contact->defense_action!=6 || state.area_contact->defense_subtype ||
       state.area_contact->effect || state.area_contact->strength || state.area_contact->loss || state.area_contact->score ||
       state.area_contact->result || (phase==171 && state.area_contact->weakened)))
        throw std::invalid_argument("Invalid captured action4 goalkeeper preparation");
    if(((phase>=173 && phase<=187) || phase==190 || phase==191 || phase==192 || phase==193) && (state.area_contact->cursor || state.area_contact->defense_action!=6 ||
       (state.area_contact->defense_subtype!=0 && state.area_contact->defense_subtype!=128) ||
       state.area_contact->effect || state.area_contact->loss!=state.area_contact->strength/4 ||
       (state.area_contact->result>1 && state.area_contact->result!=4) || state.area_sequence_result!=state.area_contact->result))
        throw std::invalid_argument("Invalid action4 goalkeeper comparison context");
    if(((phase==175 || phase==187) && state.area_contact->result!=0) || ((phase>=176 && phase<=186) && state.area_contact->result!=1))
        throw std::invalid_argument("Action4 goalkeeper handler requires its corresponding result");
    if((phase==190 || phase==191 || phase==192 || phase==193) && state.area_contact->result!=4)throw std::invalid_argument("Sampled keeper restart requires result4");
    if((phase==191 || phase==192 || phase==193) && (state.current.field.ball_owner!=20 || state.current.field.ball_x!=68 || state.current.field.ball_y!=132 || state.current.field.ball_cell!=30))throw std::invalid_argument("Invalid captured result4 restart formation");
    if(phase==187 && (!state.score || state.score->left_goals!=0 || state.score->right_goals!=1 || state.score->kickoff!=0))throw std::invalid_argument("Invalid sampled result0 goal score");
    if(phase==183 && (state.current.clock.remaining || !state.current.clock.extra_time || state.current.clock.ball_mode || (state.current.clock.script_flags&64u)))
        throw std::invalid_argument("Invalid action4 restart added-time boundary");
    if(phase==184 && state.current.clock.remaining)throw std::invalid_argument("Invalid action4 restart period-end boundary");
    if(shot_pass_compared && (!state.action || !state.area_pass_contact || state.area_contact->cursor!=state.area_defender_cursor || state.area_contact->defense_action!=2 ||
       state.area_contact->defense_subtype || state.area_contact->effect || state.area_contact->cost!=state.area_pass_contact->captured_cost ||
       state.area_contact->strength || state.area_contact->loss || state.area_contact->score!=255 || state.area_contact->result ||
       state.area_contact->weakened || state.area_sequence_result || state.action->menu_route))
        throw std::invalid_argument("Invalid shot pass zero-range comparison");
    if(phase==84 && (state.current.clock.remaining || !state.current.clock.extra_time ||
       state.current.clock.ball_mode || (state.current.clock.script_flags&64u)))
        throw std::invalid_argument("Invalid area contact added-time boundary");
    if(phase==91 && (state.current.clock.remaining || !state.current.clock.extra_time || state.current.clock.ball_mode || (state.current.clock.script_flags&64u)))throw std::invalid_argument("Invalid area prefix added-time boundary");
    if(phase==92 && state.current.clock.remaining)throw std::invalid_argument("Area prefix period end requires exhausted match clock");
    if(phase==85 && state.current.clock.remaining)
        throw std::invalid_argument("Area period end requires exhausted match clock");
    // The sampled pass winner retains $0616 after its selection count clears;
    // Field entry, input, clock, movement, contact and its menu retain one.
    const bool sampled_pass_cursor_residue=(phase==1 || phase==9 || phase==10 || phase==13 || phase==15 || phase==18 || phase==19 || (phase==20 && state.human_defense_choices)) && state.area_defender_cursor==1 &&
       state.score && state.score->left_goals==2 && state.score->right_goals==2 && state.score->kickoff==4 &&
       state.current.field.side==0 && state.current.field.ball_owner==5 && state.saved_ball_flight_flag==1;
    const bool ordinary_pass_control_cursor=(phase==1 || phase==9) && state.area_defender_cursor<=5 && state.current.field.side==0 &&
       state.current.field.ball_owner>=1 && state.current.field.ball_owner<=10 && !state.current.field.selected_defenders &&
       state.saved_ball_flight_flag==1 && !state.shot_flight && !state.ball_trajectory;
    const bool score23_recovery_cursor=(phase==1 || phase==2 || phase==6 || phase==9 || phase==10 || phase==12 || phase==13) && state.area_defender_cursor==1 && state.score &&
       state.score->left_goals==2 && state.score->right_goals==3 && state.score->kickoff==4 && state.current.clock.remaining>=349 && state.current.clock.remaining<=353 &&
       (state.current.field.ball_cell==87 || ((phase==9 || phase==10 || phase==12 || phase==13) && state.current.field.ball_cell==75) ||
        (phase==12 && state.current.field.ball_cell==63)) && state.saved_ball_flight_flag==1 &&
       ((phase==2 && state.current.field.side==0 && state.current.field.ball_owner==5) ||
        ((phase==1 || phase==6 || phase==9 || phase==10 || phase==12 || phase==13) && state.current.field.side==11 && state.current.field.ball_owner==19));
    const bool score25_player7_cursor=(phase==1 || phase==9 || phase==10 || phase==13 || phase==15 || phase==18 || phase==19 || (phase==20 && state.human_defense_choices)) &&
       (state.area_defender_cursor==3 || (phase==20 && state.area_defender_cursor<=5)) &&
       state.score && state.score->kickoff==4 &&
       ((state.score->left_goals==2 && state.score->right_goals==5 && state.current.clock.remaining>=304 && state.current.clock.remaining<=321) ||
        (state.score->left_goals==3 && state.score->right_goals==6 && state.current.clock.remaining>=(phase==20 ? 263 : 269) && state.current.clock.remaining<=279)) && state.current.field.side==0 && state.current.field.ball_owner==7 &&
       state.current.field.ball_cell==103 && state.saved_ball_flight_flag==1;
    const bool score25_shot_keeper_cursor=(phase>=52 && phase<=56) && state.score && state.score->kickoff==4 &&
       ((state.score->left_goals==2 && state.score->right_goals==5 && state.current.clock.remaining==304 && state.area_defender_cursor==5) ||
        (state.score->left_goals==3 && state.score->right_goals==6 && state.current.clock.remaining==263 && state.area_defender_cursor==4)) && state.current.field.side==0 && state.current.field.ball_owner==7 &&
       state.current.field.ball_cell==233 && state.saved_ball_flight_flag==0 && state.shot_flight && state.shot_flight->destination==233;
    if((phase<82 && state.area_defender_cursor && !player3_one_two_contact && !ordinary_pass_control_cursor && !sampled_pass_cursor_residue && !score23_recovery_cursor && !score25_player7_cursor && !score25_shot_keeper_cursor) ||
       (phase>=82 && phase<=94 && (state.area_defender_cursor>state.current.field.selected_defenders ||
        ((phase==88 || (phase>=90 && phase<=93)) ? state.area_defender_cursor!=state.current.field.selected_defenders :
         state.area_defender_cursor>=state.current.field.selected_defenders))) ||
       (phase==87 && !state.area_defender_cursor) ||
       (phase==89 && state.area_contact->result<2))
        throw std::invalid_argument("Invalid saved area defender cursor or exit at phase "+std::to_string(phase)+
            ", owner "+std::to_string(state.current.field.ball_owner));
    const bool player5_dribble_terminal=player5_dribble_contact && state.area_sequence_result==3 && state.current.clock.remaining==366;
    const bool player7_shot_base=state.human_defense_choices && (phase==19 || phase==20) && state.score && state.action && state.recovery && state.scheduling && state.control && state.experience &&
       state.human_defense_choices->actions==std::array<std::uint8_t,5>{1,1,1,1,1} && state.human_defense_choices->subtypes==std::array<std::uint8_t,5>{} &&
       state.score->left_goals==2 && state.score->right_goals==5 && state.score->kickoff==4 && state.current.clock.extra_time==13 &&
       state.current.field.side==0 && state.current.field.ball_owner==7 && state.current.field.ball_x==117 && state.current.field.ball_y==139 && state.current.field.ball_cell==103 &&
       state.current.field.roster[7].type==23 && state.current.field.roster[7].stamina==(phase==19 ? 210 : 130) && state.current.field.selected_defenders==5 &&
       state.scheduling->selected_ids==std::array<std::uint8_t,5>{12,14,15,16,18} && state.action->action==0 && state.action->cursor==224 && state.saved_ball_flight_flag==1;
    bool player7_shot_choices=false;
    if(player7_shot_base) {
       const auto cursor=state.area_defender_cursor;
       const bool initial=cursor==3 && !state.area_contact && state.current.clock.remaining==311 && state.recovery->opposing_player==18;
       if(phase==19 || initial) player7_shot_choices=cursor==3 && state.recovery->opposing_player==18 && state.current.clock.remaining==311;
       else if(cursor<=5) {
          const bool clocked=state.area_contact && cursor<5 && state.current.field.motion[field_motion_index(state.scheduling->selected_ids[cursor])].status==6;
          const auto expected_opponent=state.area_contact && cursor<5 ? state.scheduling->selected_ids[cursor] : cursor ? state.scheduling->selected_ids[cursor-1] : 18;
          player7_shot_choices=(state.current.clock.remaining==311-cursor-(clocked ? 1 : 0) ||
             (cursor==5 && !state.area_contact && state.current.clock.remaining==304 && (state.current.field.elapsed==8 || (state.current.field.elapsed==0 && state.current.field.movement_region==135)))) && state.recovery->opposing_player==expected_opponent &&
             (cursor || state.control->last_carrier_cell==(state.area_contact ? 128 : 0));
       }
       if(player7_shot_choices) {
          for(unsigned slot=0;slot<5;++slot) {
             const auto status=state.current.field.motion[field_motion_index(state.scheduling->selected_ids[slot])].status;
             if(initial || phase==19) { if(status) player7_shot_choices=false; }
             else if(slot<cursor) { if(status!=6) player7_shot_choices=false; }
             else if(slot>cursor || !state.area_contact) { if(status) player7_shot_choices=false; }
             else if(status!=0 && (status!=6 || state.area_sequence_result!=0)) player7_shot_choices=false;
          }
       }
    }
    const bool player7_shot36_base=state.human_defense_choices && (phase==19 || phase==20) && state.score && state.action && state.recovery && state.scheduling && state.control && state.experience &&
       state.human_defense_choices->actions==std::array<std::uint8_t,5>{1,1,1,1,1} && state.human_defense_choices->subtypes==std::array<std::uint8_t,5>{} &&
       state.score->left_goals==3 && state.score->right_goals==6 && state.score->kickoff==4 && state.current.clock.extra_time==13 &&
       state.current.field.side==0 && state.current.field.ball_owner==7 && state.current.field.ball_x==113 && state.current.field.ball_y==143 && state.current.field.ball_cell==103 &&
       state.current.field.roster[7].type==23 && state.current.field.roster[7].stamina==(phase==19 ? 140 : 60) && state.current.field.selected_defenders==4 &&
       state.scheduling->selected_ids==std::array<std::uint8_t,5>{12,15,16,21,0} && state.action->action==0 && state.action->cursor==224 && state.saved_ball_flight_flag==1 &&
       !state.current.field.random_first && !state.current.field.random_second;
    bool player7_shot36_choices=false;
    if(player7_shot36_base) {
       const auto cursor=state.area_defender_cursor;
       const bool initial=cursor==3 && !state.area_contact && state.current.clock.remaining==269 && state.recovery->opposing_player==21;
       if(phase==19 || initial) player7_shot36_choices=initial;
       else if(cursor<=4) {
          const bool clocked=state.area_contact && cursor<4 && state.current.field.motion[field_motion_index(state.scheduling->selected_ids[cursor])].status==6;
          const auto expected_opponent=state.area_contact && cursor<4 ? state.scheduling->selected_ids[cursor] : cursor ? state.scheduling->selected_ids[cursor-1] : 21;
          player7_shot36_choices=(state.current.clock.remaining==269-cursor-(clocked ? 1 : 0) ||
             (cursor==4 && !state.area_contact && state.current.clock.remaining==263 && (state.current.field.elapsed==7 || (state.current.field.elapsed==0 && state.current.field.movement_region==135)))) && state.recovery->opposing_player==expected_opponent &&
             (cursor || state.control->last_carrier_cell==(state.area_contact ? 128 : 0));
       }
       if(player7_shot36_choices && state.current.clock.remaining>=265 && state.current.clock.remaining<=269 &&
          state.current.field.elapsed!=270-state.current.clock.remaining)player7_shot36_choices=false;
       if(player7_shot36_choices) {
          for(unsigned slot=0;slot<4;++slot) {
             const auto status=state.current.field.motion[field_motion_index(state.scheduling->selected_ids[slot])].status;
             if(initial || phase==19) { if(status) player7_shot36_choices=false; }
             else if(slot<cursor) { if(status!=6) player7_shot36_choices=false; }
             else if(slot>cursor || !state.area_contact) { if(status) player7_shot36_choices=false; }
             else if(status!=0 && (status!=6 || state.area_sequence_result!=0)) player7_shot36_choices=false;
          }
       }
    }
    const bool player3_one_two_choices=state.human_defense_choices && (phase==19 || phase==20) && state.score && state.action && state.recovery && state.scheduling && state.control && state.experience &&
       state.human_defense_choices->actions==std::array<std::uint8_t,5>{1,2,2,0,0} && state.human_defense_choices->subtypes==std::array<std::uint8_t,5>{} &&
       state.score->left_goals==1 && state.score->right_goals==0 && state.score->kickoff==4 &&
       state.current.clock.remaining==(player3_one_two_contact && state.current.field.motion[field_motion_index(3)].status==5 ? 135 : player3_one_two_contact && state.current.field.motion[field_motion_index(19)].status==6 ? 137 : player3_one_two_contact && state.current.field.motion[field_motion_index(16)].status==6 ? 138 : 139) && state.current.clock.extra_time==0 &&
       state.current.field.side==0 && state.current.field.ball_owner==3 && state.current.field.ball_x==120 && state.current.field.ball_y==140 && state.current.field.ball_cell==115 &&
       state.current.field.roster[3].stamina==(phase==19 ? 310 : 250) && state.current.field.selected_defenders==3 &&
       state.scheduling->selected_ids==std::array<std::uint8_t,5>{16,19,20,0,0} && (phase==20 || (state.current.field.random_first==32 && state.current.field.random_second==42)) &&
       state.recovery->opposing_player==(player3_one_two_contact ? (state.area_defender_cursor==2 ? 20 : state.area_defender_cursor==1 ? 19 : 16) : 20) && state.action->action==3 && state.action->cursor==162 && state.action->move_flags==3 &&
       state.action->power==(phase==19 ? 5 : player3_one_two_contact && state.current.field.motion[field_motion_index(19)].status==6 ? 7 : 8) && (phase==19 ? state.control->motion_requested==1 : state.control->motion_requested<=1) &&
       state.saved_ball_flight_flag==1 && state.area_defender_cursor<=(player3_one_two_contact ? 2 : 0) && (!state.area_contact || player3_one_two_contact);
    if(state.human_defense_choices && !player3_one_two_choices && !player7_shot_choices && !player7_shot36_choices && ((phase!=19 && phase!=20) || !state.score || !state.action || !state.recovery || !state.scheduling || !state.control || !state.experience ||
       state.human_defense_choices->actions!=std::array<std::uint8_t,5>{1,1,1,1,1} ||
       state.human_defense_choices->subtypes!=std::array<std::uint8_t,5>{} ||
       state.score->left_goals!=2 || state.score->right_goals!=2 || state.score->kickoff!=4 ||
       state.current.clock.remaining!=(player5_dribble_terminal ? 366 : 367) || state.current.clock.extra_time!=13 ||
       (!player5_dribble_possession && (state.current.field.side!=0 || state.current.field.ball_owner!=5)) || state.current.field.ball_x!=108 ||
       state.current.field.ball_y!=108 || state.current.field.ball_cell!=87 || state.current.field.roster[5].type!=23 ||
       state.current.field.roster[5].stamina!=(phase==19 ? 10 : 0) || state.current.field.selected_defenders!=5 ||
       state.scheduling->selected_ids!=std::array<std::uint8_t,5>{14,16,18,19,20} ||
       state.recovery->opposing_player!=(player5_dribble_contact ? 14 : 20) || state.action->action!=2 || state.action->cursor!=227 ||
       (phase==19 ? state.area_defender_cursor!=1 : state.area_defender_cursor>1) ||
       (phase==20 && state.area_defender_cursor==0 && state.control->last_carrier_cell!=0) || state.saved_ball_flight_flag!=1))
        throw std::invalid_argument("Invalid sampled five-defender dribble context");
    if(player3_one_two_contact && (state.recovery->ball_variant!=2 || (state.area_defender_cursor ? (state.control->facing!=0 && state.control->facing!=64) : state.control->facing!=64) || state.control->motion_requested || state.control->last_carrier_cell!=128 ||
       state.area_contact->cursor!=state.area_defender_cursor || state.area_contact->defense_action!=(state.area_defender_cursor ? 2 : 1) || state.area_contact->defense_subtype || state.area_contact->effect ||
       state.area_contact->defender_power!=8 || state.area_contact->cost!=(state.area_defender_cursor ? 50 : 60) || state.area_contact->weakened ||
       (state.area_sequence_result==255 ? (state.area_contact->strength || state.area_contact->loss || state.area_contact->score || state.area_contact->result) :
         state.area_defender_cursor==2 ? (state.area_sequence_result!=2 || state.area_contact->strength!=6 || state.area_contact->loss!=1 || state.area_contact->score!=74 || state.area_contact->result!=2) :
         state.area_defender_cursor==1 ? (state.area_sequence_result!=1 || state.area_contact->strength!=5 || state.area_contact->loss!=1 || state.area_contact->score!=102 || state.area_contact->result!=1) :
         (state.area_sequence_result!=0 || state.area_contact->strength!=1 || state.area_contact->loss || state.area_contact->score!=255 || state.area_contact->result))))
        throw std::invalid_argument("Invalid player3 one-two first defender comparison");
    if(player7_shot_contact && (state.area_defender_cursor>=5 || state.recovery->ball_variant!=1 || state.control->last_carrier_cell!=128 ||
       state.control->motion_requested!=((state.area_defender_cursor==0 && state.current.field.motion[field_motion_index(state.recovery->opposing_player)].status==0) ? 2 : 0) ||
       state.area_contact->cursor!=state.area_defender_cursor || state.area_contact->defense_action!=1 ||
       state.area_contact->defense_subtype || state.area_contact->effect || state.area_contact->defender_power!=18 || state.area_contact->cost!=60 ||
       state.area_contact->strength || state.area_contact->loss || state.area_contact->score!=(state.area_sequence_result==0 ? 255 : 0) || state.area_contact->result ||
       state.area_contact->weakened || (state.area_sequence_result!=255 && state.area_sequence_result!=0)))
        throw std::invalid_argument("Invalid player7 defender preparation");
    if(player5_dribble_terminal && ((state.current.field.elapsed!=2 && (state.current.field.elapsed!=0 || state.current.field.movement_region!=129)) || state.current.field.motion[field_motion_index(5)].status!=5 || state.control->facing!=64))
        throw std::invalid_argument("Invalid sampled player5 terminal clock state");
    if(player5_dribble_contact && (state.area_defender_cursor || state.area_contact->cursor || state.area_contact->defense_action!=1 ||
       state.area_contact->defense_subtype || state.area_contact->effect || state.area_contact->defender_power!=18 || state.area_contact->cost!=60 ||
       state.area_contact->weakened || state.area_contact->score ||
       (state.area_sequence_result==255 ? (state.area_contact->strength || state.area_contact->loss || state.area_contact->result) :
        (state.area_sequence_result!=3 || state.area_contact->strength!=4 || state.area_contact->loss!=1 || state.area_contact->result!=3))))
        throw std::invalid_argument("Invalid sampled player5 dribble defender comparison");
    if(state.area_contact && !((phase>=421 && phase<=426) || (phase>=409 && phase<=414) || (phase>=336 && phase<=344) || phase==357 || phase==358 || phase==359 || phase==360 || phase==361 || phase==362 || phase==363 || phase==364 || phase==369 || phase==370 || phase==371 || phase==372 || phase==373 || phase==374 || phase==375 || phase==376 || phase==377 || phase==378 || phase==379 || phase==380 || phase==381 || phase==382 || phase==383 || phase==384 || phase==385 || phase==386 || phase==387 || phase==388 || (phase==389 || phase==390 || phase==391) || player5_dribble_contact || player7_shot_contact || player3_one_two_contact || score23_defender || (phase>=315 && phase<=326) || (phase>=305 && phase<=309) || (phase==111 || phase==112) || (phase>=117 && phase<=122) || (phase>=124 && phase<=149) || shot_pass_compared || phase==171 || phase==172 || phase==173 || phase==174 || phase==175 || phase==176 || phase==177 || phase==178 || phase==179 || phase==180 || phase==181 || phase==182 || phase==183 || phase==184 || phase==185 || phase==186 || phase==187 || phase==189 || phase==190 || phase==191 || phase==192 || phase==193 || (phase>=252 && phase<=255))) {
        const auto& c=*state.area_contact;
        if(c.cursor!=state.area_defender_cursor || c.cursor>=state.current.field.selected_defenders ||
           state.recovery->opposing_player!=state.scheduling->selected_ids[c.cursor] ||
           c.defense_action!=state.area_defense->actions[c.cursor] ||
           (c.defense_action!=2 && c.defense_action!=4) ||
           (c.defense_subtype&127u)!=state.area_defense->subtypes[c.cursor] ||
           c.result>3 || c.cost>1534 || c.loss!=c.strength/4)
            throw std::invalid_argument("Invalid saved area comparison");
    }
    if(((phase>=117 && phase<=122) || (phase>=124 && phase<=149)) && (state.area_contact->cursor!=state.area_defender_cursor || state.area_contact->defense_action!=2 ||
       (state.area_contact->defense_subtype!=0 && state.area_contact->defense_subtype!=128) ||
       state.area_contact->result!=state.area_sequence_result || state.area_contact->weakened ||
       state.area_contact->loss!=state.area_contact->strength/4))
        throw std::invalid_argument("Invalid pass defender comparison context");
    if(phase==124 && state.area_sequence_result!=2)throw std::invalid_argument("Positioned blocked pass requires result2");
    if(phase>=125 && phase<=127 && state.area_sequence_result<2)throw std::invalid_argument("Pass terminal clock requires result2 or3");
    if((phase>=128 && phase<=133) && (state.area_sequence_result!=3 || state.current.field.ball_owner!=state.recovery->opposing_player || ((phase==129 || phase==130) && state.current.field.elapsed) || ((phase>=131 && phase<=133) && state.current.field.elapsed>1) || (phase>=130 && state.current.field.selected_defenders)))
        throw std::invalid_argument("Captured pass requires the defending player as owner");
    if((phase>=134 && phase<=141) && (state.area_sequence_result!=2 || state.current.field.selected_defenders || state.area_defender_cursor>3 || ((phase==135 || phase==137 || phase==138) && state.current.field.elapsed) || ((phase>=139 && phase<=141) && state.current.field.elapsed>1) || (phase>=139 && state.current.field.side!=0)))
        throw std::invalid_argument("Blocked pass recovery needs a cleared defender queue");
    if((pass_capture_tick || pass_blocked_tick) && (state.area_sequence_result!=(pass_capture_tick ? 3 : 2) || state.current.field.selected_defenders || state.current.field.elapsed!=1))
        throw std::invalid_argument("Area pass control tick requires its completed clock state");
    if((phase==81 && state.area_sequence_result!=0) ||
       (phase>=82 && phase<=86 && state.area_sequence_result!=state.area_contact->result) ||
       (phase==89 && state.area_sequence_result!=state.area_contact->result) ||
       ((phase==87 || phase==88 || (phase>=90 && phase<=92)) &&
        state.area_sequence_result!=255 && state.area_sequence_result>1) ||
       (phase==93 && state.area_sequence_result>1) ||
       (phase==94 && state.area_sequence_result!=state.area_contact->result) ||
       (phase==95 && state.area_sequence_result!=255))
        throw std::invalid_argument("Invalid saved final area sequence result");
    if(phase==68 && state.current.field.selected_defenders)
        throw std::invalid_argument("Empty area defense completion requires no selected players");
    if (bool(state.shot_flight) != ((phase >= 46 && phase <= 56) || phase==59 || (phase>=150 && phase<=159) || phase==188 || phase==189 || phase==243 || phase==364 || (phase>=400 && phase<=414) || (phase>=365 && phase<=391) || score23_shot_flight || score25_shot || area_flight))
        throw std::invalid_argument("Flight setup requires exactly one shot context");
    if (state.shot_flight && (state.shot_flight->destination >= 240 || (state.shot_flight->counter != 255 && !(area_flight && (state.shot_flight->counter==38 || state.shot_flight->counter==77))) || (area_flight && (state.shot_flight->destination!=state.area_ai_target->cell || state.shot_flight->loose_ball))))
        throw std::invalid_argument("Unsupported initial shot flight context");
    const bool trajectory=(phase>=47 && phase<=56) || phase==59 || (phase>=150 && phase<=159) || phase==188 || phase==189 || (phase>=400 && phase<=414) || (phase>=292 && phase<=294) || (phase>=328 && phase<=330) || (phase>=365 && phase<=391) || area_flight;
    if(bool(state.ball_trajectory)!=trajectory)
        throw std::invalid_argument("Numeric flight requires exactly one trajectory context");
    if(state.ball_trajectory) {
        const auto& t=*state.ball_trajectory;const auto& f=state.current.field;
        if(!state.recovery || !state.scheduling || t.flags!=(area_flight?1:state.saved_ball_flight_flag) || t.direction!=state.recovery->x_jitter_control ||
            (t.x>>8)!=f.ball_x || (t.y>>8)!=f.ball_y ||
            (!((((phase>=124 && phase<=127) || (phase>=134 && phase<=137)) && state.area_sequence_result==2) || phase==371 || phase==372 || phase==373 || phase==374 || phase==375 || phase==376 || phase==377 || phase==378 || phase==379 || phase==380 || phase==381 || phase==382 || phase==383 || phase==384 || phase==385 || phase==386 || phase==387 || phase==388 || (phase==389 || phase==390 || phase==391)) && field_cell_at(f.ball_x,f.ball_y)!=f.ball_cell) || (f.elapsed && !(phase==411 || phase==351 || phase==372 || phase==373 || phase==374 || phase==383 || phase==385 || phase==386 || phase==387 || phase==388 || (phase==389 || phase==390 || phase==391) || (phase>=114 && phase<=122) || (phase>=124 && phase<=128) || (phase>=131 && phase<=133) || phase==134 || phase==136 || (phase>=139 && phase<=141) || (phase>=150 && phase<=156) || phase==158 || phase==159 || pass_capture_tick || pass_blocked_tick)) ||
            ((phase==50 || phase==51 || phase==52 || phase==54 || phase==55) && f.selected_defenders) ||
            ((phase==47 || phase==328) && (!t.vx && !t.vy)) ||
            ((phase==47 || phase==328) && f.ball_cell==state.shot_flight->destination) ||
            ((phase==48 || phase==329) && f.ball_cell==state.shot_flight->destination) ||
            (((phase>=49 && phase<=56) || phase==59 || phase==97 || (phase>=150 && phase<=159) || phase==188 || phase==189 || phase==330) && f.ball_cell!=state.shot_flight->destination) ||
            (phase==96 && (f.ball_cell==state.shot_flight->destination || (!t.vx && !t.vy))) ||
            (phase==97 && f.selected_defenders) ||
            (phase==98 && f.ball_cell==state.shot_flight->destination) ||
            (phase==99 && f.ball_cell!=state.shot_flight->destination) ||
            ((phase==365 || phase==366) && f.ball_cell==state.shot_flight->destination) ||
            (phase==365 && !t.vx && !t.vy) ||
            ((phase>=367 && phase<=391) && f.ball_cell!=state.shot_flight->destination) ||
            (phase==345 && (f.ball_cell==state.shot_flight->destination || (!t.vx && !t.vy))) ||
            (phase==346 && f.ball_cell==state.shot_flight->destination) ||
            (phase==347 && f.ball_cell!=state.shot_flight->destination) ||
            (phase==348 && f.ball_cell!=state.shot_flight->destination) ||
            (phase==100 && (f.ball_cell!=state.shot_flight->destination || f.selected_defenders || f.ball_owner!=state.area_ai_target->recipient)) ||
            (phase==101 && (f.selected_defenders || f.ball_owner!=state.area_ai_target->recipient)) ||
            (phase==50 && (t.vx || t.vy)))
            throw std::invalid_argument("Unsupported numeric flight boundary");
    }
    const bool sampled_kickoff_loose_defenders=(phase==150 || phase==151 || phase==188 || phase==189) && !state.saved_ball_flight_flag && state.shot_flight && state.shot_flight->loose_ball &&
       state.current.field.side==0 && state.current.field.ball_owner==7 && state.current.field.ball_x==188 && state.current.field.ball_y==124 &&
       state.current.field.ball_cell==209 && state.current.field.random_first==0 && state.current.field.random_second==128 &&
       state.current.field.selected_defenders==2 && state.scheduling && state.scheduling->selected_ids[0]==19 && state.scheduling->selected_ids[1]==14 &&
       state.score && state.score->left_goals==0 && state.score->right_goals==1 && state.score->kickoff==4 && state.kickoff_formation;
    if(((phase>=150 && phase<=156) || phase==158 || phase==159 || phase==188 || phase==189) && ((!state.saved_ball_flight_flag && !sampled_kickoff_loose_defenders) || state.current.field.side!=0 ||
       !state.current.field.selected_defenders || state.current.field.selected_defenders>5 || state.area_defender_cursor>=state.current.field.selected_defenders))
        throw std::invalid_argument("Shot pass defenders require a nonempty side0 prefix and valid cursor");
    if((phase==188 || phase==189) && (!sampled_kickoff_loose_defenders || state.area_defender_cursor || state.area_sequence_result!=(phase==188 ? 255 : 2) ||
       !state.recovery || state.recovery->opposing_player!=19 || state.recovery->ball_variant))
        throw std::invalid_argument("Prepared sampled loose-shot defender requires the first observed player");
    if(phase==189 && (!state.area_contact || state.area_contact->cursor || state.area_contact->defense_action || state.area_contact->defense_subtype ||
       state.area_contact->effect || state.area_contact->defender_power!=19 || state.area_contact->cost || state.area_contact->strength!=9 ||
       state.area_contact->loss!=2 || state.area_contact->score!=78 || state.area_contact->result!=2 || state.area_contact->weakened))
        throw std::invalid_argument("Unsupported sampled loose-shot comparison");
    if(phase>=150 && phase<=152 && state.area_sequence_result!=255)
        throw std::invalid_argument("Pending shot pass defender must not retain the prior result");
    if(phase==157 && (!state.saved_ball_flight_flag || state.current.field.side!=0 || state.current.field.selected_defenders ||
       state.current.field.elapsed || !state.area_defender_cursor || state.area_defender_cursor>5 || state.area_sequence_result!=255))
        throw std::invalid_argument("Completed shot pass defenders require a cleared prefix and elapsed time");
    if(phase==158 && (state.current.clock.remaining || !state.current.clock.extra_time || state.current.clock.ball_mode || (state.current.clock.script_flags&64u)))
        throw std::invalid_argument("Invalid shot pass added-time boundary");
    if(phase==159 && state.current.clock.remaining)throw std::invalid_argument("Invalid shot pass period-end boundary");
    if(phase==155 && (!state.control || (state.control->facing!=0 && state.control->facing!=64)))
        throw std::invalid_argument("Shot pass result needs captured facing");
    if((phase==51 && !state.shot_flight->loose_ball) || (((phase>=52 && phase<=56) || phase==59) && state.shot_flight->loose_ball))
        throw std::invalid_argument("Flight return phase does not match saved loose/keeper route");
    if(bool(state.keeper_decision)!=(phase==55 || phase==56 || phase==59 || phase==390 || phase==391) ||
       (state.keeper_decision && (state.keeper_decision->cursor ||
        state.keeper_decision->result_code>(phase==55?1:(phase==390 || phase==391)?0:3) || state.keeper_decision->defense_action>3 ||
        ((phase==55 || phase==390 || phase==391) && (state.keeper_decision->defense_action || state.keeper_decision->defense_subtype || state.keeper_decision->effect)))))
        throw std::invalid_argument("Stunned keeper decision needs one valid result/cursor context");
    if((phase>=194 && phase<=200) && (!state.action || !state.recovery || !state.control || !state.experience || !state.score ||
       !state.kickoff_formation || state.current.field.side!=11 || state.current.field.ball_owner!=16 ||
       state.current.field.ball_x!=141 || state.current.field.ball_y!=148 || state.current.field.ball_cell!=140 ||
       state.current.field.roster[16].type!=23 || state.current.field.selected_defenders ||
       state.current.field.random_first || state.current.field.random_second || state.experience->opponent_stage ||
       !((state.current.clock.remaining==443 && state.current.clock.extra_time==5 && state.score->left_goals==1 && state.score->right_goals==0) ||
         (phase<=200 && state.current.clock.remaining==406 && state.current.clock.extra_time==9 && state.score->left_goals==2 && state.score->right_goals==1) ||
         (phase<=200 && state.current.clock.remaining==296 && state.current.clock.extra_time==13 && state.score->left_goals==3 && state.score->right_goals==5)) ||
       state.control->last_carrier_cell!=(phase>=199 ? 128 : phase>=197 ? 0 : 140) || state.score->kickoff!=4 ||
       state.current.clock.script_flags!=(phase>=196 ? 0 : 128) || state.current.clock.ball_mode!=(phase>=196 ? 0 : 128) ||
       (phase>=198 && (state.current.field.elapsed!=0 || state.current.field.movement_region!=140)) ||
       (phase>=199 && (state.recovery->special_owner_flags!=0 || state.recovery->ball_variant!=1 || state.control->facing!=((state.score->left_goals==3 && state.score->right_goals==5 && state.current.clock.remaining==296) ? 0 : 64) || state.control->display_parameter!=0 || state.control->motion_requested!=0)) ||
       state.action->action!=1 || state.action->power!=(phase==194 ? 22 : 21) || state.recovery->attack_subtype!=(phase==194 ? 0 : 128)))
        throw std::invalid_argument("Invalid sampled opponent AI choice state");
    if(bool(state.sampled_field_pass)!=(phase==112 || phase==200 || phase==229 || phase==235 || phase==261 || phase==326 || phase==344) || (state.sampled_field_pass && phase!=344 &&
       (state.sampled_field_pass->recipient!=(phase==326 ? 7 : phase==112 ? 255 : phase==229 ? 12 : 19) || state.sampled_field_pass->destination!=((phase==200 && state.score && state.score->left_goals==3 && state.score->right_goals==5) ? 70 : phase==112 ? 87 : phase==261 ? 63 : phase==235 ? 79 : phase==229 ? 142 : 103) ||
        state.sampled_field_pass->counter!=255 || state.sampled_field_pass->distance!=((phase==200 && state.score && state.score->left_goals==3 && state.score->right_goals==5) ? 4 : phase==112 ? 2 : 3))))
        throw std::invalid_argument("Invalid sampled opponent pass target context");
    if(phase==344) {
        const auto recipient=state.sampled_field_pass->recipient;
        std::uint8_t x=0,y=0;
        if(recipient!=255 && recipient==*state.ordinary_pass_recipient) {
            const auto& actor=state.current.field.motion[field_motion_index(recipient)];
            x=static_cast<std::uint8_t>(actor.x>>8);y=static_cast<std::uint8_t>(actor.y>>8);
        }
        if(recipient!=*state.ordinary_pass_recipient || state.sampled_field_pass->counter!=255 ||
           state.sampled_field_pass->destination>=240 || state.sampled_field_pass->destination!=resolve_pass_target_cell(recipient,*state.human_pass_cursor,x,y))
            throw std::invalid_argument("Invalid resolved ordinary pass target");
    }
    if(phase==200 || phase==229 || phase==235 || phase==261) {
        const auto& recipient=state.current.field.motion[field_motion_index(state.sampled_field_pass->recipient)];
        if(field_cell_at(static_cast<std::uint8_t>(recipient.x>>8),static_cast<std::uint8_t>(recipient.y>>8))!=state.sampled_field_pass->destination)
            throw std::invalid_argument("Sampled opponent pass target disagrees with receiver position");
    }
    const bool postgoal_mixed=(phase==205 || (phase>=215 && phase<=218) || phase==227 || phase==228 || phase==229) && state.area_defense &&
       state.area_defense->actions==std::array<std::uint8_t,4>{0,6,6,255};
    const bool postgoal_skipped=(phase>=205 && phase<=207) && state.area_defense &&
       state.area_defense->actions==std::array<std::uint8_t,4>{6,6,6,255};
    const bool postgoal_field_contact=((phase>=201 && phase<=204) || postgoal_mixed || postgoal_skipped) && state.action && state.recovery && state.control && state.experience && state.score && state.scheduling &&
       state.kickoff_formation && state.saved_ball_flight_flag==1 && state.current.field.side==11 && state.current.field.ball_owner==19 &&
       state.current.field.ball_x==111 && state.current.field.ball_y==141 && state.current.field.ball_cell==91 && state.current.field.elapsed==((phase==228 || phase==229) ? 19 : 1) &&
       state.current.field.selected_defenders==3 && state.scheduling->selected_ids==std::array<std::uint8_t,5>{3,8,9,0,0} &&
       !state.current.field.random_first && !state.current.field.random_second && !state.experience->opponent_stage &&
       state.current.clock.remaining==((phase==228 || phase==229) ? 385 : 403) && state.current.clock.extra_time==((phase==228 || phase==229) ? 13 : 9) && !state.current.clock.script_flags && !state.current.clock.ball_mode &&
       state.score->left_goals==2 && state.score->right_goals==1 && state.score->kickoff==4 &&
       !state.control->display_parameter && !state.control->motion_requested && state.action->action==((phase==228 || phase==229) ? 1 : 2) &&
       state.action->power==((phase==207 || (phase>=216 && phase<=218) || phase==227 || phase==228 || phase==229) ? 24 : 21) && (phase<206 || state.recovery->attack_subtype==((phase==206 || phase==215 || phase==228 || phase==229) ? 0 : 128));
    const bool third_return_contact=(phase>=201 && phase<=207) && state.action && state.recovery && state.control && state.experience && state.score && state.scheduling &&
       state.current.field.side==11 && state.current.field.ball_owner==18 && state.current.field.ball_x==110 && state.current.field.ball_y==108 && state.current.field.ball_cell==87 &&
       state.current.field.elapsed==1 && state.current.field.selected_defenders==1 && state.scheduling->selected_ids==std::array<std::uint8_t,5>{5,0,0,0,0} &&
       (phase!=201 || (state.current.field.random_first==60 && state.current.field.random_second==131)) && state.current.clock.remaining==153 && !state.current.clock.extra_time &&
       !state.current.clock.script_flags && !state.current.clock.ball_mode && state.score->left_goals==1 && !state.score->right_goals && state.score->kickoff==4 &&
       !state.control->display_parameter && !state.control->motion_requested && state.action->action==2 && state.action->power==(phase==207 ? 4 : 12);
    if(((phase>=201 && phase<=208) || ((phase>=215 && phase<=218) || phase==227 || (phase==228 || phase==229))) && !postgoal_field_contact && !third_return_contact && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling ||
       !state.kickoff_formation || state.saved_ball_flight_flag!=1 || state.current.field.side!=11 || state.current.field.ball_owner!=19 ||
       state.current.field.ball_x!=112 || state.current.field.ball_y!=140 || state.current.field.ball_cell!=103 || state.current.field.elapsed!=((phase==228 || phase==229) ? 19 : phase==208 ? 0 : 1) ||
       state.current.field.selected_defenders!=3 || state.scheduling->selected_ids!=std::array<std::uint8_t,5>{3,7,8,0,0} ||
       state.current.field.random_first || state.current.field.random_second || state.experience->opponent_stage ||
       state.current.clock.remaining!=((phase==228 || phase==229) ? 422 : phase==208 ? 439 : 440) || state.current.clock.extra_time!=((phase==228 || phase==229) ? 9 : 5) || state.current.clock.script_flags || state.current.clock.ball_mode ||
       state.score->left_goals!=1 || state.score->right_goals!=0 || state.score->kickoff!=4 ||
       state.control->display_parameter || state.control->motion_requested || state.action->action!=((phase==228 || phase==229) ? 1 : phase>=206 ? 2 : 1) || state.action->power!=(((phase>=207 && phase<=208) || ((phase>=216 && phase<=218) || phase==227 || (phase==228 || phase==229))) ? 24 : 21) ||
       ((phase==206 || phase==215) && state.recovery->attack_subtype!=0) || (((phase>=207 && phase<=208) || ((phase>=216 && phase<=218) || phase==227)) && state.recovery->attack_subtype!=128)))
        throw std::invalid_argument("Invalid sampled opponent contact presentation");
    if((phase==217 || phase==218 || phase==227 || (phase==228 || phase==229)) && (state.recovery->opposing_player!=3 || state.control->last_carrier_cell!=0 || ((phase==218 || phase==227 || (phase==228 || phase==229)) && state.current.field.roster[3].stamina!=0)))
        throw std::invalid_argument("Invalid sampled mixed defender preparation");
    if((phase==227 || (phase==228 || phase==229)) && state.area_sequence_result!=4)throw std::invalid_argument("Invalid sampled mixed defender result");
    if((phase==228 || phase==229) && (state.recovery->attack_subtype!=0 || state.control->facing!=(postgoal_field_contact ? 0 : 64)))throw std::invalid_argument("Invalid sampled result4 pass handoff");
    if(phase==202 && (!state.area_defense || state.action->cursor!=0 || state.action->menu_route!=0 || state.action->auxiliary!=0 ||
       state.action->flash_counter!=(third_return_contact ? 192 : postgoal_field_contact ? 13 : 128) || state.area_defense->blink!=((postgoal_field_contact || third_return_contact) ? 13 : 7) || state.area_defense->pressed || state.area_defense->actions!=std::array<std::uint8_t,4>{255,255,255,255} ||
       state.area_defense->subtypes!=std::array<std::uint8_t,4>{0,0,0,0}))
        throw std::invalid_argument("Invalid sampled field defense prompt");
    if((phase==203 || phase==204) && third_return_contact && (!state.area_defense || state.action->cursor>(phase==203 ? 1 : 0) || (state.action->flash_counter!=128 && state.action->flash_counter!=192) ||
       (state.area_defense->actions!=std::array<std::uint8_t,4>{0,255,255,255} && state.area_defense->actions!=std::array<std::uint8_t,4>{255,255,255,255}) ||
       state.area_defense->subtypes!=std::array<std::uint8_t,4>{0,0,0,0} ||
       (state.action->cursor==1 && (state.area_defense->actions[0]!=0 || state.area_defense->blink!=0 || state.action->flash_counter!=128))))
        throw std::invalid_argument("Invalid third-return defense selection");
    if((phase>=203 && phase<=208) || ((phase>=215 && phase<=218) || phase==227 || (phase==228 || phase==229))) {
        const auto& defense=*state.area_defense;const auto cursor=state.action->cursor;
        if(state.action->menu_route!=0 || state.action->auxiliary!=0 || cursor>3 || defense.blink>13 ||
           (phase==204 ? (!(defense.pressed&128u) || cursor>=3 || defense.actions[cursor]==255 ||
                          state.recovery->opposing_player!=state.scheduling->selected_ids[cursor]) : defense.pressed!=0) ||
           (phase>=205 && cursor!=(third_return_contact ? 1 : 3)) ||
           ((phase>=206 && phase<=208) && defense.actions!=(third_return_contact ? std::array<std::uint8_t,4>{0,255,255,255} : std::array<std::uint8_t,4>{6,6,6,255})) ||
           (((phase>=215 && phase<=218) || phase==227 || (phase==228 || phase==229)) && defense.actions!=std::array<std::uint8_t,4>{0,6,6,255}))
            throw std::invalid_argument("Invalid sampled field defense poll");
        for(unsigned i=0;i<4;++i)
            if((defense.actions[i]!=255 && defense.actions[i]>6) || defense.subtypes[i]!=0 ||
               (i>=3 && defense.actions[i]!=255) || (i<cursor && defense.actions[i]==255))
                throw std::invalid_argument("Invalid sampled field defense choices");
    }
    const bool postgoal_sampled_shot=(phase>=211 && phase<=214 && state.current.clock.extra_time==13);
    const bool later_sampled_shot=(phase>=211 && phase<=214 && (state.current.clock.extra_time==9 || postgoal_sampled_shot));
    if((phase>=209 && phase<=214) && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling ||
       !state.kickoff_formation || state.saved_ball_flight_flag!=1 || state.current.field.side!=11 || state.current.field.ball_owner!=19 ||
       state.current.field.ball_x!=(postgoal_sampled_shot ? 92 : later_sampled_shot ? 93 : 95) || state.current.field.ball_y!=(postgoal_sampled_shot ? 141 : 140) || state.current.field.ball_cell!=67 || state.current.field.elapsed!=(phase==212 ? 2 : phase>=213 ? 0 : 1) ||
       state.current.field.selected_defenders || state.current.field.roster[19].type!=23 || state.current.field.random_first || state.current.field.random_second ||
       state.experience->opponent_stage || state.current.clock.remaining!=(postgoal_sampled_shot ? (phase>=212 ? 381 : 382) : later_sampled_shot ? (phase>=212 ? 418 : 419) : (phase>=212 ? 434 : 435)) || state.current.clock.extra_time!=(postgoal_sampled_shot ? 13 : later_sampled_shot ? 9 : 5) ||
       state.control->controlled_player!=(later_sampled_shot ? 4 : 2) || state.control->last_carrier_cell!=(phase==214 ? 128 : phase>=212 ? 0 : 67) || state.score->left_goals!=(postgoal_sampled_shot ? 2 : 1) || state.score->right_goals!=(postgoal_sampled_shot ? 1 : 0) ||
       state.score->kickoff!=4 || state.action->action!=0 || state.action->power!=(phase==209 ? 24 : 22) || state.recovery->attack_subtype!=(phase==209 ? 0 : 128) ||
       state.current.clock.script_flags!=(phase>=211 ? 0 : 128) || state.current.clock.ball_mode!=(phase>=211 ? 0 : 128) ||
       (phase==214 && (state.recovery->ball_variant!=1 || state.control->display_parameter!=0 || state.control->motion_requested!=2))))
        throw std::invalid_argument("Invalid sampled opponent shot choice");
    if((phase>=230 && phase<=235) && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       state.saved_ball_flight_flag!=1 || state.current.field.side!=11 || state.current.field.ball_owner!=12 ||
       !((state.current.field.ball_x==134 && state.current.field.ball_y==160 && state.current.clock.remaining==421 && state.current.clock.extra_time==9 && state.score->left_goals==1 && state.score->right_goals==0) ||
         (state.current.field.ball_x==133 && state.current.field.ball_y==161 && state.current.clock.remaining==384 && state.current.clock.extra_time==13 && state.score->left_goals==2 && state.score->right_goals==1)) ||
       state.current.field.ball_cell!=130 || state.current.field.elapsed!=((phase==234 || phase==235) ? 0 : 1) ||
       state.current.field.roster[12].type!=23 || state.current.field.selected_defenders || state.current.field.random_first || state.current.field.random_second ||
       state.experience->opponent_stage ||
       state.current.clock.script_flags!=(phase>=232 ? 0 : 128) || state.current.clock.ball_mode!=(phase>=232 ? 0 : 128) || state.control->controlled_player!=6 ||
       state.control->last_carrier_cell!=((phase==234 || phase==235) ? 128 : phase==233 ? 0 : 130) || state.control->display_parameter!=((phase==234 || phase==235) ? 0 : 2) || state.control->motion_requested!=((phase==234 || phase==235) ? 0 : 1) ||
       state.score->kickoff!=4 || state.action->action!=1 ||
       state.action->power!=(phase==230 ? 24 : 21) || state.recovery->attack_subtype!=(phase==230 ? 0 : 128)))
        throw std::invalid_argument("Invalid sampled player12 pass choice");
    if((phase==234 || phase==235) && (state.current.field.movement_region!=140 || state.recovery->ball_variant!=1 || state.control->facing!=64))throw std::invalid_argument("Invalid sampled player12 pass handoff");
    if((phase==236 || phase==237) && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation ||
       state.saved_ball_flight_flag!=1 || state.current.field.side!=11 || state.current.field.ball_owner!=19 ||
       !((state.current.field.ball_x==93 && state.current.field.ball_y==140 && state.current.clock.remaining==419 && state.current.clock.extra_time==9 && state.score->left_goals==1 && state.score->right_goals==0) ||
         (state.current.field.ball_x==92 && state.current.field.ball_y==141 && state.current.clock.remaining==382 && state.current.clock.extra_time==13 && state.score->left_goals==2 && state.score->right_goals==1)) ||
       state.current.field.ball_cell!=67 || state.current.field.elapsed!=1 ||
       state.current.field.roster[19].type!=23 || state.current.field.selected_defenders || state.current.field.random_first || state.current.field.random_second ||
       state.experience->opponent_stage ||
       state.current.clock.script_flags!=128 || state.current.clock.ball_mode!=128 || state.control->controlled_player!=4 ||
       state.control->last_carrier_cell!=67 || state.control->display_parameter!=2 || state.control->motion_requested!=1 ||
       state.score->kickoff!=4 || state.action->action!=0 ||
       state.action->power!=(phase==236 ? 21 : 22) || state.recovery->attack_subtype!=(phase==236 ? 0 : 128)))
        throw std::invalid_argument("Invalid sampled player19 shot choice");
    if((phase>=238 && phase<=243) && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling || !state.kickoff_formation || !state.area_defense ||
       state.saved_ball_flight_flag!=1 || state.current.field.side!=0 || state.current.field.ball_owner!=5 || state.current.field.ball_x!=108 || state.current.field.ball_y!=108 || state.current.field.ball_cell!=87 ||
       state.current.field.elapsed!=(phase>=242 ? 0 : phase==241 ? 4 : phase==240 ? 2 : 1) || state.current.field.selected_defenders!=1 || state.scheduling->selected_ids!=std::array<std::uint8_t,5>{17,0,0,0,0} ||
       state.current.field.roster[5].stamina!=15 || state.current.field.roster[17].type!=23 || state.current.field.roster[17].stamina!=100 ||
       state.current.field.random_first || state.current.field.random_second || state.current.clock.remaining!=(phase>=241 ? 408 : phase==240 ? 410 : 411) || state.current.clock.extra_time!=9 ||
       state.score->left_goals!=1 || state.score->right_goals!=1 || state.score->kickoff!=4 || state.action->action!=0 || state.action->power!=22 ||
       state.recovery->opposing_player!=17 || state.recovery->attack_subtype!=128 || state.recovery->ball_variant!=(phase>=239 ? 1 : 0) || state.control->motion_requested!=(phase==239 ? 2 : 0) || state.area_sequence_result!=(phase>=239 ? 0 : 255) ||
       state.area_defense->actions!=std::array<std::uint8_t,4>{1,6,6,255} || state.area_defense->subtypes!=std::array<std::uint8_t,4>{0,0,0,0} || state.area_defense->blink!=13 || state.area_defense->pressed))
        throw std::invalid_argument("Invalid sampled home shot defender choice");
    if((phase>=240 && phase<=243) && (state.current.field.roster[5].type!=23 || state.current.field.motion[field_motion_index(17)].status!=6 || state.control->facing!=64))throw std::invalid_argument("Invalid sampled home contact clock");
    if((phase>=241 && phase<=243) && state.area_defender_cursor!=1)throw std::invalid_argument("Invalid sampled home shot defender cursor");
    if((phase==242 || phase==243) && state.current.field.movement_region!=129)throw std::invalid_argument("Invalid sampled home shot movement capture");
    if(phase==243 && (!state.shot_flight || state.shot_flight->distance!=108 || state.shot_flight->destination!=233 || state.shot_flight->counter!=255 || state.shot_flight->loose_ball))throw std::invalid_argument("Invalid sampled home shot target");
    if((phase>=219 && phase<=226) && (!state.action || !state.recovery || !state.control || !state.experience || !state.score || !state.scheduling ||
       !state.kickoff_formation || state.saved_ball_flight_flag!=(state.current.clock.extra_time==13 ? 0 : 1) || state.current.field.side!=(phase>=225 ? 0 : 11) || state.current.field.ball_owner!=19 ||
       state.current.field.ball_x!=52 || state.current.field.ball_y!=124 || state.current.field.ball_cell!=5 || state.current.field.elapsed ||
       state.current.field.selected_defenders!=1 || state.current.field.roster[0].type!=1 || state.scheduling->selected_ids!=std::array<std::uint8_t,5>{0,0,0,0,0} ||
       state.current.field.random_first || state.current.field.random_second || state.experience->opponent_stage ||
       state.current.clock.remaining!=(state.current.clock.extra_time==13 ? 381 : state.current.clock.extra_time==9 ? 418 : 434) || (state.current.clock.extra_time!=5 && state.current.clock.extra_time!=9 && state.current.clock.extra_time!=13) || state.current.clock.script_flags || state.current.clock.ball_mode ||
       state.score->left_goals!=(state.current.clock.extra_time==13 ? 2 : 1) || state.score->right_goals!=(state.current.clock.extra_time==13 ? (phase>=224 ? 2 : 1) : phase>=224 ? 1 : 0) || state.score->kickoff!=4 ||
       state.action->action!=0 || state.action->power!=22 || state.action->menu_route!=4 ||
       (phase==219 ? state.action->cursor!=0 : (state.action->cursor!=224 && state.action->cursor!=225 && state.action->cursor!=227)) ||
       state.recovery->attack_subtype!=128 || state.recovery->opposing_player!=(phase>=224 ? 1 : 0) || state.recovery->ball_variant!=1 ||
       state.recovery->x_jitter_control!=(state.current.clock.extra_time==13 ? 144 : 142) || state.control->last_carrier_cell!=128 ||
       state.control->motion_requested!=(phase==224 ? 3 : 0) || (phase>=224 && state.control->facing!=64)))
        throw std::invalid_argument("Invalid sampled goalkeeper prompt");
    if(bool(state.keeper_menu_action)!=((phase>=220 && phase<=226) || (phase>=269 && phase<=275) || (phase>=296 && phase<=302)) ||
       (state.keeper_menu_action && (*state.keeper_menu_action>1 || state.action->flash_counter!=13 ||
         (*state.keeper_menu_action==1)!=(state.action->cursor==227))))
        throw std::invalid_argument("Invalid sampled goalkeeper menu choice");
    if((phase>=223 && phase<=226) && ((state.keeper_menu_action!=0 && !(state.current.clock.extra_time==9 && state.keeper_menu_action==1)) || state.current.field.roster[0].stamina!=(state.current.clock.extra_time==13 ? 10 : 0)))
        throw std::invalid_argument("Invalid sampled home goalkeeper comparison");
    if(phase==226 && (state.current.field.keepers[0].timer || state.current.field.keepers[0].penalty || state.current.field.keepers[0].stun))
        throw std::invalid_argument("Invalid sampled opponent goal restart");
    const bool keeper_selected=phase==388 || (phase==389 || phase==390 || phase==391) || score23_keeper || player19_return_keeper || (phase>=219 && phase<=226) || phase==53 || phase==56 || ((phase>=169 && phase<=187) || phase==190 || phase==191 || phase==192 || phase==193) || (phase==59 && !state.current.field.keepers[1].stun);
    if((phase==388 || (phase==389 || phase==390 || phase==391)) && (state.current.field.selected_defenders!=1 || state.scheduling->selected_ids[0]!=11 || state.current.field.keepers[1].stun || ((phase==389 || phase==390 || phase==391) && state.recovery->opposing_player!=11)))
        throw std::invalid_argument("Same-cell goalkeeper arrival requires the selected unstunned keeper");
    const bool keeper_possession=phase==57;
    const bool goal_restart=phase==60;
    const bool kickoff_placed=phase>=61 && phase<=64;
    const bool kickoff_shot=(phase>=415 && phase<=427) || phase==350 || phase==352 || phase==353 || phase==354 || phase==355 || phase==356 || phase==357 || phase==358 || phase==359 || phase==360 || phase==361 || phase==362 || phase==363 || phase==364 || (phase>=365 && phase<=391) || score25_shot || extended_opponent || score23_human_pass || score24_dribble || score25_human_pass || (phase==111 || phase==112) || (phase>=194 && phase<=255) || phase==15 || phase==1 || phase==2 || phase==6 || phase==7 || (phase==8 && state.current.field.selected_defenders) || phase==27 || phase==43 || (phase>=45 && phase<=53) || (phase>=56 && phase<=60) || phase==95 || (phase>=150 && phase<=159) || phase==188 || phase==189 || area_flight || area_action4 || area_action6 || area_state;
    if((kickoff_placed && !state.kickoff_formation) ||
       (!kickoff_placed && !(phase>=9 && phase<=13) && !(phase>=18 && phase<=20) && !ordinary_human_target && !kickoff_shot && state.kickoff_formation))
        throw std::invalid_argument("Kickoff formation phase needs captured keeper positions");
    if (bool(state.kickoff_receiver)!=(phase>=62 && phase<=64) ||
        (state.kickoff_receiver && (*state.kickoff_receiver<=state.current.field.side ||
         *state.kickoff_receiver>state.current.field.side+10)))
        throw std::invalid_argument("Kickoff receiver requires a field player on the selected side");
    if (phase==63) {
        const auto& field=state.current.field;
        const auto& owner=field.motion[field_motion_index(field.ball_owner)];
        if(field.ball_x!=(owner.x>>8) || field.ball_y!=(owner.y>>8) ||
           field.ball_cell!=field_cell_at(field.ball_x,field.ball_y))
            throw std::invalid_argument("Kickoff ball must be at the initial carrier");
    }
    const bool sampled_right_goal_formation=(phase>=61 && phase<=64) && state.score && state.score->left_goals==0 && state.score->right_goals==1 && state.score->kickoff==4 &&
       state.current.field.side==0 && state.current.field.ball_owner==(phase==64 ? 7 : 8) && (phase==61 || (state.kickoff_receiver && *state.kickoff_receiver==7)) && state.current.field.elapsed==1 && state.current.field.selected_defenders==1 &&
       state.scheduling && state.scheduling->selected_ids[0]==0 && state.recovery && state.recovery->opposing_player==0 &&
       state.current.field.random_first==127 && state.current.field.random_second==128 && !state.current.field.keepers[1].penalty;
    const bool sampled_draw_formation=(phase>=61 && phase<=64) && state.score && state.score->kickoff==4 &&
       ((state.score->left_goals==1 && state.score->right_goals==1 && ((state.current.clock.remaining==434 && state.current.clock.extra_time==5) || (state.current.clock.remaining==418 && state.current.clock.extra_time==9))) ||
        (state.score->left_goals==2 && state.score->right_goals==2 && state.current.clock.remaining==381 && state.current.clock.extra_time==13) ||
        (state.score->left_goals==2 && state.score->right_goals==3 && state.current.clock.remaining==360 && state.current.clock.extra_time==13) ||
        (state.score->left_goals==2 && state.score->right_goals==4 && state.current.clock.remaining==348 && state.current.clock.extra_time==13) ||
        (state.score->left_goals==2 && state.score->right_goals==5 && state.current.clock.remaining==335 && state.current.clock.extra_time==13) ||
        (state.score->left_goals==3 && state.score->right_goals==6 && state.current.clock.remaining==293 && state.current.clock.extra_time==13)) &&
       state.current.field.side==0 && state.current.field.ball_owner==(phase==64 ? 5 : 8) && (phase==61 || (state.kickoff_receiver && *state.kickoff_receiver==5)) &&
       state.current.field.elapsed==0 && state.current.field.selected_defenders==1 && state.scheduling && state.scheduling->selected_ids[0]==0 &&
       state.current.field.random_first==0 && state.current.field.random_second==0;
    if(kickoff_placed && !sampled_right_goal_formation && !sampled_draw_formation && (!state.score || (state.score->kickoff!=0 && state.score->kickoff!=4) || !state.scheduling ||
       (phase==64 ? state.current.field.ball_owner!=*state.kickoff_receiver :
                    state.current.field.ball_owner!=state.current.field.side+8) || state.current.field.elapsed ||
       state.current.field.selected_defenders>1 || (state.current.field.selected_defenders && state.scheduling->selected_ids[0]!=11)))
        throw std::invalid_argument("Prepared kickoff formation requires the selected side and initial carrier");
    if(phase==64 && (!state.control || state.control->controlled_player!=1))
        throw std::invalid_argument("Kickoff control handoff needs the initialized controlled player");
    if(phase>=53 && phase<=56) {
        const auto& f=state.current.field;
        const auto keeper=f.side^11u;
        if(!state.recovery || state.recovery->opposing_player!=keeper ||
           (keeper_selected && (f.selected_defenders!=1 || !state.scheduling ||
             state.scheduling->selected_ids[0]!=keeper || f.keepers[f.side==0?1:0].stun)) ||
           (!keeper_selected && (!f.keepers[f.side==0?1:0].stun || !state.action || state.action->action)))
            throw std::invalid_argument("Keeper phase does not match opposing keeper setup/status");
    }
    if(keeper_possession && (state.current.field.side!=11 || state.current.field.ball_owner!=11 ||
       state.current.field.selected_defenders!=1 || !state.scheduling || state.scheduling->selected_ids[0]!=11 ||
       !state.recovery || state.recovery->opposing_player!=11 || state.current.field.elapsed || state.current.field.keepers[1].stun))
        throw std::invalid_argument("Keeper possession requires its own captured keeper and selected prefix");
    if(phase==58 && (state.current.field.side!=0 || state.current.field.selected_defenders || state.current.field.elapsed!=1 ||
       !state.recovery || state.recovery->opposing_player!=11))
        throw std::invalid_argument("Punch recovery requires the prepared empty no-jitter handoff");
    if(phase==59 && (!state.score || !state.keeper_decision || state.keeper_decision->result_code>1 ||
       !state.recovery || state.recovery->opposing_player!=11 ||
       (state.keeper_decision->result_code==1 && (state.recovery->attack_subtype&63u)>=3) ||
       state.current.field.selected_defenders!=(state.current.field.keepers[1].stun?0:1) ||
       (state.current.field.selected_defenders && (!state.scheduling || state.scheduling->selected_ids[0]!=11)) ||
       (state.current.field.keepers[1].stun && (state.keeper_decision->defense_action || state.keeper_decision->defense_subtype || state.keeper_decision->effect))))
        throw std::invalid_argument("Goal celebration needs a scored keeper result and matching normal/stunned prefix");
    const bool sampled_right_goal_restart=goal_restart && state.score && state.score->left_goals==0 && state.score->right_goals==1 && state.score->kickoff==4 &&
       state.current.clock.ball_mode==0 && state.current.field.side==0 && state.current.field.ball_owner==12 && state.current.field.elapsed==1 &&
       state.current.field.ball_x==56 && state.current.field.ball_y==124 && state.current.field.ball_cell==17 &&
       state.current.field.random_first==127 && state.current.field.random_second==128 && state.current.field.selected_defenders==1 &&
       state.scheduling && state.scheduling->selected_ids[0]==0 && state.recovery && state.recovery->opposing_player==0 &&
       !state.current.field.keepers[0].timer && !state.current.field.keepers[0].penalty && !state.current.field.keepers[0].stun;
    if(goal_restart && !sampled_right_goal_restart && (!state.score || state.score->kickoff!=4 || state.current.clock.ball_mode!=0 ||
       state.current.field.side!=11 || state.current.field.ball_owner<1 || state.current.field.ball_owner>10 ||
       state.current.field.elapsed || state.current.field.selected_defenders>1 || !state.scheduling || !state.recovery ||
       state.recovery->opposing_player!=11 || state.current.field.keepers[1].timer || state.current.field.keepers[1].penalty ||
       state.current.field.keepers[1].stun || (state.current.field.selected_defenders && state.scheduling->selected_ids[0]!=11)))
        throw std::invalid_argument("Post-goal kickoff requires switched side, saved score and cleared keeper");
    const bool restart_options = phase >= 40 && phase <= 42;
    if (bool(state.restart_options) != restart_options)
        throw std::invalid_argument("Restart options require exactly one menu context");
    if (state.restart_options) {
        const auto cursor = state.restart_options->cursor;
        if ((state.phase == FieldSessionPhase::restart_options && cursor != 4 && cursor != 68) ||
            (state.phase == FieldSessionPhase::restart_tactics && cursor != 0) ||
            (state.phase == FieldSessionPhase::restart_options_done && cursor != 4) ||
            state.current.clock.ball_mode != 0)
            throw std::invalid_argument("Invalid restart menu boundary");
    }
    const bool sampled_pass_race = phase==112 && bool(state.clearance_race);
    const bool race_pending = (phase>=34 && phase<=37) || area_action6_race || sampled_pass_race;
    const bool clearance = state.phase == FieldSessionPhase::capture_clearance_flight || (race_pending && !sampled_pass_race);
    const bool transitional_clearance = state.phase == FieldSessionPhase::capture_clearance_flight ||
        state.phase == FieldSessionPhase::clearance_race_timeout || state.phase == FieldSessionPhase::clearance_recovery_ready;
    const bool post_possession = player19_goal_possession || phase==301 || phase==302 || phase==309 || player5_dribble_possession || (phase==225 || phase==226) || area_flight || area_action4 || area_action6 || ((state.phase == FieldSessionPhase::keeper_interaction || state.phase == FieldSessionPhase::story_transition) && state.current.field.selected_defenders != 0) || state.phase == FieldSessionPhase::selected_recovered_owner ||
        state.phase == FieldSessionPhase::keeper_area_recovered_owner || clearance || keeper_possession || goal_restart || kickoff_placed;
    if (bool(state.clearance_flight) != clearance)
        throw std::invalid_argument("Clearance flight phase requires exactly one setup context");
    if (transitional_clearance && (state.current.field.side != 11 || state.current.field.ball_owner == 0 || state.current.field.ball_owner >= 11))
        throw std::invalid_argument("Clearance requires the original side0 carrier and active side11");
    if (bool(state.clearance_race) != race_pending)
        throw std::invalid_argument("Race handoff requires exactly one race context");
    if (state.clearance_race) {
        const auto& r = *state.clearance_race;
        if (sampled_pass_race ? (r.remaining!=2 || r.active_candidate!=5 || r.opposing_candidate!=19 ||
             state.current.field.elapsed!=1 || state.current.field.selected_defenders!=0 ||
             field_cell_at(static_cast<std::uint8_t>(state.current.field.motion[field_motion_index(5)].x>>8),
                           static_cast<std::uint8_t>(state.current.field.motion[field_motion_index(5)].y>>8))!=state.current.field.ball_cell ||
             (state.current.field.ball_owner==5 &&
              ((state.current.field.motion[field_motion_index(5)].x>>8)!=state.current.field.ball_x ||
               (state.current.field.motion[field_motion_index(5)].y>>8)!=state.current.field.ball_y)) ||
             state.current.field.motion[field_motion_index(5)].target_cell!=state.current.field.ball_cell ||
             state.current.field.motion[field_motion_index(19)].target_cell!=state.current.field.ball_cell) :
            (r.active_candidate < 12 || r.active_candidate > 21 || r.opposing_candidate < 1 || r.opposing_candidate > 10 ||
            (phase==113 ? state.current.field.elapsed>1 : state.current.field.elapsed != ((state.phase == FieldSessionPhase::clearance_recovered_owner || phase==109 || phase==110) ? 0 : 1)) ||
            (((phase>=35 && phase<=37) || phase==107 || phase==109 || phase==110) && r.remaining != 0) ||
            (state.phase == FieldSessionPhase::clearance_race_winner &&
             state.current.field.ball_owner != r.active_candidate && state.current.field.ball_owner != r.opposing_candidate)))
            throw std::invalid_argument("Unsupported clearance race handoff");
    }
    const bool staged = phase >= 23 && phase <= 46;
    if (staged != bool(state.shot_progress) && !(state.phase == FieldSessionPhase::period_end && state.shot_progress))
        throw std::invalid_argument("Staged shot phase requires exactly one progress context");
    if (state.shot_progress) {
        const auto& p = *state.shot_progress;
        const auto count = state.current.field.selected_defenders;
        if (!state.scheduling || !state.action || !state.recovery || !state.control || !state.experience ||
            (state.current.field.side != 0 && !post_possession) || state.action->action != 0 || count > 5 ||
            p.cursor > count || p.result_code > 4 ||
            (p.defense_action != 0 && p.defense_action != 2) ||
            (p.defense_subtype != 0 && p.defense_subtype != 128))
            throw std::invalid_argument("Unsupported shot progress context");
        if ((state.phase == FieldSessionPhase::selected_ball_recovery && p.result_code != 2) ||
            (state.phase == FieldSessionPhase::selected_defense_animation &&
             (p.result_code != 3 || p.defense_action != 0)) ||
            (state.phase == FieldSessionPhase::selected_restart_entry && p.result_code != 4) ||
            ((state.phase == FieldSessionPhase::selected_recovered_owner ||
              state.phase == FieldSessionPhase::selected_boundary_restart) && p.result_code != 2) ||
            (clearance && (p.result_code != 3 || p.defense_action != 0)) ||
            (phase>=38 && phase<=42 && p.result_code!=4) ||
            (phase >= 28 && phase<=42 && p.cursor == 0))
            throw std::invalid_argument("Selected handoff phase does not match its terminal result");
        if (phase >= 43 && phase <= 46 &&
            (p.result_code >= 2 || p.cursor != count))
            throw std::invalid_argument("Attacking presentation requires a completed nonterminal prefix");
        if (phase >= 44 && (state.control->last_carrier_cell ||
            state.action->menu_route || (p.cursor && state.recovery->opposing_player != p.defender_id)))
            throw std::invalid_argument("Attack clock state needs ordinary selected field context");
        if (p.cursor == 0) {
            if (p.result_code || p.defender_id || p.defense_action || p.defense_subtype)
                throw std::invalid_argument("Unused shot contact data must be zero");
        } else if (p.defender_id != state.scheduling->selected_ids[p.cursor - 1])
            throw std::invalid_argument("Shot cursor must retain the last selected defender");
        if (state.phase == FieldSessionPhase::shot_defender_next && (p.cursor >= count || p.result_code >= 2))
            throw std::invalid_argument("No next selected defender");
        if (state.phase == FieldSessionPhase::shot_contact_added_time && p.cursor == 0)
            throw std::invalid_argument("Contact clock pause requires a completed contact");
        if ((state.phase == FieldSessionPhase::shot_dispatch_clock ||
             state.phase == FieldSessionPhase::shot_dispatch_added_time ||
             state.phase == FieldSessionPhase::shot_dispatch_ready) &&
            p.result_code < 2 && p.cursor != count)
            throw std::invalid_argument("Nonterminal dispatch requires the completed prefix");
        if ((state.phase == FieldSessionPhase::shot_dispatch_clock ||
             state.phase == FieldSessionPhase::shot_dispatch_added_time) && p.result_code >= 2)
            throw std::invalid_argument("Terminal contact skips final dispatch clock");
    }
    if ((state.phase == FieldSessionPhase::added_time) != bool(state.pending))
        throw std::invalid_argument("Added-time session needs exactly one pending terminal contact");
    if (state.pending) validate_contact(state.current.field, *state.pending);
    if (state.experience) validate_experience_state(*state.experience);
    if (state.recovery && state.recovery->opposing_player > 21)
        throw std::invalid_argument("Unsupported recovery opposing player");
    if (static_cast<unsigned>(state.phase) >= 5 && !state.recovery)
        throw std::invalid_argument("Recovery continuation needs captured recovery context");
    const bool selected_phase = state.phase == FieldSessionPhase::field_contact_pending ||
        ((state.phase == FieldSessionPhase::keeper_interaction || state.phase == FieldSessionPhase::story_transition) && state.current.field.selected_defenders != 0) || static_cast<unsigned>(state.phase) >= 18;
    if ((state.phase == FieldSessionPhase::keeper_interaction || state.phase == FieldSessionPhase::story_transition) && state.current.field.selected_defenders && !state.scheduling)
        throw std::invalid_argument("Selected recovery continuation needs saved scheduling context");
    if (static_cast<unsigned>(state.phase) >= 5 && !selected_phase &&
        state.current.field.selected_defenders != 0)
        throw std::invalid_argument("Recovery continuation requires zero selected defenders");
    if (static_cast<unsigned>(state.phase) >= 6 && state.phase != FieldSessionPhase::selected_boundary_restart) {
        if(!keeper_possession) field_motion_index(state.current.field.ball_owner);
        if (!transitional_clearance && !goal_restart && !player19_goal_possession && phase!=301 && phase!=302 && phase!=225 && phase!=226 && (state.current.field.ball_owner < 11 ? 0 : 11) != state.current.field.side)
            throw std::invalid_argument("Recovered owner must belong to the possession side");
        const auto& field = state.current.field;
        // Before kickoff placement, the original can still hold an uninitialized ball position.
        if (phase!=61 && phase!=62 && phase!=177 && phase!=178 && phase!=179 && (field.ball_x < 48 || field.ball_x >= 208 || field.ball_y < 80 || field.ball_y >= 176))
            throw std::invalid_argument("Recovered-owner continuation requires an interior ball");
    }
    if (static_cast<unsigned>(state.phase) >= 9 && (!state.control || !state.experience))
        throw std::invalid_argument("Control-ready state needs captured control/experience context (phase " + std::to_string(phase) + ", control " + std::to_string(bool(state.control)) + ", experience " + std::to_string(bool(state.experience)) + ")");
    if (static_cast<unsigned>(state.phase) >= 9 && !post_possession && state.current.field.side == 11 &&
        (state.control->controlled_player < 1 || state.control->controlled_player > 10))
        throw std::invalid_argument("Opposing control-ready state needs a side0 field defender");
    if (static_cast<unsigned>(state.phase) >= 13 && !state.scheduling)
        throw std::invalid_argument("Control-cycle continuation needs captured scheduling context");
    if ((state.phase == FieldSessionPhase::field_round_pending || state.phase == FieldSessionPhase::control_added_time) &&
        state.current.field.elapsed != 1)
        throw std::invalid_argument("Control-cycle continuation requires elapsed1");
    if (state.scheduling) {
        const auto count = state.current.field.selected_defenders;
        if (count > 5) throw std::invalid_argument("Selected defender prefix exceeds five slots");
        for (unsigned i = 0; i < 5; ++i) {
            if (i < count && !keeper_selected && !keeper_possession && !goal_restart && !kickoff_placed) field_motion_index(state.scheduling->selected_ids[i]);
            else if (i < count) { /* Keeper prefix checked above; not a field-motion slot. */ }
            else if (state.scheduling->selected_ids[i] != 0)
                throw std::invalid_argument("Nonzero unused selected defender slot");
        }
    }
    if (selected_phase) {
        const auto& field = state.current.field;
        if ((state.phase == FieldSessionPhase::field_contact_pending && field.selected_defenders == 0) ||
            field.selected_defenders > (field.side == 0 || post_possession ? 5 : 4))
            throw std::invalid_argument("Field-contact continuation needs a selected opposing prefix");
        for (unsigned i = 0; i < field.selected_defenders; ++i)
            if (area_flight && (state.scheduling->selected_ids[i]<1 || state.scheduling->selected_ids[i]>10))
                throw std::invalid_argument("Area flight selected players must oppose possession");
            else if (!post_possession &&
                (state.scheduling->selected_ids[i] < 11 ? 0 : 11) == field.side)
                throw std::invalid_argument("Selected contact player must oppose possession");
    }
    if (state.action && state.action->action > 9 &&
        (static_cast<unsigned>(state.phase) >= 19 ||
         (state.phase == FieldSessionPhase::human_action_menu && (state.action->cursor & 128u))))
        throw std::invalid_argument("Unsupported captured action ID");
    if (static_cast<unsigned>(state.phase) >= 18 && (!state.action || (state.current.field.side != 0 && !(phase>=415 && phase<=420) && !post_possession && !area_state && !extended_opponent && !(phase>=194 && phase<=243)) || state.action->menu_route >= 6))
        throw std::invalid_argument("Human attacking menu requires captured action context and side0");
    if(bool(state.human_pass_cursor)!=(ordinary_human_target || score23_human_pass || score25_human_pass || (phase==111 || phase==112) || (phase>=244 && phase<=255)) || (!state.human_pass_cursor && state.human_pass_steps) || (state.human_pass_cursor && phase>=244 && phase<=248 &&
       ((state.human_pass_steps==0 && *state.human_pass_cursor!=87) ||
        (state.human_pass_steps==1 && *state.human_pass_cursor!=99 && *state.human_pass_cursor!=75 && *state.human_pass_cursor!=88 && *state.human_pass_cursor!=86) ||
        (state.human_pass_steps==2 && *state.human_pass_cursor!=87 && *state.human_pass_cursor!=111 && *state.human_pass_cursor!=100 && *state.human_pass_cursor!=98 && *state.human_pass_cursor!=63 && *state.human_pass_cursor!=76 && *state.human_pass_cursor!=74 && *state.human_pass_cursor!=89 && *state.human_pass_cursor!=85) || state.human_pass_steps>2 || (phase>=245 && state.human_pass_steps!=0) ||
        !state.score || state.score->left_goals!=2 || state.score->right_goals!=2 || state.score->kickoff!=4 ||
        state.current.clock.remaining!=381 || state.current.clock.extra_time!=13 || state.current.clock.ball_mode!=(phase>=247 ? 0 : 129) || state.saved_ball_flight_flag!=0 ||
        state.current.field.side!=0 || state.current.field.ball_owner!=5 || state.current.field.ball_x!=108 || state.current.field.ball_y!=108 || state.current.field.ball_cell!=87 ||
        state.current.field.roster[5].type!=23 || state.current.field.roster[5].stamina!=(phase>=247 ? 11 : 31) || state.current.field.selected_defenders ||
        state.action->action!=1 || state.action->cursor!=225 || state.action->menu_route!=0 || state.recovery->attack_subtype!=(phase>=247 ? 128 : 0) || (phase>=247 && state.action->power!=21) || (phase>=247 && state.recovery->ball_variant!=(phase==248 ? 1 : 0)))))
        throw std::invalid_argument("Invalid sampled human pass target cursor");
    if(((phase==111 || phase==112) || (phase>=249 && phase<=255)) && (!state.score || !state.recovery || !state.scheduling || !state.action ||
       !state.human_pass_cursor || *state.human_pass_cursor!=87 || state.human_pass_steps || state.score->left_goals!=2 || state.score->right_goals!=2 || state.score->kickoff!=4 ||
       state.current.clock.remaining!=(((phase==111 || phase==112) || phase==255) ? 372 : phase==254 ? 373 : 374) || state.current.clock.extra_time!=13 || state.current.clock.ball_mode!=(((phase==111 || phase==112) || phase>=251) ? 0 : 129) || state.saved_ball_flight_flag!=1 ||
       state.current.field.side!=0 || (state.current.field.ball_owner!=3 && !(sampled_pass_race && state.current.field.ball_owner==5)) || state.current.field.ball_x!=108 || state.current.field.ball_y!=108 || state.current.field.ball_cell!=87 ||
       state.current.field.elapsed!=(sampled_pass_race ? 1 : (phase==111 || phase==112) ? 0 : phase==255 ? 3 : phase==254 ? 2 : 1) || ((phase==111 || phase==112) && state.current.field.movement_region!=129) ||
       state.current.field.roster[3].type!=23 || state.current.field.roster[3].stamina || state.current.field.selected_defenders!=(sampled_pass_race ? 0 : 1) || state.scheduling->selected_ids!=(sampled_pass_race ? std::array<std::uint8_t,5>{} : std::array<std::uint8_t,5>{17,0,0,0,0}) ||
       state.action->action!=1 || state.action->cursor!=225 || state.action->menu_route!=0 || state.action->power!=(((phase==111 || phase==112) || phase>=251) ? 0 : 21) ||
       state.recovery->attack_subtype || state.recovery->opposing_player!=(((phase==111 || phase==112) || phase>=251) ? 17 : 1) ||
       (((phase==111 || phase==112) || phase>=252) && (state.recovery->ball_variant!=1 || state.control->facing!=64 || state.current.field.roster[17].stamina!=100)) ||
       (((phase==111 || phase==112) || phase>=252) && state.current.field.motion[field_motion_index(17)].status!=(((phase==111 || phase==112) || phase>=254) ? 6 : 0)) || state.area_defender_cursor!=(((phase==111 || phase==112) || phase==255) ? 1 : 0)))
        throw std::invalid_argument("Invalid sampled player3 pass target");
    if(((phase==111 || phase==112) || (phase>=252 && phase<=255)) && (state.area_contact->cursor || state.area_contact->defense_action!=1 || state.area_contact->defense_subtype || state.area_contact->effect ||
       state.area_contact->defender_power!=18 || state.area_contact->cost!=60 || state.area_contact->strength || state.area_contact->loss || state.area_contact->weakened ||
       state.area_contact->score!=(((phase==111 || phase==112) || phase>=253) ? 255 : 0) || state.area_contact->result || state.area_sequence_result!=(((phase==111 || phase==112) || phase>=253) ? 0 : 255)))
        throw std::invalid_argument("Invalid sampled player3 defender comparison");
    if (static_cast<unsigned>(state.phase) >= 20 && phase!=427 && !(phase>=415 && phase<=420) && phase!=350 && phase!=352 && phase!=353 && phase!=354 && phase!=355 && phase!=356 && phase!=357 && phase!=358 && phase!=359 && phase!=360 && phase!=361 && phase!=362 && phase!=363 && phase!=364 && !(phase>=365 && phase<=391) && !postgoal_opponent_action && !ordinary_human_target && !score23_human_pass && !score24_dribble && !score25_human_pass && !player14_pass && !state.human_defense_choices && !(phase>=194 && phase<=255) && phase!=111 && phase!=112 && !area_flight && !area_action4 && !area_action6 && phase!=78 && phase!=79 && phase!=80 && phase!=81 && phase!=82 && phase!=83 && phase!=84 && phase!=85 && phase!=86 && phase!=87 && phase!=88 && phase!=89 && phase!=90 && phase!=91 && phase!=92 && phase!=93 && phase!=94 && state.action->action != 0)
        throw std::invalid_argument("Only ordinary shots are supported at attack-prepared phase");
}

PhaseSessionResult resolve_session_terminal(const PhaseSessionState& input,
    PendingTerminalContact contact, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ContactPreludeContent& prelude) {
    validate_phase_session(input);
    if (input.phase != FieldSessionPhase::field && input.phase != FieldSessionPhase::player_control &&
        input.phase != FieldSessionPhase::ball_recovery && input.phase != FieldSessionPhase::attack_prepared)
        throw std::invalid_argument("Session is waiting at a continuation boundary");
    validate_contact(input.current.field, contact);
    auto state = input;
    if (state.recovery) state.recovery->opposing_player = contact.defender_id;
    if (contact.result_code == 2) {
        const auto& defender = state.current.field.motion[field_motion_index(contact.defender_id)];
        state.current.field.ball_x = static_cast<std::uint8_t>(defender.x >> 8);
        state.current.field.ball_y = static_cast<std::uint8_t>(defender.y >> 8);
    }
    auto applied = apply_contact_prelude({state.current, contact.defender_id, contact.defense_action, false}, prelude);
    state.current = applied.state;
    if (applied.clock.continuation == ClockContinuation::period_end) {
        state.phase = FieldSessionPhase::period_end;
        state.pending.reset();
        return {std::move(state), std::move(applied), std::nullopt};
    }
    if (applied.clock.continuation == ClockContinuation::added_time_presentation) {
        state.phase = FieldSessionPhase::added_time;
        state.pending = contact;
        return {std::move(state), std::move(applied), std::nullopt};
    }
    auto result = finish(std::move(state), contact, primary, movement);
    result.prelude = std::move(applied);
    return result;
}

PhaseSessionResult resume_session_terminal(const PhaseSessionState& post_presentation,
    const PlayerProfileMappings& primary, const FieldPhaseContent& movement) {
    validate_phase_session(post_presentation);
    if (post_presentation.phase != FieldSessionPhase::added_time || !post_presentation.pending)
        throw std::invalid_argument("No added-time terminal contact to resume");
    return finish(post_presentation, *post_presentation.pending, primary, movement);
}

PhaseSessionResult resolve_session_rewarded_terminal(const PhaseSessionState& input,
    PendingTerminalContact contact, const PlayerProfileMappings& primary,
    const FieldPhaseContent& movement, const ContactPreludeContent& prelude,
    const ExperienceContent& experience) {
    validate_phase_session(input);
    validate_contact(input.current.field, contact);
    if (!input.experience) throw std::invalid_argument("Rewarded contact needs captured experience context");
    auto state = input;
    std::vector<ExperienceRewardResult> rewards;
    const auto count = contact.result_code == 2 ? 2u : 1u;
    for (unsigned i = 0; i < count; ++i) {
        auto reward = award_player_experience(*state.experience,
            state.current.field.roster[contact.defender_id].type, 2, experience);
        state.experience = reward.state;
        rewards.push_back(std::move(reward));
    }
    auto result = resolve_session_terminal(state, contact, primary, movement, prelude);
    result.rewards = std::move(rewards);
    return result;
}
}

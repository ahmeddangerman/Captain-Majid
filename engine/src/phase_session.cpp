#include "captain/phase_session.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>
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
    if ((state.current.field.side != 0 && state.current.field.side != 11) ||
        state.current.field.ball_owner > 21 || static_cast<unsigned>(state.phase) > 72)
        throw std::invalid_argument("Unsupported phase session state");
    const auto phase = static_cast<unsigned>(state.phase);
    const bool area_setup=phase==65;
    const bool area_menu=(phase>=66 && phase<=72) || ((phase==18 || phase==19) && state.keeper_area);
    const bool area_state=area_setup || area_menu;
    if(((phase>=65 && phase<=72) && !state.keeper_area) || (state.keeper_area && !area_state))
        throw std::invalid_argument("Keeper-area setup needs exactly one captured selection counter");
    if(state.keeper_area && (!state.action || !state.recovery || !state.scheduling ||
       state.keeper_area->selection_counter!=255 || !state.recovery->ball_variant ||
       state.current.field.ball_y<96 || state.current.field.ball_y>=160 ||
       (state.current.field.ball_x>=80 && state.current.field.ball_x<176) ||
       state.action->menu_route!=(((state.current.field.ball_x<80 ? 0 : 11)==state.current.field.side)?2:1)))
        throw std::invalid_argument("Keeper-area setup requires its original area and route");
    if(area_menu && (state.action->menu_route!=2 || (phase>=66 ? state.current.field.side!=11 : state.current.field.side!=0)))
        throw std::invalid_argument("Prepared own-area menu requires its matching attack or defense side");
    if(bool(state.area_defense)!=(phase==67 || (phase>=69 && phase<=72)))
        throw std::invalid_argument("Initial area defense requires its captured choice arrays");
    if(phase==67 || phase==69) {
        if(!state.current.field.selected_defenders || state.current.field.selected_defenders>4 || state.action->cursor || state.area_defense->pressed)
            throw std::invalid_argument("Area defense initialization requires a nonempty prefix and cursor0");
        for(unsigned i=0;i<4;++i)if(state.area_defense->actions[i]!=255 || state.area_defense->subtypes[i])
            throw std::invalid_argument("Initial defense actions must be unselected with unused subtypes zero");
    }
    if(phase>=70 && phase<=72) {
        const auto count=state.current.field.selected_defenders;
        if(!count || count>4 || state.action->cursor>count || (phase==71 && state.action->cursor==count))
            throw std::invalid_argument("Defensive poll requires a valid selected cursor");
        for(unsigned i=0;i<4;++i) {
            const auto action=state.area_defense->actions[i];
            if((action!=255 && action>6) || state.area_defense->subtypes[i] || (i>=count && action!=255))
                throw std::invalid_argument("Defensive poll supports ordinary field actions in the active prefix");
        }
        if(((phase==70 || phase==72) && state.area_defense->pressed) || (phase==71 && !(state.area_defense->pressed&128u)))
            throw std::invalid_argument("Pending defensive preparation requires the captured confirm input");
        if(phase==72 && state.action->cursor!=count)
            throw std::invalid_argument("Completed defense requires the full selected prefix");
        if(state.action->cursor==count)for(unsigned i=0;i<count;++i)
            if(state.area_defense->actions[i]==255)
                throw std::invalid_argument("Full defensive prefix requires selected actions");
        if(phase==71 && (state.area_defense->actions[state.action->cursor]==255 ||
           state.recovery->opposing_player!=state.scheduling->selected_ids[state.action->cursor]))
            throw std::invalid_argument("Pending defensive preparation requires a confirmed selected field player");
    }
    if(phase==68 && state.current.field.selected_defenders)
        throw std::invalid_argument("Empty area defense completion requires no selected players");
    if (bool(state.shot_flight) != ((phase >= 46 && phase <= 56) || phase==59))
        throw std::invalid_argument("Flight setup requires exactly one shot context");
    if (state.shot_flight && (state.shot_flight->destination >= 240 || state.shot_flight->counter != 255))
        throw std::invalid_argument("Unsupported initial shot flight context");
    const bool trajectory=(phase>=47 && phase<=56) || phase==59;
    if(bool(state.ball_trajectory)!=trajectory)
        throw std::invalid_argument("Numeric flight requires exactly one trajectory context");
    if(state.ball_trajectory) {
        const auto& t=*state.ball_trajectory;const auto& f=state.current.field;
        if(!state.recovery || !state.scheduling || t.flags || t.direction!=state.recovery->x_jitter_control ||
            (t.x>>8)!=f.ball_x || (t.y>>8)!=f.ball_y ||
            field_cell_at(f.ball_x,f.ball_y)!=f.ball_cell || f.elapsed ||
            ((phase==50 || phase==51 || phase==52 || phase==54 || phase==55) && f.selected_defenders) ||
            (phase==47 && (!t.vx && !t.vy)) ||
            (phase==47 && f.ball_cell==state.shot_flight->destination) ||
            (phase==48 && f.ball_cell==state.shot_flight->destination) ||
            (phase>=49 && f.ball_cell!=state.shot_flight->destination) ||
            (phase==50 && (t.vx || t.vy)))
            throw std::invalid_argument("Unsupported numeric flight boundary");
    }
    if((phase==51 && !state.shot_flight->loose_ball) || (((phase>=52 && phase<=56) || phase==59) && state.shot_flight->loose_ball))
        throw std::invalid_argument("Flight return phase does not match saved loose/keeper route");
    if(bool(state.keeper_decision)!=(phase==55 || phase==56 || phase==59) ||
       (state.keeper_decision && (state.keeper_decision->cursor ||
        state.keeper_decision->result_code>(phase==55?1:3) || state.keeper_decision->defense_action>3 ||
        (phase==55 && (state.keeper_decision->defense_action || state.keeper_decision->defense_subtype || state.keeper_decision->effect)))))
        throw std::invalid_argument("Stunned keeper decision needs one valid result/cursor context");
    const bool keeper_selected=phase==53 || phase==56 || (phase==59 && !state.current.field.keepers[1].stun);
    const bool keeper_possession=phase==57;
    const bool goal_restart=phase==60;
    const bool kickoff_placed=phase>=61 && phase<=64;
    const bool kickoff_shot=phase==1 || phase==2 || phase==6 || phase==7 || phase==27 || phase==43 || (phase>=45 && phase<=49) || phase==52 || phase==53 || (phase>=56 && phase<=60) || area_state;
    if((kickoff_placed && !state.kickoff_formation) ||
       (!kickoff_placed && !(phase>=9 && phase<=13) && !(phase>=18 && phase<=20) && !kickoff_shot && state.kickoff_formation))
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
    if(kickoff_placed && (!state.score || state.score->kickoff!=4 || !state.scheduling ||
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
    if(goal_restart && (!state.score || state.score->kickoff!=4 || state.current.clock.ball_mode!=0 ||
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
    const bool race_pending = phase>=34 && phase<=37;
    const bool clearance = state.phase == FieldSessionPhase::capture_clearance_flight || race_pending;
    const bool transitional_clearance = state.phase == FieldSessionPhase::capture_clearance_flight ||
        state.phase == FieldSessionPhase::clearance_race_timeout || state.phase == FieldSessionPhase::clearance_recovery_ready;
    const bool post_possession = state.phase == FieldSessionPhase::selected_recovered_owner || clearance || keeper_possession || goal_restart || kickoff_placed;
    if (bool(state.clearance_flight) != clearance)
        throw std::invalid_argument("Clearance flight phase requires exactly one setup context");
    if (transitional_clearance && (state.current.field.side != 11 || state.current.field.ball_owner == 0 || state.current.field.ball_owner >= 11))
        throw std::invalid_argument("Clearance requires the original side0 carrier and active side11");
    if (bool(state.clearance_race) != race_pending)
        throw std::invalid_argument("Race handoff requires exactly one race context");
    if (state.clearance_race) {
        const auto& r = *state.clearance_race;
        if (r.active_candidate < 12 || r.active_candidate > 21 || r.opposing_candidate < 1 || r.opposing_candidate > 10 ||
            state.current.field.elapsed != (state.phase == FieldSessionPhase::clearance_recovered_owner ? 0 : 1) ||
            (phase>=35 && r.remaining != 0) ||
            (state.phase == FieldSessionPhase::clearance_race_winner &&
             state.current.field.ball_owner != r.active_candidate && state.current.field.ball_owner != r.opposing_candidate))
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
    const bool selected_phase = state.phase == FieldSessionPhase::field_contact_pending || static_cast<unsigned>(state.phase) >= 18;
    if (static_cast<unsigned>(state.phase) >= 5 && !selected_phase &&
        state.current.field.selected_defenders != 0)
        throw std::invalid_argument("Recovery continuation requires zero selected defenders");
    if (static_cast<unsigned>(state.phase) >= 6 && state.phase != FieldSessionPhase::selected_boundary_restart) {
        if(!keeper_possession) field_motion_index(state.current.field.ball_owner);
        if (!transitional_clearance && !goal_restart && (state.current.field.ball_owner < 11 ? 0 : 11) != state.current.field.side)
            throw std::invalid_argument("Recovered owner must belong to the possession side");
        const auto& field = state.current.field;
        if (field.ball_x < 48 || field.ball_x >= 208 || field.ball_y < 80 || field.ball_y >= 176)
            throw std::invalid_argument("Recovered-owner continuation requires an interior ball");
    }
    if (static_cast<unsigned>(state.phase) >= 9 && (!state.control || !state.experience))
        throw std::invalid_argument("Control-ready state needs captured control/experience context");
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
            if (!post_possession &&
                (state.scheduling->selected_ids[i] < 11 ? 0 : 11) == field.side)
                throw std::invalid_argument("Selected contact player must oppose possession");
    }
    if (state.action && state.action->action > 9 &&
        (static_cast<unsigned>(state.phase) >= 19 ||
         (state.phase == FieldSessionPhase::human_action_menu && (state.action->cursor & 128u))))
        throw std::invalid_argument("Unsupported captured action ID");
    if (static_cast<unsigned>(state.phase) >= 18 && (!state.action || (state.current.field.side != 0 && !post_possession && !area_state) || state.action->menu_route >= 6))
        throw std::invalid_argument("Human attacking menu requires captured action context and side0");
    if (static_cast<unsigned>(state.phase) >= 20 && state.action->action != 0)
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

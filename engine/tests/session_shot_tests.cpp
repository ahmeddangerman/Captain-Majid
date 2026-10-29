#include "captain/session_shot.hpp"
#include "captain/phase_session_io.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
namespace {
void require(bool ok, const char* message) { if (!ok) { std::cerr << message << '\n'; std::exit(1); } }
template<class F> void rejects(F call) {
    bool rejected = false; try { call(); } catch (const std::invalid_argument&) { rejected = true; }
    require(rejected, "Invalid session shot accepted");
}
std::string bytes(const captain::PhaseSessionState& s) {
    std::ostringstream out; captain::write_phase_session(out, s); return out.str();
}
}
int main() {
    using namespace captain;
    PhaseSessionState s{};
    s.phase = FieldSessionPhase::attack_prepared;
    s.experience = ExperienceState{}; s.recovery = RecoveryContext{};
    s.control = ControlContext{}; s.scheduling = SchedulingContext{}; s.action = ActionContext{};
    s.current.field.side = 0; s.current.field.ball_owner = 3;
    s.current.field.ball_x = 120; s.current.field.ball_y = 100;
    s.current.clock.remaining = 452; s.action->power = 1;
    s.recovery->attack_subtype = 128;
    for (auto& p : s.current.field.roster) p = {1,0,200};
    s.current.field.roster[3].stamina = 123;
    for (auto& m : s.current.field.motion) { m.x = 120u << 8; m.y = 100u << 8; }
    FieldPlayerProfiles profiles{}; profiles.skills.resize(1);
    FieldDefenseContent defense{};
    for (auto& row : defense.skill_selectors) row.fill(4);
    for (auto& moves : defense.moves) moves = {{7,0,30,0}};
    defense.strength_curve.fill(240);
    FieldComparisonContent comparison{}; comparison.variation.resize(1);
    comparison.eligibility.resize(1); comparison.variation[0] = {100,100};
    FieldPhaseContent movement{}; movement.profiles.speeds.resize(1); movement.geometry.direction_thresholds.fill(65535); ContactPreludeContent prelude{}; ExperienceContent experience{};
    std::vector<FieldDefenseChoice> choices;
    auto run = [&] { return resolve_session_shot_defenders(s, choices, false, profiles,
        defense, comparison, movement, prelude, experience); };
    auto r = run();
    require(static_cast<unsigned>(r.state.phase) == 21 && r.defenses.result_code == 0 &&
        r.state.action->power == 1 && r.state.current.field.roster[3].stamina == 123,
        "Empty selection advances without preparing/debiting attacker");
    auto encoded = bytes(r.state); require(encoded.size() == 361 && encoded[10] == 'O', "Session24 layout");
    std::istringstream saved(encoded); require(bytes(load_phase_session(saved)) == encoded, "Session14 roundtrip");
    auto legacy = encoded; legacy[10] = '7';
    rejects([&] { std::istringstream old(legacy); load_phase_session(old); });
    auto prior = s; s = r.state; rejects(run); s = prior;
    s.current.field.selected_defenders = 2; s.scheduling->selected_ids = {12,13,0,0,0};
    FieldDefenseChoice c{}; c.player_id = 12; c.action = 0;
    c.comparison.random_first = 100; c.comparison.random_second = 100;
    choices = {c,c}; choices[1].player_id = 13;
    comparison.thresholds = {255,220,0,0};
    auto before = bytes(s); r = run();
    require(r.defenses.result_code == 2 && r.defenses.contacts.size() == 1 && r.terminal &&
        r.state.phase == FieldSessionPhase::ball_recovery && r.state.current.field.selected_defenders == 0 &&
        r.state.current.field.roster[3].stamina == 123 && r.state.current.field.roster[13].stamina == 200 &&
        r.terminal->rewards.size() == 2, "Stopped block joins rewarded recovery and skips later defender");
    require(bytes(s) == before, "Caller state remains unchanged");
    comparison.thresholds = {255,220,180,0}; r = run();
    require(r.defenses.result_code == 3 && r.state.phase == FieldSessionPhase::player_control &&
        r.state.current.field.side == 11 && r.state.current.field.ball_owner == 12 &&
        r.terminal->rewards.size() == 1, "Capture joins possession handoff");
    s.current.clock.remaining = 1; prelude.field_elapsed.fill(2);
    choices[0].comparison.random_first = 128;
    r = run();
    require(r.state.phase == FieldSessionPhase::added_time && r.state.current.clock.extra_time == 12 &&
        r.state.current.field.random_first == 128 && r.state.current.field.random_second == 100,
        "Reached contact random bytes drive clock and saved recovery state");
    choices[0].comparison.random_first = 100;
    // Pause before possession changes; resume cannot replay comparison or rewards.
    s.current.clock.remaining = 1; s.current.clock.extra_time = 1;
    prelude.field_elapsed.fill(2); r = run();
    require(r.state.phase == FieldSessionPhase::added_time && r.state.pending &&
        r.state.current.field.ball_owner == 3, "Clock pause retains pending capture");
    auto resumed = resume_session_terminal(r.state, profiles, movement);
    require(resumed.state.current.field.ball_owner == 12 && resumed.rewards.empty(), "Resume hands off once");
    s.current.clock.remaining = 452; prelude.field_elapsed.fill(0);
    comparison.thresholds = {0,0,0,0}; choices[0].comparison.random_first = 0;
    s.current.field.motion[0].status = 9; s.current.field.keepers[1].penalty = 7;
    s.current.field.keepers[0].stun = 3; s.current.field.keepers[1].stun = 4;
    r = run();
    require(r.defenses.result_code == 4 && static_cast<unsigned>(r.state.phase) == 22 && r.restart &&
        r.state.current.field.motion[0].status == 0 && r.state.current.field.keepers[1].penalty == 0 &&
        r.state.current.field.keepers[0].stun == 0 && r.state.current.field.keepers[1].stun == 0,
        "Interruption persists clears and pauses for restart");
    encoded = bytes(r.state); std::istringstream restart_saved(encoded);
    require(bytes(load_phase_session(restart_saved)) == encoded, "Restart snapshot roundtrip");
    choices[0].comparison.random_first = 100; r = run();
    require(r.defenses.contacts.size() == 2 && static_cast<unsigned>(r.state.phase) == 21 &&
        r.state.current.field.roster[3].stamina == 123, "Passed defenses reach flight boundary");
    choices[1].player_id = 14; rejects(run); choices[1].player_id = 13;
    choices.pop_back(); rejects(run); choices.push_back(c); choices[1].player_id = 13;
    choices[0].action = 1; rejects(run); choices[0].action = 0;
    choices[0].subtype = 1; rejects(run); choices[0].subtype = 0;
    s.scheduling->selected_ids[1] = 12; choices[1].player_id = 12; rejects(run);
    std::cout << "Session shot defender checks passed\n";
}

#include "research_protocol.hpp"
#include "captain/defender_contact.hpp"
#include "captain/shot_power.hpp"
#include "captain/goalkeeper.hpp"
#include "captain/energy.hpp"
#include "captain/keeper_state.hpp"
#include "captain/reference_random.hpp"
#include "captain/shot_outcome.hpp"
#include "captain/defender_outcome.hpp"
#include "captain/ball_recovery.hpp"
#include "captain/defender_selection.hpp"
#include "captain/field_defender_preparation.hpp"
#include "captain/player_movement.hpp"
#include "captain/match_clock.hpp"

#include <array>
#include <charconv>
#include <sstream>
#include <stdexcept>

namespace {
bool number(const std::string& token, std::uint32_t& value) {
    if (token.empty()) return false;
    // from_chars rejects signs/overflow; require consumption of the entire token.
    const auto parsed = std::from_chars(token.data(), token.data() + token.size(), value);
    return parsed.ec == std::errc{} && parsed.ptr == token.data() + token.size();
}

bool next_number(std::istringstream& stream, std::uint32_t& value,
                 std::uint32_t maximum = 255) {
    std::string token;
    return (stream >> token) && number(token, value) && value <= maximum;
}

template<std::size_t N>
bool read_fields(std::istringstream& stream, std::array<std::uint32_t, N>& values,
                 std::size_t word_fields = 0) {
    std::string token;
    for (std::size_t i = 0; i < N; ++i) {
        if (!(stream >> token) || !number(token, values[i]) ||
            values[i] > (i < word_fields ? 65535u : 255u)) return false;
    }
    return !(stream >> token);
}

template<std::size_t N>
std::string format_fields(const std::array<unsigned, N>& values) {
    std::string result;
    for (std::size_t i = 0; i < N; ++i) {
        if (i) result += ' ';
        result += std::to_string(values[i]);
    }
    return result;
}

bool defender_input(std::istringstream& stream, captain::DefenderContactInput& input) {
    std::array<std::uint32_t, 15> values{};
    if (!read_fields(stream, values, 2) || values[6] > 1 || values[7] > 1) return false;
    input = {static_cast<std::uint16_t>(values[0]), static_cast<std::uint16_t>(values[1]),
             static_cast<std::uint8_t>(values[2]), static_cast<std::uint8_t>(values[3]),
             static_cast<std::uint8_t>(values[4]), static_cast<std::uint8_t>(values[5]),
             values[6] != 0, values[7] != 0,
             static_cast<std::uint8_t>(values[8]), static_cast<std::uint8_t>(values[9]),
             static_cast<std::uint8_t>(values[10]),
             {static_cast<std::uint8_t>(values[11]), static_cast<std::uint8_t>(values[12]),
              static_cast<std::uint8_t>(values[13]), static_cast<std::uint8_t>(values[14])}};
    return true;
}

bool defender_output(std::istringstream& stream, std::string& output) {
    captain::DefenderContactInput input{};
    if (!defender_input(stream, input)) return false;
    try {
        const auto result = captain::calculate_defender_contact(input);
        const std::array<unsigned, 8> values{result.defender_power, result.strength, result.loss,
            result.score, result.result_code, result.incoming_power, result.subtype,
            result.special ? 1u : 0u};
        output = format_fields(values);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    }
}

bool research_output(const std::string& command, std::istringstream& stream, std::string& output) {
    if (command == "clockstep") {
        std::array<std::uint32_t, 7> values{};
        if (!read_fields(stream, values, 1)) return false;
        const auto result = captain::advance_match_clock({static_cast<std::uint16_t>(values[0]),
            static_cast<std::uint8_t>(values[1]), static_cast<std::uint8_t>(values[2]),
            static_cast<std::uint8_t>(values[3]), static_cast<std::uint8_t>(values[4])},
            static_cast<std::uint8_t>(values[5]), static_cast<std::uint8_t>(values[6]));
        output = format_fields(std::array<unsigned, 8>{result.state.remaining, result.state.extra_time,
            result.state.flags, result.state.script_flags, result.state.ball_mode,
            static_cast<unsigned>(result.continuation), result.request_clock_display ? 1u : 0u,
            result.request_warning_cue ? 1u : 0u});
        return true;
    }
    if (command == "playermove") {
        std::array<std::uint32_t, 7> prefix{};
        for (std::size_t i = 0; i < prefix.size(); ++i)
            if (!next_number(stream, prefix[i], i < 3 ? 65535 : 255)) return false;
        captain::MovementGeometryContent content{};
        std::uint32_t value{};
        for (auto& threshold : content.direction_thresholds) {
            if (!next_number(stream, value, 65535)) return false;
            threshold = static_cast<std::uint16_t>(value);
        }
        for (auto& sign : content.axis_signs) {
            if (!next_number(stream, value, 2)) return false;
            sign = static_cast<std::uint8_t>(value);
        }
        std::string extra;
        if (stream >> extra) return false;
        try {
            const auto result = captain::advance_field_player({
                {static_cast<std::uint16_t>(prefix[0]), static_cast<std::uint16_t>(prefix[1]),
                 static_cast<std::uint8_t>(prefix[3]), static_cast<std::uint8_t>(prefix[4])},
                static_cast<std::uint16_t>(prefix[2]), static_cast<std::uint8_t>(prefix[5]),
                static_cast<std::uint8_t>(prefix[6])}, content);
            output = format_fields(std::array<unsigned, 6>{result.player.x, result.player.y,
                result.player.target_cell, result.player.status, result.direction, result.iterations});
            return true;
        } catch (const std::invalid_argument&) { return false; }
    }
    if (command == "fieldaxis") {
        std::array<std::uint32_t, 3> values{};
        if (!read_fields(stream, values, 2) || values[2] > 1) return false;
        output = std::to_string(captain::advance_field_coordinate(
            static_cast<std::uint16_t>(values[0]), static_cast<std::uint16_t>(values[1]),
            static_cast<captain::FieldAxis>(values[2])));
        return true;
    }
    if (command == "defenderprep") {
        std::array<std::uint32_t, 4> prefix{};
        for (auto& value : prefix)
            if (!next_number(stream, value)) return false;
        captain::FieldDefenderPreparationInput input{};
        input.defense_action = static_cast<std::uint8_t>(prefix[0]);
        input.situation = static_cast<std::uint8_t>(prefix[1]);
        input.subtype = static_cast<std::uint8_t>(prefix[2]);
        input.level = static_cast<std::uint8_t>(prefix[3]);
        std::uint32_t value{};
        for (auto& skill : input.base_skills) {
            if (!next_number(stream, value)) return false;
            skill = static_cast<std::uint8_t>(value);
        }
        captain::FieldDefenseContent content{};
        for (auto& selectors : content.skill_selectors)
            for (auto& selector : selectors) {
                if (!next_number(stream, value)) return false;
                selector = static_cast<std::uint8_t>(value);
            }
        for (auto& moves : content.moves) {
            std::uint32_t count{};
            if (!next_number(stream, count, 64)) return false;
            for (std::uint32_t i = 0; i < count; ++i) {
                std::array<std::uint32_t, 4> fields{};
                for (auto& field : fields)
                    if (!next_number(stream, field)) return false;
                moves.push_back({static_cast<std::uint8_t>(fields[0]),
                    static_cast<std::uint8_t>(fields[1]), static_cast<std::uint8_t>(fields[2]),
                    static_cast<std::uint8_t>(fields[3])});
            }
        }
        for (auto& strength : content.strength_curve) {
            if (!next_number(stream, value)) return false;
            strength = static_cast<std::uint8_t>(value);
        }
        std::string extra;
        if (stream >> extra) return false;
        try {
            const auto r = captain::prepare_field_defender(input, content);
            output = format_fields(std::array<unsigned, 5>{r.skill_selector, r.skill_index,
                r.base_strength, r.move_effect, r.stamina_cost});
            return true;
        } catch (const std::invalid_argument&) { return false; }
    }
    if (command == "defenderselection") {
        std::array<std::uint32_t, 4> prefix{};
        for (auto& value : prefix)
            if (!next_number(stream, value)) return false;
        if (prefix[0] != 0 && prefix[0] != 11) return false;
        captain::DefenderSelectionInput input{};
        input.possession_side = static_cast<std::uint8_t>(prefix[0]);
        input.ball_x = static_cast<std::uint8_t>(prefix[1]);
        input.ball_y = static_cast<std::uint8_t>(prefix[2]);
        input.radius = static_cast<std::uint8_t>(prefix[3]);
        for (auto& player : input.players) {
            std::array<std::uint32_t, 3> fields{};
            if (!next_number(stream, fields[0]) || !next_number(stream, fields[1]) ||
                !next_number(stream, fields[2], 1)) return false;
            player = {static_cast<std::uint8_t>(fields[0]),
                static_cast<std::uint8_t>(fields[1]), fields[2] != 0};
        }
        std::string extra;
        if (stream >> extra) return false;
        try {
            const auto selected = captain::select_field_defenders(input);
            output = std::to_string(selected.size());
            for (const auto player_id : selected) {
                output += ' ';
                output += std::to_string(player_id);
            }
            return true;
        } catch (const std::invalid_argument&) { return false; }
    }
    if (command == "ballrecovery") {
        std::array<std::uint32_t, 8> prefix{};
        for (std::size_t i = 0; i < prefix.size(); ++i) {
            const auto maximum = i == 6 ? 1u : (i == 7 ? 10u : 255u);
            if (!next_number(stream, prefix[i], maximum)) return false;
        }
        captain::BallRecoveryInput input{};
        input.ball_x = static_cast<std::uint8_t>(prefix[0]);
        input.ball_y = static_cast<std::uint8_t>(prefix[1]);
        input.ball_owner = static_cast<std::uint8_t>(prefix[2]);
        input.opposing_player = static_cast<std::uint8_t>(prefix[3]);
        input.random_e2 = static_cast<std::uint8_t>(prefix[4]);
        input.random_e3 = static_cast<std::uint8_t>(prefix[5]);
        input.ball_update_active = prefix[6] != 0;
        for (auto& player : input.players) {
            std::array<std::uint32_t, 3> fields{};
            if (!next_number(stream, fields[0]) || !next_number(stream, fields[1]) ||
                !next_number(stream, fields[2], 1)) return false;
            player = {static_cast<std::uint8_t>(fields[0]),
                static_cast<std::uint8_t>(fields[1]), fields[2] != 0};
        }
        for (std::uint32_t i = 0; i < prefix[7]; ++i) {
            std::array<std::uint32_t, 2> fields{};
            if (!next_number(stream, fields[0], 21) || !next_number(stream, fields[1])) return false;
            input.selected_defenders.push_back({static_cast<std::uint8_t>(fields[0]),
                static_cast<std::uint8_t>(fields[1])});
        }
        std::string extra;
        if (stream >> extra) return false;
        try {
            const auto result = captain::recover_ball(input);
            output = format_fields(std::array<unsigned, 6>{result.ball_x, result.ball_y,
                result.ball_owner, result.possession_side, result.attack_subtype,
                result.direct_defender_capture ? 1u : 0u});
            return true;
        } catch (const std::invalid_argument&) { return false; }
    }
    if (command == "defenderoutcome") {
        std::array<std::uint32_t, 5> v{};
        if (!read_fields(stream, v) || v[0] > 3) return false;
        try {
            const auto r = captain::resolve_defender_outcome({
                static_cast<std::uint8_t>(v[0]), static_cast<std::uint8_t>(v[1]),
                static_cast<std::uint8_t>(v[2]), static_cast<std::uint8_t>(v[3]),
                static_cast<std::uint8_t>(v[4])});
            output = format_fields(std::array<unsigned, 3>{static_cast<unsigned>(r.route),
                r.possession_side, r.ball_owner});
            return true;
        } catch (const std::invalid_argument&) { return false; }
    }
    if (command == "outcome") {
        std::array<std::uint32_t, 29> v{};
        if (!read_fields(stream, v)) return false;
        try {
            const auto r = captain::resolve_shot_outcome({
                static_cast<std::uint8_t>(v[0]), static_cast<std::uint8_t>(v[1]),
                static_cast<std::uint8_t>(v[2]), static_cast<std::uint8_t>(v[3]),
                static_cast<std::uint8_t>(v[4]), static_cast<std::uint8_t>(v[5]),
                static_cast<std::uint8_t>(v[6]), static_cast<std::uint8_t>(v[7]),
                static_cast<std::uint8_t>(v[8]), static_cast<std::uint8_t>(v[9]),
                static_cast<std::uint8_t>(v[10]), static_cast<std::uint8_t>(v[11]),
                static_cast<std::uint8_t>(v[12]), static_cast<std::uint8_t>(v[13]),
                static_cast<std::uint8_t>(v[14]), static_cast<std::uint8_t>(v[15]),
                static_cast<std::uint8_t>(v[16]), static_cast<std::uint8_t>(v[17]),
                static_cast<std::uint8_t>(v[18]), static_cast<std::uint8_t>(v[19]),
                static_cast<std::uint8_t>(v[20]),
                {static_cast<std::uint8_t>(v[21]), static_cast<std::uint8_t>(v[22]),
                 static_cast<std::uint8_t>(v[23]), static_cast<std::uint8_t>(v[24]),
                 static_cast<std::uint8_t>(v[25]), static_cast<std::uint8_t>(v[26]),
                 static_cast<std::uint8_t>(v[27]), static_cast<std::uint8_t>(v[28])}});
            output = format_fields(std::array<unsigned, 15>{static_cast<unsigned>(r.event),
                r.result_code, r.possession_side, r.ball_owner, r.left_goals, r.right_goals,
                r.keeper_timer, r.keeper_penalty, r.keeper_penalty_duration, r.keeper_stun,
                r.ball_x, r.ball_y, r.defenders, r.ball_active, r.kickoff});
            return true;
        } catch (const std::invalid_argument&) { return false; }
    }
    if (command == "energy") {
        std::array<std::uint32_t, 8> v{};
        if (!read_fields(stream, v, 2) || v[6] > 1 || v[7] > 1) return false;
        const auto r = captain::calculate_energy({static_cast<std::uint16_t>(v[0]),
            static_cast<std::uint16_t>(v[1]), static_cast<std::uint8_t>(v[2]),
            static_cast<std::uint8_t>(v[3]), static_cast<std::uint8_t>(v[4]),
            static_cast<std::uint8_t>(v[5]), v[6] != 0, v[7] != 0});
        output = format_fields(std::array<unsigned, 4>{r.stamina, r.cost,
            r.weakened ? 1u : 0u, r.carry ? 1u : 0u});
        return true;
    }
    if (command == "story") {
        std::array<std::uint32_t, 6> v{};
        if (!read_fields(stream, v)) return false;
        const auto r = captain::apply_keeper_story_override({static_cast<std::uint8_t>(v[0]),
            static_cast<std::uint8_t>(v[1]), static_cast<std::uint8_t>(v[2]),
            static_cast<std::uint8_t>(v[3]), static_cast<std::uint8_t>(v[4]),
            static_cast<std::uint8_t>(v[5])});
        output = format_fields(std::array<unsigned, 3>{r.result_code, r.overridden ? 1u : 0u, r.defender_index});
        return true;
    }
    if (command == "keeperstate") {
        std::array<std::uint32_t, 7> v{};
        if (!read_fields(stream, v) || v[5] > 1 || v[6] > 1) return false;
        const auto r = captain::update_keeper_state({static_cast<std::uint8_t>(v[0]),
            static_cast<std::uint8_t>(v[1]), static_cast<std::uint8_t>(v[2]),
            static_cast<std::uint8_t>(v[3]), static_cast<std::uint8_t>(v[4]), v[5] != 0, v[6] != 0});
        output = format_fields(std::array<unsigned, 3>{r.timer, r.penalty, r.penalty_duration});
        return true;
    }
    if (command == "random") {
        std::array<std::uint32_t, 7> v{};
        if (!read_fields(stream, v) || v[6] > 1) return false;
        const auto r = captain::advance_reference_random({static_cast<std::uint8_t>(v[0]),
            static_cast<std::uint8_t>(v[1]), static_cast<std::uint8_t>(v[2])},
            {static_cast<std::uint8_t>(v[3]), static_cast<std::uint8_t>(v[4]),
             static_cast<std::uint8_t>(v[5]), v[6] != 0});
        output = format_fields(std::array<unsigned, 4>{r.state.index, r.state.first,
            r.state.second, r.carry ? 1u : 0u});
        return true;
    }
    if (command == "defender") return defender_output(stream, output);
    if (command == "skill") {
        std::array<std::uint32_t, 3> v{};
        if (!read_fields(stream, v)) return false;
        output = std::to_string(captain::standard_skill_index(v[0], v[1], v[2]));
        return true;
    }
    if (command == "action") {
        std::array<std::uint32_t, 6> v{};
        if (!read_fields(stream, v, 1) || v[3] > 1 || v[5] > 1) return false;
        const auto result = captain::calculate_action_power({static_cast<std::uint16_t>(v[0]),
            static_cast<std::uint8_t>(v[1]), static_cast<std::uint8_t>(v[2]), v[3] != 0,
            static_cast<std::uint8_t>(v[4]), v[5] != 0});
        output = format_fields(std::array<unsigned, 4>{result.base, result.multiplier, result.power, result.subtype});
        return true;
    }
    if (command == "keeper") {
        std::array<std::uint32_t, 18> v{};
        if (!read_fields(stream, v, 2) || v[6] > 1) return false;
        try {
            const auto result = captain::calculate_goalkeeper({static_cast<std::uint16_t>(v[0]),
                {static_cast<std::uint16_t>(v[1]), static_cast<std::uint8_t>(v[2]),
                 static_cast<std::uint8_t>(v[3]), static_cast<std::uint8_t>(v[4]),
                 static_cast<std::uint8_t>(v[5]), v[6] != 0, false,
                 static_cast<std::uint8_t>(v[8]), static_cast<std::uint8_t>(v[9]),
                 static_cast<std::uint8_t>(v[10])},
                static_cast<std::uint8_t>(v[7]), static_cast<std::uint8_t>(v[11]),
                static_cast<std::uint8_t>(v[12]), static_cast<std::uint8_t>(v[13]),
                {static_cast<std::uint8_t>(v[14]), static_cast<std::uint8_t>(v[15]),
                 static_cast<std::uint8_t>(v[16]), static_cast<std::uint8_t>(v[17])}});
            output = format_fields(std::array<unsigned, 8>{result.base, result.strength, result.ratio,
                result.state_adjustment, result.score, result.result_code, result.subtype,
                result.special ? 1u : 0u});
            return true;
        } catch (const std::invalid_argument&) { return false; }
    }
    return false;
}

}

namespace captain::research {
std::optional<std::string> evaluate(std::string_view command, std::string_view fields) {
    std::istringstream input{std::string(fields)};
    std::string output;
    if (!research_output(std::string(command), input, output)) return std::nullopt;
    return output;
}
}

// Read-only diagnostic for comparing a saved pass target with original RAM.
#include "captain/clearance_race.hpp"
#include "captain/phase_session_io.hpp"
#include "captain/shot_content.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        if (argc != 4) throw std::invalid_argument("Expected pack, session and private race table");
        std::ifstream pack(argv[1], std::ios::binary), session(argv[2], std::ios::binary), table(argv[3]);
        if (!pack || !session || !table) throw std::invalid_argument("Cannot open input");
        const auto content = captain::load_shot_content(pack);
        const auto state = captain::load_phase_session(session);
        if (!content.movement || state.phase != captain::FieldSessionPhase::ordinary_pass_flight_target ||
            !state.sampled_field_pass || !state.recovery || state.sampled_field_pass->recipient != 255 ||
            state.sampled_field_pass->destination != state.current.field.ball_cell)
            throw std::invalid_argument("Expected an empty same-cell pass target");
        captain::ClearanceRaceContent race{};
        for (auto& pair : race.speeds) for (auto& value : pair) {
            unsigned item;
            if (!(table >> item) || item > 65535) throw std::invalid_argument("Invalid race table");
            value = static_cast<std::uint16_t>(item);
        }
        table >> std::ws;
        if (table.peek() != std::char_traits<char>::eof()) throw std::invalid_argument("Trailing race table data");
        const auto result = captain::resolve_field_loose_ball_race(state.current.field, state.recovery->opposing_player,
            state.sampled_field_pass->distance, content.field_players, *content.movement, race);
        std::cout << result.rounds << ' ' << unsigned(result.remaining) << ' ' << unsigned(result.active_candidate)
                  << ' ' << unsigned(result.opposing_candidate) << ' ' << (result.winner ? int(*result.winner) : -1) << '\n';
        for (const auto& player : result.field.motion)
            std::cout << player.x << ' ' << player.y << ' ' << unsigned(player.target_cell) << ' ' << unsigned(player.status) << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}

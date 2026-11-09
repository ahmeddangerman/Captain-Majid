#include "captain/control_travel.hpp"
#include "captain/shot_content.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
    using captain::advance_control_travel;
    try {
        if (argc == 3 && std::string(argv[1]) == "--profile-batch") {
            std::ifstream stream(argv[2], std::ios::binary);
            const auto content = captain::load_shot_content(stream);
            if (!content.movement || !content.control_movement) return 2;
            unsigned id, type, low, high, level, side, held, facing, travel;
            while (std::cin >> id >> type >> low >> high >> level >> side >> held >> facing >> travel) {
                if (id > 21 || type > 255 || low > 255 || high > 255 || level > 255 ||
                    side > 255 || held > 255 || facing > 255 || travel > 65535) return 2;
                captain::PhaseSessionState state{};
                state.phase = captain::FieldSessionPhase::control_ready;
                state.experience = captain::ExperienceState{};
                state.recovery = captain::RecoveryContext{};
                state.control = captain::ControlContext{};
                state.current.field.side = static_cast<std::uint8_t>(side);
                state.current.field.ball_owner = static_cast<std::uint8_t>(id);
                state.current.field.ball_x = 124; state.current.field.ball_y = 124;
                state.current.field.roster[id].type = static_cast<std::uint8_t>(type);
                state.current.field.roster[id].stamina = static_cast<std::uint16_t>(low | (high << 8));
                state.current.field.roster[id].level = static_cast<std::uint8_t>(level);
                state.control->controlled_player = 1;
                state.control->facing = static_cast<std::uint8_t>(facing);
                state.control->travel_accumulator = static_cast<std::uint16_t>(travel);
                const auto result = captain::advance_profiled_session_control_travel(
                    state, static_cast<std::uint8_t>(held), content.field_players,
                    *content.movement, *content.control_movement);
                std::cout << (result.loaded_speed ? int(*result.loaded_speed) : -1) << ' '
                          << result.state.control->travel_accumulator << '\n';
            }
            return std::cin.eof() ? 0 : 2;
        }
        if (argc == 2 && std::string(argv[1]) == "--batch") {
            unsigned travel, speed, side, held, facing;
            while (std::cin >> travel >> speed >> side >> held >> facing) {
                if (travel > 65535 || speed > 65535 || side > 255 || held > 255 || facing > 255) return 2;
                std::cout << advance_control_travel(travel, speed, side, held, facing) << '\n';
            }
            return std::cin.eof() ? 0 : 2;
        }
        if (advance_control_travel(100, 928, 0, 0, 64) != 100) return 1;
        if (advance_control_travel(100, 928, 0, 1, 64) != 332) return 1;
        if (advance_control_travel(100, 928, 0, 2, 0) != 65403) return 1;
        if (advance_control_travel(65500, 928, 11, 0, 64) != 196) return 1;
        bool rejected = false;
        try { advance_control_travel(0, 0, 1, 0, 0); }
        catch (const std::invalid_argument&) { rejected = true; }
        return rejected ? 0 : 1;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

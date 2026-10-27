#include "captain/field_targets.hpp"
#include "captain/defender_selection.hpp"
#include <stdexcept>

namespace captain {
std::uint8_t field_ball_region(std::uint8_t x, std::uint8_t y) {
    if (field_cell_at(x, y) == 255)
        throw std::invalid_argument("Field target update needs an in-field ball");
    return static_cast<std::uint8_t>((x - 48u) / 32u + 5u * ((y - 80u) / 32u));
}

FieldTargetResult select_field_target(const FieldTargetInput& input,
                                     const FieldTargetContent& content) {
    if ((input.possession_side != 0 && input.possession_side != 11) ||
        input.player_id == 0 || input.player_id == 11 || input.player_id > 21)
        throw std::invalid_argument("Field target needs a field player and side0/11");
    const auto region = field_ball_region(input.ball_x, input.ball_y);
    const unsigned team = input.player_id < 11 ? 0 : 1;
    const auto local = unsigned(input.player_id) - (team ? 11u : 0u);
    const auto template_region = team ? content.region_mirror[region] : region;
    if (template_region >= 15) throw std::invalid_argument("Unsupported mirrored ball region");
    const auto& shape = input.teams[team];
    if (shape.formation >= 4) throw std::invalid_argument("Unsupported field formation");
    const bool on_ball = (team ? 11 : 0) == input.possession_side;
    const auto index = unsigned(template_region) * 20u + (local - 1u) * 2u + (input.random_first & 1u);
    std::uint8_t target;
    if (on_ball) target = content.on_ball[shape.formation][index];
    else {
        if (shape.defense_tactic >= 3) throw std::invalid_argument("Unsupported defensive tactic");
        target = content.off_ball[unsigned(shape.formation) * 3u + shape.defense_tactic][index];
    }
    if (team && target != 240) {
        const auto target_x = static_cast<std::uint8_t>((target / 12u) * 8u + 52u);
        const auto target_y = static_cast<std::uint8_t>((target % 12u) * 8u + 84u);
        target = field_cell_at(static_cast<std::uint8_t>(0u - unsigned(target_x)),
                               static_cast<std::uint8_t>(0u - unsigned(target_y)));
    }
    const auto random = static_cast<std::uint8_t>(input.random_first >> 1);
    bool overridden = false;
    if (input.decision_threshold >= random) {
        if (!on_ball) {
            target = 240;
            overridden = true;
        } else if (local < 5) {
            target = field_cell_at(input.possession_side == 0 ? 200 : 56,
                                   static_cast<std::uint8_t>(input.player.y >> 8));
            overridden = true;
        }
    }
    return {target, random, region, template_region, overridden};
}
}

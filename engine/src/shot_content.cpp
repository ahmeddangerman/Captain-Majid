#include "captain/shot_content.hpp"
#include <stdexcept>
#include <string>

namespace captain {
namespace {
class Reader {
public:
    explicit Reader(std::istream& source) : input(source) {}
    std::uint8_t byte() {
        const auto value = input.get();
        if (value == std::char_traits<char>::eof())
            throw std::invalid_argument("Truncated shot content pack");
        return static_cast<std::uint8_t>(value);
    }
    template<std::size_t N> void bytes(std::array<std::uint8_t, N>& values) {
        for (auto& value : values) value = byte();
    }
    std::uint16_t word() {
        const auto low = byte();
        return static_cast<std::uint16_t>(unsigned(low) | (unsigned(byte()) << 8));
    }
    std::size_t count(std::size_t minimum, std::size_t maximum) {
        const auto low = byte();
        const auto value = unsigned(low) | (unsigned(byte()) << 8);
        if (value < minimum || value > maximum)
            throw std::invalid_argument("Shot content section count outside supported bounds");
        return value;
    }
    std::vector<std::uint8_t> vector(std::size_t minimum, std::size_t maximum) {
        std::vector<std::uint8_t> values(count(minimum, maximum));
        for (auto& value : values) value = byte();
        return values;
    }
    std::vector<FieldDefenseMove> moves() {
        const auto size = byte();
        if (size > 64) throw std::invalid_argument("Too many moves in shot content");
        std::vector<FieldDefenseMove> values;
        values.reserve(size);
        for (unsigned i = 0; i < size; ++i)
            values.push_back({byte(), byte(), byte(), byte()});
        return values;
    }
    void end() {
        if (input.peek() != std::char_traits<char>::eof())
            throw std::invalid_argument("Trailing data in shot content pack");
        if (input.bad()) throw std::invalid_argument("Unable to read shot content pack");
    }
private:
    std::istream& input;
};
void field_content(Reader& reader, FieldDefenseContent& content) {
    for (auto& selectors : content.skill_selectors) reader.bytes(selectors);
    for (auto& moves : content.moves) moves = reader.moves();
    reader.bytes(content.strength_curve);
}
}

ShotContent load_shot_content(std::istream& input) {
    Reader reader(input);
    const std::string magic = "CMJSHOT";
    for (const auto expected : magic)
        if (reader.byte() != static_cast<std::uint8_t>(expected))
            throw std::invalid_argument("Unsupported shot content pack format/version");
    const auto version = reader.byte();
    if ((version < '1' || version > '9') && version!='A' && version!='B' && version!='C' && version!='D')
        throw std::invalid_argument("Unsupported shot content pack format/version");
    ShotContent content{};
    reader.bytes(content.source_payload_sha256);
    PlayerProfileMappings mappings{};
    reader.bytes(mappings.ordinary);
    mappings.alternate = reader.vector(0, 221);
    reader.bytes(mappings.generic_slot_offsets);
    mappings.team_types = reader.vector(0, 256);
    static_cast<PlayerProfileMappings&>(content.field_players) = mappings;
    static_cast<PlayerProfileMappings&>(content.goalkeepers) = mappings;
    content.field_players.skills.resize(reader.count(1, 256));
    for (auto& skills : content.field_players.skills) reader.bytes(skills);
    content.goalkeepers.skills.resize(reader.count(1, 256));
    for (auto& skills : content.goalkeepers.skills) reader.bytes(skills);
    field_content(reader, content.attack);
    field_content(reader, content.defense);
    reader.bytes(content.keeper.skill_selectors);
    for (auto& moves : content.keeper.moves) moves = reader.moves();
    reader.bytes(content.keeper.strength_curve);
    reader.bytes(content.field_comparison.attack_groups);
    reader.bytes(content.field_comparison.defense_groups);
    content.field_comparison.eligibility = reader.vector(1, 256);
    content.field_comparison.variation.resize(reader.count(1, 128));
    for (auto& pair : content.field_comparison.variation) reader.bytes(pair);
    reader.bytes(content.field_comparison.thresholds);
    content.keeper_range = reader.byte();
    content.keeper_offset = reader.byte();
    reader.bytes(content.keeper_thresholds);
    reader.bytes(content.punch_intervals);
    if (version >= '2') {
        content.approach = ShotApproachContent{};
        reader.bytes(content.approach->outer_distance);
        reader.bytes(content.approach->inner_distance);
    }
    if (version >= '3') {
        content.flight = FlightContactContent{};
        reader.bytes(content.flight->defense_actions);
        reader.bytes(content.flight->eligibility);
        for (auto& pair : content.flight->variation) reader.bytes(pair);
        for (auto& row : content.flight->thresholds) reader.bytes(row);
    }
    if (version >= '4') {
        content.movement = FieldPhaseContent{};
        auto& movement = *content.movement;
        for (auto& threshold : movement.geometry.direction_thresholds) threshold = reader.word();
        reader.bytes(movement.geometry.axis_signs);
        const auto profile_entry = [&reader]() {
            return MovementProfileEntry{reader.byte(), reader.byte(), reader.byte()};
        };
        for (auto& entry : movement.profiles.ordinary) entry = profile_entry();
        movement.profiles.alternate.resize(reader.count(0, 221));
        for (auto& entry : movement.profiles.alternate) entry = profile_entry();
        movement.profiles.speeds.resize(reader.count(1, 256));
        for (auto& row : movement.profiles.speeds)
            for (auto& speed : row) speed = reader.word();
        reader.bytes(movement.targets.region_mirror);
        for (auto& formation : movement.targets.on_ball) reader.bytes(formation);
        for (auto& formation : movement.targets.off_ball) reader.bytes(formation);
    }
    if (version >= '5') {
        content.contact = ContactPreludeContent{};
        reader.bytes(content.contact->field_elapsed);
        reader.bytes(content.contact->keeper_elapsed);
        reader.bytes(content.contact->defender_status);
    }
    if (version >= '6') {
        content.experience = ExperienceContent{};
        reader.bytes(content.experience->campaign_offsets);
        reader.bytes(content.experience->player_mappings);
        reader.bytes(content.experience->reward_factors);
        for (auto& base : content.experience->stage_bases) base = reader.word();
    }
    if (version >= '7') {
        content.control_movement = ControlMovementContent{};
        for (auto& speed : content.control_movement->carrier_speeds) speed = reader.word();
    }
    if (version >= '8') {
        content.world_step = WorldStepContent{};
        for (auto& speed : content.world_step->chase_speeds) speed = reader.word();
        for (auto& maximum : content.world_step->stamina_curve) maximum = reader.word();
    }
    if (version >= '9') {
        content.action_menu = ActionMenuContent{};
        for (auto& row : content.action_menu->directions) reader.bytes(row);
        reader.bytes(content.action_menu->icons);
        reader.bytes(content.action_menu->titles);
        reader.bytes(content.action_menu->backgrounds);
    }
    if(version>='A') {
        content.area_defense_menu=AreaDefenseMenuContent{};
        reader.bytes(content.area_defense_menu->titles);
        for(auto& row:content.area_defense_menu->directions) {
            reader.bytes(row);
            for(auto action:row)if(action>6 && action!=255)
                throw std::invalid_argument("Unsupported defensive menu action");
        }
    }
    if(version>='B') {
        content.area_defense_menu->field_icons=std::array<std::uint8_t,7>{};
        reader.bytes(*content.area_defense_menu->field_icons);
    }
    if(version>='C') {
        content.area_defense_menu->special_options=AreaDefenseSpecialOptions{};
        auto& rules=content.area_defense_menu->special_options->field_action2;
        rules.resize(reader.count(1,256));
        for(auto& rule:rules) {
            const auto present=reader.byte();
            if(present>1)throw std::invalid_argument("Invalid defensive special-option presence");
            if(present) {
                const auto subtype=reader.byte(),conditional=reader.byte(),required=reader.byte();
                if(subtype>127 || conditional>1 || (!conditional && required))
                    throw std::invalid_argument("Invalid defensive special-option rule");
                rule=AreaDefenseSpecialOption{subtype,conditional ? std::optional<std::uint8_t>{required} : std::nullopt};
            }
        }
    }
    if(version>='D') {
        content.area_defense_menu->special_cursor=std::array<std::uint8_t,2>{};
        reader.bytes(*content.area_defense_menu->special_cursor);
    }
    reader.end();
    return content;
}
}

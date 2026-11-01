#include "captain/field_comparison_content.hpp"
#include <stdexcept>
namespace captain {
FieldComparisonSelection select_area_comparison(std::uint8_t attack,
    std::uint8_t defense, const AreaComparisonContent& content,
    const std::vector<std::uint8_t>& eligibility) {
    if ((attack!=1 && attack!=4 && attack!=6) || (defense!=2 && defense!=4))
        throw std::invalid_argument("Unsupported area comparison actions");
    const unsigned index=unsigned(content.attack_groups[attack])*4+content.defense_groups[defense];
    if(index>=content.variation.size() || index>=eligibility.size() || !content.variation[index][0])
        throw std::invalid_argument("Missing area comparison data");
    const auto pair=content.variation[index];
    classify_score(0,content.thresholds);
    return {static_cast<std::uint8_t>(index),pair[0],pair[1],bool(eligibility[index]&128u),content.thresholds};
}
FieldComparisonSelection select_field_comparison(std::uint8_t attack_action,
    std::uint8_t defense_action, std::uint8_t defender_id,
    const FieldComparisonContent& content) {
    if (attack_action >= 5 || defense_action >= 5 || defender_id > 21)
        throw std::invalid_argument("Unsupported field comparison actions or slot");
    const auto group = static_cast<std::uint8_t>(unsigned(content.attack_groups[attack_action]) * 4);
    const auto defense = defender_id == 0 || defender_id == 11
        ? 2u : unsigned(content.defense_groups[defense_action]);
    const auto index = static_cast<std::uint8_t>(group + defense);
    const auto pair = index & 127u;
    if (index >= content.eligibility.size() || pair >= content.variation.size())
        throw std::invalid_argument("Missing field comparison content");
    const auto& variation = content.variation[pair];
    return {index, variation[0], variation[1], bool(content.eligibility[index] & 128u),
            content.thresholds};
}
DefenderContactInput configure_field_comparison(DefenderContactInput input,
    std::uint8_t attack_action, std::uint8_t defense_action,
    std::uint8_t defender_id, const FieldComparisonContent& content) {
    const auto selected = select_field_comparison(attack_action,defense_action,defender_id,content);
    input.range = selected.range;
    input.offset = selected.offset;
    input.eligible = selected.eligible;
    input.thresholds = selected.thresholds;
    input.defender_slot = defender_id;
    return input;
}
}

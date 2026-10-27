#pragma once
#include "captain/defender_contact.hpp"
#include <vector>
namespace captain {
struct FieldComparisonContent {
    std::array<std::uint8_t,5> attack_groups, defense_groups;
    std::vector<std::uint8_t> eligibility;
    std::vector<std::array<std::uint8_t,2>> variation;
    std::array<std::uint8_t,4> thresholds;
};
struct FieldComparisonSelection {
    std::uint8_t table_index, range, offset;
    bool eligible;
    std::array<std::uint8_t,4> thresholds;
};
FieldComparisonSelection select_field_comparison(std::uint8_t attack_action,
    std::uint8_t defense_action, std::uint8_t defender_id,
    const FieldComparisonContent& content);
DefenderContactInput configure_field_comparison(DefenderContactInput input,
    std::uint8_t attack_action, std::uint8_t defense_action,
    std::uint8_t defender_id, const FieldComparisonContent& content);
}

#pragma once
#include <array>
#include <cstdint>
#include <vector>
namespace captain {
struct AiAreaChoiceContent {
 std::vector<std::uint8_t> profile_rows;
 std::array<std::array<std::uint8_t,2>,16> cells{};
 std::vector<std::array<std::uint8_t,48>> rows;
};
struct AiAreaChoice { std::uint8_t category,row,candidate; };
struct AiAreaAction { AiAreaChoice choice; std::uint8_t action,subtype; };
// The candidate2 recipient-selection branch is not supported by this operation.
AiAreaAction choose_ai_area_action(std::uint8_t type,std::uint8_t flags,
 std::uint8_t alternate,std::uint8_t x,std::uint8_t y,std::uint8_t random,
 const AiAreaChoiceContent& content);
// Direct opponent types only; generic team resolution and final adjustments pending.
AiAreaChoice choose_ai_area_candidate(std::uint8_t type,std::uint8_t flags,
 std::uint8_t alternate,std::uint8_t x,std::uint8_t y,std::uint8_t random,
 const AiAreaChoiceContent& content);
// Eight packed candidates for an already selected AI category. This does not
// select the category or apply the later action/subtype adjustments.
std::uint8_t choose_ai_action_candidate(const std::array<std::uint8_t,4>& candidates,
                                       std::uint8_t random);
}

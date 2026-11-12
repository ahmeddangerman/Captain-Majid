#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include <optional>
#include "captain/field_player_profile.hpp"
namespace captain {
struct AiKeeperContent {
 std::uint8_t team_id=0;
 std::array<std::array<std::uint8_t,3>,221> profile_rows{};
 std::array<std::array<std::uint8_t,12>,11> rows{};
 std::array<std::array<std::uint8_t,2>,118> special_options{};
};
struct AiKeeperChoice { std::uint8_t action=0,subtype=0; };
AiKeeperChoice choose_opponent_keeper_action(const FieldPlayerRecord& player,const PlayerProfileMappings& profiles,
 std::uint8_t menu_route,std::uint8_t random,const AiKeeperContent& content);
struct AiRecipientResult {
 std::uint8_t action,random_first;
 std::optional<std::uint8_t> recipient{},cell{};
};
// Input entry is private profile-selected recipient data; stops before subtype lookup.
AiRecipientResult choose_ai_area_recipient(std::uint8_t entry,std::uint8_t carrier,
 std::uint8_t ball_x,std::uint8_t random_first,std::uint8_t random_second,
 const std::array<std::array<std::uint8_t,2>,10>& teammates);
struct AiAreaChoiceContent {
 std::vector<std::uint8_t> profile_rows;
 std::array<std::array<std::uint8_t,2>,16> cells{};
 std::vector<std::array<std::uint8_t,48>> rows;
 std::vector<std::uint8_t> recipient_profile_rows;
 std::vector<std::array<std::uint8_t,8>> recipient_rows;
};
AiRecipientResult choose_profiled_ai_area_recipient(std::uint8_t type,std::uint8_t flags,
 std::uint8_t alternate,std::uint8_t carrier,std::uint8_t ball_x,
 std::uint8_t random_first,std::uint8_t random_second,
 const std::array<std::array<std::uint8_t,2>,10>& teammates,const AiAreaChoiceContent& content);
struct AiAreaChoice { std::uint8_t category,row,candidate; };
struct AiFieldChoiceContent {
 std::array<std::uint8_t,256> profile_rows{};
 std::array<std::uint8_t,256> recipient_profile_rows{};
 std::array<std::array<std::uint8_t,2>,128> low_x_categories{};
 std::vector<std::array<std::uint8_t,48>> rows;
 std::optional<std::array<std::uint8_t,118>> empty_pass_options{};
 std::optional<std::array<std::uint8_t,118>> empty_action2_options{};
 std::optional<std::array<std::uint8_t,118>> no_variant_zero_shot_options{};
};
// Field-contact route0; unsupported profiles and absent cell categories reject.
AiAreaChoice choose_ai_field_candidate(std::uint8_t type,std::uint8_t flags,
 std::uint8_t alternate,std::uint8_t x,std::uint8_t y,std::uint8_t random,
 const AiFieldChoiceContent& content);
AiAreaChoice choose_generic_ai_field_candidate(std::uint8_t id,std::uint8_t flags,
 std::uint8_t alternate,std::uint8_t x,std::uint8_t y,std::uint8_t random,
 const PlayerProfileMappings& profiles,const AiFieldChoiceContent& content);
struct AiAreaAction { AiAreaChoice choice; std::uint8_t action,subtype; };
struct AiAreaPlan { std::uint8_t action,subtype,random_first; std::optional<std::uint8_t> recipient{},cell{}; };
// Field route candidate2: fallback or a pass with a proven empty special-option list.
AiAreaPlan plan_ai_field_pass_candidate(std::uint8_t type,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t carrier,std::uint8_t ball_x,std::uint8_t ball_y,std::uint8_t first,std::uint8_t second,
 const std::array<std::array<std::uint8_t,2>,10>& teammates,
 const AiFieldChoiceContent& field,const AiAreaChoiceContent& shared);
AiAreaPlan plan_generic_ai_field_pass(std::uint8_t carrier,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t ball_x,std::uint8_t ball_y,std::uint8_t first,std::uint8_t second,
 const std::array<std::array<std::uint8_t,2>,10>& teammates,const PlayerProfileMappings& profiles,
 const AiFieldChoiceContent& field,const AiAreaChoiceContent& shared);
AiAreaPlan plan_ai_area_action(std::uint8_t type,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t carrier,std::uint8_t ball_x,std::uint8_t ball_y,std::uint8_t first,
 std::uint8_t second,std::uint8_t variant,const std::array<std::array<std::uint8_t,2>,10>& teammates,
 const AiAreaChoiceContent& content);
// The candidate2 recipient-selection branch is not supported by this operation.
AiAreaAction choose_ai_area_action(std::uint8_t type,std::uint8_t flags,
 std::uint8_t alternate,std::uint8_t x,std::uint8_t y,std::uint8_t random,
 const AiAreaChoiceContent& content);
struct AiGoalAreaChoiceContent {
 std::array<std::uint8_t,121> profile_rows{};
 std::vector<std::array<std::uint8_t,48>> rows;
};
AiAreaChoice choose_generic_goal_area_candidate(std::uint8_t carrier,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t x,std::uint8_t y,std::uint8_t random,const PlayerProfileMappings& profiles,
 const AiGoalAreaChoiceContent& goal,const AiAreaChoiceContent& area);
AiAreaPlan plan_generic_goal_area_pass(std::uint8_t carrier,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t x,std::uint8_t y,std::uint8_t first,std::uint8_t second,std::uint8_t variant,
 const std::array<std::array<std::uint8_t,2>,10>& teammates,const PlayerProfileMappings& profiles,
 const AiGoalAreaChoiceContent& goal,const AiAreaChoiceContent& area);
struct AiPassDefenseSpecial { std::uint8_t subtype=0; std::optional<std::uint8_t> teammate_type{}; };
struct AiPassDefenseContent {
 std::vector<std::uint8_t> profile_rows;
 std::array<std::array<std::uint8_t,2>,128> cells{};
 std::vector<std::array<std::uint8_t,48>> rows;
 std::vector<std::optional<AiPassDefenseSpecial>> tackle_options;
 std::vector<std::optional<AiPassDefenseSpecial>> block_options;
};
// Pass-defense candidate only; generic types and final special-move adjustments are not represented.
AiAreaChoice choose_ai_pass_defense_candidate(std::uint8_t type,std::uint8_t flags,
 std::uint8_t alternate,std::uint8_t x,std::uint8_t y,std::uint8_t random,
 const AiPassDefenseContent& content);
// Returns supported ordinary defense choices only when the original special-option lookup is empty or bypassed.
std::uint8_t choose_ordinary_ai_pass_defense(std::uint8_t type,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t x,std::uint8_t y,std::uint8_t random,std::uint8_t ball_variant,
 const std::array<std::uint8_t,10>& home_types,const AiPassDefenseContent& content);
// Generic opposing records use team mapping for candidate selection, but raw type0 for special options.
std::uint8_t choose_generic_ai_pass_defense(std::uint8_t id,std::uint8_t flags,std::uint8_t alternate,
 std::uint8_t x,std::uint8_t y,std::uint8_t random,std::uint8_t ball_variant,
 const std::array<std::uint8_t,10>& home_types,const PlayerProfileMappings& profiles,const AiPassDefenseContent& content);
// Direct opponent types only; generic team resolution and final adjustments pending.
AiAreaChoice choose_ai_area_candidate(std::uint8_t type,std::uint8_t flags,
 std::uint8_t alternate,std::uint8_t x,std::uint8_t y,std::uint8_t random,
 const AiAreaChoiceContent& content);
// Eight packed candidates for an already selected AI category. This does not
// select the category or apply the later action/subtype adjustments.
std::uint8_t choose_ai_action_candidate(const std::array<std::uint8_t,4>& candidates,
                                       std::uint8_t random);
}

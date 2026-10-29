#pragma once
#include <array>
#include <cstdint>
namespace captain {
struct KickoffPosition { std::uint16_t x=0,y=0; };
struct KickoffFormationContent {
 std::array<std::uint8_t,44> on_ball{},off_ball{};
};
// DC07 setup with explicit formation tables and raw coordinates for all22 slots.
std::array<KickoffPosition,22> place_kickoff_formation(
 const std::array<KickoffPosition,22>& positions,std::uint8_t side,
 std::uint8_t left_formation,std::uint8_t right_formation,const KickoffFormationContent& content);
}

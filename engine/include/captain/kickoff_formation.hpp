#pragma once
#include <array>
#include <cstdint>
namespace captain {
struct KickoffPosition { std::uint16_t x=0,y=0; };
struct KickoffFormationContent {
 std::array<std::uint8_t,44> on_ball{},off_ball{};
};
struct RestartFormationContent {
 std::array<std::array<std::uint8_t,22>,8> cells{};
 std::array<std::uint8_t,8> taker_cells{};
};
std::array<KickoffPosition,22> place_restart_formation(
 const std::array<KickoffPosition,22>& positions,std::uint8_t side,std::uint8_t taker,
 std::uint8_t selection,const RestartFormationContent& content);
// DC07 setup with explicit formation tables and raw coordinates for all22 slots.
std::array<KickoffPosition,22> place_kickoff_formation(
 const std::array<KickoffPosition,22>& positions,std::uint8_t side,
 std::uint8_t left_formation,std::uint8_t right_formation,const KickoffFormationContent& content);
}

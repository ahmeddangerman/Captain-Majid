#pragma once
#include <array>
#include <cstdint>
namespace captain {
// Original DB4B team selection, using explicit opponent-team and random bytes.
std::uint8_t select_kickoff_side(std::uint8_t opponent_team,std::uint8_t random);
// Explicit table from the selected game version; no RNG advancement is inferred.
std::uint8_t select_kickoff_receiver(std::uint8_t side,std::uint8_t random,
 const std::array<std::uint8_t,5>& candidates);
}

#pragma once
#include <array>
#include <cstdint>
#include <iosfwd>

namespace captain {
// Replaceable distance values for the 20 valid on-field distance bands.
struct PassDistanceContent { std::array<std::uint8_t,20> values{}; };

// Computes the observed $DF8B distance for an in-field ball and target cell.
std::uint8_t calculate_pass_distance(std::uint8_t ball_x,std::uint8_t ball_y,
                                     std::uint8_t target_cell,const PassDistanceContent& content);
// Strict private CMJPDST1 format: eight-byte magic followed by 20 table bytes.
PassDistanceContent read_pass_distance_content(std::istream& input);
}

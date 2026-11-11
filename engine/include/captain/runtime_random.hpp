#pragma once
#include "captain/reference_random.hpp"
#include "captain/field_phase.hpp"
#include <array>
#include <cstdint>
#include <optional>
namespace captain {
// Values are usable only where known is true. No sound or RAM is synthesized.
struct RuntimeRandomMemory {
 std::array<std::uint8_t,2048> bytes{};
 std::array<bool,2048> known{};
};
// Refresh known field bytes, invalidating unrepresented bytes in $0300..$03ff.
void project_field_random_memory(const FieldPhaseState&,RuntimeRandomMemory&);
std::optional<ReferenceRandomResult> advance_runtime_random(
 ReferenceRandomState state,const RuntimeRandomMemory& memory,bool incoming_carry);
}

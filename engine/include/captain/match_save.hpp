#pragma once
#include "captain/match_runtime.hpp"
#include <array>
#include <cstdint>
#include <istream>
#include <ostream>
#include <string>

namespace captain {
// Identity covers the exact input bytes and role/presence of each content file.
using MatchContentIdentity = std::array<std::uint8_t,32>;
MatchContentIdentity match_content_identity(const std::string& pack,
 const std::optional<std::string>& distance, const std::optional<std::string>& velocity);
void write_match_save(std::ostream&, const MatchRuntimeState&, const MatchContentIdentity&);
MatchRuntimeState load_match_save(std::istream&, const MatchContentIdentity&);
}

#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace captain::research {
// Development protocol only: inputs are explicit research fixtures, not player commands.
std::optional<std::string> evaluate(std::string_view command, std::string_view fields);
}

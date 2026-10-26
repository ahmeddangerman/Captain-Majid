#include "captain/action_strength.hpp"

namespace captain {
std::uint16_t reduce_action_strength(std::uint16_t power, std::uint8_t loss) {
    const auto remaining = static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(power) + 65536u - loss);
    return (remaining & 0x8000u) ? 0 : remaining;
}
}

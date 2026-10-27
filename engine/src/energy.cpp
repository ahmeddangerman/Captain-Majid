#include "captain/energy.hpp"

namespace captain {
EnergyResult calculate_energy(const EnergyInput& input) {
    if (input.skip_home_side && input.side == 0) {
        return {input.stamina, input.cost, false, input.incoming_carry};
    }

    auto cost = input.cost;
    const bool exempt = input.side == 0 && input.action == 0 && input.subtype >= 3;
    if (input.player_id == 0x20 && !exempt) {
        cost = static_cast<std::uint16_t>(unsigned(cost) + unsigned(cost) / 2);
    }

    const auto difference = static_cast<std::uint16_t>(unsigned(input.stamina) - unsigned(cost));
    const bool weakened = (difference & 0x8000U) != 0;
    const auto remaining = weakened ? std::uint16_t{0} : difference;
    const bool carry = !weakened && input.stamina >= cost;
    return {remaining, cost, weakened, carry};
}
}

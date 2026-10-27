#include "captain/field_handoff.hpp"

#include <utility>

namespace captain {
FieldContactHandoffResult resolve_field_contact_handoff(
    const FieldContactHandoffInput& input) {
    auto field = resolve_field_contacts(input.contacts, input.incoming_power,
                                        input.initial_result_code);
    const auto outcome = resolve_defender_outcome({field.result_code,
        input.possession_side, input.ball_owner, input.defender_id,
        input.defense_action, input.recovery});
    return {std::move(field), outcome};
}
}

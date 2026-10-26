#include "captain/action_strength.hpp"
#include "captain/session.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}
}

int main() {
    using captain::reduce_action_strength;
    require(reduce_action_strength(300, 37) == 263, "ordinary contact loss");
    require(reduce_action_strength(256, 1) == 255, "borrow across low byte");
    require(reduce_action_strength(20, 37) == 0, "underflow clamps to zero");
    require(reduce_action_strength(37, 37) == 0, "exact depletion");
    require(reduce_action_strength(0, 0) == 0, "empty power");
    require(reduce_action_strength(32768, 1) == 32767, "boundary crosses sign bit");
    require(reduce_action_strength(32768, 0) == 0, "sign-bit clamp");
    require(reduce_action_strength(65535, 0) == 0, "original high-bit behavior");

    captain::Session session;
    const auto initial = session.snapshot();
    require(initial == "CM1|rules=contact-loss-v1|power=300|loss=37|next=0|owner=0|participants=7:0,42:1",
            "snapshot encodes all fixture state");
    using Reply = captain::ContactReply;
    require(session.apply_contact(999, 0) == Reply::unknown_participant, "unknown participant rejected");
    require(session.apply_contact(42, 0) == Reply::unauthorized, "opposite team rejected");
    require(session.apply_contact(7, 1) == Reply::out_of_order, "future command rejected");
    require(session.snapshot() == initial, "rejected commands preserve all state");
    require(session.apply_contact(7, 0) == Reply::accepted, "authorized contact accepted");
    const auto first = session.snapshot();
    require(first.find("power=263|loss=37|next=1|") != std::string::npos, "one contact advances fixture");
    require(session.apply_contact(7, 0) == Reply::out_of_order, "duplicate rejected");
    require(session.snapshot() == first, "duplicate leaves state unchanged");
    captain::Session replay;
    require(replay.apply_contact(7, 0) == Reply::accepted, "replay first command");
    for (std::uint32_t i = 1; i < 10; ++i) {
        require(session.apply_contact(7, i) == Reply::accepted, "next command accepted");
        require(replay.apply_contact(7, i) == Reply::accepted, "replay command accepted");
        require(session.snapshot() == replay.snapshot(), "full replay state identical");
    }
    require(session.snapshot().find("power=0|loss=37|next=10|") != std::string::npos,
            "power cannot wrap after depletion");
    captain::Session reordered(300, 37, {{42, 1}, {7, 0}});
    require(reordered.snapshot() == initial, "participant input order does not affect serialization");
    require(captain::Session(300, 38).snapshot() != initial, "different rules data changes snapshot");
    captain::Session additional(300, 37, {{7, 0}, {42, 1}, {99, 1}});
    require(additional.apply_contact(99, 0) == Reply::unauthorized,
            "additional participant represented without hardcoded two-client identities");
    bool invalid = false;
    try { captain::Session bad(300, 37, {{7, 0}, {7, 1}}); }
    catch (const std::invalid_argument&) { invalid = true; }
    require(invalid, "duplicate identities rejected");
    invalid = false;
    try { captain::Session bad(300, 37, {{7, 2}}); }
    catch (const std::invalid_argument&) { invalid = true; }
    require(invalid, "invalid team rejected");
    invalid = false;
    try { captain::Session bad(300, 37, {}); }
    catch (const std::invalid_argument&) { invalid = true; }
    require(invalid, "empty participant list rejected");
    invalid = false;
    try { captain::Session bad(300, 37, {{7, 1}}, 0); }
    catch (const std::invalid_argument&) { invalid = true; }
    require(invalid, "controlling team must have a participant");
    std::cout << "Core checks passed\n";
}

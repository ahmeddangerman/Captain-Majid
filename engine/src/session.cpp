#include "captain/session.hpp"
#include "captain/action_strength.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace captain {
const char* reply_name(ContactReply reply) {
    switch (reply) {
    case ContactReply::accepted: return "accepted";
    case ContactReply::unknown_participant: return "unknown_participant";
    case ContactReply::unauthorized: return "unauthorized";
    case ContactReply::out_of_order: return "out_of_order";
    case ContactReply::exhausted: return "exhausted";
    }
    throw std::logic_error("Invalid contact reply");
}

Session::Session(std::uint16_t power, std::uint8_t loss,
                 std::vector<Participant> participants, std::uint8_t attacking_team)
    : power_(power), loss_(loss), participants_(std::move(participants)),
      attacking_team_(attacking_team) {
    if (participants_.empty() || attacking_team_ > 1) {
        throw std::invalid_argument("Session requires participants and a valid team");
    }
    std::sort(participants_.begin(), participants_.end(),
              [](const Participant& a, const Participant& b) { return a.id < b.id; });
    bool has_owner = false;
    for (std::size_t i = 0; i < participants_.size(); ++i) {
        if (participants_[i].team > 1 ||
            (i > 0 && participants_[i - 1].id == participants_[i].id)) {
            throw std::invalid_argument("Participant teams must be valid and IDs unique");
        }
        has_owner = has_owner || participants_[i].team == attacking_team_;
    }
    if (!has_owner) {
        throw std::invalid_argument("Controlling team requires a participant");
    }
}

ContactReply Session::apply_contact(std::uint32_t participant, std::uint32_t sequence) {
    const auto found = std::find_if(participants_.begin(), participants_.end(),
        [participant](const Participant& value) { return value.id == participant; });
    if (found == participants_.end()) return ContactReply::unknown_participant;
    if (found->team != attacking_team_) return ContactReply::unauthorized;
    if (sequence != next_sequence_) return ContactReply::out_of_order;
    if (next_sequence_ == std::numeric_limits<std::uint32_t>::max()) {
        return ContactReply::exhausted;
    }
    power_ = reduce_action_strength(power_, loss_);
    ++next_sequence_;
    return ContactReply::accepted;
}

std::string Session::snapshot() const {
    std::string result = "CM1|rules=contact-loss-v1|power=" + std::to_string(power_) +
        "|loss=" + std::to_string(loss_) + "|next=" + std::to_string(next_sequence_) +
        "|owner=" + std::to_string(attacking_team_) + "|participants=";
    for (std::size_t i = 0; i < participants_.size(); ++i) {
        if (i) result += ',';
        result += std::to_string(participants_[i].id) + ':' +
                  std::to_string(participants_[i].team);
    }
    return result;
}
}

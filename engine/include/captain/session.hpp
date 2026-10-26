#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace captain {
struct Participant {
    std::uint32_t id;
    std::uint8_t team;
};

enum class ContactReply { accepted, unknown_participant, unauthorized, out_of_order, exhausted };
const char* reply_name(ContactReply reply);

// Developer fixture for a verified subroutine, not a full match state.
class Session {
public:
    Session(std::uint16_t power = 300, std::uint8_t loss = 37,
            std::vector<Participant> participants = {{7, 0}, {42, 1}},
            std::uint8_t attacking_team = 0);
    ContactReply apply_contact(std::uint32_t participant, std::uint32_t sequence);
    std::string snapshot() const;

private:
    std::uint16_t power_;
    std::uint8_t loss_;
    std::vector<Participant> participants_;
    std::uint8_t attacking_team_;
    std::uint32_t next_sequence_ = 0;
};
}

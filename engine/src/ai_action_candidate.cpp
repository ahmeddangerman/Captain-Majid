#include "captain/ai_action_candidate.hpp"
namespace captain {
std::uint8_t choose_ai_action_candidate(const std::array<std::uint8_t,4>& candidates,std::uint8_t random) {
    const auto index=static_cast<unsigned>(random&7u);
    const auto packed=candidates[index>>1];
    return static_cast<std::uint8_t>((index&1u) ? packed&15u : packed>>4);
}
}

#pragma once
#include "captain/match_runtime.hpp"

namespace captain {
struct ClientButtonSample { std::uint8_t held=0,pressed=0; };
class ClientButtons {
public:
 void press(std::uint8_t mask);
 void release(std::uint8_t mask);
 void clear();
 ClientButtonSample take();
private:
 std::uint8_t held_=0,pressed_=0;
};
// Chooses only an input or a presentation acknowledgement; it never supplies an outcome.
MatchInput client_input(const MatchRuntimeState&,std::uint8_t held,std::uint8_t pressed);
}

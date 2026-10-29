#include "captain/kickoff_selection.hpp"
#include <stdexcept>
namespace captain {
std::uint8_t select_kickoff_side(std::uint8_t opponent_team,std::uint8_t random) {
 return opponent_team!=3 && (random&128u) ? 11 : 0;
}
std::uint8_t select_kickoff_receiver(std::uint8_t side,std::uint8_t random,
 const std::array<std::uint8_t,5>& candidates) {
 if(side!=0 && side!=11)throw std::invalid_argument("Kickoff side must be0 or11");
 for(auto id:candidates)if(id<1 || id>10)throw std::invalid_argument("Kickoff candidates must be field-player slots1..10");
 auto index=static_cast<unsigned>(random&7u);
 if(index>=5)index-=5;
 return static_cast<std::uint8_t>(side+candidates[index]);
}
}

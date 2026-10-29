#pragma once
#include "captain/phase_session.hpp"
#include "captain/shot_content.hpp"
#include "captain/shot_outcome.hpp"
namespace captain {
struct KeeperSaveResult {
 PhaseSessionState state;
 ShotEvent event;
 ExperienceRewardResult reward;
 bool request_possession_sound=false;
};
// Prepared after display9; only normal keeper results2/3. No goal/score writes.
KeeperSaveResult resolve_session_keeper_save(const PhaseSessionState& input,const ShotContent& content);
}

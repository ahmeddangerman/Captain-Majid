#pragma once
#include "captain/phase_session.hpp"
#include "captain/shot_approach.hpp"
namespace captain {
struct SessionApproachResult { PhaseSessionState state; ShotApproachResult approach; };
SessionApproachResult prepare_session_shot_approach(const PhaseSessionState& input,
    const ShotApproachContent& content);
}

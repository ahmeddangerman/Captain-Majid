#pragma once

#include "captain/energy.hpp"
#include "captain/goalkeeper.hpp"
#include "captain/shot_outcome.hpp"
#include "captain/shot_power.hpp"

namespace captain {
struct ShotSequenceInput {
    EnergyInput energy;
    ActionPowerInput action;
    GoalkeeperInput keeper;
    ShotOutcomeInput outcome;
};
struct ShotSequenceResult {
    EnergyResult energy;
    ActionPowerResult action;
    GoalkeeperResult keeper;
    ShotOutcomeResult outcome;
};

// Runs the supported shot stages in order using caller-selected content and
// random bytes. Match setup, field contacts, and phase continuation are separate.
ShotSequenceResult resolve_explicit_shot(const ShotSequenceInput& input);
}

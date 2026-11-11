#pragma once
#include "captain/phase_session.hpp"
#include "captain/shot_content.hpp"
#include "captain/control_session.hpp"
#include <cstdint>
#include <optional>
#include <vector>

namespace captain {
enum class UnsupportedCapability { content, initialization, input_sequence, timing, action, presentation, phase };
enum class MatchInputKind { tick, action, presentation_complete };
struct MatchInput { MatchInputKind kind; std::uint64_t sequence; };
struct MatchEvent { ControlPresentationRequest presentation; };
// Initial state is a development snapshot, not independently verified match startup.
struct MatchConfig { std::optional<PhaseSessionState> initial{}; std::uint32_t seed=0; std::uint16_t clock=0; };
struct MatchContent { std::optional<ShotContent> data{}; };
struct MatchRuntimeState { PhaseSessionState session{}; std::uint64_t last_sequence=0; };
struct MatchStepResult { MatchRuntimeState state{}; std::vector<MatchEvent> events{}; std::optional<UnsupportedCapability> unsupported{}; };
MatchStepResult create_match(const MatchConfig&,const MatchContent&);
MatchStepResult step_match(const MatchRuntimeState&,const MatchInput&,const MatchContent&);
const char* capability_name(UnsupportedCapability);
}

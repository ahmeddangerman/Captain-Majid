#pragma once
#include "captain/phase_session.hpp"
#include "captain/shot_content.hpp"
#include "captain/control_session.hpp"
#include "captain/runtime_random.hpp"
#include <cstdint>
#include <optional>
#include <vector>

namespace captain {
enum class UnsupportedCapability { content, initialization, input_sequence, random_state, timing, action, presentation, phase };
enum class MatchInputKind { tick, action, presentation_complete };
enum class MatchAction { none, pass };
// Inputs express intent only; action outcomes and random samples are never supplied.
struct MatchInput { MatchInputKind kind; std::uint64_t sequence; std::uint8_t held=0, pressed=0; MatchAction action=MatchAction::none; std::optional<std::uint8_t> target{}; };
struct MatchEvent { ControlPresentationRequest presentation; };
// Initial state is a development snapshot, not independently verified match startup.
struct MatchConfig { std::optional<PhaseSessionState> initial{}; std::uint32_t seed=0; std::uint16_t clock=0; std::optional<std::uint8_t> random_index{}; };
struct MatchContent { std::optional<ShotContent> data{}; };
struct MatchRuntimeState { PhaseSessionState session{}; std::uint64_t last_sequence=0; std::optional<std::uint8_t> random_index{}; RuntimeRandomMemory random_memory{}; };
struct MatchStepResult { MatchRuntimeState state{}; std::vector<MatchEvent> events{}; std::optional<UnsupportedCapability> unsupported{}; };
MatchStepResult create_match(const MatchConfig&,const MatchContent&);
MatchStepResult step_match(const MatchRuntimeState&,const MatchInput&,const MatchContent&);
const char* capability_name(UnsupportedCapability);
}

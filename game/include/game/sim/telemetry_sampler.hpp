#ifndef GAME_SIM_TELEMETRY_SAMPLER_HPP
#define GAME_SIM_TELEMETRY_SAMPLER_HPP

#include <engine/systems/system.hpp>
#include <game/sim/seek_items_policy.hpp>
#include <game/sim/telemetry_row.hpp>
#include <game/sim/telemetry_writer.hpp>

/// Samples one row per fixed step and decides when the run is over.
///
/// Registered with addFixedSystem from a plugin mounted after GamePlugin, so
/// it runs last in the step: after Movement, EnemyAI, Collision and Combat.
///
/// Termination lives here rather than in a frame hook because this is the
/// only thing that runs after CombatSystem sets gameOver and before
/// GamePlugin's next frame-begin consumes it -- which would clear the flag,
/// request the menu, and leave the run wandering an empty scene instead of
/// stopping.
class TelemetrySampler : public de::System
{
public:
    TelemetrySampler(const SeekItemsPolicy& policy, int maxSteps);

    /// Where rows go. Null formats nothing, which is what the unit tests use
    /// when they only care about the sampled values.
    TelemetrySampler& setWriter(TelemetryWriter* writer);

    void run(entt::registry& registry) override;

    const TelemetryRow& lastRow() const { return m_row; }
    const SimSummary& summary() const { return m_summary; }

    /// True once the run has ended for any reason.
    bool finished() const { return m_finished; }

private:
    void finish(entt::registry& registry, const char* outcome);

    const SeekItemsPolicy& m_policy;
    TelemetryWriter* m_writer = nullptr;
    int m_maxSteps = 0;

    TelemetryRow m_row;
    SimSummary m_summary;
    bool m_started = false;
    bool m_finished = false;

    de::Vector2D<float> m_previousPosition;
    int m_previousHealth = -1;

    /// Distance at the step the watchdog last reset, and when that was. A run
    /// that stops making progress should fail loudly rather than burn every
    /// remaining step in silence.
    float m_watchdogDistance = 0.0f;
    int m_watchdogStep = 0;
    int m_watchdogItems = 0;
};

#endif // GAME_SIM_TELEMETRY_SAMPLER_HPP

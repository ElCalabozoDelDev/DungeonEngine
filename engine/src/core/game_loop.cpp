#include <SDL_timer.h>
#include <algorithm>
#include <engine/core/game_loop.hpp>
#include <iostream>

namespace de
{
bool GameLoop::isPaused() const
{
    const auto* paused = m_registry.ctx().find<Paused>();
    return paused != nullptr && paused->value;
}

bool GameLoop::run()
{
    m_controlFlow = ControlFlow::Loop;
    m_registry.ctx().emplace<ControlFlow&>(m_controlFlow);
    m_registry.ctx().insert_or_assign(DeltaTime{});

    m_hookSetup.publishUntil(
        [this] { return m_registry.ctx().contains<StartupError>(); },
        m_registry);

    if (auto* error = m_registry.ctx().find<StartupError>())
    {
        std::cerr << "Startup failed: " << error->message << std::endl;
        m_hookTeardown.publish(m_registry);
        return false;
    }

    // SDL_GetTicks() only has millisecond resolution, a sixth of a frame at
    // 60Hz. The performance counter is what the delta is worth measuring
    // with. It is also a local, not a function-level static: the old one made
    // a second run() in the same process start with a stale timestamp and a
    // huge first delta.
    const Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 previousCounter = SDL_GetPerformanceCounter();
    float accumulator = 0.0f;

    while (m_controlFlow == ControlFlow::Loop)
    {
        DeltaTime& dt = m_registry.ctx().get<DeltaTime>();

        // On the synthetic clock (setFrameDelta) the counter is not read at
        // all, so the delta is the same float every frame and the run is
        // reproducible.
        float frameDelta = m_frameDelta;
        if (m_frameDelta <= 0.0f)
        {
            const Uint64 currentCounter = SDL_GetPerformanceCounter();
            frameDelta = static_cast<float>(
                static_cast<double>(currentCounter - previousCounter) /
                static_cast<double>(frequency));
            previousCounter = currentCounter;
        }

        dt.value = std::min(frameDelta, MaxFrameDelta);
        dt.elapsed += static_cast<double>(dt.value);

        m_hookFrameBegin.publish(m_registry);

        // Pausing means the simulation does not advance: no fixed steps and
        // no accumulation, so unpausing does not replay the pause as a
        // backlog. Frame systems keep running, which is what draws the pause
        // overlay.
        if (!isPaused())
        {
            accumulator += dt.value;
        }

        // Fixed systems consume whole steps; whatever is left over is carried
        // into the next frame and reported as `alpha`.
        //
        // Pause is checked before every step, not once per frame: a step that
        // pauses (the snake dying) must be the last one, rather than the rest
        // of a slow frame's steps running on a finished game.
        int steps = 0;
        while (accumulator >= dt.fixed && steps < MaxFixedStepsPerFrame)
        {
            if (isPaused())
            {
                accumulator = 0.0f;
                break;
            }
            for (auto& system : m_fixedSystems)
            {
                system->run(m_registry);
            }
            for (auto& system : m_fixedSystemsLast)
            {
                system->run(m_registry);
            }
            accumulator -= dt.fixed;
            ++steps;
        }
        if (steps == MaxFixedStepsPerFrame)
        {
            // Too far behind to catch up; drop the backlog rather than spiral
            // into ever-longer frames.
            accumulator = 0.0f;
        }
        dt.alpha = dt.fixed > 0.0f ? accumulator / dt.fixed : 0.0f;

        for (auto& system : m_systems)
        {
            system->run(m_registry);
        }

        for (auto& system : m_systemsLast)
        {
            system->run(m_registry);
        }

        m_hookFrameEnd.publish(m_registry);
    }

    m_hookTeardown.publish(m_registry);
    return true;
}

} // namespace de

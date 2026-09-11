#ifndef DE_CORE_GAME_LOOP_HPP
#define DE_CORE_GAME_LOOP_HPP

#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <algorithm>
#include <engine/core/delta_time.hpp>
#include <engine/core/hook.hpp>
#include <engine/core/paused.hpp>
#include <engine/core/startup_error.hpp>
#include <engine/plugins/plugin.hpp>
#include <engine/systems/system.hpp>
#include <entt/entt.hpp>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace de
{
enum class ControlFlow
{
    Exit,
    Loop
};

class GameLoop
{
public:
    /// Longest frame the loop will admit. A frame longer than this (dragging
    /// the window, a breakpoint, the machine sleeping) is treated as this
    /// long, so entities step slowly rather than teleporting through walls.
    static constexpr float MaxFrameDelta = 0.25f;

    /// Cap on fixed steps per frame. Without it, a machine that cannot keep up
    /// runs more steps each frame, which makes it slower still.
    static constexpr int MaxFixedStepsPerFrame = 5;

    /// Takes ownership of the plugin, then mounts it.
    ///
    /// Plugins register callbacks that capture `this`, so the loop has to
    /// outlive them. The previous signature took a bare reference, and
    /// `addPlugin(GamePlugin())` compiled happily and left every one of those
    /// callbacks pointing at a destroyed temporary.
    GameLoop& addPlugin(std::unique_ptr<Plugin> plugin)
    {
        Plugin* raw = plugin.get();
        m_plugins.push_back(std::move(plugin));
        raw->mount(*this);
        return *this;
    }

    /// Runs once per frame, with DeltaTime::value.
    GameLoop& addSystem(std::shared_ptr<System> system)
    {
        m_systems.push_back(std::move(system));
        return *this;
    }

    /// Runs zero or more times per frame, always advancing DeltaTime::fixed.
    /// Movement, physics and collision belong here: their results must not
    /// depend on how fast the machine happens to be drawing.
    GameLoop& addFixedSystem(std::shared_ptr<System> system)
    {
        m_fixedSystems.push_back(std::move(system));
        return *this;
    }

    /// Runs once per frame, after every addSystem(). Rendering goes here.
    GameLoop& addSystemLast(std::shared_ptr<System> system)
    {
        m_systemsLast.push_back(std::move(system));
        return *this;
    }

    GameLoop& addSetupCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookSetup.connect(std::move(callback));
        return *this;
    }

    GameLoop&
    addFrameBeginCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookFrameBegin.connect(std::move(callback));
        return *this;
    }

    GameLoop& addFrameEndCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookFrameEnd.connect(std::move(callback));
        return *this;
    }

    GameLoop& addTeardownCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookTeardown.connect(std::move(callback), true);
        return *this;
    }

    /// Runs setup, then frames until something asks to exit.
    ///
    /// Returns false when a setup callback reported a StartupError, in which
    /// case no frame runs. Teardown runs either way.
    bool run()
    {
        m_controlFlow = ControlFlow::Loop;
        m_registry.ctx().emplace<ControlFlow&>(m_controlFlow);

        m_hookSetup.publish(m_registry);

        if (auto* error = m_registry.ctx().find<StartupError>())
        {
            std::cerr << "Startup failed: " << error->message << std::endl;
            m_hookTeardown.publish(m_registry);
            return false;
        }

        // SDL_GetTicks() only has millisecond resolution, a sixth of a frame
        // at 60Hz. The performance counter is what the delta is worth
        // measuring with. It is also a local, not a function-level static:
        // the old one made a second run() in the same process start with a
        // stale timestamp and a huge first delta.
        const Uint64 frequency = SDL_GetPerformanceFrequency();
        Uint64 previousCounter = SDL_GetPerformanceCounter();
        float accumulator = 0.0f;

        while (m_controlFlow == ControlFlow::Loop)
        {
            DeltaTime& dt = m_registry.ctx().get<DeltaTime>();

            const Uint64 currentCounter = SDL_GetPerformanceCounter();
            const float frameDelta = static_cast<float>(
                static_cast<double>(currentCounter - previousCounter) /
                static_cast<double>(frequency));
            previousCounter = currentCounter;

            dt.value = std::min(frameDelta, MaxFrameDelta);
            dt.elapsed += static_cast<double>(dt.value);

            m_hookFrameBegin.publish(m_registry);

            // Pausing means the simulation does not advance: no fixed steps
            // and no accumulation, so unpausing does not replay the pause as
            // a backlog. Frame systems keep running, which is what draws the
            // pause overlay.
            const auto* paused = m_registry.ctx().find<Paused>();
            const bool isPaused = paused != nullptr && paused->value;

            // Fixed systems consume whole steps; whatever is left over is
            // carried into the next frame and reported as `alpha`.
            if (!isPaused)
            {
                accumulator += dt.value;
            }
            int steps = 0;
            while (!isPaused && accumulator >= dt.fixed &&
                   steps < MaxFixedStepsPerFrame)
            {
                for (auto& system : m_fixedSystems)
                {
                    system->run(m_registry);
                }
                accumulator -= dt.fixed;
                ++steps;
            }
            if (steps == MaxFixedStepsPerFrame)
            {
                // Too far behind to catch up; drop the backlog rather than
                // spiral into ever-longer frames.
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

private:
    entt::registry m_registry;
    ControlFlow m_controlFlow = ControlFlow::Exit;

    std::vector<std::unique_ptr<Plugin>> m_plugins;
    std::vector<std::shared_ptr<System>> m_fixedSystems;
    std::vector<std::shared_ptr<System>> m_systems;
    std::vector<std::shared_ptr<System>> m_systemsLast;

    Hook<void(entt::registry&)> m_hookSetup;
    Hook<void(entt::registry&)> m_hookFrameBegin;
    Hook<void(entt::registry&)> m_hookFrameEnd;
    Hook<void(entt::registry&)> m_hookTeardown;
};

} // namespace de

#endif // DE_CORE_GAME_LOOP_HPP

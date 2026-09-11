#ifndef DE_CORE_GAME_LOOP_HPP
#define DE_CORE_GAME_LOOP_HPP
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <engine/core/delta_time.hpp>
#include <engine/core/hook.hpp>
#include <engine/core/startup_error.hpp>
#include <engine/plugins/plugin.hpp>
#include <engine/systems/system.hpp>
#include <entt/entt.hpp>
#include <iostream>

namespace de
{
enum class ControlFlow
{
    Exit,
    Loop
};

class GameLoop
{
private:
    entt::registry m_registry;
    ControlFlow m_controlFlow;
    std::vector<std::shared_ptr<System>> m_systems;
    std::vector<std::shared_ptr<System>> m_systemsLast;

    Hook<void(entt::registry&)> m_hookSetup;
    Hook<void(entt::registry&)> m_hookFrameBegin;
    Hook<void(entt::registry&)> m_hookFrameEnd;
    Hook<void(entt::registry&)> m_hookTeardown;

public:
    GameLoop& addPlugin(Plugin& plugin)
    {
        plugin.mount(*this);
        return *this;
    }

    GameLoop& addSystem(std::shared_ptr<System> system)
    {
        m_systems.push_back(system);
        return *this;
    }

    GameLoop& addSystemLast(std::shared_ptr<System> system)
    {
        m_systemsLast.push_back(system);
        return *this;
    }

    GameLoop& addSetupCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookSetup.connect(callback);
        return *this;
    }

    GameLoop&
    addFrameBeginCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookFrameBegin.connect(callback);
        return *this;
    }

    GameLoop& addFrameEndCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookFrameEnd.connect(callback);
        return *this;
    }

    GameLoop& addTeardownCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookTeardown.connect(callback, true);
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

        static Uint32 lastTime = SDL_GetTicks();
        while (m_controlFlow == ControlFlow::Loop)
        {
            DeltaTime& dt = m_registry.ctx().get<DeltaTime>();
            Uint32 currentTime = SDL_GetTicks();
            dt.value = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;

            m_hookFrameBegin.publish(m_registry);

            for (auto& system : m_systems)
            {
                system->run(m_registry);
            }

            for (auto it = m_systemsLast.rbegin(); it != m_systemsLast.rend();
                 ++it)
            {
                (*it)->run(m_registry);
            }

            m_hookFrameEnd.publish(m_registry);
        }

        m_hookTeardown.publish(m_registry);
        return true;
    }
};

} // namespace de

#endif // DE_CORE_GAME_LOOP_HPP

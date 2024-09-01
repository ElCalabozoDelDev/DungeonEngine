#ifndef GAME_LOOP_HPP
#define GAME_LOOP_HPP

#include <entt/entt.hpp>
#include "System.hpp"
#include "hook.hpp"

enum class ControlFlow {
  Exit,
  Loop
};

class GameLoop {
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
    GameLoop& addSystem(std::shared_ptr<System> system) {
        m_systems.push_back(system);
        return *this;
    }

    GameLoop& addSystemLast(std::shared_ptr<System> system) {
        m_systemsLast.push_back(system);
        return *this;
    }

    GameLoop& addSetupCallback(std::function<void(entt::registry&)> callback) {
        m_hookSetup.connect(callback);
        return *this;
    }

    GameLoop& addFrameBeginCallback(std::function<void(entt::registry&)> callback) {
        m_hookFrameBegin.connect(callback);
        return *this;
    }

    GameLoop& addFrameEndCallback(std::function<void(entt::registry&)> callback) {
        m_hookFrameEnd.connect(callback);
        return *this;
    }

    GameLoop& addTeardownCallback(std::function<void(entt::registry&)> callback) {
        m_hookTeardown.connect(callback, true);
        return *this;
    }

    void run() {
        m_controlFlow = ControlFlow::Loop;
        m_registry.ctx().emplace<ControlFlow&>(m_controlFlow);

        m_hookSetup.publish(m_registry);

        while (m_controlFlow == ControlFlow::Loop) {
            m_hookFrameBegin.publish(m_registry);

            for (auto& system : m_systems) {
                system->run(m_registry);
            }

            for (auto it = m_systemsLast.rbegin(); it != m_systemsLast.rend(); ++it) {
                (*it)->run(m_registry);
            }

            m_hookFrameEnd.publish(m_registry);
        }

        m_hookTeardown.publish(m_registry);
    }
};

#endif
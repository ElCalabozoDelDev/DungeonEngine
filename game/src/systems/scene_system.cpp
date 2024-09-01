#include "systems/scene_system.hpp"

void SceneSystem::run(entt::registry& registry) {
        if (m_currentScene) {
            m_currentScene->onUpdate(registry);
        }
    }

void SceneSystem::changeScene(entt::registry& registry, std::unique_ptr<Scene> newScene) {
    if (m_currentScene) {
        m_currentScene->onExit(registry);
    }

    m_currentScene = std::move(newScene);

    if (m_currentScene) {
        m_currentScene->onEnter(registry);
    }
}
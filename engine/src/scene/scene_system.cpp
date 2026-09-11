#include <engine/scene/scene_system.hpp>
#include <utility>

namespace de
{
void SceneSystem::run(entt::registry& registry)
{
    // Apply a requested switch here, between frames' worth of work, rather
    // than wherever it was asked for.
    if (m_pendingScene)
    {
        setScene(registry, std::move(m_pendingScene));
        m_pendingScene.reset();
    }

    if (m_currentScene)
    {
        m_currentScene->onUpdate(registry);
    }
}

void SceneSystem::setScene(entt::registry& registry,
                           std::unique_ptr<Scene> newScene)
{
    if (m_currentScene)
    {
        m_currentScene->onExit(registry);
    }

    m_currentScene = std::move(newScene);

    if (m_currentScene)
    {
        m_currentScene->onEnter(registry);
    }
}

void SceneSystem::requestScene(std::unique_ptr<Scene> newScene)
{
    m_pendingScene = std::move(newScene);
}

} // namespace de

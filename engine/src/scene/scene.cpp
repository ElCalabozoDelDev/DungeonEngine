#include <engine/scene/scene.hpp>

namespace de
{
void Scene::destroyTracked(entt::registry& registry)
{
    for (auto entity : m_tracked)
    {
        if (registry.valid(entity))
        {
            registry.destroy(entity);
        }
    }
    m_tracked.clear();
}

entt::entity Scene::track(entt::entity entity)
{
    m_tracked.push_back(entity);
    return entity;
}

void Scene::track(std::span<const entt::entity> entities)
{
    m_tracked.insert(m_tracked.end(), entities.begin(), entities.end());
}

} // namespace de

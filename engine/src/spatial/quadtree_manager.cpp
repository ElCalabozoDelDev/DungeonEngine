#include <engine/spatial/quadtree_manager.hpp>
#include <entt/entity/fwd.hpp>

namespace de
{
std::unique_ptr<QuadtreeManager> QuadtreeManager::s_pInstance = nullptr;

void QuadtreeManager::clear() { m_quadtrees.clear(); }

bool QuadtreeManager::createQuadtree(
    std::function<Box<float>(const entt::entity&)> getBox,
    const std::string& name, const Box<float>& box)
{
    if (m_quadtrees.find(name) != m_quadtrees.end())
    {
        return false; // Ya existe un Quadtree con ese nombre
    }

    m_quadtrees[name] = std::make_unique<
        Quadtree<entt::entity, std::function<Box<float>(const entt::entity&)>>>(
        box, getBox);
    return true;
}

Quadtree<entt::entity, std::function<Box<float>(const entt::entity&)>>*
QuadtreeManager::getQuadtree(const std::string& name)
{
    auto it = m_quadtrees.find(name);
    if (it != m_quadtrees.end())
    {
        return it->second.get();
    }
    return nullptr;
}

std::vector<entt::entity> QuadtreeManager::query(const std::string& name,
                                                 const Box<float>& box)
{
    auto it = m_quadtrees.find(name);
    if (it != m_quadtrees.end())
    {
        return it->second->query(box);
    }
    return {};
}

bool QuadtreeManager::deleteQuadtree(const std::string& name)
{
    return m_quadtrees.erase(name) > 0;
}

bool QuadtreeManager::insertIntoQuadtree(const std::string& name,
                                         entt::entity entity)
{
    auto it = m_quadtrees.find(name);
    if (it == m_quadtrees.end())
    {
        return false;
    }
    return it->second->add(entity);
}

} // namespace de

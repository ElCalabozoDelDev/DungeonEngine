#include "core/quadtree_manager.hpp"
#include <entt/entity/fwd.hpp>

std::unique_ptr<QuadtreeManager> QuadtreeManager::s_pInstance = nullptr;

void QuadtreeManager::clear() { m_quadtrees.clear(); }

void QuadtreeManager::checkIntersections(std::vector<entt::entity*> nodes1,
                                         std::vector<entt::entity*> nodes2)
{
    assert(nodes1.size() == nodes2.size());
    std::sort(std::begin(nodes1), std::end(nodes1));
    std::sort(std::begin(nodes2), std::end(nodes2));
    for (auto i = std::size_t(0); i < nodes1.size(); ++i)
        assert(nodes1[i] == nodes2[i]);
}

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


std::vector<entt::entity> QuadtreeManager::query(const std::string& name, const Box<float>& box)
{
    auto it = m_quadtrees.find(name);
    if (it != m_quadtrees.end())
    {
        return it->second->query(box);
    }
    return {};
}

// Define un margen adicional para la consulta del Quadtree
constexpr float EXTENDED_MARGIN = 48.0f; // Ajustar este valor según el tamaño de los tiles

bool QuadtreeManager::deleteQuadtree(const std::string& name)
{
    return m_quadtrees.erase(name) > 0;
}

void QuadtreeManager::insertIntoQuadtree(const std::string& name, entt::entity entity)
{
    auto it = m_quadtrees.find(name);
    if (it != m_quadtrees.end())
    {
        it->second->add(entity);
    }
}
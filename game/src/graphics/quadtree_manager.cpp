#include "graphics/quadtree_manager.hpp"

std::unique_ptr<QuadtreeManager> QuadtreeManager::s_pInstance;

void QuadtreeManager::addNode(std::string id, entt::entity node)
{
    if(m_nodesMap.find(id) == m_nodesMap.end()){
        m_nodesMap[id] = new std::vector<entt::entity>();
    }
    m_nodesMap[id]->push_back(node);
}

void QuadtreeManager::removeNode(std::string id) { m_nodesMap.erase(id); }

void QuadtreeManager::clear() { m_nodesMap.clear(); }


void QuadtreeManager::checkIntersections(std::vector<quadtree::Box<float>*> nodes1,
                            std::vector<quadtree::Box<float>*> nodes2)
{
    assert(nodes1.size() == nodes2.size());
    std::sort(std::begin(nodes1), std::end(nodes1));
    std::sort(std::begin(nodes2), std::end(nodes2));
    for (auto i = std::size_t(0); i < nodes1.size(); ++i)
        assert(nodes1[i] == nodes2[i]);
}
std::vector<entt::entity>* QuadtreeManager::getNodes(std::string id) { return m_nodesMap[id]; }
#ifndef QUADTREE_MANAGER_HPP
#define QUADTREE_MANAGER_HPP

#include "Quadtree.hpp"
#include <entt/entt.hpp>
#include <map>
#include <vector>

using namespace quadtree;
class QuadtreeManager
{
public:
    static QuadtreeManager* Instance()
    {
        if(s_pInstance.get() == 0)
        {
            s_pInstance.reset(new QuadtreeManager());
        }
        return s_pInstance.get();
    }
    void addNode(std::string id, entt::entity node);

    void removeNode(std::string id);
    void clear();


    void checkIntersections(std::vector<quadtree::Box<float>*> nodes1,
                            std::vector<quadtree::Box<float>*> nodes2);
    std::vector<entt::entity>* getNodes(std::string id);


    ~QuadtreeManager() {}
private:
    QuadtreeManager() {}
    QuadtreeManager(const QuadtreeManager&);
    QuadtreeManager& operator=(const QuadtreeManager&);
    static std::map<std::string, std::vector<entt::entity>*> m_nodesMap;
    static std::unique_ptr<QuadtreeManager> s_pInstance;
};

#endif
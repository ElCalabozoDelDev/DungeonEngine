#ifndef QUADTREE_MANAGER_HPP
#define QUADTREE_MANAGER_HPP

#include "Quadtree.hpp"
#include <entt/entt.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace quadtree;

class QuadtreeManager
{
public:
    static QuadtreeManager* Instance()
    {
        if (!s_pInstance)
        {
            s_pInstance.reset(new QuadtreeManager());
        }
        return s_pInstance.get();
    }

    void clear();

    void checkIntersections(std::vector<entt::entity*> nodes1,
                            std::vector<entt::entity*> nodes2);

    bool createQuadtree(std::function<Box<float>(const entt::entity&)> getBox,
                        const std::string& name, const Box<float>& box);

    Quadtree<entt::entity, std::function<Box<float>(const entt::entity&)>>*
    getQuadtree(const std::string& name);

    std::vector<entt::entity> query(const std::string& name,
                                    const Box<float>& box);
    bool deleteQuadtree(const std::string& name);
    void insertIntoQuadtree(const std::string& name, entt::entity entity);

    ~QuadtreeManager() {}

private:
    QuadtreeManager() {}

    QuadtreeManager(const QuadtreeManager&) = delete;
    QuadtreeManager& operator=(const QuadtreeManager&) = delete;

    std::map<std::string,
             std::unique_ptr<Quadtree<
                 entt::entity, std::function<Box<float>(const entt::entity&)>>>>
        m_quadtrees;

    static std::unique_ptr<QuadtreeManager>
        s_pInstance; // Declaración sin inicializar
};

#endif

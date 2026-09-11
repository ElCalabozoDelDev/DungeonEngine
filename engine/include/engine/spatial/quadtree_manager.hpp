#ifndef DE_SPATIAL_QUADTREE_MANAGER_HPP
#define DE_SPATIAL_QUADTREE_MANAGER_HPP
#include <engine/spatial/quadtree.hpp>
#include <entt/entt.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace de
{
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

    bool createQuadtree(std::function<Box<float>(const entt::entity&)> getBox,
                        const std::string& name, const Box<float>& box);

    Quadtree<entt::entity, std::function<Box<float>(const entt::entity&)>>*
    getQuadtree(const std::string& name);

    std::vector<entt::entity> query(const std::string& name,
                                    const Box<float>& box);
    bool deleteQuadtree(const std::string& name);
    bool insertIntoQuadtree(const std::string& name, entt::entity entity);

    ~QuadtreeManager() {}

private:
    QuadtreeManager() {}

    QuadtreeManager(const QuadtreeManager&) = delete;
    QuadtreeManager& operator=(const QuadtreeManager&) = delete;

    std::map<std::string,
             std::unique_ptr<Quadtree<
                 entt::entity, std::function<Box<float>(const entt::entity&)>>>>
        m_quadtrees;

    static std::unique_ptr<QuadtreeManager> s_pInstance;
};

} // namespace de

#endif // DE_SPATIAL_QUADTREE_MANAGER_HPP

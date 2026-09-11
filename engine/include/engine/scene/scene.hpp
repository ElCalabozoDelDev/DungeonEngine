#ifndef DE_SCENE_SCENE_HPP
#define DE_SCENE_SCENE_HPP
#include <entt/entt.hpp>

namespace de
{
class Scene
{
public:
    virtual void onEnter(entt::registry& registry) = 0;
    virtual void onUpdate(entt::registry& registry) = 0;
    virtual void onExit(entt::registry& registry) = 0;
    virtual ~Scene() = default;
};

} // namespace de

#endif // DE_SCENE_SCENE_HPP

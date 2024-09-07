#ifndef SCENE_HPP
#define SCENE_HPP

#include <entt/entt.hpp>

class Scene {
public:
    virtual void onEnter(entt::registry& registry) = 0;
    virtual void onUpdate(entt::registry& registry) = 0;
    virtual void onExit(entt::registry& registry) = 0;
    virtual ~Scene() = default;
};

#endif
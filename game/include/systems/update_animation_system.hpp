#ifndef UPDATE_ANIMATION_SYSTEM_HPP
#define UPDATE_ANIMATION_SYSTEM_HPP

#include "entt/entt.hpp"
class UpdateAnimationSystem {
    public:
        void update(entt::registry& registry, float deltaTime);
};
#endif // UPDATE_ANIMATION_SYSTEM_HPP
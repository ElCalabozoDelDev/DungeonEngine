#ifndef COLLISION_SYSTEM_HPP
#define COLLISION_SYSTEM_HPP

#include "components/velocity_component.hpp"
#include "systems/system.hpp"
#include "components/transform_component.hpp"
#include "core/quadtree_manager.hpp"

class CollisionSystem : public System {
public:
    void run(entt::registry& registry) override;
    void resolveCollision(VelocityComponent& velocity, TransformComponent& playerTransform, Box<float>& playerBox, const Box<float>& collidableBox);
    void resolvePlayerEnemyCollision(VelocityComponent& velocity, TransformComponent& playerTransform, Box<float>& playerBox, const Box<float>& enemyBox);
};

#endif
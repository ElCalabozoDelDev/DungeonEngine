#ifndef UPDATE_POSITION_HPP
#define UPDATE_POSITION_HPP

#include "entt.hpp"

class TransformSystem {
public:
    void update(entt::registry& registry, float deltaTime);
};
#endif // UPDATE_POSITION_HPP
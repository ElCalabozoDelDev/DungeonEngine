#ifndef UPDATE_POSITION_HPP
#define UPDATE_POSITION_HPP

#include "core/vector_2d.hpp"
#include "systems/system.hpp"
#include "entt/entt.hpp"

class TransformSystem final : public System {
public:
    void run(entt::registry& registry) override;
    void updateSpritePosition(entt::registry& registry, entt::entity entity, const Vector2D<float>& newPosition);
};
#endif // UPDATE_POSITION_HPP
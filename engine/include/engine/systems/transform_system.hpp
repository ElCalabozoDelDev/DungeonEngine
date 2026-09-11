#ifndef DE_SYSTEMS_TRANSFORM_SYSTEM_HPP
#define DE_SYSTEMS_TRANSFORM_SYSTEM_HPP
#include "entt/entt.hpp"
#include <engine/core/vector_2d.hpp>
#include <engine/systems/system.hpp>

namespace de
{
class TransformSystem final : public System
{
public:
    void run(entt::registry& registry) override;
    void updateSpritePosition(entt::registry& registry, entt::entity entity,
                              const Vector2D<float>& newPosition);
};

} // namespace de

#endif // DE_SYSTEMS_TRANSFORM_SYSTEM_HPP

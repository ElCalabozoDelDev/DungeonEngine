#include <engine/components/camera_bounds_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/loaders/config.hpp>
#include <engine/spatial/quadtree_manager.hpp>
#include <engine/systems/transform_system.hpp>
// #include "core/quadtree.hpp"
#include <engine/components/dimension_component.hpp>
#include <engine/core/constants.hpp>

namespace de
{
void TransformSystem::run(entt::registry& registry)
{
    DeltaTime deltaTime = registry.ctx().get<DeltaTime>();
    auto config = registry.ctx().get<Config>();

    auto camView = registry.view<CameraBoundsComponent>();
    auto cameraEntity = *camView.begin();
    auto& camera = registry.get<CameraBoundsComponent>(cameraEntity);

    auto view = registry.view<TransformComponent, VelocityComponent,
                              SpriteComponent, DimensionComponent>();
    for (auto entity : view)
    {
        auto& trf = view.get<TransformComponent>(entity);
        auto& vel = view.get<VelocityComponent>(entity);
        auto& spr = view.get<SpriteComponent>(entity);
        auto& dim = view.get<DimensionComponent>(entity);

        trf.position += vel.velocity * deltaTime.value;

        const int levelWidth = camera.levelWidth;
        const int levelHeight = camera.levelHeight;

        if (trf.position.getX() < 0)
        {
            trf.position.setX(0);
        }
        else if (trf.position.getX() + dim.width > levelWidth)
        {
            trf.position.setX(levelWidth - dim.width);
        }
        if (trf.position.getY() < 0)
        {
            trf.position.setY(0);
        }
        else if (trf.position.getY() + dim.height > levelHeight)
        {
            trf.position.setY(levelHeight - dim.height);
        }
        updateSpritePosition(registry, entity, trf.position);
    }
}

void TransformSystem::updateSpritePosition(entt::registry& registry,
                                           entt::entity entity,
                                           const Vector2D<float>& newPosition)
{

    // Bail out if the entity went away
    if (!registry.valid(entity) || !registry.all_of<TransformComponent>(entity))
    {
        return; // nothing to update
    }

    // Write the new position back
    registry.get<TransformComponent>(entity).position = newPosition;
}

} // namespace de

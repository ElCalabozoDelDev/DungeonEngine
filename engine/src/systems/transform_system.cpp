#include <engine/components/camera_bounds_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/systems/transform_system.hpp>

namespace de
{
void TransformSystem::run(entt::registry& registry)
{
    const DeltaTime deltaTime = registry.ctx().get<DeltaTime>();

    auto camView = registry.view<CameraBoundsComponent>();
    if (camView.begin() == camView.end())
    {
        // Level bounds come from the camera; without one there is nothing to
        // clamp against, and *begin() would have been undefined behaviour.
        return;
    }
    auto cameraEntity = *camView.begin();
    auto& camera = registry.get<CameraBoundsComponent>(cameraEntity);

    auto view = registry.view<TransformComponent, VelocityComponent,
                              SpriteComponent, DimensionComponent>();
    for (auto entity : view)
    {
        auto& trf = view.get<TransformComponent>(entity);
        auto& vel = view.get<VelocityComponent>(entity);
        auto& dim = view.get<DimensionComponent>(entity);

        // A fixed system: one step is DeltaTime::fixed long, however long the
        // frame that ran it was.
        trf.position += vel.velocity * deltaTime.fixed;

        const int levelWidth = camera.levelWidth;
        const int levelHeight = camera.levelHeight;

        if (trf.position.x < 0)
        {
            trf.position.x = 0;
        }
        else if (trf.position.x + dim.width > levelWidth)
        {
            trf.position.x = levelWidth - dim.width;
        }
        if (trf.position.y < 0)
        {
            trf.position.y = 0;
        }
        else if (trf.position.y + dim.height > levelHeight)
        {
            trf.position.y = levelHeight - dim.height;
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

#include <algorithm>
#include <engine/components/dimension_component.hpp>
#include <engine/components/solid_body_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/systems/collision_system.hpp>

namespace de
{
namespace
{
Box<float> boxOf(entt::registry& registry, entt::entity entity)
{
    const auto& transform = registry.get<TransformComponent>(entity);
    const auto& dimension = registry.get<DimensionComponent>(entity);
    return Box<float>(transform.position.getX(), transform.position.getY(),
                      dimension.width, dimension.height);
}

/// Layers whose tiles are solid.
constexpr Layer SolidLayers[] = {Layer::Collision, Layer::Overlay};

} // namespace

void CollisionSystem::resolveAABB(entt::registry& registry, entt::entity entity,
                                  Box<float>& box, const Box<float>& obstacle)
{
    const float overlapX = std::min(box.getRight(), obstacle.getRight()) -
                           std::max(box.getLeft(), obstacle.getLeft());
    const float overlapY = std::min(box.getBottom(), obstacle.getBottom()) -
                           std::max(box.getTop(), obstacle.getTop());

    if (overlapX <= 0.0f || overlapY <= 0.0f)
    {
        return;
    }

    auto& transform = registry.get<TransformComponent>(entity);
    auto* velocity = registry.try_get<VelocityComponent>(entity);

    // Push along whichever axis overlaps least: that is the shallower
    // penetration, so it is the way the body came in.
    if (overlapX < overlapY)
    {
        const float direction =
            box.getLeft() < obstacle.getLeft() ? -1.0f : 1.0f;
        transform.position.setX(transform.position.getX() +
                                direction * overlapX);
        if (velocity != nullptr)
        {
            velocity->velocity.setX(0.0f);
        }
    }
    else
    {
        const float direction = box.getTop() < obstacle.getTop() ? -1.0f : 1.0f;
        transform.position.setY(transform.position.getY() +
                                direction * overlapY);
        if (velocity != nullptr)
        {
            velocity->velocity.setY(0.0f);
        }
    }

    // Keep the working box in step, so the next obstacle this frame is tested
    // against the corrected position.
    box.setLeft(transform.position.getX());
    box.setTop(transform.position.getY());
}

void CollisionSystem::run(entt::registry& registry)
{
    auto* spatial = registry.ctx().find<SpatialIndex>();
    if (spatial == nullptr)
    {
        return;
    }

    // Every solid body, not "everything in the Object layer, which must be
    // the player" as the prototype assumed.
    auto bodies =
        registry
            .view<SolidBodyComponent, TransformComponent, DimensionComponent>();

    for (auto entity : bodies)
    {
        Box<float> box = boxOf(registry, entity);

        for (Layer layer : SolidLayers)
        {
            for (entt::entity tile : spatial->query(layer, box))
            {
                if (!registry.all_of<TransformComponent, DimensionComponent>(
                        tile))
                {
                    continue;
                }
                const Box<float> tileBox = boxOf(registry, tile);
                if (box.intersects(tileBox))
                {
                    resolveAABB(registry, entity, box, tileBox);
                }
            }
        }
    }
}

} // namespace de

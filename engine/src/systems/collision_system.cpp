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
    return Box<float>(transform.position.x, transform.position.y,
                      dimension.width, dimension.height);
}

/// Layers whose tiles are solid.
constexpr Layer SolidLayers[] = {Layer::Collision, Layer::Overlay};

} // namespace

void CollisionSystem::resolveAABB(entt::registry& registry, entt::entity entity,
                                  Box<float>& box, const Box<float>& obstacle)
{
    const float overlapX = std::min(box.right(), obstacle.right()) -
                           std::max(box.left(), obstacle.left());
    const float overlapY = std::min(box.bottom(), obstacle.bottom()) -
                           std::max(box.top(), obstacle.top());

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
        const float direction = box.left() < obstacle.left() ? -1.0f : 1.0f;
        transform.position.x = transform.position.x + direction * overlapX;
        if (velocity != nullptr)
        {
            velocity->velocity.x = 0.0f;
        }
    }
    else
    {
        const float direction = box.top() < obstacle.top() ? -1.0f : 1.0f;
        transform.position.y = transform.position.y + direction * overlapY;
        if (velocity != nullptr)
        {
            velocity->velocity.y = 0.0f;
        }
    }

    // Keep the working box in step, so the next obstacle this frame is tested
    // against the corrected position.
    box.x = transform.position.x;
    box.y = transform.position.y;
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

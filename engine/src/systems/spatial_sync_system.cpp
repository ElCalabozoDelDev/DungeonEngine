#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/systems/spatial_sync_system.hpp>

namespace de
{
void SpatialSyncSystem::run(entt::registry& registry)
{
    auto* spatial = registry.ctx().find<SpatialIndex>();
    if (spatial == nullptr)
    {
        return;
    }

    // Only entities that can move are worth checking; update() itself costs a
    // box comparison for those that did not actually move.
    for (auto entity : registry.view<TransformComponent, VelocityComponent>())
    {
        spatial->update(entity);
    }
}

} // namespace de

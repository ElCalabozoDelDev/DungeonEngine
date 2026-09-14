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

    // Every entity in the Object layer, not just those with a
    // VelocityComponent: a game that moves sprites itself (the snake steps
    // by stride, the bat integrates its own velocity) left them filed where
    // they spawned. Tile layers do not move and are not re-checked.
    spatial->updateLayer(registry, Layer::Object);
}

} // namespace de

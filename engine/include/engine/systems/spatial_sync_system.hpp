#ifndef DE_SYSTEMS_SPATIAL_SYNC_SYSTEM_HPP
#define DE_SYSTEMS_SPATIAL_SYNC_SYSTEM_HPP

#include <engine/systems/system.hpp>
#include <entt/entt.hpp>

namespace de
{
/// Re-files moving entities in the spatial index.
///
/// Runs as a fixed system, so anything integration moved is back in the right
/// quadtree node before collision or rendering query it. Without this, an
/// entity stays filed under the position it had when it was inserted: queries
/// near where it actually is never find it, and queries near where it used to
/// be still do.
class SpatialSyncSystem final : public System
{
public:
    void run(entt::registry& registry) override;
};

} // namespace de

#endif // DE_SYSTEMS_SPATIAL_SYNC_SYSTEM_HPP

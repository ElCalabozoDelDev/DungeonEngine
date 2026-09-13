#ifndef DE_SYSTEMS_SPATIAL_SYNC_SYSTEM_HPP
#define DE_SYSTEMS_SPATIAL_SYNC_SYSTEM_HPP

#include <engine/systems/system.hpp>
#include <entt/entt.hpp>

namespace de
{
/// Re-files the Object layer of the spatial index.
///
/// Registered with addFixedSystemLast(), so it runs after every system that
/// moves things in the step -- the game's included -- and the index is right
/// before collision or rendering query it. Without this, an
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

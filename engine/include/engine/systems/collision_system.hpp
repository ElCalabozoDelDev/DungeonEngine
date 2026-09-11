#ifndef DE_SYSTEMS_COLLISION_SYSTEM_HPP
#define DE_SYSTEMS_COLLISION_SYSTEM_HPP

#include <engine/spatial/quadtree.hpp>
#include <engine/systems/system.hpp>
#include <entt/entt.hpp>

namespace de
{
/// Keeps solid bodies out of solid tiles.
///
/// Runs as a fixed system, after integration has moved things and
/// SpatialSyncSystem has re-filed them. Resolution is the classic
/// minimum-translation AABB push: the axis with the smaller overlap is the one
/// the body came in through, so that is the one it gets pushed back along.
class CollisionSystem final : public System
{
public:
    void run(entt::registry& registry) override;

    /// Pushes `box` out of `obstacle` along the axis of least overlap and
    /// writes the correction back into the transform and velocity.
    ///
    /// One function, not the two byte-identical ones the prototype had.
    static void resolveAABB(entt::registry& registry, entt::entity entity,
                            Box<float>& box, const Box<float>& obstacle);
};

} // namespace de

#endif // DE_SYSTEMS_COLLISION_SYSTEM_HPP

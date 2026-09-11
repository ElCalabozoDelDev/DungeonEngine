#ifndef DE_COMPONENTS_SOLID_BODY_COMPONENT_HPP
#define DE_COMPONENTS_SOLID_BODY_COMPONENT_HPP

namespace de
{
/// Marks an entity that cannot pass through the level's solid tiles.
///
/// CollisionSystem pushes these back out of anything they overlap on the
/// Collision and Overlay layers. Entities without it (loose items, decoration)
/// are ignored.
struct SolidBodyComponent
{
};

} // namespace de

#endif // DE_COMPONENTS_SOLID_BODY_COMPONENT_HPP

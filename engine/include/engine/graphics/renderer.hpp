#ifndef DE_GRAPHICS_RENDERER_HPP
#define DE_GRAPHICS_RENDERER_HPP

#include <engine/graphics/camera2d.hpp>
#include <entt/entt.hpp>
#include <span>

namespace de
{
/// Draws each entity's current sprite-sheet frame: TransformComponent,
/// TextureComponent, SpriteComponent and DimensionComponent are required.
///
/// Entities arrive as a span rather than a vector by value, and the camera is
/// the single source of the world-to-screen mapping.
void drawSprites(entt::registry& registry,
                 std::span<const entt::entity> sprites, const Camera2D& camera);

/// Draws each tile from the tileset the loader resolved for it. Tiles with no
/// tileset are skipped.
void drawTiles(entt::registry& registry, std::span<const entt::entity> tiles,
               const Camera2D& camera);

} // namespace de

#endif // DE_GRAPHICS_RENDERER_HPP

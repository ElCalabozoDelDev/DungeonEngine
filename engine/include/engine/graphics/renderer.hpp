#ifndef DE_GRAPHICS_RENDERER_HPP
#define DE_GRAPHICS_RENDERER_HPP

#include <SDL.h>
#include <engine/components/dimension_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/tile_component.hpp>
#include <engine/components/tile_set_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/graphics/camera2d.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/spatial/visibility_map.hpp>
#include <entt/entt.hpp>
#include <span>
#include <vector>

namespace de
{
/// Draw helpers shared by the render passes.
///
/// Entities arrive as a span rather than a vector by value, and the camera is
/// the single source of the world-to-screen mapping.
class Renderer
{
public:
    static constexpr Uint8 RememberedTileAlpha = 90;

    static void renderSprites(entt::registry& registry,
                              std::span<const entt::entity> visibleSprites,
                              const Camera2D& camera)
    {
        const auto& textures = registry.ctx().get<TextureCache>();
        const auto* visibility = registry.ctx().find<VisibilityMap>();

        for (auto entity : visibleSprites)
        {
            const auto& trf = registry.get<TransformComponent>(entity);
            if (visibility != nullptr && visibility->enabled)
            {
                // Entities are only drawn in the live FOV — remembered fog
                // keeps geometry, not moving threats or loot tells.
                if (visibility->appearanceAt(trf.position.getX(),
                                             trf.position.getY()) !=
                    VisibilityMap::Appearance::Visible)
                {
                    continue;
                }
            }

            const auto& tex = registry.get<TextureComponent>(entity);
            const auto& spr = registry.get<SpriteComponent>(entity);
            const auto& dim = registry.get<DimensionComponent>(entity);

            TextureCache::DrawParams params;
            params.source = TextureCache::frameRect(
                static_cast<int>(dim.width), static_cast<int>(dim.height),
                spr.spriteRow, spr.currentSprite);
            params.position = camera.worldToScreen(trf.position);
            params.scale = camera.zoom;
            textures.draw(tex.id, params);
        }
    }

    static void renderTiles(entt::registry& registry,
                            std::span<const entt::entity> visibleTiles,
                            const Camera2D& camera)
    {
        const auto& textures = registry.ctx().get<TextureCache>();
        const auto* visibility = registry.ctx().find<VisibilityMap>();

        for (auto entity : visibleTiles)
        {
            const auto& trf = registry.get<TransformComponent>(entity);
            Uint8 alpha = 255;
            if (visibility != nullptr && visibility->enabled)
            {
                switch (visibility->appearanceAt(trf.position.getX(),
                                                 trf.position.getY()))
                {
                    case VisibilityMap::Appearance::Hidden:
                        continue;
                    case VisibilityMap::Appearance::Remembered:
                        alpha = RememberedTileAlpha;
                        break;
                    case VisibilityMap::Appearance::Visible:
                        break;
                }
            }

            const auto& tile = registry.get<TileComponent>(entity);

            // The tileset was resolved at load time
            if (tile.tileset == entt::null)
            {
                continue;
            }
            const auto& tileset = registry.get<TileSetComponent>(tile.tileset);
            const auto& texture = registry.get<TextureComponent>(tile.tileset);
            const auto& dimension =
                registry.get<DimensionComponent>(tile.tileset);

            if (tileset.numColumns <= 0)
            {
                continue;
            }

            // Tile position inside the tileset
            const int tileIndex = tile.tileId - tileset.firstGridID;
            const int tileRow = tileIndex / tileset.numColumns;
            const int tileCol = tileIndex % tileset.numColumns;

            TextureCache::DrawParams params;
            params.source = TextureCache::tileRect(
                static_cast<int>(dimension.width),
                static_cast<int>(dimension.height), tileRow, tileCol,
                tileset.margin, tileset.spacing);
            params.position = camera.worldToScreen(trf.position);
            params.scale = camera.zoom;
            params.alpha = alpha;
            textures.draw(texture.id, params);
        }
    }
};

} // namespace de

#endif // DE_GRAPHICS_RENDERER_HPP

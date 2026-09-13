#include <engine/components/dimension_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/tile_component.hpp>
#include <engine/components/tile_set_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/graphics/renderer.hpp>
#include <engine/graphics/texture_cache.hpp>

namespace de
{
void drawSprites(entt::registry& registry,
                 std::span<const entt::entity> sprites, const Camera2D& camera)
{
    const auto& textures = registry.ctx().get<TextureCache>();

    for (auto entity : sprites)
    {
        const auto& trf = registry.get<TransformComponent>(entity);
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

void drawTiles(entt::registry& registry, std::span<const entt::entity> tiles,
               const Camera2D& camera)
{
    const auto& textures = registry.ctx().get<TextureCache>();

    for (auto entity : tiles)
    {
        const auto& trf = registry.get<TransformComponent>(entity);
        const auto& tile = registry.get<TileComponent>(entity);

        // The tileset was resolved at load time
        if (tile.tileset == entt::null)
        {
            continue;
        }
        const auto& tileset = registry.get<TileSetComponent>(tile.tileset);
        const auto& texture = registry.get<TextureComponent>(tile.tileset);
        const auto& dimension = registry.get<DimensionComponent>(tile.tileset);

        if (tileset.numColumns <= 0)
        {
            continue;
        }

        // Tile position inside the tileset
        const int tileIndex = tile.tileId - tileset.firstGridID;
        const int tileRow = tileIndex / tileset.numColumns;
        const int tileCol = tileIndex % tileset.numColumns;

        TextureCache::DrawParams params;
        params.source =
            TextureCache::tileRect(static_cast<int>(dimension.width),
                                   static_cast<int>(dimension.height), tileRow,
                                   tileCol, tileset.margin, tileset.spacing);
        params.position = camera.worldToScreen(trf.position);
        params.scale = camera.zoom;
        textures.draw(texture.id, params);
    }
}

} // namespace de

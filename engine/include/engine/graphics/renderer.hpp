#ifndef DE_GRAPHICS_RENDERER_HPP
#define DE_GRAPHICS_RENDERER_HPP

#include <SDL.h>
#include <engine/components/camera_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/tile_component.hpp>
#include <engine/components/tile_set_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/graphics/sdl_resources.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/config.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace de
{
class Renderer
{
public:
    static void renderSprites(entt::registry& registry,
                              std::vector<entt::entity> visibleSprites,
                              float offsetX, float offsetY, float zoomLevel)
    {
        SDL_Renderer* renderer = registry.ctx().get<MainRenderer>().get();
        const auto& textures = registry.ctx().get<TextureCache>();
        const auto& config = registry.ctx().get<Config>();
        // Camera position
        auto cameraPos = registry
                             .get<TransformComponent>(
                                 registry.view<CameraComponent>().front())
                             .position;
        for (auto entity : visibleSprites)
        {
            auto& trf = registry.get<TransformComponent>(entity);
            auto& tex = registry.get<TextureComponent>(entity);
            auto& spr = registry.get<SpriteComponent>(entity);
            auto& dim = registry.get<DimensionComponent>(entity);
            // Offset the sprite by the camera
            int renderX = static_cast<int>(
                (trf.position.getX() - cameraPos.getX()) * zoomLevel +
                config.screenWidth / 2.0f + offsetX);
            int renderY = static_cast<int>(
                (trf.position.getY() - cameraPos.getY()) * zoomLevel +
                config.screenHeight / 2.0f + offsetY);

            TextureCache::DrawParams params;
            params.source = TextureCache::frameRect(
                dim.width, dim.height, spr.spriteRow, spr.currentSprite);
            params.position = SDL_Point{renderX, renderY};
            params.scale = zoomLevel;
            textures.draw(tex.id, params);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }

    static void renderTiles(entt::registry& registry,
                            std::vector<entt::entity> visibleTiles,
                            float offsetX, float offsetY, float zoomLevel)
    {
        SDL_Renderer* pRenderer = registry.ctx().get<MainRenderer>().get();
        const auto& textures = registry.ctx().get<TextureCache>();
        const auto& config = registry.ctx().get<Config>();
        // Camera position
        auto cameraPos = registry
                             .get<TransformComponent>(
                                 registry.view<CameraComponent>().front())
                             .position;

        for (auto entity : visibleTiles)
        {
            // Tile components
            auto& trf = registry.get<TransformComponent>(entity);
            auto& tile = registry.get<TileComponent>(entity);

            // The tileset was resolved at load time
            if (tile.tileset == entt::null)
            {
                continue;
            }
            auto& tileset = registry.get<TileSetComponent>(tile.tileset);
            auto& texture = registry.get<TextureComponent>(tile.tileset);
            auto& dimension = registry.get<DimensionComponent>(tile.tileset);

            // Tile position, offset by the camera
            int renderX = static_cast<int>(
                (trf.position.getX() - cameraPos.getX()) * zoomLevel +
                config.screenWidth / 2.0f + offsetX);
            int renderY = static_cast<int>(
                (trf.position.getY() - cameraPos.getY()) * zoomLevel +
                config.screenHeight / 2.0f + offsetY);

            // Tile position inside the tileset
            int tileIndex = tile.tileId - tileset.firstGridID;
            int tileRow = tileIndex / tileset.numColumns;
            int tileCol = tileIndex % tileset.numColumns;

            TextureCache::DrawParams params;
            params.source = TextureCache::tileRect(
                dimension.width, dimension.height, tileRow, tileCol,
                tileset.margin, tileset.spacing);
            params.position = SDL_Point{renderX, renderY};
            params.scale = zoomLevel;
            textures.draw(texture.id, params);
        }

        // Restore the draw colour
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255); // black
    }
};

} // namespace de

#endif // DE_GRAPHICS_RENDERER_HPP

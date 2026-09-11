#ifndef DE_GRAPHICS_RENDERER_HPP
#define DE_GRAPHICS_RENDERER_HPP
// #include "SDL2/SDL.h"
#include "entt/entt.hpp"
#include <engine/components/camera_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/tile_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/tile_set_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/graphics/texture_manager.hpp>
#include <engine/loaders/config.hpp>
#include <iostream>
#include <ostream>

namespace de
{
class Renderer
{
public:
    static void renderSprites(entt::registry& registry,
                              std::vector<entt::entity> visibleSprites,
                              float offsetX, float offsetY, float zoomLevel)
    {
        SDL_Renderer* renderer = registry.ctx().get<SDL_Renderer*>();
        auto config = registry.ctx().get<Config>();
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
            TextureManager::Instance()->drawFrame(
                tex.id, renderX, renderY, dim.width, dim.height, spr.spriteRow,
                spr.currentSprite, renderer, 0, 255, zoomLevel, SDL_FLIP_NONE);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }

    static void renderTiles(entt::registry& registry,
                            std::vector<entt::entity> visibleTiles,
                            float offsetX, float offsetY, float zoomLevel)
    {
        auto config = registry.ctx().get<Config>();
        auto* pRenderer = registry.ctx().get<SDL_Renderer*>();
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

            // Locate the tileset this tile belongs to
            entt::entity tilesetId = getTilesetByID(registry, tile.tileId);
            auto& tileset = registry.get<TileSetComponent>(tilesetId);
            auto& texture = registry.get<TextureComponent>(tilesetId);
            auto& dimension = registry.get<DimensionComponent>(tilesetId);

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

            // Draw the tile
            TextureManager::Instance()->drawTile(
                texture.id, tileset.margin, tileset.spacing, renderX, renderY,
                dimension.width, dimension.height, tileRow, tileCol, zoomLevel,
                pRenderer);
        }

        // Restore the draw colour
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255); // black
    }

private:
    static entt::entity getTilesetByID(entt::registry& registry, int tileID)
    {
        auto view = registry.view<TileLayerComponent>();
        for (auto entity : view)
        {
            auto& layer = view.get<TileLayerComponent>(entity);
            auto& tilesets = layer.tileSetEntities;
            for (int i = 0; i < tilesets.size(); i++)
            {
                if (i + 1 <= tilesets.size() - 1)
                {
                    auto tileset =
                        registry.get<TileSetComponent>(tilesets.at(i));
                    auto nextTileset =
                        registry.get<TileSetComponent>(tilesets.at(i + 1));
                    if (tileID >= tileset.firstGridID &&
                        tileID < nextTileset.firstGridID)
                    {
                        return tilesets.at(i);
                    }
                }
                else
                {
                    return tilesets.at(i);
                }
            }
        }

        std::cerr << "did not find tileset, returning empty tileset\n";
        entt::entity t;
        return t;
    }
};

} // namespace de

#endif // DE_GRAPHICS_RENDERER_HPP

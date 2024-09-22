#ifndef RENDERER_HPP
#define RENDERER_HPP

// #include "SDL2/SDL.h"
#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "entt/entt.hpp"
#include "components/tile_layer_component.hpp"
#include "components/tile_set_component.hpp"
#include "core/texture_manager.hpp"
#include "components/position_component.hpp"
#include "components/sprite_component.hpp"
#include "components/texture_component.hpp"
#include "loaders/config.hpp"
#include <iostream>

class Renderer {
public:
    static void renderSprites(entt::registry &registry, std::vector<entt::entity> visibleSprites) {
        SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();

        for (auto entity : visibleSprites) {
            auto &pos = registry.get<PositionComponent>(entity);
            auto &tex = registry.get<TextureComponent>(entity);
            auto &spr = registry.get<SpriteComponent>(entity);
            TheTextureManager::Instance()->drawFrame(tex.id, pos.position.getX(), pos.position.getY(), spr.spriteWidth, spr.spriteHeight, spr.spriteRow, spr.currentSprite, renderer, 0, 255, SDL_FLIP_NONE);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }

    static void renderTiles(entt::registry& registry, std::vector<entt::entity> visibleTiles) {
        auto config = registry.ctx().get<Config>();
        auto *pRenderer = registry.ctx().get<SDL_Renderer *>();
        for (auto entity : visibleTiles) {
            auto& layer = registry.get<TileLayerComponent>(entity);
            auto& pos = registry.get<PositionComponent>(entity);
            auto& tileIDs = layer.tileIDs;

            
            // Obtener la posición de la cámara
            auto cameraPos = registry.get<PositionComponent>(registry.view<CameraComponent>().front()).position;
            auto cameraBounds = registry.get<CameraBoundsComponent>(registry.view<CameraComponent>().front());

            float offsetX = 0.0f;
            float offsetY = 0.0f;

            if (cameraBounds.levelWidth < config.screenWidth) {
                offsetX = static_cast<float>(config.screenWidth - cameraBounds.levelWidth) / 2.0f;
            }

            if (cameraBounds.levelHeight < config.screenHeight) {
                offsetY = static_cast<float>(config.screenHeight - cameraBounds.levelHeight) / 2.0f;
            }

            int x = pos.position.getX() / layer.tileSize;
            int y = pos.position.getY() / layer.tileSize;
            for (int i = 0; i < layer.numRows; i++) {
                for (int j = 0; j < layer.numColumns; j++) {

                    int id = tileIDs[i + y][j + x];
                    if (id == 0) {
                        continue;
                    }
                    // Posición del tile en la pantalla (ajustada por la cámara)
                    int renderX = (j * layer.tileSize) - cameraPos.m_x + offsetX;
                    int renderY = (i * layer.tileSize) - cameraPos.m_y + offsetY;
                    
                    // Verificar que el tile esté dentro de la pantalla antes de dibujarlo
                    if (renderX + layer.tileSize < 0 || renderX > config.screenWidth ||
                        renderY + layer.tileSize < 0 || renderY > config.screenHeight) {
                        continue;  // Tile fuera de la pantalla, no dibujar
                    }


                    entt::entity tilesetId = getTilesetByID(registry, id);
                    auto tileset = registry.get<TileSetComponent>(tilesetId);
                    auto texture = registry.get<TextureComponent>(tilesetId);
                    id--;

                    TheTextureManager::Instance()->drawTile(texture.id, tileset.margin, tileset.spacing,
                        renderX, renderY, layer.tileSize, layer.tileSize,
                        (id - (tileset.firstGridID - 1)) / tileset.numColumns,
                        (id - (tileset.firstGridID - 1)) % tileset.numColumns,
                        pRenderer);
                }
            }
        }
    }
private:
    static entt::entity getTilesetByID(entt::registry& registry, int tileID)
    {
        auto view = registry.view<TileLayerComponent>();
        for (auto entity : view)
        {
            auto& layer = view.get<TileLayerComponent>(entity);
            auto& tilesets = layer.tilesets;
            for(int i = 0; i < tilesets->size(); i++)
            {
                if( i + 1 <= tilesets->size() - 1)
                {
                    auto tileset = registry.get<TileSetComponent>( tilesets->at(i));
                    auto nextTileset = registry.get<TileSetComponent>(tilesets->at(i + 1));
                    if(tileID >= tileset.firstGridID && tileID <nextTileset.firstGridID)
                    {
                        return tilesets->at(i);
                    }
                }
                else
                {
                    return tilesets->at(i);
                }
            }

        }
        
        std::cout << "did not find tileset, returning empty tileset\n";
        entt::entity t;
        return t;
    }

};
#endif // RENDERER_HPP
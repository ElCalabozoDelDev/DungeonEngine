#ifndef RENDERER_HPP
#define RENDERER_HPP

// #include "SDL2/SDL.h"
#include "entt/entt.hpp"
#include "imgui.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "components/tile_layer_component.hpp"
#include "components/tile_set_component.hpp"
#include "core/texture_manager.hpp"
#include "components/position_component.hpp"
#include "components/sprite_component.hpp"
#include "components/texture_component.hpp"
#include "loaders/config.hpp"
#include "widgets/gui.hpp"
#include <iostream>
class Renderer {
public:
    static void renderSprites(entt::registry &registry, std::vector<entt::entity> visibleSprites) {
        SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
        auto config = registry.ctx().get<Config>();
        // Obtener la posición de la cámara
        auto cameraPos = registry.get<PositionComponent>(registry.view<CameraComponent>().front()).position;

        for (auto entity : visibleSprites) {
            auto &pos = registry.get<PositionComponent>(entity);
            auto &tex = registry.get<TextureComponent>(entity);
            auto &spr = registry.get<SpriteComponent>(entity);

            // Ajustar la posición del sprite en base a la cámara
            int renderX = static_cast<int>(pos.position.getX() - cameraPos.m_x + config.screenWidth / 2.0f);
            int renderY = static_cast<int>(pos.position.getY() - cameraPos.m_y + config.screenHeight / 2.0f);

            TheTextureManager::Instance()->drawFrame(tex.id, renderX, renderY, spr.spriteWidth, spr.spriteHeight, spr.spriteRow, spr.currentSprite, renderer, 0, 255, SDL_FLIP_NONE);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }

    static void renderTiles(entt::registry& registry, std::vector<entt::entity> visibleTiles) {
        auto config = registry.ctx().get<Config>();
        auto *pRenderer = registry.ctx().get<SDL_Renderer *>();

        // Obtener la posición de la cámara
        auto cameraPos = registry.get<PositionComponent>(registry.view<CameraComponent>().front()).position;

        for (auto entity : visibleTiles) {
            auto& layer = registry.get<TileLayerComponent>(entity);
            auto& pos = registry.get<PositionComponent>(entity);
            auto& tileIDs = layer.tileIDs;

            int x = pos.position.getX() / layer.tileSize;
            int y = pos.position.getY() / layer.tileSize;
            float offsetX = (config.screenWidth - registry.get<CameraBoundsComponent>(registry.view<CameraComponent>().front()).levelWidth) / 2.0f;
            float offsetY = (config.screenHeight - registry.get<CameraBoundsComponent>(registry.view<CameraComponent>().front()).levelHeight) / 2.0f;
            
            offsetX = std::max(0.0f, offsetX);  // Si el mapa es más grande, no aplicar offset
            offsetY = std::max(0.0f, offsetY);
            
            for (int i = 0; i < layer.numRows; i++) {
                for (int j = 0; j < layer.numColumns; j++) {

                    int id = tileIDs[i + y][j + x];
                    if (id == 0) {
                        continue;  // Saltar tiles vacíos
                    }

                    // Posición del tile en la pantalla (ajustada por la cámara)
                    int renderX = (j * layer.tileSize) - cameraPos.m_x + config.screenWidth / 2.0f + offsetX;
                    int renderY = (i * layer.tileSize) - cameraPos.m_y + config.screenHeight / 2.0f + offsetY;
                    // Verificar que el tile esté dentro de la pantalla antes de dibujarlo
                    if (renderX + layer.tileSize < 0 || renderX > config.screenWidth ||
                        renderY + layer.tileSize < 0 || renderY > config.screenHeight) {
                        continue;  // Tile fuera de la pantalla, no dibujar
                    }

                    entt::entity tilesetId = getTilesetByID(registry, id);
                    auto tileset = registry.get<TileSetComponent>(tilesetId);
                    auto texture = registry.get<TextureComponent>(tilesetId);
                    id--;

                    // Renderizar el tile ajustado por la cámara
                    TheTextureManager::Instance()->drawTile(texture.id, tileset.margin, tileset.spacing,
                        renderX, renderY, layer.tileSize, layer.tileSize,
                        (id - (tileset.firstGridID - 1)) / tileset.numColumns,
                        (id - (tileset.firstGridID - 1)) % tileset.numColumns,
                        pRenderer);
                }
            }
        }
    }

    static void renderGUI(entt::registry &registry) {
        SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
        registry.view<std::unique_ptr<gui::WidgetComponent>>().each([&registry](auto entity, auto &widget_component) {
            widget_component->frame_begin();
            widget_component->frame_update(registry);
            widget_component->frame_end();
        });
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
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
#ifndef RENDERER_HPP
#define RENDERER_HPP

// #include "SDL2/SDL.h"
#include "components/tile_component.hpp"
#include "entt/entt.hpp"
#include "graphics/render.hpp"
#include "imgui.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include "components/camera_component.hpp"
#include "components/tile_layer_component.hpp"
#include "components/tile_set_component.hpp"
#include "core/texture_manager.hpp"
#include "components/transform_component.hpp"
#include "components/sprite_component.hpp"
#include "components/texture_component.hpp"
#include "loaders/config.hpp"
#include "widgets/gui.hpp"
#include <iostream>
#include <ostream>

class Renderer {
public:
    static void renderSprites(entt::registry &registry, std::vector<entt::entity> visibleSprites) {
        SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
        auto config = registry.ctx().get<Config>();
        // Obtener la posición de la cámara
        auto cameraPos = registry.get<TransformComponent>(registry.view<CameraComponent>().front()).position;
        for (auto entity : visibleSprites) {
            auto &trf = registry.get<TransformComponent>(entity);
            auto &tex = registry.get<TextureComponent>(entity);
            auto &spr = registry.get<SpriteComponent>(entity);
            // Ajustar la posición del sprite en base a la cámara
            int renderX = static_cast<int>(trf.position.getX() - cameraPos.m_x + config.screenWidth / 2.0f);
            int renderY = static_cast<int>(trf.position.getY() - cameraPos.m_y + config.screenHeight / 2.0f);

            TextureManager::Instance()->drawFrame(tex.id, renderX, renderY, spr.spriteWidth, spr.spriteHeight, spr.spriteRow, spr.currentSprite, renderer, 0, 255, SDL_FLIP_NONE);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }

    static void renderTiles(entt::registry& registry, std::vector<entt::entity> visibleTiles) {
        auto config = registry.ctx().get<Config>();
        auto *pRenderer = registry.ctx().get<SDL_Renderer *>();
        // Obtener la posición de la cámara
        auto cameraPos = registry.get<TransformComponent>(registry.view<CameraComponent>().front()).position;

        for (auto entity : visibleTiles) {
            // Obtener los componentes del tile
            auto& trf = registry.get<TransformComponent>(entity);
            auto& tile = registry.get<TileComponent>(entity);

            // Encontrar el tileset correspondiente al tile
            entt::entity tilesetId = getTilesetByID(registry, tile.tileId);
            auto& tileset = registry.get<TileSetComponent>(tilesetId);
            auto& texture = registry.get<TextureComponent>(tilesetId);

            // Calcular la posición del tile ajustada por la cámara
            int renderX = static_cast<int>(trf.position.getX() - cameraPos.m_x + config.screenWidth / 2.0f);
            int renderY = static_cast<int>(trf.position.getY() - cameraPos.m_y + config.screenHeight / 2.0f);

            // Verificar si el tile está dentro del área visible de la pantalla
            if (renderX + tileset.tileWidth < 0 || renderX > config.screenWidth ||
                renderY + tileset.tileHeight < 0 || renderY > config.screenHeight) {
                continue;  // Tile fuera de la pantalla, no dibujar
            }

            // Calcular la posición del tile dentro del tileset
            int tileIndex = tile.tileId - tileset.firstGridID;
            int tileRow = tileIndex / tileset.numColumns;
            int tileCol = tileIndex % tileset.numColumns;

            // Renderizar el tile
            TextureManager::Instance()->drawTile(
                texture.id, tileset.margin, tileset.spacing,
                renderX, renderY, tileset.tileWidth, tileset.tileHeight,
                tileRow, tileCol, pRenderer
            );
        }

        // Opcional: establecer el color de fondo del renderizador
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);  // Fondo negro
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
    static void renderGraphics(entt::registry &registry, AABB &cameraView) {
        registry.view<std::shared_ptr<Render>>().each([&registry, &cameraView](entt::entity entity, std::shared_ptr<Render> &render) {
            if (render) {
                render->draw(registry, cameraView);
            }
        });
    }
private:
    static entt::entity getTilesetByID(entt::registry& registry, int tileID)
    {
        auto view = registry.view<TileLayerComponent>();
        for (auto entity : view)
        {
            auto& layer = view.get<TileLayerComponent>(entity);
            auto& tilesets = layer.tileSetEntities;
            for(int i = 0; i < tilesets.size(); i++)
            {
                if( i + 1 <= tilesets.size() - 1)
                {
                    auto tileset = registry.get<TileSetComponent>( tilesets.at(i));
                    auto nextTileset = registry.get<TileSetComponent>(tilesets.at(i + 1));
                    if(tileID >= tileset.firstGridID && tileID <nextTileset.firstGridID)
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
#endif // RENDERER_HPP
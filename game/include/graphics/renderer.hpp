#ifndef RENDERER_HPP
#define RENDERER_HPP

// #include "SDL2/SDL.h"
#include "components/tile_component.hpp"
#include "entt/entt.hpp"
#include "components/camera_component.hpp"
#include "components/tile_layer_component.hpp"
#include "components/tile_set_component.hpp"
#include "graphics/texture_manager.hpp"
#include "components/transform_component.hpp"
#include "components/sprite_component.hpp"
#include "components/texture_component.hpp"
#include "loaders/config.hpp"
#include <iostream>
#include <ostream>
#include <components/dimension_component.hpp>

class Renderer {
public:
	static void renderSprites(entt::registry& registry, std::vector<entt::entity> visibleSprites, float offsetX, float offsetY) {
		SDL_Renderer* renderer = registry.ctx().get<SDL_Renderer*>();
		auto config = registry.ctx().get<Config>();
		// Obtener la posición de la cámara
		auto cameraPos = registry.get<TransformComponent>(registry.view<CameraComponent>().front()).position;
		for (auto entity : visibleSprites) {
			auto& trf = registry.get<TransformComponent>(entity);
			auto& tex = registry.get<TextureComponent>(entity);
			auto& spr = registry.get<SpriteComponent>(entity);
			auto& dim = registry.get<DimensionComponent>(entity);
			// Ajustar la posición del sprite en base a la cámara
			int renderX = static_cast<int>(trf.position.getX() - cameraPos.getX() + config.screenWidth / 2.0f) + offsetX;
			int renderY = static_cast<int>(trf.position.getY() - cameraPos.getY() + config.screenHeight / 2.0f) + offsetY;

			TextureManager::Instance()->drawFrame(tex.id, renderX, renderY, dim.width, dim.height, spr.spriteRow, spr.currentSprite, renderer, 0, 255, SDL_FLIP_NONE);
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	}

	static void renderTiles(entt::registry& registry, std::vector<entt::entity> visibleTiles, float offsetX, float offsetY) {
		auto config = registry.ctx().get<Config>();
		auto* pRenderer = registry.ctx().get<SDL_Renderer*>();
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
			auto& dimension = registry.get<DimensionComponent>(tilesetId);

			// Calcular la posición del tile ajustada por la cámara
			int renderX = static_cast<int>(trf.position.getX() - cameraPos.getX() + config.screenWidth / 2.0f) + offsetX;
			int renderY = static_cast<int>(trf.position.getY() - cameraPos.getY() + config.screenHeight / 2.0f) + offsetY;

			// Calcular la posición del tile dentro del tileset
			int tileIndex = tile.tileId - tileset.firstGridID;
			int tileRow = tileIndex / tileset.numColumns;
			int tileCol = tileIndex % tileset.numColumns;

			// Renderizar el tile
			TextureManager::Instance()->drawTile(
				texture.id, tileset.margin, tileset.spacing,
				renderX, renderY, dimension.width, dimension.height,
				tileRow, tileCol, pRenderer
			);
		}

		// Opcional: establecer el color de fondo del renderizador
		SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);  // Fondo negro
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
					auto tileset = registry.get<TileSetComponent>(tilesets.at(i));
					auto nextTileset = registry.get<TileSetComponent>(tilesets.at(i + 1));
					if (tileID >= tileset.firstGridID && tileID < nextTileset.firstGridID)
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
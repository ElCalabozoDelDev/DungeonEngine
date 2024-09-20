#include "world/tile_layer.hpp"
#include "components/position_component.hpp"
#include "core/texture_manager.hpp"
#include "core/camera.hpp"
#include "loaders/config.hpp"
#include "components/tile_set_component.hpp"
#include "components/tile_layer_component.hpp"
#include "components/texture_component.hpp"

void TileLayer::render(entt::registry& registry)
{
    auto config = registry.ctx().get<Config>();
    auto *pRenderer = registry.ctx().get<SDL_Renderer *>();
    auto view = registry.view<TileLayerComponent, PositionComponent>();
    int x, y, x2, y2 = 0;
    for (auto entity : view)
    {
        auto& layer = view.get<TileLayerComponent>(entity);
        auto& pos = view.get<PositionComponent>(entity);
        auto& tileIDs = layer.tileIDs;
        x = pos.position.getX() / layer.tileSize;
        y = pos.position.getY() / layer.tileSize;
        
        x2 = int(pos.position.getX()) % layer.tileSize;
        y2 = int(pos.position.getY()) % layer.tileSize;
    
        for(int i = 0; i < layer.numRows; i++)
        {
            for(int j = 0; j < layer.numColumns; j++)
            {
                int id = tileIDs[i + y][j + x];
                
                if(id == 0)
                {
                    continue;
                }
                
                if(((j * layer.tileSize) - x2) - TheCamera::Instance()->getPosition(registry).m_x < -layer.tileSize || ((j * layer.tileSize) - x2) - TheCamera::Instance()->getPosition(registry).m_x > config.screenWidth)
                {
                    continue;
                }
                
                entt::entity tilesetId = getTilesetByID(registry, id);
                auto tileset = registry.get<TileSetComponent>(tilesetId);
                auto texture = registry.get<TextureComponent>(tilesetId);
                id--;

                TheTextureManager::Instance()->drawTile(texture.id, tileset.margin, tileset.spacing, ((j * layer.tileSize) - x2) - TheCamera::Instance()->getPosition(registry).m_x, ((i * layer.tileSize) - y2), layer.tileSize, layer.tileSize, (id - (tileset.firstGridID - 1)) / tileset.numColumns, (id - (tileset.firstGridID - 1)) % tileset.numColumns, pRenderer);    
            }
        }
    }
}

entt::entity TileLayer::getTilesetByID(entt::registry& registry, int tileID)
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
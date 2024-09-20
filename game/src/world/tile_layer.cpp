#include "world/tile_layer.hpp"
#include "core/texture_manager.hpp"
#include "core/camera.hpp"
#include "loaders/config.hpp"
TileLayer::TileLayer(int tileSize, int mapWidth, int mapHeight, const std::vector<TileSet>& tilesets) : m_tileSize(tileSize), m_tilesets(tilesets), m_position(0,0), m_velocity(0,0)
{
    m_numColumns = mapWidth;
    m_numRows = mapHeight;
    
    m_mapWidth = mapWidth;
}

void TileLayer::update(Level* pLevel)
{
}

void TileLayer::render(entt::registry& registry)
{
    auto config = registry.ctx().get<Config>();
    auto *pRenderer = registry.ctx().get<SDL_Renderer *>();
    int x, y, x2, y2 = 0;
    
    x = m_position.getX() / m_tileSize;
    y = m_position.getY() / m_tileSize;
    
    x2 = int(m_position.getX()) % m_tileSize;
    y2 = int(m_position.getY()) % m_tileSize;
    
    for(int i = 0; i < m_numRows; i++)
    {
        for(int j = 0; j < m_numColumns; j++)
        {
            int id = m_tileIDs[i + y][j + x];
            
            if(id == 0)
            {
                continue;
            }
            
            if(((j * m_tileSize) - x2) - TheCamera::Instance()->getPosition(registry).m_x < -m_tileSize || ((j * m_tileSize) - x2) - TheCamera::Instance()->getPosition(registry).m_x > config.screenWidth)
            {
                continue;
            }
            
            TileSet tileset = getTilesetByID(id);
            
            id--;

            TheTextureManager::Instance()->drawTile(tileset.name, tileset.margin, tileset.spacing, ((j * m_tileSize) - x2) - TheCamera::Instance()->getPosition(registry).m_x, ((i * m_tileSize) - y2), m_tileSize, m_tileSize, (id - (tileset.firstGridID - 1)) / tileset.numColumns, (id - (tileset.firstGridID - 1)) % tileset.numColumns, pRenderer);    
        }
    }
}

TileSet TileLayer::getTilesetByID(int tileID)
{
    for(int i = 0; i < m_tilesets.size(); i++)
    {
        if( i + 1 <= m_tilesets.size() - 1)
        {
            if(tileID >= m_tilesets[i].firstGridID && tileID < m_tilesets[i + 1].firstGridID)
            {
                return m_tilesets[i];
            }
        }
        else
        {
            return m_tilesets[i];
        }
    }
    
    std::cout << "did not find tileset, returning empty tileset\n";
    TileSet t;
    return t;
}
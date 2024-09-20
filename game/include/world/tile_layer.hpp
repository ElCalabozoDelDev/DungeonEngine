#ifndef TILE_LAYER_HPP
#define TILE_LAYER_HPP

#include "core/vector_2d.hpp"
#include "world/layer.hpp"
#include "world/level.hpp"
#include "world/tile_set.hpp"
#include "entt/entt.hpp"
#include <vector>

class TileLayer : public Layer
{
public:
    
    TileLayer(int tileSize, int mapWidth, int mapHeight, const std::vector<TileSet>& tilesets);
    
    virtual ~TileLayer() {}
    
    virtual void update(Level* pLevel) override;
    virtual void render(entt::registry& registry) override;
    
    void setTileIDs(const std::vector<std::vector<int>>& data) { m_tileIDs = data; }
    void setTileSize(int tileSize) { m_tileSize = tileSize; }
    void setMapWidth(int mapWidth) { m_mapWidth = mapWidth; }
    int getMapWidth() { return m_mapWidth; }
    
    int getTileSize() { return m_tileSize; }
    
    const std::vector<std::vector<int>>& getTileIDs() { return m_tileIDs; }
    
    TileSet getTilesetByID(int tileID);
    
    const Vector2D getPosition() { return m_position; }
    
    void setPosition(Vector2D position) { m_position = position; }
    
    
private:
    
    int m_numColumns;
    int m_numRows;
    int m_tileSize;
    
    int m_mapWidth;
    
    Vector2D m_position;
    Vector2D m_velocity;
    Vector2D m_acceleration;
    
    float diff;
    
    const std::vector<TileSet>& m_tilesets;
    
    std::vector<std::vector<int>> m_tileIDs;
};

#endif
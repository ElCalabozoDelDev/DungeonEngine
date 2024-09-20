#ifndef TILE_LAYER_COMPONENT_HPP
#define TILE_LAYER_COMPONENT_HPP

#include <vector>
#include <entt/entity/fwd.hpp>

struct TileLayerComponent
{
    int tileSize;
    const std::vector<entt::entity> *tilesets;
    std::vector<std::vector<int>> tileIDs;
    int numColumns;
    int numRows;
    int mapWidth;
};

#endif

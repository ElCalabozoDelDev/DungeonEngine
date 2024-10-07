#ifndef TILE_LAYER_COMPONENT_HPP
#define TILE_LAYER_COMPONENT_HPP

#include <string>
#include <vector>
#include <entt/entity/fwd.hpp>

struct TileLayerComponent
{
    std::vector<entt::entity> tileEntities; // Entidades que representan cada tile en esta capa
    std::vector<entt::entity> tileSetEntities; // Entidades que representan cada tileset en esta capa
};

#endif

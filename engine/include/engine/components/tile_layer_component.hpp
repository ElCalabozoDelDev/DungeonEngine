#ifndef DE_COMPONENTS_TILE_LAYER_COMPONENT_HPP
#define DE_COMPONENTS_TILE_LAYER_COMPONENT_HPP
#include <entt/entity/fwd.hpp>
#include <string>
#include <vector>

namespace de
{
struct TileLayerComponent
{
    std::vector<entt::entity> tileEntities; // One entity per tile in this layer
    std::vector<entt::entity>
        tileSetEntities; // Tilesets available to this layer
};

} // namespace de

#endif // DE_COMPONENTS_TILE_LAYER_COMPONENT_HPP

#ifndef DE_COMPONENTS_TILE_COMPONENT_HPP
#define DE_COMPONENTS_TILE_COMPONENT_HPP

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

namespace de
{
struct TileComponent
{
    /// Global tile id from the .tmj.
    int tileId = 0;

    /// The tileset entity this tile is drawn from, resolved once by the
    /// loader. Previously the renderer searched every layer and tileset for
    /// this on every visible tile, every frame.
    entt::entity tileset = entt::null;
};

} // namespace de

#endif // DE_COMPONENTS_TILE_COMPONENT_HPP

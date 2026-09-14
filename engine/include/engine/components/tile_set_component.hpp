#ifndef DE_COMPONENTS_TILE_SET_COMPONENT_HPP
#define DE_COMPONENTS_TILE_SET_COMPONENT_HPP

namespace de
{
struct TileSetComponent
{
    int firstGridID = 1;
    int spacing = 0;
    int margin = 0;
    int numColumns = 0;
    int tileCount = 0;
};

} // namespace de

#endif // DE_COMPONENTS_TILE_SET_COMPONENT_HPP

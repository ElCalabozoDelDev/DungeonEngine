#ifndef DE_COMPONENTS_TILE_SET_COMPONENT_HPP
#define DE_COMPONENTS_TILE_SET_COMPONENT_HPP

namespace de
{
struct TileSetComponent
{
    int firstGridID;
    int spacing;
    int margin;
    int numColumns;
    int tileCount;
};

} // namespace de

#endif // DE_COMPONENTS_TILE_SET_COMPONENT_HPP

#ifndef TILESET_COMPONENT_HPP
#define TILESET_COMPONENT_HPP

struct TileSetComponent
{
    int firstGridID;
    int tileWidth;
    int tileHeight;
    int spacing;
    int margin;
    int width;
    int height;
    int numColumns;
    int tileCount;
};
#endif
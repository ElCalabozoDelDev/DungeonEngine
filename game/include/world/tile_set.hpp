#ifndef TILESET_HPP
#define TILESET_HPP

#include <string>
struct TileSet
{
    int firstGridID;
    int tileWidth;
    int tileHeight;
    int spacing;
    int margin;
    int width;
    int height;
    int numColumns;
    std::string name;
};
#endif
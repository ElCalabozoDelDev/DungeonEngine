#include <string>

#ifndef TILESET_HPP
#define TILESET_HPP
struct Tileset
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
#ifndef LEVEL_PARSER_HPP
#define LEVEL_PARSER_HPP

#include "tinyxml2.h"
#include "world/level.hpp"
#include <vector>

using namespace tinyxml2;

class LevelParser {
    public:
    Level *parseLevel(const char *levelFile);

    private:
    void parseTilesets(XMLElement *pTilesetRoot, std::vector<Tileset> *pTilesets);
    void parseTileLayer(XMLElement *pTileElement, std::vector<Layer *> *pLayers,
                        const std::vector<Tileset> *pTilesets);
    int m_tileSize;
    int m_width;
    int m_height;
};

#endif
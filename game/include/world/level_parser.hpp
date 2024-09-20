#ifndef LEVEL_PARSER_HPP
#define LEVEL_PARSER_HPP

#include "tinyxml2.h"
#include "world/level.hpp"
#include <entt/entity/fwd.hpp>
#include <vector>
#include "entt/entt.hpp"
using namespace tinyxml2;

class LevelParser {
    public:
    Level *parseLevel(entt::registry& registry, const char *levelFile);

    private:
    void parseTilesets(entt::registry& registry, XMLElement *pTilesetRoot, std::vector<TileSet> *pTilesets);
    void parseTileLayer(entt::registry& registry, XMLElement *pTileElement, std::vector<Layer *> *pLayers,
                        const std::vector<TileSet> *pTilesets);
    int m_tileSize;
    int m_width;
    int m_height;
};

#endif
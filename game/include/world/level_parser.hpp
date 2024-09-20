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
    void parseTextures(entt::registry& registry, XMLElement* pTextureRoot);
    void parseTilesets(entt::registry& registry, XMLElement *pTilesetRoot, std::vector<entt::entity> *pTilesets);
    void parseObjectLayer(entt::registry& registry, XMLElement* pObjectElement, std::vector<entt::entity> *pLayers, Level* pLevel);
    void parseTileLayer(entt::registry& registry, XMLElement *pTileElement, std::vector<entt::entity> *pLayers,
                        const std::vector<entt::entity> *pTilesets);
    int m_tileSize;
    int m_width;
    int m_height;
};

#endif
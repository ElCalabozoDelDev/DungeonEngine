#ifndef LEVEL_PARSER_HPP
#define LEVEL_PARSER_HPP

#include "tinyxml2.h"
#include <entt/entity/fwd.hpp>
#include <vector>
#include "entt/entt.hpp"
using namespace tinyxml2;

class TMXLoader {
    public:
    TMXLoader(std::vector<entt::entity> *entities) {
        m_pEntities = entities;
    }
    void loadLevel(entt::registry& registry, const char *levelFile);
    int getTileSize() const { return m_tileSize; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    private:
    void loadTextures(entt::registry& registry, XMLElement* pTextureRoot);
    void loadTilesets(entt::registry& registry, XMLElement *pTilesetRoot);
    void loadObjectLayer(entt::registry& registry, XMLElement* pObjectElement);
    void loadTileLayer(entt::registry& registry, XMLElement *pTileElement);
    int m_tileSize;
    int m_width;
    int m_height;
    std::vector<entt::entity> *m_pEntities;
    std::vector<entt::entity> m_tilesets;
	std::vector<entt::entity> m_layers;
};

#endif
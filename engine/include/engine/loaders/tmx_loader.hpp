#ifndef DE_LOADERS_TMX_LOADER_HPP
#define DE_LOADERS_TMX_LOADER_HPP

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <tinyxml2.h>
#include <vector>

namespace de
{
/// Loads a Tiled (.tmx) map into the registry: textures, tilesets, tile layers
/// and object layers.
///
/// Objects are tagged with ObjectTypeComponent carrying their Tiled `type`
/// string; turning "Player" into a PlayerComponent is the game's job.
class TMXLoader
{
public:
    explicit TMXLoader(std::vector<entt::entity>* entities)
        : m_pEntities(entities)
    {
    }

    void loadLevel(entt::registry& registry, const char* levelFile);

    int getTileWidth() const { return m_tilewidth; }
    int getTileHeight() const { return m_tileheight; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    void loadTextures(entt::registry& registry,
                      tinyxml2::XMLElement* pTextureRoot);
    void loadTilesets(entt::registry& registry,
                      tinyxml2::XMLElement* pTilesetRoot);
    void loadObjectLayer(entt::registry& registry,
                         tinyxml2::XMLElement* pObjectElement);
    void loadTileLayer(entt::registry& registry,
                       tinyxml2::XMLElement* pTileElement);

    int m_tilewidth = 0;
    int m_tileheight = 0;
    int m_width = 0;
    int m_height = 0;
    std::vector<entt::entity>* m_pEntities;
    std::vector<entt::entity> m_tilesets;
    std::vector<entt::entity> m_layers;
};

} // namespace de

#endif // DE_LOADERS_TMX_LOADER_HPP

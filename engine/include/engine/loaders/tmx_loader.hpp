#ifndef DE_LOADERS_TMX_LOADER_HPP
#define DE_LOADERS_TMX_LOADER_HPP

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <expected>
#include <filesystem>
#include <string>
#include <tinyxml2.h>
#include <vector>

namespace de
{
/// Loads a Tiled (.tmx) map into the registry: textures, tilesets, tile layers
/// and object layers.
///
/// Every path inside the file -- tileset images and texture properties alike
/// -- is resolved relative to the .tmx itself, so a level is self-contained
/// and does not depend on the working directory.
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

    /// Returns a message describing the problem instead of walking off a null
    /// element or silently producing a level full of garbage tiles.
    std::expected<void, std::string>
    loadLevel(entt::registry& registry, const std::filesystem::path& levelFile);

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
    std::expected<void, std::string>
    loadTileLayer(entt::registry& registry, tinyxml2::XMLElement* pTileElement);

    /// The tileset a global tile id belongs to: the one with the highest
    /// firstgid not above `gid`. Resolved once per tile at load time and
    /// cached in TileComponent, instead of being searched for on every tile
    /// of every frame.
    entt::entity tilesetFor(entt::registry& registry, int gid) const;

    std::filesystem::path m_levelDir;
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

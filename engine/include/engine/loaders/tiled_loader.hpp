#ifndef DE_LOADERS_TILED_LOADER_HPP
#define DE_LOADERS_TILED_LOADER_HPP

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <expected>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace de
{
/// Loads a Tiled JSON (.tmj) map into the registry: textures, tilesets, tile
/// layers and object layers.
///
/// Paths inside the file (tileset images, external .tsj sources) are resolved
/// relative to the .tmj itself.
///
/// Objects are tagged with ObjectTypeComponent carrying their Tiled `type`
/// string; turning "Player" into gameplay components is the game's job.
class TiledLoader
{
public:
    explicit TiledLoader(std::vector<entt::entity>* entities)
        : m_pEntities(entities)
    {
    }

    std::expected<void, std::string>
    loadLevel(entt::registry& registry, const std::filesystem::path& levelFile);

    int getTileWidth() const { return m_tilewidth; }
    int getTileHeight() const { return m_tileheight; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    std::expected<void, std::string>
    loadExternalTileset(entt::registry& registry, int firstGid,
                        const std::filesystem::path& sourcePath);
    std::expected<void, std::string>
    loadInlineTileset(entt::registry& registry, const nlohmann::json& tileset,
                      int firstGid);
    void loadObjectLayer(entt::registry& registry,
                         const nlohmann::json& objectGroup);
    std::expected<void, std::string> loadTileLayer(entt::registry& registry,
                                                   const nlohmann::json& layer,
                                                   int tileLayerCount);

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

#endif // DE_LOADERS_TILED_LOADER_HPP

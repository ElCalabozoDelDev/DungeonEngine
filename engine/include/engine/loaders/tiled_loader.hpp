#ifndef DE_LOADERS_TILED_LOADER_HPP
#define DE_LOADERS_TILED_LOADER_HPP

#include <entt/entt.hpp>
#include <expected>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace de
{
/// Which tile layers are drawn where, by name.
///
/// A map with a single tile layer draws it under the objects whatever it is
/// called, so a fresh Tiled map works without renaming anything.
struct TiledLayerNames
{
    /// Tagged BottomLayerComponent: drawn under the objects.
    std::vector<std::string> bottom{"Bottom", "Collision"};
    /// Tagged OverlayLayerComponent: drawn over the objects.
    std::vector<std::string> overlay{"Overlay"};
};

/// What a successful load produced.
struct LoadedLevel
{
    /// Every entity the load created -- tilesets, layers, tiles, objects and
    /// the level itself. Whoever loads the level owns them.
    std::vector<entt::entity> entities;

    /// Map size in tiles, and the size of one tile in world units.
    int width = 0;
    int height = 0;
    int tileWidth = 0;
    int tileHeight = 0;
};

/// Loads a Tiled JSON (.tmj) map into the registry: textures, tilesets, tile
/// layers and object layers.
///
/// Paths inside the file (tileset images, external .tsj sources) are resolved
/// relative to the .tmj itself.
///
/// Objects get a transform, a sprite and ObjectTypeComponent carrying their
/// Tiled `type` string; turning "Player" into gameplay components -- movement
/// included -- is the game's job.
class TiledLoader
{
public:
    explicit TiledLoader(TiledLayerNames names = {});

    /// On failure, every entity created before the error is destroyed again,
    /// so a bad map leaves nothing behind.
    std::expected<LoadedLevel, std::string>
    load(entt::registry& registry, const std::filesystem::path& levelFile);

private:
    std::expected<void, std::string>
    loadInto(entt::registry& registry, const std::filesystem::path& levelFile);
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
    entt::entity create(entt::registry& registry);

    TiledLayerNames m_names;
    std::filesystem::path m_levelDir;
    LoadedLevel m_level;
    std::vector<entt::entity> m_tilesets;
    std::vector<entt::entity> m_layers;
};

} // namespace de

#endif // DE_LOADERS_TILED_LOADER_HPP

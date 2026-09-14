#include <algorithm>
#include <cstdint>
#include <engine/components/animation_component.hpp>
#include <engine/components/bottom_layer_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/level_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/overlay_layer_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/tile_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/tile_set_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/vector_2d.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/tiled_loader.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

namespace de
{
namespace
{
bool contains(const std::vector<std::string>& names, const std::string& name)
{
    return std::find(names.begin(), names.end(), name) != names.end();
}

int jsonInt(const nlohmann::json& j, const char* key, int fallback = 0)
{
    if (!j.contains(key) || !j[key].is_number())
    {
        return fallback;
    }
    return j[key].get<int>();
}

std::string jsonString(const nlohmann::json& j, const char* key)
{
    if (!j.contains(key) || !j[key].is_string())
    {
        return {};
    }
    return j[key].get<std::string>();
}

} // namespace

TiledLoader::TiledLoader(TiledLayerNames names) : m_names(std::move(names)) {}

std::expected<LoadedLevel, std::string>
TiledLoader::load(entt::registry& registry,
                  const std::filesystem::path& levelFile)
{
    m_level = LoadedLevel{};
    m_tilesets.clear();
    m_layers.clear();

    if (auto result = loadInto(registry, levelFile); !result)
    {
        for (auto entity : m_level.entities)
        {
            if (registry.valid(entity))
            {
                registry.destroy(entity);
            }
        }
        m_level = LoadedLevel{};
        return std::unexpected(result.error());
    }
    return std::move(m_level);
}

entt::entity TiledLoader::create(entt::registry& registry)
{
    auto entity = registry.create();
    m_level.entities.push_back(entity);
    return entity;
}

std::expected<void, std::string>
TiledLoader::loadInto(entt::registry& registry,
                      const std::filesystem::path& levelFile)
{
    std::ifstream in(levelFile);
    if (!in)
    {
        return std::unexpected("could not read level '" + levelFile.string() +
                               "'");
    }

    nlohmann::json root;
    try
    {
        in >> root;
    }
    catch (const nlohmann::json::exception& ex)
    {
        return std::unexpected("could not parse level '" + levelFile.string() +
                               "': " + ex.what());
    }

    m_levelDir = levelFile.parent_path();

    m_level.tileWidth = jsonInt(root, "tilewidth");
    m_level.tileHeight = jsonInt(root, "tileheight");
    m_level.width = jsonInt(root, "width");
    m_level.height = jsonInt(root, "height");
    if (m_level.tileWidth <= 0 || m_level.tileHeight <= 0 ||
        m_level.width <= 0 || m_level.height <= 0)
    {
        return std::unexpected(
            "level '" + levelFile.string() +
            "' has a non-positive width/height/tilewidth/tileheight");
    }

    if (root.contains("properties") && root["properties"].is_array())
    {
        for (const auto& prop : root["properties"])
        {
            const std::string name = jsonString(prop, "name");
            const std::string value = jsonString(prop, "value");
            if (!name.empty() && !value.empty())
            {
                registry.ctx().get<TextureCache>().load(
                    name, (m_levelDir / value).lexically_normal().string());
            }
        }
    }

    if (root.contains("tilesets") && root["tilesets"].is_array())
    {
        for (const auto& tileset : root["tilesets"])
        {
            const int firstGid = jsonInt(tileset, "firstgid", 1);
            if (tileset.contains("source") && tileset["source"].is_string())
            {
                const auto source =
                    m_levelDir / tileset["source"].get<std::string>();
                if (auto result =
                        loadExternalTileset(registry, firstGid, source);
                    !result)
                {
                    return result;
                }
            }
            else if (auto result =
                         loadInlineTileset(registry, tileset, firstGid);
                     !result)
            {
                return result;
            }
        }
    }

    if (root.contains("layers") && root["layers"].is_array())
    {
        int tileLayerCount = 0;
        for (const auto& layer : root["layers"])
        {
            if (jsonString(layer, "type") == "tilelayer")
            {
                ++tileLayerCount;
            }
        }

        for (const auto& layer : root["layers"])
        {
            const std::string type = jsonString(layer, "type");
            if (type == "objectgroup")
            {
                loadObjectLayer(registry, layer);
            }
            else if (type == "tilelayer")
            {
                if (auto result =
                        loadTileLayer(registry, layer, tileLayerCount);
                    !result)
                {
                    return result;
                }
            }
        }
    }

    auto levelEntity = create(registry);
    auto& levelComponent = registry.emplace<LevelComponent>(levelEntity);
    levelComponent.tilesets = m_tilesets;
    levelComponent.layers = m_layers;

    return {};
}

std::expected<void, std::string>
TiledLoader::loadExternalTileset(entt::registry& registry, int firstGid,
                                 const std::filesystem::path& sourcePath)
{
    std::ifstream in(sourcePath);
    if (!in)
    {
        return std::unexpected("could not read tileset '" +
                               sourcePath.string() + "'");
    }

    nlohmann::json tileset;
    try
    {
        in >> tileset;
    }
    catch (const nlohmann::json::exception& ex)
    {
        return std::unexpected("could not parse tileset '" +
                               sourcePath.string() + "': " + ex.what());
    }

    // Image paths inside a .tsj are relative to the tileset file.
    const auto previousDir = m_levelDir;
    m_levelDir = sourcePath.parent_path();
    auto result = loadInlineTileset(registry, tileset, firstGid);
    m_levelDir = previousDir;
    return result;
}

std::expected<void, std::string>
TiledLoader::loadInlineTileset(entt::registry& registry,
                               const nlohmann::json& tileset, int firstGid)
{
    const std::string name = jsonString(tileset, "name");
    const std::string image = jsonString(tileset, "image");
    if (name.empty() || image.empty())
    {
        std::cerr << "TiledLoader: skipping a tileset with no name or image\n";
        return {};
    }

    auto tileSetEntity = create(registry);
    auto& set = registry.emplace<TileSetComponent>(tileSetEntity);
    auto& dimension = registry.emplace<DimensionComponent>(tileSetEntity);
    registry.emplace<TextureComponent>(tileSetEntity, name);

    const int imageWidth = jsonInt(tileset, "imagewidth");
    set.firstGridID = firstGid;
    dimension.width = static_cast<float>(jsonInt(tileset, "tilewidth"));
    dimension.height = static_cast<float>(jsonInt(tileset, "tileheight"));
    set.spacing = jsonInt(tileset, "spacing");
    set.margin = jsonInt(tileset, "margin");
    set.tileCount = jsonInt(tileset, "tilecount");
    const int tileWidth = jsonInt(tileset, "tilewidth");
    set.numColumns = tileWidth + set.spacing > 0
                         ? imageWidth / (tileWidth + set.spacing)
                         : 0;

    registry.ctx().get<TextureCache>().load(
        name, (m_levelDir / image).lexically_normal().string());

    m_tilesets.push_back(tileSetEntity);
    return {};
}

entt::entity TiledLoader::tilesetFor(entt::registry& registry, int gid) const
{
    entt::entity best = entt::null;
    int bestFirstGid = 0;
    for (auto tilesetEntity : m_tilesets)
    {
        const auto& tileset = registry.get<TileSetComponent>(tilesetEntity);
        if (tileset.firstGridID <= gid && tileset.firstGridID >= bestFirstGid)
        {
            best = tilesetEntity;
            bestFirstGid = tileset.firstGridID;
        }
    }
    return best;
}

void TiledLoader::loadObjectLayer(entt::registry& registry,
                                  const nlohmann::json& objectGroup)
{
    if (!objectGroup.contains("objects") || !objectGroup["objects"].is_array())
    {
        return;
    }

    for (const auto& object : objectGroup["objects"])
    {
        auto entity = create(registry);

        int numFrames = 1;
        int spriteRow = 0;
        int spriteCol = 0;
        float animationTime = 0.0f;
        std::string textureID;
        const std::string type = jsonString(object, "type");

        const float x = object.contains("x") && object["x"].is_number()
                            ? object["x"].get<float>()
                            : 0.0f;
        const float y = object.contains("y") && object["y"].is_number()
                            ? object["y"].get<float>()
                            : 0.0f;
        const int width = jsonInt(object, "width");
        const int height = jsonInt(object, "height");

        if (object.contains("properties") && object["properties"].is_array())
        {
            for (const auto& prop : object["properties"])
            {
                const std::string propertyName = jsonString(prop, "name");
                if (propertyName.empty() || !prop.contains("value"))
                {
                    continue;
                }
                if (propertyName == "totalFrames" && prop["value"].is_number())
                {
                    numFrames = prop["value"].get<int>();
                }
                else if (propertyName == "textureID" &&
                         prop["value"].is_string())
                {
                    textureID = prop["value"].get<std::string>();
                }
                else if (propertyName == "spriteRow" &&
                         prop["value"].is_number())
                {
                    spriteRow = prop["value"].get<int>();
                }
                else if (propertyName == "spriteCol" &&
                         prop["value"].is_number())
                {
                    spriteCol = prop["value"].get<int>();
                }
                else if (propertyName == "animationTime" &&
                         prop["value"].is_number())
                {
                    animationTime = prop["value"].get<float>();
                }
            }
        }

        registry.emplace<TransformComponent>(entity, Vector2D<float>(x, y));
        registry.emplace<TextureComponent>(entity, textureID);
        registry.emplace<DimensionComponent>(entity, static_cast<float>(width),
                                             static_cast<float>(height));
        registry.emplace<SpriteComponent>(entity, spriteRow, spriteCol, 0);

        AnimationComponent animation;
        animation.currentFrame = 0;
        animation.totalFrames = numFrames;
        animation.animationTime = animationTime;
        registry.emplace<AnimationComponent>(entity, animation);
        registry.emplace<ObjectTypeComponent>(entity, type);

        m_layers.push_back(entity);
    }
}

std::expected<void, std::string>
TiledLoader::loadTileLayer(entt::registry& registry,
                           const nlohmann::json& layer, int tileLayerCount)
{
    if (!layer.contains("data") || !layer["data"].is_array())
    {
        return std::unexpected("a tile layer has no data array");
    }

    const std::size_t expectedCount = static_cast<std::size_t>(m_level.width) *
                                      static_cast<std::size_t>(m_level.height);
    if (layer["data"].size() != expectedCount)
    {
        return std::unexpected(
            "a tile layer data array has " +
            std::to_string(layer["data"].size()) + " tile ids; expected " +
            std::to_string(expectedCount) + " (width * height)");
    }

    std::vector<std::uint32_t> gids;
    gids.reserve(expectedCount);
    for (const auto& cell : layer["data"])
    {
        if (!cell.is_number())
        {
            return std::unexpected("a tile layer has a non-integer gid");
        }
        gids.push_back(cell.get<std::uint32_t>());
    }

    const std::string name = jsonString(layer, "name");

    auto layerEntity = create(registry);
    auto& tileLayer = registry.emplace<TileLayerComponent>(layerEntity);
    tileLayer.tileSetEntities = m_tilesets;

    if (contains(m_names.overlay, name))
    {
        registry.emplace<OverlayLayerComponent>(layerEntity);
    }
    else if (tileLayerCount == 1 || contains(m_names.bottom, name))
    {
        registry.emplace<BottomLayerComponent>(layerEntity);
    }

    bool reportedMissingTileset = false;
    for (int row = 0; row < m_level.height; ++row)
    {
        for (int col = 0; col < m_level.width; ++col)
        {
            const int tileId = static_cast<int>(
                gids[static_cast<std::size_t>(row) *
                         static_cast<std::size_t>(m_level.width) +
                     static_cast<std::size_t>(col)]);
            if (tileId == 0)
            {
                continue;
            }

            const entt::entity tileset = tilesetFor(registry, tileId);
            if (tileset == entt::null)
            {
                if (!reportedMissingTileset)
                {
                    std::cerr << "TiledLoader: layer '" << name
                              << "' references tile " << tileId
                              << ", which no tileset covers; skipping it and "
                                 "any others like it\n";
                    reportedMissingTileset = true;
                }
                continue;
            }

            auto tileEntity = create(registry);
            registry.emplace<TileComponent>(tileEntity, tileId, tileset);
            registry.emplace<TransformComponent>(
                tileEntity,
                Vector2D<float>(static_cast<float>(col * m_level.tileWidth),
                                static_cast<float>(row * m_level.tileHeight)));
            registry.emplace<DimensionComponent>(
                tileEntity, static_cast<float>(m_level.tileWidth),
                static_cast<float>(m_level.tileHeight));

            tileLayer.tileEntities.push_back(tileEntity);
        }
    }

    m_layers.push_back(layerEntity);
    return {};
}

} // namespace de

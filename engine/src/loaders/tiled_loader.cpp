#include <SDL.h>
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
#include <engine/components/velocity_component.hpp>
#include <engine/core/vector_2d.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/tiled_loader.hpp>
#include <fstream>
#include <iostream>
#include <string>

namespace de
{
namespace
{
bool isBottomName(const std::string& name)
{
    return name == "Bottom" || name == "Collision" ||
           name == "Capa de patrones 1";
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

std::expected<void, std::string>
TiledLoader::loadLevel(entt::registry& registry,
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
    m_tilesets.clear();
    m_layers.clear();

    m_tilewidth = jsonInt(root, "tilewidth");
    m_tileheight = jsonInt(root, "tileheight");
    m_width = jsonInt(root, "width");
    m_height = jsonInt(root, "height");
    if (m_tilewidth <= 0 || m_tileheight <= 0 || m_width <= 0 || m_height <= 0)
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

    int tileLayerCount = 0;
    if (root.contains("layers") && root["layers"].is_array())
    {
        for (const auto& layer : root["layers"])
        {
            const std::string type = jsonString(layer, "type");
            if (type == "tilelayer")
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

    auto levelEntity = registry.create();
    auto& levelComponent = registry.emplace<LevelComponent>(levelEntity);
    levelComponent.tilesets = m_tilesets;
    levelComponent.layers = m_layers;
    m_pEntities->push_back(levelEntity);

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

    auto tileSetEntity = registry.create();
    auto& set = registry.emplace<TileSetComponent>(tileSetEntity);
    auto& dimension = registry.emplace<DimensionComponent>(tileSetEntity);
    registry.emplace<TextureComponent>(tileSetEntity, name);

    const int imageWidth = jsonInt(tileset, "imagewidth");
    set.firstGridID = firstGid;
    dimension.width = jsonInt(tileset, "tilewidth");
    dimension.height = jsonInt(tileset, "tileheight");
    set.spacing = jsonInt(tileset, "spacing");
    set.margin = jsonInt(tileset, "margin");
    set.tileCount = jsonInt(tileset, "tilecount");
    set.numColumns = dimension.width + set.spacing > 0
                         ? imageWidth / (dimension.width + set.spacing)
                         : 0;

    registry.ctx().get<TextureCache>().load(
        name, (m_levelDir / image).lexically_normal().string());

    m_tilesets.push_back(tileSetEntity);
    m_pEntities->push_back(tileSetEntity);
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
        auto entity = registry.create();

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
        registry.emplace<DimensionComponent>(entity, width, height);
        registry.emplace<SpriteComponent>(entity, spriteRow, spriteCol, 0);
        registry.emplace<VelocityComponent>(entity, Vector2D<float>(0, 0));

        AnimationComponent animation;
        animation.currentFrame = 0;
        animation.totalFrames = numFrames;
        animation.animationTime = animationTime;
        registry.emplace<AnimationComponent>(entity, animation);
        registry.emplace<ObjectTypeComponent>(entity, type);

        m_pEntities->push_back(entity);
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

    const std::size_t expectedCount =
        static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height);
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

    auto layerEntity = registry.create();
    auto& tileLayer = registry.emplace<TileLayerComponent>(layerEntity);
    tileLayer.tileSetEntities = m_tilesets;

    const bool soleLayer = tileLayerCount == 1;
    if (name == "Bottom" || soleLayer || isBottomName(name))
    {
        registry.emplace<BottomLayerComponent>(layerEntity);
    }
    if (name == "Overlay")
    {
        registry.emplace<OverlayLayerComponent>(layerEntity);
    }

    bool reportedMissingTileset = false;
    for (int rows = 0; rows < m_height; rows++)
    {
        for (int cols = 0; cols < m_width; cols++)
        {
            const int tileId =
                static_cast<int>(gids[static_cast<std::size_t>(rows) *
                                          static_cast<std::size_t>(m_width) +
                                      static_cast<std::size_t>(cols)]);
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

            auto tileEntity = registry.create();
            const int tileX = cols * m_tilewidth;
            const int tileY = rows * m_tileheight;

            registry.emplace<TileComponent>(tileEntity, tileId, tileset);
            registry.emplace<TransformComponent>(
                tileEntity, Vector2D<float>(static_cast<float>(tileX),
                                            static_cast<float>(tileY)));
            registry.emplace<DimensionComponent>(tileEntity, m_tilewidth,
                                                 m_tileheight);

            tileLayer.tileEntities.push_back(tileEntity);
            m_pEntities->push_back(tileEntity);
        }
    }

    m_layers.push_back(layerEntity);
    m_pEntities->push_back(layerEntity);

    return {};
}

} // namespace de
